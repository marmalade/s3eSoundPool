/*
 * android-specific implementation of the s3eSoundPool extension.
 * Add any platform-specific functionality here.
 */
/*
 * NOTE: This file was originally written by the extension builder, but will not
 * be overwritten (unless --force is specified) and is intended to be modified.
 */
#include "s3eSoundPool_internal.h"

#define S3E_CURRENT_EXT SOUNDPOOL

#include "s3eEdk.h"
#include "s3eEdkError.h"
#include "s3eEdk_android.h"
#include <jni.h>
#include "IwDebug.h"

#ifndef MIN
#define MIN(x,y) ((x)<=(y) ? (x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>=(y) ? (x):(y))
#endif

static jobject g_Obj;
static jmethodID g_s3eSoundPoolTerminate;
static jmethodID g_s3eSoundPoolGetMasterVolume;
static jmethodID g_s3eSoundPoolSetMasterVolume;
static jmethodID g_s3eSoundPoolGetSampleVolume;
static jmethodID g_s3eSoundPoolSetSampleVolume;
static jmethodID g_s3eSoundPoolGetSampleStatus;
static jmethodID g_s3eSoundPoolPauseAllSamples;
static jmethodID g_s3eSoundPoolResumeAllSamples;
static jmethodID g_s3eSoundPoolStopAllSamples;
static jmethodID g_s3eSoundPoolSampleLoad;
static jmethodID g_s3eSoundPoolSampleUnload;
static jmethodID g_s3eSoundPoolSamplePlay;
static jmethodID g_s3eSoundPoolSampleStop;
static jmethodID g_s3eSoundPoolSamplePause;
static jmethodID g_s3eSoundPoolSampleResume;

void JNICALL s3eSoundPool_SetError(JNIEnv* env, jobject obj, jint code);
void JNICALL s3eSoundPool_SampleEnded(JNIEnv* env, jobject obj, jint sampleId);

enum s3eSoundPoolSampleStatus
{
    S3E_SOUNDPOOL_STATUS_ERROR      = -1,
    S3E_SOUNDPOOL_STATUS_STOPPED    = 0,
    S3E_SOUNDPOOL_STATUS_PLAYING    = 1,
    S3E_SOUNDPOOL_STATUS_PAUSED     = 2,
};

s3eResult s3eSoundPoolInit_platform()
{
    // Get the environment from the pointer
    JNIEnv* env = s3eEdkJNIGetEnv();
    jobject obj = NULL;
    jmethodID cons = NULL;

    const JNINativeMethod nativeMethods[] =
    {
        {"SetError",        "(I)V",        (void*)&s3eSoundPool_SetError},
        {"SampleEnded",     "(I)V",        (void*)&s3eSoundPool_SampleEnded},
    };
    const int numNativeMethods = sizeof(nativeMethods)/sizeof(nativeMethods[0]);

    // Get the extension class
    jclass cls = s3eEdkAndroidFindClass("s3eSoundPool");
    if (!cls)
        goto fail;

    // Get its constructor
    cons = env->GetMethodID(cls, "<init>", "()V");
    if (!cons)
        goto fail;

    // Construct the java class
    obj = env->NewObject(cls, cons);
    if (!obj)
        goto fail;

    // Get all the extension methods
    g_s3eSoundPoolTerminate = env->GetMethodID(cls, "Terminate", "()V");
    if (!g_s3eSoundPoolTerminate)
        goto fail;

    g_s3eSoundPoolGetMasterVolume = env->GetMethodID(cls, "GetMasterVolume", "()I");
    if (!g_s3eSoundPoolGetMasterVolume)
        goto fail;

    g_s3eSoundPoolSetMasterVolume = env->GetMethodID(cls, "SetMasterVolume", "(I)V");
    if (!g_s3eSoundPoolSetMasterVolume)
        goto fail;

    g_s3eSoundPoolGetSampleVolume = env->GetMethodID(cls, "GetSampleVolume", "(I)I");
    if (!g_s3eSoundPoolGetSampleVolume)
        goto fail;

    g_s3eSoundPoolSetSampleVolume = env->GetMethodID(cls, "SetSampleVolume", "(II)I");
    if (!g_s3eSoundPoolSetSampleVolume)
        goto fail;

    g_s3eSoundPoolGetSampleStatus = env->GetMethodID(cls, "GetSampleStatus", "(I)I");
    if (!g_s3eSoundPoolGetSampleStatus)
        goto fail;

    g_s3eSoundPoolPauseAllSamples = env->GetMethodID(cls, "PauseAllSamples", "()I");
    if (!g_s3eSoundPoolPauseAllSamples)
        goto fail;

    g_s3eSoundPoolResumeAllSamples = env->GetMethodID(cls, "ResumeAllSamples", "()I");
    if (!g_s3eSoundPoolResumeAllSamples)
        goto fail;

    g_s3eSoundPoolStopAllSamples = env->GetMethodID(cls, "StopAllSamples", "()I");
    if (!g_s3eSoundPoolStopAllSamples)
        goto fail;

    g_s3eSoundPoolSampleLoad = env->GetMethodID(cls, "SampleLoad", "(Ljava/lang/String;)I");
    if (!g_s3eSoundPoolSampleLoad)
        goto fail;

    g_s3eSoundPoolSampleUnload = env->GetMethodID(cls, "SampleUnload", "(I)I");
    if (!g_s3eSoundPoolSampleUnload)
        goto fail;

    g_s3eSoundPoolSamplePlay = env->GetMethodID(cls, "SamplePlay", "(III)I");
    if (!g_s3eSoundPoolSamplePlay)
        goto fail;

    g_s3eSoundPoolSampleStop = env->GetMethodID(cls, "SampleStop", "(I)I");
    if (!g_s3eSoundPoolSampleStop)
        goto fail;

    g_s3eSoundPoolSamplePause = env->GetMethodID(cls, "SamplePause", "(I)I");
    if (!g_s3eSoundPoolSamplePause)
        goto fail;

    g_s3eSoundPoolSampleResume = env->GetMethodID(cls, "SampleResume", "(I)I");
    if (!g_s3eSoundPoolSampleResume)
        goto fail;

    if(env->RegisterNatives(cls, nativeMethods, numNativeMethods) != JNI_OK)
        goto fail;

    IwTrace(SOUNDPOOL, ("SOUNDPOOL init success"));
    g_Obj = env->NewGlobalRef(obj);
    env->DeleteLocalRef(obj);
    env->DeleteGlobalRef(cls);

    // Add any platform-specific initialisation code here
    return S3E_RESULT_SUCCESS;

fail:
    jthrowable exc = env->ExceptionOccurred();
    if (exc)
    {
        env->ExceptionDescribe();
        env->ExceptionClear();
        IwTrace(s3eSoundPool, ("One or more java methods could not be found"));
    }
    return S3E_RESULT_ERROR;

}

