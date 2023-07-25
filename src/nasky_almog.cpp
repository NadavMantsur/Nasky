#include <cassert>      // assert
#include <fcntl.h>      // open
#include <sys/ioctl.h>  // ioctl
#include <sys/socket.h> // socketpair
#include <vector>       // vector
#include <linux/nbd.h>  // nbd
#include <unistd.h>     // close
#include <signal.h>     // signals
#include <algorithm>    // copy_n
#include <iostream>     // cerr
#include <thread>       //std::thread
#include <arpa/inet.h>  //ntohl
#include <stdint.h>     //int64_t

#include "driver_communicator.hpp"
#include "nbd_driver.hpp"
#include "ram_storage.hpp"
#include "storage.hpp"
#include "utils.hpp"


using namespace hrd28;


static u_int64_t ntohll(u_int64_t a)
{
      u_int32_t lo = a & 0xffffffff;
      u_int32_t hi = a >> 32U;
      lo = ntohl(lo);
      hi = ntohl(hi);
      return ((u_int64_t) lo) << 32U | hi;
}

//////////////////////////////////RAMStorage////////////////////////////////////
RAMStorage::RAMStorage(size_t capacity_) :m_storage(capacity_,0){}
RAMStorage::~RAMStorage() noexcept
{}
void RAMStorage::Read(std::shared_ptr<DriverData> data_) const
{
    assert(data_);
    
    std::copy_n(m_storage.begin() + data_->m_offset,
     data_->m_length ,data_->m_buffer.begin());
}

void RAMStorage::Write(std::shared_ptr<DriverData> data_)
{
    assert(data_);
    
    std::copy_n(data_->m_buffer.begin(), data_->m_length, 
        m_storage.begin() + data_->m_offset);
}
//////////////////////////////////RAMStorage////////////////////////////////////

//////////////////////////////////Internal//////////////////////////////////////

internal::FDGuard::FDGuard(const std::string& dev_path_)
{
    m_fd = open(dev_path_.c_str(), O_RDWR);
    if(-1 == m_fd)
    {
        DriverException open_failed(std::error_code(errno, std::system_category()), "open failed");
        
        throw(open_failed);
    }
}

internal::FDGuard::~FDGuard()
{
    try
    {
        CloseFD(); 
    }
    catch(DriverException &e)
    {
        //add code
    }
}

int internal::FDGuard::GetFD() const
{
    return m_fd;
}

void internal::FDGuard::CloseFD()
{
    int status = close(m_fd);
    if(-1 == status)
    {
        DriverException close_failed(std::error_code(errno, std::system_category()), "close failed");
        throw(close_failed);
    }
}

//////////////////////////////////Internal//////////////////////////////////////

//////////////////////////////NBDDriverCom//////////////////////////////////////

NBDDriverCom::NBDDriverCom(const std::string& dev_path_, size_t storage_capacity_)
:  m_sockets{ -1, -1 }, m_device_fd(dev_path_),m_thread_id(), m_is_connected(false), m_mutex()
{
    if (-1 == ioctl(m_device_fd.GetFD(), NBD_SET_SIZE, storage_capacity_))
    {
        DriverException ioctl_failed(std::error_code(0, std::system_category()), "ioctl failed");
        throw(ioctl_failed);
    }
    
    if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, m_sockets))
    {
        DriverException socketpair_failed(std::error_code(0, std::system_category()), "Socketpair failed");
        throw(socketpair_failed);
    }

    m_is_connected = true;
    
    if (ioctl(m_device_fd.GetFD(), NBD_CLEAR_SOCK) == -1)
    {
        DriverException ioctl_failed(std::error_code(0, std::system_category()), "ioctl failed");
        throw(ioctl_failed);
    }

    m_thread_id = std::thread(&NBDDriverCom::ThreadRoutine, this);
}

int NBDDriverCom::GetFD() const noexcept
{
    return m_sockets[APP];
}

