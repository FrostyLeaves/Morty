
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include<sys/ioctl.h>

#include <jni.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define ANDROID_APP_LOG(...) ((void)__android_log_print(ANDROID_LOG_INFO, "android_app", __VA_ARGS__))
#define ANDROID_APP_ERROR(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "android_app", __VA_ARGS__))

#define ANDROID_APP_DEBUG//debug 模式

#ifdef ANDROID_APP_DEBUG
#define ANDROID_APP_LOGV(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, "android_app", __VA_ARGS__))
#else
#define ANDROID_APP_LOGV(...) ((void)0)
#endif

void android_main(android_app* app)
{
    ANDROID_APP_LOGV("Morty main : 程序启动");
    //设置消息回调函数，就一个，整体和win32编程差不多了。

    //循环等待事情以进行处理。

    ANDROID_APP_LOGV("Morty main : 程序结束");
}