void s3eSoundPoolTerminate_platform()
{
    JNIEnv* env = s3eEdkJNIGetEnv();

    env->CallVoidMethod(g_Obj, g_s3eSoundPoolTerminate);

    env->DeleteGlobalRef(g_Obj);
    g_Obj = NULL;
}

int32 s3eSoundPoolGetInt_platform(s3eSoundPoolProperty property)
{
    JNIEnv* env = s3eEdkJNIGetEnv();

    int value = -1;
    bool gotValue = false;

    switch(property)
    {
    case S3E_SOUNDPOOL_VOLUME:
        value = env->CallIntMethod(g_Obj, g_s3eSoundPoolGetMasterVolume);
        gotValue = true;
        break;
    }

    if (!gotValue)
    {
        S3E_EXT_ERROR_SIMPLE(PARAM);
    }

    return value;
}

s3eResult s3eSoundPoolSetInt_platform(s3eSoundPoolProperty property, int32 value)
{
    JNIEnv* env = s3eEdkJNIGetEnv();

    int setValue = false;

    switch(property)
    {
    case S3E_SOUNDPOOL_VOLUME:
        {
            int volume = MAX(0, MIN(value, S3E_SOUNDPOOL_MAX_VOLUME));
            env->CallVoidMethod(g_Obj, g_s3eSoundPoolSetMasterVolume, volume);
            setValue = true;
        }
        break;
    }

    if (!setValue)
    {
        S3E_EXT_ERROR_SIMPLE(PARAM);
        return S3E_RESULT_ERROR;
    }
    
    return S3E_RESULT_SUCCESS;
}

int32 s3eSoundPoolSampleGetInt_platform(int32 sampleId, s3eSoundPoolSampleProperty property)
{
    JNIEnv* env = s3eEdkJNIGetEnv();

    int value = -1;
    bool gotValue = false;

    switch(property)
    {
    case S3E_SOUNDPOOL_STREAM_VOLUME:
        value = env->CallIntMethod(g_Obj, g_s3eSoundPoolGetSampleVolume, sampleId);
        gotValue = true;
        break;
    case S3E_SOUNDPOOL_STREAM_STATUS:
        value =  env->CallIntMethod(g_Obj, g_s3eSoundPoolGetSampleStatus, sampleId) 
            == S3E_SOUNDPOOL_STATUS_PLAYING ? 1 : 0;
        gotValue = true;
        break;
    case S3E_SOUNDPOOL_STREAM_PAUSED:
        value =  env->CallIntMethod(g_Obj, g_s3eSoundPoolGetSampleStatus, sampleId) 
            == S3E_SOUNDPOOL_STATUS_PAUSED ? 1 : 0;
        gotValue = true;
        break;
    }

    if (!gotValue)
    {
        S3E_EXT_ERROR_SIMPLE(PARAM);
    }

    return value;
}