void NBDDriverCom::Disconnect()
{
    close(m_sockets[APP]);
    close(m_sockets[DEVICE]);
    m_is_connected = false;
}

NBDDriverCom::~NBDDriverCom() noexcept
{
    m_thread_id.join();
    Disconnect();
}

static int ReadAll(int fd, char* buf, size_t count)
{
    int bytes_read = 0;
  
  while (count > 0) 
  {
        bytes_read = read(fd, buf, count);
        
        if(bytes_read == -1)
        {
            DriverException read_failed(std::error_code(0, std::system_category()), "read failed");
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
        DriverException write_failed(std::error_code(0, std::system_category()), "write failed");
        throw(write_failed);
    }
    
    buf += bytes_written;
    count -= bytes_written;
  }

  return 0;
}

std::shared_ptr<DriverData> NBDDriverCom::RecvRequest() const 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    struct nbd_request request; 
    
    ReadAll(m_sockets[APP], reinterpret_cast<char *>(&request), sizeof(struct nbd_request));
    
    
    int64_t MsgId = *(reinterpret_cast<int64_t *>(request.handle)); 
    
    std::vector<uint8_t> temp(request.len);
    
    std::shared_ptr<DriverData> ret(new DriverData(ntohl(request.type), MsgId,
     ntohll(request.from), ntohl(request.len), temp));
    
    if(ActionType::WRITE == ret->m_type)
    {
        ReadAll(m_sockets[APP], reinterpret_cast<char *>(ret->m_buffer.data()),
         ret->m_length);
    }
    
    std::cerr << "[RecvRequest] Done" << std::endl;
    return ret;
}


void NBDDriverCom::SendReply(std::shared_ptr<DriverData> reply_) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    struct nbd_reply reply ={0,0,0};
    
    reply.error = 0;
    reply.magic = htonl(NBD_REPLY_MAGIC);
    
    *(reinterpret_cast<int64_t *>(reply.handle)) = reply_->m_msg_id;
    
    WriteAll(m_sockets[APP], reinterpret_cast<char *>(&reply), sizeof(struct nbd_reply));
        
    if(ActionType::READ == reply_->m_type)
    {
        // if request->read -> write from m_buffer from reply_ 2 m_sockets[APP] (GetType)
        WriteAll(m_sockets[APP], reinterpret_cast<char *>(reply_->m_buffer.data()),
         reply_->m_length);
    }

    std::cerr << "[SendReply] Done" << std::endl;
}

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

//////////////////////////////NBDDriverCom//////////////////////////////////////


////////////////////////////////DriverData//////////////////////////////////////

static ActionType GetActionType(__be32 type_)
{
    ActionType ret;
    
    if(type_ == NBD_CMD_READ)
    {
        ret = ActionType::READ;
    }
    else if(type_ == NBD_CMD_WRITE)
    {
        ret = ActionType::WRITE;
    }
    else if(type_ == NBD_CMD_DISC)
    {
        ret = ActionType::DISC;
    }
    else if(type_ == NBD_CMD_FLUSH)
    {
        ret = ActionType::FLUSH;
    }
    else if(type_ == NBD_CMD_TRIM)
    {
        ret = ActionType::TRIM;
    }
    
    return ret;
}


DriverData::DriverData(int32_t type_, int64_t msg_id_, uint64_t offset_,
                         uint32_t length_, const std::vector<uint8_t>& buffer_):
 m_msg_id(msg_id_), m_offset(offset_), m_length(length_),
 m_buffer(buffer_)
 {
    m_type = GetActionType(type_);

 }
 ////////////////////////////////DriverData/////////////////////////////////////


////////////////////////////////DriverException/////////////////////////////////

 
 DriverException::DriverException(std::error_code error_, const std::string& what_)
        : std::system_error(error_, what_){}
 
 ////////////////////////////////DriverException/////////////////////////////////

