#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>

int main() {
    ACE_INET_Addr server_addr(8080, "127.0.0.1");
    ACE_SOCK_Connector connector;
    ACE_SOCK_Stream client_stream;

    // 尝试连接到服务器
    if (connector.connect(client_stream, server_addr) == -1) {
        ACE_DEBUG((LM_ERROR, "Connection failed\n"));
        return -1;
    }

    ACE_DEBUG((LM_INFO, "Connected to server\n"));

    // 向服务器发送消息
    const char* message = "hello reactor";
    client_stream.send_n(message, strlen(message));
    ACE_DEBUG((LM_INFO, "Sent: %s\n", message));

    // 接收服务器的回应
    char buffer[1024];
    ssize_t bytes_received = client_stream.recv(buffer, sizeof(buffer));
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';  // 确保接收到的字符串以 null 结尾
        ACE_DEBUG((LM_INFO, "Received: %s\n", buffer));
    }

    // 关闭连接
    client_stream.close();

    return 0;
}