s3eResult s3eSoundPoolSampleSetInt_platform(int32 sampleId, s3eSoundPoolSampleProperty property, int32 value)
{
    JNIEnv* env = s3eEdkJNIGetEnv();

    int setValue = false;

    switch(property)
    {
    case S3E_SOUNDPOOL_STREAM_VOLUME:
        {
            int volume = MAX(0, MIN(value, S3E_SOUNDPOOL_MAX_VOLUME));
            setValue = env->CallIntMethod(g_Obj, g_s3eSoundPoolSetSampleVolume, sampleId, volume) != 0;
        }
        break;
    }

    if (!setValue)
    {
        S3E_EXT_ERROR_SIMPLE(PARAM);
        return S3E_RESULT_ERROR;
    }
    
    return S3E_RESULT_SUCCESS;
}

s3eResult s3eSoundPoolPauseAllSamples_platform()
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    return (s3eResult)env->CallIntMethod(g_Obj, g_s3eSoundPoolPauseAllSamples);
}

s3eResult s3eSoundPoolResumeAllSamples_platform()
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    return (s3eResult)env->CallIntMethod(g_Obj, g_s3eSoundPoolResumeAllSamples);
}

s3eResult s3eSoundPoolStopAllSamples_platform()
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    return (s3eResult)env->CallIntMethod(g_Obj, g_s3eSoundPoolStopAllSamples);
}

int32 s3eSoundPoolSampleLoad_platform(const char* pPath)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    jstring pPath_jstr = env->NewStringUTF(pPath);
    int32 result = (int32)env->CallIntMethod(g_Obj, g_s3eSoundPoolSampleLoad, pPath_jstr);
    env->DeleteLocalRef(pPath_jstr);  
    return result;
}

s3eResult s3eSoundPoolSampleUnload_platform(int32 sampleId)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    return (s3eResult)env->CallIntMethod(g_Obj, g_s3eSoundPoolSampleUnload, sampleId);
}

s3eResult s3eSoundPoolSamplePlay_platform(int32 sampleId, int32 repeat, int32 loopfrom)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    return (s3eResult)env->CallIntMethod(g_Obj, g_s3eSoundPoolSamplePlay, sampleId, repeat, loopfrom);
}

s3eResult s3eSoundPoolSampleStop_platform(int32 sampleId)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    return (s3eResult)env->CallIntMethod(g_Obj, g_s3eSoundPoolSampleStop, sampleId);
}

s3eResult s3eSoundPoolSamplePause_platform(int32 sampleId)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    return (s3eResult)env->CallIntMethod(g_Obj, g_s3eSoundPoolSamplePause, sampleId);
}

s3eResult s3eSoundPoolSampleResume_platform(int32 sampleId)
{
    JNIEnv* env = s3eEdkJNIGetEnv();
    return (s3eResult)env->CallIntMethod(g_Obj, g_s3eSoundPoolSampleResume, sampleId);
}

void s3eSoundPool_SetError(JNIEnv* env, jobject obj, jint code)
{
    switch(code)
    {
    case S3E_SOUNDPOOL_ERR_NONE:
        S3E_EXT_ERROR_SIMPLE(NONE);
        break;
    case S3E_SOUNDPOOL_ERR_PARAM:
        S3E_EXT_ERROR_SIMPLE(PARAM);
        break;
    case S3E_SOUNDPOOL_ERR_TOO_MANY:
        S3E_EXT_ERROR_SIMPLE(TOO_MANY);
        break;
    case S3E_SOUNDPOOL_ERR_ALREADY_REG:
        S3E_EXT_ERROR_SIMPLE(ALREADY_REG);
        break;
    default:
        _s3eEdkErrorSet(S3E_EXT_SOUNDPOOL_HASH, code, S3E_EXT_ERROR_PRI_NORMAL, "Unrecognised error");
        break;
    }
}

void JNICALL s3eSoundPool_SampleEnded(JNIEnv* env, jobject obj, jint sampleId)
{
    s3eSoundPoolEndSampleInfo endSampleInfo = { sampleId };

    s3eEdkCallbacksEnqueue(S3E_EXT_SOUNDPOOL_HASH, S3E_SOUNDPOOL_STOP_AUDIO,
        &endSampleInfo, sizeof(endSampleInfo));
}
