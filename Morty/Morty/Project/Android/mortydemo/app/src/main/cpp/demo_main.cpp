
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
#include <android/asset_manager.h>

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
#include "MResourceManager.h"

#include "Model/MModelResource.h"
#include "Model/MModelInstance.h"

#include "M3DNode.h"
#include "MCamera.h"
#include "MScene.h"
#include "MViewport.h"

static std::string g_path = "";

static MEngine g_Engine;
static bool g_bCreatedView = false;
static bool isLoop = false;
static pthread_t loopID;

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
    ANDROID_APP_LOGV("NativeWindowCreated: %p -- %p\n", activity, window);


    M3DNode* pRootNode = g_Engine.GetObjectManager()->CreateObject<M3DNode>();
    pRootNode->SetName("RootNode");

    MCamera* pCamera = g_Engine.GetObjectManager()->CreateObject<MCamera>();
    pCamera->SetPosition(Vector3(0, 0, -20));
    pCamera->SetName("Camera");
    pCamera->SetZNearFar(Vector2(10, 500));
    pCamera->LookAt(Vector3(0, 0, 0), Vector3(0, 1, 0));
    //pCamera->SetCameraType(MCamera::MECameraType::EOrthographic);
    pRootNode->AddNode(pCamera);

    MModelResource* pJeepResource = dynamic_cast<MModelResource*>(g_Engine.GetResourceManager()->LoadResource("./Model/teaport/teaport.model"));
    MModelInstance* pJeepModel = g_Engine.GetObjectManager()->CreateObject<MModelInstance>();
    pJeepModel->SetPosition(Vector3(0, 0, 10));
    pJeepModel->SetScale(Vector3(1, 1, 1));
    pJeepModel->SetGenerateDirLightShadow(true);
    if(pJeepModel->Load(pJeepResource))
    {
        pJeepModel->SetName("Jeep");
        pRootNode->AddNode(pJeepModel);
        pJeepModel->SetRotation(Quaternion(Vector3(1, 0, 0), 0));
        pJeepModel->GetFixedChildren()[0]->DynamicCast<M3DNode>()->SetRotation(Quaternion(Vector3(1, 0, 0), 0));

        auto rot = pJeepModel->GetRotation();
        rot.RotateY(-90);

        pJeepModel->SetRotation(rot);
    }
    else
    {
        MLogManager::GetInstance()->Error("Load Failed: Jeep(is teaport).");
    }

    //循环等待事情以进行处理。
    ANDROID_APP_LOGV("创建Scene");
    MScene* pScene = g_Engine.GetObjectManager()->CreateObject<MScene>();
    pScene->SetRootNode(pRootNode);
    MAndroidRenderView* pView = new MAndroidRenderView();
    pView->SetNativeWindow(window);
    pView->SetSize(Vector2(ANativeWindow_getWidth(window), ANativeWindow_getHeight(window)));
    ANDROID_APP_LOGV("创建View");
    pView->Initialize(&g_Engine, "Morty");
    ANDROID_APP_LOGV("View Init 完成");
    pView->SetBackColor(MColor(1.0f, 0.25f, 0.25f, 1.0f));
    MViewport* pViewport = g_Engine.GetObjectManager()->CreateObject<MViewport>();
    pView->AppendViewport(pViewport);
    pViewport->SetSize(Vector2(pView->GetViewWidth(), pView->GetViewHeight()));
    pViewport->SetScene(pScene);
    g_Engine.AddView(pView);
    g_Engine.SetScene(pScene);

    g_bCreatedView = true;


}

void EngineInit() {

    MLogManager::GetInstance()->SetPrintFunction(
            [](const char* svMessage)
            {
                ANDROID_APP_LOGV("%s", svMessage);
            });

    ANDROID_APP_LOGV("创建引擎");

    g_Engine.Initialize("/sdcard/Morty");
    ANDROID_APP_LOGV("初始化");
    g_Engine.RegisterRenderProgram<MForwardRenderProgram>();
}

void EngineRele(){
    g_Engine.Release();
}

void *looper(void *args) {
    ANativeActivity *activity = (ANativeActivity *) args;
    AInputQueue *queue = (AInputQueue *) activity->instance;
    AInputEvent *event = NULL;
    ANDROID_APP_LOGV("开始主循环");
    while (isLoop) {

        g_Engine.MainLoop();

        if (!AInputQueue_hasEvents(queue)) {
            continue;
        }
        AInputQueue_getEvent(queue, &event);
        AInputQueue_finishEvent(queue, event, 1);
    }
    return args;
}

void onInputQueueCreated(ANativeActivity *activity, AInputQueue *queue) {
    isLoop = true;
    activity->instance = (void *) queue;
    ANDROID_APP_LOGV("引擎启动");
    
    EngineInit();
    pthread_create(&loopID, NULL, looper, activity);
}

void onInputQueueDestroyed(ANativeActivity *activity, AInputQueue *queue) {
    isLoop = false;
    EngineRele();

    ANDROID_APP_LOGV("Morty main : 引擎结束");
}

JNIEXPORT
void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState,
                              size_t savedStateSize) {

    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

}