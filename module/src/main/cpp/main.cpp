#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>
#include <string>
#include "il2cpp_dump.h"

// 监听线程逻辑
void listen_for_api_commands() {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) return;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    
    // 使用抽象命名空间（以 \0 开头），不需要担心文件权限
    const char* socket_name = "\0il2cpp_dump_socket";
    memcpy(addr.sun_path, socket_name, 19);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        return;
    }

    listen(server_fd, 1);

    while (true) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd > 0) {
            char buffer[512] = {0};
            // 读取辅助 App 发来的保存路径
            read(client_fd, buffer, sizeof(buffer));
            
            LOGI("API 唤醒：开始执行 Dump 至 %s", buffer);
            il2cpp_dump(buffer); // 调用核心脱壳逻辑
            
            close(client_fd);
        }
    }
}

class MyModule : public zygisk::ModuleBase {
public:
    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        // 在游戏进程中启动监听线程
        std::thread(listen_for_api_commands).detach();
    }
};
