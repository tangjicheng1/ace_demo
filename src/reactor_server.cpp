#include <ace/Event_Handler.h>
#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>
#include <ace/Reactor.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>

class MyEventHandler : public ACE_Event_Handler {
   public:
    MyEventHandler(ACE_Reactor* reactor, ACE_SOCK_Stream& client)
        : reactor_(reactor), client_stream_(client) {
    }

    virtual ~MyEventHandler() {
        client_stream_.close();
    }

    virtual int handle_input([[maybe_unused]] ACE_HANDLE h = ACE_INVALID_HANDLE) override {
        char buffer[1024];
        ssize_t bytes_received = client_stream_.recv(buffer, sizeof(buffer));
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';  // Ensure null-terminated string
            ACE_DEBUG((LM_INFO, "Server received: %s\n", buffer));

            // 服务器回复消息
            const char* reply = "greeting from reactor server";
            client_stream_.send(reply, strlen(reply));
        } else {
            ACE_DEBUG((LM_INFO, "Client disconnected\n"));
            return -1;  // Remove handler from reactor
        }
        return 0;  // Continue handling
    }

    virtual ACE_HANDLE get_handle() const override {
        return client_stream_.get_handle();
    }

   private:
    ACE_Reactor* reactor_;
    ACE_SOCK_Stream client_stream_;
};

class MyAcceptor : public ACE_Event_Handler {
   public:
    MyAcceptor(ACE_Reactor* reactor, const ACE_INET_Addr& listen_addr) : reactor_(reactor) {
        acceptor_.open(listen_addr);
        reactor_->register_handler(this, ACE_Event_Handler::ACCEPT_MASK);
    }

    virtual ~MyAcceptor() {
        acceptor_.close();
    }

    virtual int handle_input([[maybe_unused]] ACE_HANDLE h = ACE_INVALID_HANDLE) override {
        ACE_SOCK_Stream client_stream;
        if (acceptor_.accept(client_stream) == -1) {
            ACE_DEBUG((LM_ERROR, "Accept failed\n"));
            return 0;
        }

        ACE_DEBUG((LM_INFO, "Accepted new connection\n"));

        // 注册新的事件处理器以处理客户端消息
        MyEventHandler* handler = new MyEventHandler(reactor_, client_stream);
        reactor_->register_handler(handler, ACE_Event_Handler::READ_MASK);
        return 0;
    }

    virtual ACE_HANDLE get_handle() const override {
        return acceptor_.get_handle();
    }

   private:
    ACE_Reactor* reactor_;
    ACE_SOCK_Acceptor acceptor_;
};

int main() {
    ACE_Reactor reactor;
    ACE_INET_Addr listen_addr(8080);
    MyAcceptor acceptor(&reactor, listen_addr);

    ACE_DEBUG((LM_INFO, "Server started on port 8080\n"));

    reactor.run_reactor_event_loop();  // 启动 Reactor 事件循环

    return 0;
}
