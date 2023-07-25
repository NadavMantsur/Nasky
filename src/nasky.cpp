#include <cassert>      // assert
#include <fcntl.h>      // open
#include <sys/ioctl.h>  // ioctl
#include <sys/socket.h> // socketpair
#include <vector>       // vector
#include <linux/nbd.h>  // nbd, nbd_request, nbd_reply
#include <unistd.h>     // close
#include <signal.h>     //sigfillset
#include <iostream>     //cerr
#include <arpa/inet.h>  //htonl


#include "driver_communicator.hpp"
#include "nbd_driver.hpp"
#include "ram_storage.hpp"
#include "utils.hpp"
#include "driver_data.hpp"
#include "logger.hpp"

using namespace hrd28;
using namespace std;


//these functions taken from buse.c
static int ReadAll(int fd, char* buf, size_t count);
static int WriteAll(int fd, char* buf, size_t count);
static u_int64_t ntohll(u_int64_t a);

//////////////////////////////////driver_data///////////////////////////////////
///////////////////////////////////DriverData///////////////////////////////////
DriverData::DriverData(int32_t type_, int64_t msg_id_, uint64_t offset_,
                         uint32_t length_, const std::vector<uint8_t>& buffer_):
                         m_msg_id(msg_id_), m_offset(offset_),
                         m_length(length_), m_buffer(buffer_)
{
    switch(type_)
    {
        case NBD_CMD_READ:
            m_type = ActionType::READ;
            break;
        case NBD_CMD_WRITE:
            m_type = ActionType::WRITE;
            break;
        case NBD_CMD_DISC:
            m_type = ActionType::DISC;
            break;
        case NBD_CMD_FLUSH:
            m_type = ActionType::FLUSH;
            break;
        case NBD_CMD_TRIM:
            m_type = ActionType::TRIM;
            break;
        default:
            break;
    }
}

////////////////////////////////DriverException/////////////////////////////////
DriverException::DriverException(std::error_code err_, const std::string& what_) :
                                                std::system_error(err_, what_)
{}

//////////////////////////////////nbd_driver////////////////////////////////////
////////////////////////////////////FDGuard/////////////////////////////////////
internal::FDGuard::FDGuard(const std::string& dev_path_) //throws DriverException
{
    m_fd = open(dev_path_.c_str(), O_RDWR);
    
    if(-1 == m_fd)
    {
        DriverException opeFdExcep(std::error_code(errno, std::system_category()), 
                                                            "openFD failed");
        throw(opeFdExcep);
    }
}

internal::FDGuard::~FDGuard()noexcept
{
    try
    {
        CloseFD();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

int internal::FDGuard::GetFD() const
{
    return m_fd;
}

void internal::FDGuard::CloseFD() //throws DriverException
{
    if (-1 == close(m_fd))
    {
        DriverException closeFdExcep(std::error_code(0, std::system_category()), 
                                                            "CloseFD failed");
        throw(closeFdExcep);
    }
}

//////////////////////////////////NBDDriverCom//////////////////////////////////
NBDDriverCom::NBDDriverCom(const std::string& dev_path_, size_t storage_capacity_)
:  m_sockets{ -1, -1 }, m_device_fd(dev_path_),m_thread_id(), 
m_is_connected(false), m_mutex() //throws DriverException 
{
    if (-1 == ioctl(m_device_fd.GetFD(), NBD_SET_SIZE, storage_capacity_))
    {
        ELOG("NBDDriverCom() ioctl failed");
        DriverException ioctl_failed(std::error_code(0, std::system_category()), "ioctl failed");
        throw(ioctl_failed);
    }

    if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, m_sockets))
    {
        ELOG("NBDDriverCom() socketpair failed");
        DriverException socketpair_failed(std::error_code(0, std::system_category()), "Socketpair failed");
        throw(socketpair_failed);
    }

    m_is_connected = true;
    
    if (-1 == ioctl(m_device_fd.GetFD(), NBD_CLEAR_SOCK))
    {
        ELOG("ioctl failed");
    }
    
    m_thread_id = std::thread(&NBDDriverCom::ThreadRoutine, this);
}

