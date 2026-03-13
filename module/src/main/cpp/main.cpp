#include <jni.h>
#include <android/log.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>
#include <string>
#include <cstring> // 必须包含 cstring 以使用 memset

// 显式导入 zygisk 头文件
#include "zygisk.hpp"
#include "il2cpp_dump.h"

// 重新定义缺失的 LOGI 宏
#define LOG_TAG "IL2CPP_DUMPER"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// 监听线程逻辑
void listen_for_api_commands() {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        LOGE("无法创建 Socket");
        return;
    }

    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    
    // 使用抽象命名空间（以 \0 开头）
    const char* socket_name = "\0il2cpp_dump_socket";
    std::memcpy(addr.sun_path, socket_name, 19);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOGE("Socket 绑定失败");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 1) < 0) {
        LOGE("Socket 监听失败");
        close(server_fd);
        return;
    }

    LOGI("API 监听已启动...");

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd > 0) {
            char buffer[512] = {0};
            ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                LOGI("API 唤醒：开始执行 Dump 至 %s", buffer);
                // 调用核心脱壳逻辑
                il2cpp_dump(buffer); 
            }
            close(client_fd);
        }
    }
}

// 修正 zygisk 类定义
class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        // 在游戏进程中启动监听线程
        std::thread(listen_for_api_commands).detach();
    }

private:
    zygisk::Api* api;
    JNIEnv* env;
};

// 注册模块
REGISTER_ZYGISK_MODULE(MyModule)
