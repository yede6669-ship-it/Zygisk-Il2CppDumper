#include <jni.h>
#include <string>
#include <cstdio>
#include "zygisk.hpp"

// 信号文件的位置
#define SIGNAL_FILE "/data/local/tmp/dump_signal"

// 这就是我们的 Hook 函数
void hooked_onItemClick(JNIEnv* env, jobject thiz, jobject parent, jobject view, jint position, jlong id) {
    // 1. 获取 MainActivity 的类对象
    jclass mainActivityClass = env->GetObjectClass(thiz);
    
    // 2. 找到 packageNames 字段
    jfieldID fieldId = env->GetFieldID(mainActivityClass, "packageNames", "Ljava/util/List;");
    jobject listObject = env->GetObjectField(thiz, fieldId);
    
    // 3. 调用 List.get(position) 拿到包名
    jclass listClass = env->FindClass("java/util/List");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jstring pkgName = (jstring)env->CallObjectMethod(listObject, getMethod, position);
    
    const char* pkgStr = env->GetStringUTFChars(pkgName, nullptr);
    
    // 4. 写入信号文件，供游戏进程读取
    FILE* f = fopen(SIGNAL_FILE, "w");
    if (f) {
        fprintf(f, "%s", pkgStr);
        fclose(f);
        // 赋予权限，确保所有进程都能读写
        chmod(SIGNAL_FILE, 0777);
    }
    
    env->ReleaseStringUTFChars(pkgName, pkgStr);
}

class MyModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api* api, JNIEnv* env) override {
        this->api = api;
        this->env = env;
    }

    void postAppSpecialize(const zygisk::AppSpecializeArgs* args) override {
        const char* process = env->GetStringUTFChars(args->nice_name, nullptr);
        
        // 识别辅助 App：包名必须匹配
        if (process && std::string(process) == "com.unpacker.helper") {
            // 找到匿名内部类 OnItemClickListener（通常是 MainActivity$1）
            jclass listenerClass = env->FindClass("com/unpacker/helper/MainActivity$1");
            if (listenerClass) {
                JNINativeMethod methods[] = {
                    {"onItemClick", "(Landroid/widget/AdapterView;Landroid/view/View;IJ)V", (void*)hooked_onItemClick}
                };
                // 强行注册，接管原有的 Java 点击逻辑
                env->RegisterNatives(listenerClass, methods, 1);
            }
        }
        env->ReleaseStringUTFChars(args->nice_name, process);
    }
private:
    zygisk::Api* api;
    JNIEnv* env;
};

REGISTER_ZYGISK_MODULE(MyModule)
