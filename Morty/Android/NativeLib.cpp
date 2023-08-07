#define DOCTEST_CONFIG_IMPLEMENT

#include <jni.h>
#include <string>
#include <android/log.h>

#include "Main/MainEditor.h"

#include "Module/MCoreModule.h"
#include "MRenderModule.h"
#include "Module/MEditorModule.h"

#include "Scene/MScene.h"
#include "System/MObjectSystem.h"
#include "System/MResourceSystem.h"

#include "crossguid/guid.hpp"


#include "Test/Floor.h"
#include "Test/ShadowMap.h"

static MEngine* s_pEngine = nullptr;
static SDLRenderView* s_pRenderView = nullptr;
static MainEditor* s_pEditor = nullptr;

static bool bEditorClosed = false;

void OutputLog(MLogType eType, const char* svLog)
{
    static std::map<MLogType, int> PrioMapping = {
            {MLogType::EDefault, ANDROID_LOG_VERBOSE},
            {MLogType::EInfo, ANDROID_LOG_VERBOSE},
            {MLogType::EWarn, ANDROID_LOG_VERBOSE},
            {MLogType::EError, ANDROID_LOG_VERBOSE},
    };

    auto findResult = PrioMapping.find(eType);
    const int prio = findResult == PrioMapping.end() ? ANDROID_LOG_VERBOSE : findResult->second;

    const size_t nMaxLogSize = 127;
    const size_t nLogSize = strlen(svLog);

    char output[nMaxLogSize + 1];
    output[nMaxLogSize] = '\0';

    for (size_t i = 0; i < nLogSize; i += nMaxLogSize)
    {
        memcpy(output, svLog + i, nMaxLogSize);
        __android_log_print(prio, "Morty Output", "%s", output);
    }

}

extern "C" JNIEXPORT jint JNICALL
Java_me_doubleye_morty_MortyNative_InitializeEngine(
        JNIEnv* env,
        jobject /* this */,
        jstring jResourcePath) {

    //crossguid init.
    xg::initJni(env);

    const char *cstr = env->GetStringUTFChars(jResourcePath, NULL);
    std::string strResourcePath = std::string(cstr);
    env->ReleaseStringUTFChars(jResourcePath, cstr);

    if (s_pEngine)
    {
        return 0;
    }

    s_pEngine = new MEngine();
    s_pEngine->Initialize();

    s_pEngine->GetLogger()->SetPrintFunction(OutputLog);

    //register module
    MCoreModule::Register(s_pEngine);
    s_pEngine->FindSystem<MResourceSystem>()->SetSearchPath({ strResourcePath });

    MRenderModule::Register(s_pEngine);
    MEditorModule::Register(s_pEngine);

    //create window.
    s_pRenderView = new SDLRenderView();
    s_pRenderView->Initialize(s_pEngine, "Morty");


    //create editor
    s_pEditor= new MainEditor();
    s_pEditor->Initialize(s_pEngine);
    s_pRenderView->AppendContent(s_pEditor);


    MScene* pScene = s_pEngine->FindSystem<MObjectSystem>()->CreateObject<MScene>();
    s_pEditor->SetScene(pScene);

    ADD_DIRECTIONAL_LIGHT(s_pEngine, pScene);

    SHADOW_MAP_TEST(s_pEngine, pScene);


    //start run
    s_pEngine->Start();


    return 0;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_me_doubleye_morty_MortyNative_TickEngine(
        JNIEnv* env,
        jobject /* this */) {

    s_pEngine->Update();

    return bEditorClosed;
}

extern "C" JNIEXPORT jint JNICALL
Java_me_doubleye_morty_MortyNative_ReleaseEngine(
        JNIEnv* env,
        jobject /* this */) {

    if (s_pEngine)
    {
        return 0;
    }

    //stop run
    s_pEngine->Stop();

    s_pEditor->Release();
    s_pEditor = nullptr;

    s_pEngine->Release();
    s_pEngine = nullptr;

    return 0;
}

extern "C" JNIEXPORT jint JNICALL
Java_me_doubleye_morty_MortyNative_BindSDLWindow(
        JNIEnv* env,
        jobject /* this */,
        jstring jResourcePath) {

    s_pRenderView->BindSDLWindow();

    return 0;
}


extern "C" JNIEXPORT jint JNICALL
Java_me_doubleye_morty_MortyNative_UnbindSDLWindow(
        JNIEnv* env,
        jobject /* this */,
        jstring jResourcePath) {

    s_pRenderView->UnbindSDLWindow();

    return 0;
}