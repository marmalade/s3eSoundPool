/*
 * Internal header for the s3eSoundPool extension.
 *
 * This file should be used for any common function definitions etc that need to
 * be shared between the platform-dependent and platform-indepdendent parts of
 * this extension.
 */

/*
 * NOTE: This file was originally written by the extension builder, but will not
 * be overwritten (unless --force is specified) and is intended to be modified.
 */


#ifndef S3ESOUNDPOOL_INTERNAL_H
#define S3ESOUNDPOOL_INTERNAL_H

#include "s3eTypes.h"
#include "s3eSoundPool.h"
#include "s3eSoundPool_autodefs.h"


/**
 * Initialise the extension.  This is called once then the extension is first
 * accessed by s3eregister.  If this function returns S3E_RESULT_ERROR the
 * extension will be reported as not-existing on the device.
 */
s3eResult s3eSoundPoolInit();

/**
 * Platform-specific initialisation, implemented on each platform
 */
s3eResult s3eSoundPoolInit_platform();

/**
 * Terminate the extension.  This is called once on shutdown, but only if the
 * extension was loader and Init() was successful.
 */
void s3eSoundPoolTerminate();

/**
 * Platform-specific termination, implemented on each platform
 */
void s3eSoundPoolTerminate_platform();
int32 s3eSoundPoolGetInt_platform(s3eSoundPoolProperty property);

s3eResult s3eSoundPoolSetInt_platform(s3eSoundPoolProperty property, int32 value);

s3eResult s3eSoundPoolPauseAllSamples_platform();

s3eResult s3eSoundPoolResumeAllSamples_platform();

s3eResult s3eSoundPoolStopAllSamples_platform();

int32 s3eSoundPoolSampleLoad_platform(const char* pPath);

s3eResult s3eSoundPoolSampleUnload_platform(int32 sampleId);

s3eResult s3eSoundPoolSamplePlay_platform(int32 sampleId, int32 repeat, int32 loopfrom);

s3eResult s3eSoundPoolSampleStop_platform(int32 sampleId);

s3eResult s3eSoundPoolSamplePause_platform(int32 sampleId);

s3eResult s3eSoundPoolSampleResume_platform(int32 sampleId);

int32 s3eSoundPoolSampleGetInt_platform(int32 sampleId, s3eSoundPoolSampleProperty property);

s3eResult s3eSoundPoolSampleSetInt_platform(int32 sampleId, s3eSoundPoolSampleProperty property, int32 value);


#endif /* !S3ESOUNDPOOL_INTERNAL_H */