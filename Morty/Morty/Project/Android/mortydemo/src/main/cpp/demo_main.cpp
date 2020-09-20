
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

#include "C:\Users\65406\AppData\Local\Android\Sdk\ndk\21.3.6528147\sources\android\native_app_glue/android_native_app_glue.h"
#define ANDROID_APP_LOG(...) ((void)__android_log_print(ANDROID_LOG_INFO, "android_app", __VA_ARGS__))
#define ANDROID_APP_ERROR(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "android_app", __VA_ARGS__))

#define ANDROID_APP_DEBUG//debug 模式

#ifdef ANDROID_APP_DEBUG
#define ANDROID_APP_LOGV(...) ((void)__android_log_print(ANDROID_LOG_VERBOSE, "Morty Log", __VA_ARGS__))
#else
#define ANDROID_APP_LOGV(...) ((void)0)
#endif

#include "MEngine.h"
#include "MForwardRenderProgram.h"
#include "MAndroidRenderView.h"
#include "MLogManager.h"

#include "M3DNode.h"
#include "MScene.h"
#include "MViewport.h"

void android_main(android_app* app)
{
    ANDROID_APP_LOGV("程序启动");
    //设置消息回调函数，就一个，整体和win32编程差不多了。

    MLogManager::GetInstance()->SetPrintFunction(
            [](const char* svMessage)
                 {
                     ANDROID_APP_LOGV("%s", svMessage);
                 });

    MLogManager::GetInstance()->Log("Test");

    MEngine engine;
    ANDROID_APP_LOGV("创建引擎");
    engine.Initialize();
    ANDROID_APP_LOGV("初始化");
    engine.RegisterRenderProgram<MForwardRenderProgram>();
    M3DNode* pRootNode = engine.GetObjectManager()->CreateObject<M3DNode>();
    pRootNode->SetName("RootNode");
    //循环等待事情以进行处理。
    ANDROID_APP_LOGV("创建Scene");
    MScene* pScene = engine.GetObjectManager()->CreateObject<MScene>();
    pScene->SetRootNode(pRootNode);
    MAndroidRenderView* pView = new MAndroidRenderView();
    pView->SetNativeWindow(app->window);
    ANDROID_APP_LOGV("创建View");
    pView->Initialize(&engine, "Morty");
    ANDROID_APP_LOGV("View Init 完成");
    pView->SetBackColor(MColor(0.25f, 0.25f, 0.25f, 1.0f));
    MViewport* pViewport = engine.GetObjectManager()->CreateObject<MViewport>();
    pView->AppendViewport(pViewport);
    pViewport->SetSize(Vector2(pView->GetViewWidth(), pView->GetViewHeight()));
    pViewport->SetScene(pScene);
    engine.AddView(pView);
    engine.SetScene(pScene);
    ANDROID_APP_LOGV("开始主循环");
    while (engine.MainLoop());
    engine.Release();

    ANDROID_APP_LOGV("Morty main : 程序结束");
}