NBDDriverCom::~NBDDriverCom() noexcept
{
    m_thread_id.join();

    try
    {
        Disconnect();   
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}

std::shared_ptr<DriverData> NBDDriverCom::RecvRequest() const //throws DriverException
{
    std::lock_guard<std::mutex> lock(m_mutex);

    struct nbd_request request; 

    // request.magic = htonl(NBD_REQUEST_MAGIC);
    std::cout << "RecvRequest1" << endl;
    ReadAll(m_sockets[APP], reinterpret_cast<char *>(&request), 
                                                    sizeof(struct nbd_request));
    std::cout << "RecvRequest2" << endl;
    int64_t MsgId = *(reinterpret_cast<int64_t *>(request.handle));
    
    std::vector<uint8_t> temp(request.len);

    std::shared_ptr<DriverData> ret(new DriverData(ntohl(request.type), MsgId,
                                ntohll(request.from), ntohl(request.len), temp));

    //in case of WRITE
    if (ActionType::WRITE == ret->m_type)
    {
        ReadAll(m_sockets[APP], reinterpret_cast<char *>(ret->m_buffer.data()), 
                                                                ret->m_length);
    }
    
    return ret;
}


void NBDDriverCom::SendReply(shared_ptr<DriverData> reply_) const   //throws DriverException
{
    lock_guard<::mutex> lock(m_mutex);
    
    struct nbd_reply reply ={0,0,0};
    
    reply.error = 0;
    reply.magic = htonl(NBD_REPLY_MAGIC);
    
    *(reinterpret_cast<int64_t *>(reply.handle)) = reply_->m_msg_id;
    
    WriteAll(m_sockets[APP], reinterpret_cast<char *>(&reply), sizeof(struct nbd_reply));
        
    if(ActionType::READ == reply_->m_type)
    {
        WriteAll(m_sockets[APP], reinterpret_cast<char *>(reply_->m_buffer.data()),
         reply_->m_length);
    }

    cerr << "[SendReply] Done" << endl;
}

int NBDDriverCom::GetFD() const noexcept
{
    return m_sockets[APP];
}

void NBDDriverCom::Disconnect() //throws DriverException
{
    if ( -1 == close(m_sockets[APP]))
    {
        ELOG("Disconnect() close APP socket failed");
        DriverException closeSocketExcep(std::error_code(0, std::system_category()), 
                                                    "close(m_sockets[APP]) failed");
        throw(closeSocketExcep);
    }

    if (-1 == close(m_sockets[DEVICE]))
    {
        ELOG("Disconnect() close DEVICE socket failed");
        DriverException closeSocketExcep(std::error_code(0, std::system_category()), 
                                                "close(m_sockets[DEVICE]) failed");
        throw(closeSocketExcep);
    }

    m_is_connected = false;
}

//private
void NBDDriverCom::ThreadRoutine()
{
    sigset_t sigset;
    
    int nbd = m_device_fd.GetFD();
    int socket_fd_ = m_sockets[DEVICE];
    try
    {
        if (0 != sigfillset(&sigset)) 
        {
            DriverException block_failed(std::error_code(0, std::system_category()), "block failed");
            throw(block_failed);
        }
        
        if (0 != pthread_sigmask(SIG_SETMASK, &sigset, NULL))
        {
            DriverException block_failed(std::error_code(0, std::system_category()), "block failed");
            throw(block_failed);
        }
        
        if (-1 == ioctl(nbd, NBD_SET_SOCK, socket_fd_))
        {
            DriverException block_failed(std::error_code(0, std::system_category()), "block failed");
            throw(block_failed);
        }
        
        int flags = (0 | NBD_FLAG_SEND_TRIM | NBD_FLAG_SEND_FLUSH);
        
        if ((flags != 0) && (0 != ioctl(nbd, NBD_SET_FLAGS, flags)))
        {
            DriverException ioctl_failed(std::error_code(0, std::system_category()), "ioctl failed");
            throw(ioctl_failed);
        }
        
        if(-1 == ioctl(nbd, NBD_DO_IT))
        {
             DriverException ioctl_failed(std::error_code(0, std::system_category()), "ioctl failed");
             throw(ioctl_failed);
        }
    }
    catch(DriverException &e_)
    {
        if (ioctl(nbd, NBD_CLEAR_QUE) == -1 || ioctl(nbd, NBD_CLEAR_SOCK) == -1)
        {
             DriverException ioctl_failed(std::error_code(errno, std::system_category()), "ioctl failed");
             throw(ioctl_failed);
        }
        
        exit(-1);
    }
}

/////////////////////////////////ram_storage////////////////////////////////////
//////////////////////////////////RAMStorage////////////////////////////////////
RAMStorage::RAMStorage(size_t capacity_): m_storage(capacity_, 0)
{}

RAMStorage::~RAMStorage() noexcept
{}

void RAMStorage::Read(std::shared_ptr<DriverData> data_) const
{
    assert(NULL != data_);

    std::copy_n(m_storage.begin() + data_->m_offset, data_->m_length, 
                                                    data_->m_buffer.begin());
}

void RAMStorage::Write(std::shared_ptr<DriverData> data_)
{
    assert(NULL != data_);

    std::copy_n(data_->m_buffer.begin(), data_->m_length, 
                                        m_storage.begin() + data_->m_offset);
}

///////////////////////////////static functions/////////////////////////////////
static int ReadAll(int fd, char* buf, size_t count)
{
    int bytes_read = 0;
  
  while (count > 0) 
  {
        bytes_read = read(fd, buf, count);
        
        if(bytes_read == -1)
        {
            DriverException read_failed(error_code(0, system_category()), "read failed");
            throw(read_failed);
        }
        
        buf += bytes_read;
        
        count -= bytes_read;
  }
  
  return 0;
}

static int WriteAll(int fd, char* buf, size_t count)
{
  int bytes_written = 0;

  while (count > 0) 
  {
    bytes_written = write(fd, buf, count);
    
    if(bytes_written == -1)
    {
        cout << errno << ::endl;
        DriverException write_failed(error_code(0, system_category()), "write failed");
        throw(write_failed);
    }
    
    buf += bytes_written;
    count -= bytes_written;
  }

  return 0;
}

static u_int64_t ntohll(u_int64_t a)
{
      u_int32_t lo = a & 0xffffffff;
      u_int32_t hi = a >> 32U;
      lo = ntohl(lo);
      hi = ntohl(hi);
      return ((u_int64_t) lo) << 32U | hi;
}