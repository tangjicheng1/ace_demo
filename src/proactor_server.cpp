#include <ace/Asynch_Acceptor.h>
#include <ace/Asynch_IO.h>
#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>
#include <ace/Proactor.h>
#include <ace/Signal.h>
#include <ace/config-macros.h>
#include <cstring>

// 客户端连接的处理器
class MyServiceHandler : public ACE_Service_Handler {
   public:
    MyServiceHandler()
        : ACE_Service_Handler(), handle_(ACE_INVALID_HANDLE), read_mb_(nullptr),
          write_mb_(nullptr) {
    }

    virtual void open(ACE_HANDLE handle, ACE_Message_Block&) {
        // 保存客户端的连接句柄
        this->handle_ = handle;

        // 初始化异步读取流
        if (this->reader_.open(*this, this->handle_) == -1) {
            ACE_ERROR((LM_ERROR, "Failed to open read stream\n"));
            return;
        }

        // 初始化异步写入流
        if (this->writer_.open(*this, this->handle_) == -1) {
            ACE_ERROR((LM_ERROR, "Failed to open write stream\n"));
            return;
        }

        // 为读取操作分配缓冲区
        ACE_NEW_NORETURN(this->read_mb_, ACE_Message_Block(1024));

        // 启动异步读取操作
        ACE_DEBUG((LM_INFO, "Connection established, starting to read\n"));
        this->initiate_read();
    }

    // 开始读取数据
    void initiate_read() {
        // 准备异步读取
        if (this->reader_.read(*this->read_mb_, 1024) == -1) {
            ACE_ERROR((LM_ERROR, "Failed to initiate read\n"));
        }
    }

    // 处理读取事件
    virtual void handle_read_stream(const ACE_Asynch_Read_Stream::Result& result) {
        if (!result.success() || result.bytes_transferred() == 0) {
            ACE_ERROR((LM_ERROR, "Read failed or connection closed\n"));
            this->close();
            return;
        }

        // 输出接收到的数据
        ACE_DEBUG((LM_INFO, "Data received: %.*s\n", result.bytes_transferred(),
                   this->read_mb_->rd_ptr()));

        // 发送响应给客户端
        this->initiate_write();
    }

    // 发送回复给客户端
    void initiate_write() {
        const char* response = "greeting from proactor server";
        size_t response_len = std::strlen(response);

        // 为写入操作分配缓冲区
        ACE_NEW_NORETURN(this->write_mb_, ACE_Message_Block(response_len));

        // 将数据复制到缓冲区
        this->write_mb_->copy(response, response_len);

        // 启动异步写入操作
        if (this->writer_.write(*this->write_mb_, response_len) == -1) {
            ACE_ERROR((LM_ERROR, "Failed to initiate write\n"));
        }
    }

    // 处理写入事件
    virtual void handle_write_stream(const ACE_Asynch_Write_Stream::Result& result) {
        if (!result.success() || result.bytes_transferred() == 0) {
            ACE_ERROR((LM_ERROR, "Write failed\n"));
        } else {
            ACE_DEBUG((LM_INFO, "Response sent to client\n"));
        }

        // 关闭连接
        this->close();
    }

    // 关闭连接并释放资源
    void close() {
        if (this->handle_ != ACE_INVALID_HANDLE) {
            ACE_OS::close(this->handle_);
            this->handle_ = ACE_INVALID_HANDLE;
        }

        if (this->read_mb_) {
            this->read_mb_->release();
            this->read_mb_ = nullptr;
        }

        if (this->write_mb_) {
            this->write_mb_->release();
            this->write_mb_ = nullptr;
        }
    }

   private:
    ACE_HANDLE handle_;
    ACE_Asynch_Read_Stream reader_;
    ACE_Asynch_Write_Stream writer_;
    ACE_Message_Block* read_mb_;
    ACE_Message_Block* write_mb_;
};

// 接受客户端连接的接收器
class MyAcceptor : public ACE_Asynch_Acceptor<MyServiceHandler> {
   public:
    MyAcceptor() : ACE_Asynch_Acceptor<MyServiceHandler>() {
    }

    virtual int validate_connection([[maybe_unused]] const ACE_Asynch_Accept::Result& result,
                                    const ACE_INET_Addr& remote, const ACE_INET_Addr&) {
        ACE_DEBUG(
            (LM_INFO, "Connection from %s:%d\n", remote.get_host_name(), remote.get_port_number()));
        return 0;
    }
};

int main(int argc, char* argv[]) {
    ACE_UNUSED_ARG(argc);
    ACE_UNUSED_ARG(argv);

    ACE_INET_Addr listen_addr(8080);
    MyAcceptor acceptor;

    // 设置 Proactor 事件循环
    if (acceptor.open(listen_addr) == -1) {
        ACE_ERROR_RETURN((LM_ERROR, "Failed to open acceptor\n"), 1);
    }

    ACE_DEBUG((LM_INFO, "Proactor server started on port 8080\n"));

    // 启动事件循环
    ACE_Proactor::instance()->proactor_run_event_loop();

    return 0;
}
