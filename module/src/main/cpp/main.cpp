#include <jni.h>
#include <android/log.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>
#include <string>
#include <cstring>

#include "zygisk.hpp"
#include "il2cpp_dump.h"

// 重新定义宏，确保不依赖外部头文件
#define LOG_TAG "IL2CPP_DUMPER"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void listen_for_api_commands() {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) return;

    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    
    // 必须与 Java 端的 Socket 名严格一致
    const char* socket_name = "\0il2cpp_dump_socket";
    std::memcpy(addr.sun_path, socket_name, 19);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        return;
    }

    listen(server_fd, 5);
    LOGI("Socket API 监听中...");

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd > 0) {
            char buffer[512] = {0};
            if (read(client_fd, buffer, sizeof(buffer) - 1) > 0) {
                LOGI("收到外部指令，保存路径: %s", buffer);
                il2cpp_dump(buffer); // 执行 Dump
            }
            close(client_fd);
        }
    }
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        // 核心：在游戏进程中分离线程启动 API
        std::thread(listen_for_api_commands).detach();
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
