#include <iostream>
#include <string>

#include "dispatcher_callback_nazmeya.hpp"

using namespace hrd28;


//subscriber class
class Subscriber 
{
public:
    void Notify(const std::string& msg_)
    {
        std::cout << "Received message: " << msg_ << std::endl;
    }

    void ServiceEnded()
    {
        std::cout << "Service ended" << std::endl;
    }
};

int main()
{
    Dispatcher<std::string> dispatcher;
    Subscriber subscriber;
    Callback<std::string, Subscriber> callback(subscriber, &dispatcher, 
                                &Subscriber::Notify, &Subscriber::ServiceEnded);

    dispatcher.NotifyAll("Hello, world!");

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
//Almog's tests

// #include <memory>
// #include <iostream>
// #include <string>
// #include "dispatcher_callback.hpp"

// using namespace hrd28;


// class Tapuz
// {
// public:
//     explicit Tapuz() =default;
//     ~Tapuz() =default;
    
//     void Hello(const std::string &msg_ )
//     {
//         std::cout << "Tapuz ::";

//         std::cout << msg_<< std::endl;
//     }
    
//     void OutOf()
//     {
//         std::cout << "Tapuz Out Of SERVICE" << std::endl;
//     }
// };

// class Bnana
// {
// public:
//     explicit Bnana() =default;
//     ~Bnana() =default;
    
//     void Hello(const std::string &msg_ )
//     {
//         std::cout << "Bnana ::";

//         std::cout << msg_<< std::endl;
//     }
    
//     void OutOf()
//     {
//         std::cout << " Bnana Out Of SERVICE" << std::endl;
//     }
// };


// class Mandarina
// {
// public:
//     explicit Mandarina() =default;
//     ~Mandarina() =default;
    
//     void Hello(const std::string &msg_ )
//     {
//         std::cout << "Mandarina ::";

//         std::cout << msg_<< std::endl;
//     }
    
//     void OutOf()
//     {
//         std::cout << " Mandarina Out Of SERVICE" << std::endl;
//     }
// };


// int main()
// {
//     using namespace hrd28;
    
//     Dispatcher<std::string> dispatcher_;
    
//     Tapuz tapuz;
//     Bnana bnana;
    
    
//     Mandarina mandrina;
    
//     Callback<std::string, Tapuz> sub(tapuz, &dispatcher_, &Tapuz::Hello, &Tapuz::OutOf);
    
//     dispatcher_.NotifyAll("done!");
    
//     Callback<std::string, Bnana> sub2(bnana, &dispatcher_, &Bnana::Hello, &Bnana::OutOf);
//     dispatcher_.NotifyAll("forever!");
    
//     Callback<std::string, Mandarina> sub3(mandrina, &dispatcher_, &Mandarina::Hello, &Mandarina::OutOf);
    
//     dispatcher_.NotifyAll("All together ");

//     return 0;
// }


////////////////////////////////////////////////////////////////////////////////
//class tests

// #include <iostream>
// #include <string>
// #include <memory>

// #include "dispatcher_callback.hpp"

// using namespace std;
// using namespace hrd28;

// template <typename MSG = std::string>
// class DataModel
// {
// public:
//     explicit DataModel(Dispatcher<MSG>& dispatcher_): m_dis(dispatcher_) {}
//     ~DataModel()=default;

//     void Change(const std::string& s_)
//     {
//         m_dis.NotifyAll(s_);
//     }

// private:
//     Dispatcher<MSG>& m_dis;
// };

// template <typename MSG = std::string>
// class ControlWindow //for the usr
// {
// public:
//     explicit ControlWindow(DataModel<MSG> *dm_): m_data_model(dm_) {}
//     ~ControlWindow()=default;

//     ControlWindow(const ControlWindow& o_)=delete;
//     ControlWindow &operator= (const ControlWindow& o_)=delete;

//     void Control(const std::string& s_)
//     {
//         m_data_model->Change(s_);
//     }

// private:
//     DataModel<MSG> *m_data_model;
// };

// class ViewWindow //our sub 
// {
// public:
//     explicit ViewWindow()=default;
//     ~ViewWindow()=default;

//     ViewWindow(const ViewWindow& o_)=delete;
//     ViewWindow &operator= (const ViewWindow& o_)=delete;

//     void Display(const std::string& msg_)
//     {
//         std::cout << msg_ << std::endl;
//     }
// };

// int main()
// {
//     hrd28::Dispatcher<std::string> dispatcher;

//     ViewWindow viewWindow;
//     ControlWindow controlWindow(dispatcher);

//     return 0;
// }








////////////////////////////////////////////////////////////////////////////////
// #include <iostream>
// #include <vector>
// #include "dispatcher_callback.hpp"

// // Example message type
// struct Message {
//     std::string content;
// };

// // Observer interface
// class Observer {
// public:
//     virtual void Update(const Message& msg) = 0;
//     virtual ~Observer() {}
// };

// // ViewWindow class
// class ViewWindow : public Observer {
// public:
//     explicit ViewWindow(hrd28::Dispatcher<Message>& dispatcher) {
//         hrd28::Callback<Message, ViewWindow> callback(*this, &dispatcher, 
//                                                             &ViewWindow::Update);
//     }

//     void Update(const Message& msg) override {
//         std::cout << "ViewWindow received message: " << msg.content << std::endl;
//     }
// };

// // DataModel class
// class DataModel {
// public:
//     DataModel(hrd28::Dispatcher<Message>& dispatcher) : dispatcher_(dispatcher) {}

//     void BroadcastMessage(const Message& msg) {
//         std::cout << "DataModel broadcasting message: " << msg.content << std::endl;
//         dispatcher_.NotifyAll(msg);
//     }

// private:
//     hrd28::Dispatcher<Message>& dispatcher_;
// };

// // ControlWindow class
// class ControlWindow : public Observer {
// public:
//     explicit ControlWindow(hrd28::Dispatcher<Message>& dispatcher) {
//         hrd28::Callback<Message, ControlWindow> callback(*this, &dispatcher, &ControlWindow::Update);
//     }

//     void Update(const Message& msg) override {
//         std::cout << "ControlWindow received message: " << msg.content << std::endl;
//     }
// };

// int main() {
//     hrd28::Dispatcher<Message> dispatcher;

//     // Create observers
//     ViewWindow viewWindow(dispatcher);
//     ControlWindow controlWindow(dispatcher);

//     // Create DataModel
//     DataModel dataModel(dispatcher);

//     // Test scenarios
//     Message msg1, msg2, msg3;
//     msg1.content = "Message 1";
//     msg2.content = "Message 2";
//     msg3.content = "Message 3";

//     // Broadcast to 1 observer
//     dataModel.BroadcastMessage(msg1);

//     // Broadcast to 2 observers
//     dataModel.BroadcastMessage(msg2);

//     // Broadcast to 3 observers
//     dataModel.BroadcastMessage(msg3);

//     // Remove 1 observer during broadcast
//     std::cout << "Removing ViewWindow observer" << std::endl;
//     // Note: In a real application, you would have a mechanism to remove observers properly.
//     // For simplicity, we directly call the Update method here.
//     viewWindow.Update(msg3);

//     // Remove all observers during broadcast
//     std::cout << "Removing all observers" << std::endl;
//     // Note: Same as above, this is a simplified approach for demonstration purposes.
//     controlWindow.Update(msg3);

//     // Add observer during broadcast
//     std::cout << "Adding new ViewWindow observer" << std::endl;
//     ViewWindow newViewWindow(dispatcher);
//     dataModel.BroadcastMessage(msg3);

//     // Add 2 observers during broadcast
//     std::cout << "Adding 2 new observers" << std::endl;
//     ControlWindow controlWindow2(dispatcher);
//     ControlWindow controlWindow3(dispatcher);
//     dataModel.BroadcastMessage(msg3);

//     return 0;
// }
