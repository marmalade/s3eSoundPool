/*
Generic implementation of the s3eSoundPool extension.
This file should perform any platform-indepedentent functionality
(e.g. error checking) before calling platform-dependent implementations.
*/

/*
 * NOTE: This file was originally written by the extension builder, but will not
 * be overwritten (unless --force is specified) and is intended to be modified.
 */


#include "s3eSoundPool_internal.h"
s3eResult s3eSoundPoolInit()
{
    //Add any generic initialisation code here
    return s3eSoundPoolInit_platform();
}

void s3eSoundPoolTerminate()
{
    //Add any generic termination code here
    s3eSoundPoolTerminate_platform();
}

int32 s3eSoundPoolGetInt(s3eSoundPoolProperty property)
{
	return s3eSoundPoolGetInt_platform(property);
}

s3eResult s3eSoundPoolSetInt(s3eSoundPoolProperty property, int32 value)
{
	return s3eSoundPoolSetInt_platform(property, value);
}

s3eResult s3eSoundPoolPauseAllSamples()
{
	return s3eSoundPoolPauseAllSamples_platform();
}

s3eResult s3eSoundPoolResumeAllSamples()
{
	return s3eSoundPoolResumeAllSamples_platform();
}

s3eResult s3eSoundPoolStopAllSamples()
{
	return s3eSoundPoolStopAllSamples_platform();
}

int32 s3eSoundPoolSampleLoad(const char* pPath)
{
	return s3eSoundPoolSampleLoad_platform(pPath);
}

s3eResult s3eSoundPoolSampleUnload(int32 sampleId)
{
	return s3eSoundPoolSampleUnload_platform(sampleId);
}

s3eResult s3eSoundPoolSamplePlay(int32 sampleId, int32 repeat, int32 loopfrom)
{
	return s3eSoundPoolSamplePlay_platform(sampleId, repeat, loopfrom);
}

s3eResult s3eSoundPoolSampleStop(int32 sampleId)
{
	return s3eSoundPoolSampleStop_platform(sampleId);
}

s3eResult s3eSoundPoolSamplePause(int32 sampleId)
{
	return s3eSoundPoolSamplePause_platform(sampleId);
}

s3eResult s3eSoundPoolSampleResume(int32 sampleId)
{
	return s3eSoundPoolSampleResume_platform(sampleId);
}

int32 s3eSoundPoolSampleGetInt(int32 sampleId, s3eSoundPoolSampleProperty property)
{
	return s3eSoundPoolSampleGetInt_platform(sampleId, property);
}

s3eResult s3eSoundPoolSampleSetInt(int32 sampleId, s3eSoundPoolSampleProperty property, int32 value)
{
	return s3eSoundPoolSampleSetInt_platform(sampleId, property, value);
}
