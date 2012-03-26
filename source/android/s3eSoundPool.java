/*
java implementation of the s3eSoundPool extension.

Add android-specific functionality here.

These functions are called via JNI from native code.
*/
/*
 * NOTE: This file was originally written by the extension builder, but will not
 * be overwritten (unless --force is specified) and is intended to be modified.
 */
import com.ideaworks3d.marmalade.LoaderAPI;
import com.ideaworks3d.marmalade.SuspendResumeEvent;
import com.ideaworks3d.marmalade.SuspendResumeListener;

import android.content.res.AssetFileDescriptor;
import android.media.AudioManager;
import android.media.MediaPlayer;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

class s3eSoundPool implements SuspendResumeListener
{
    static final int MAX_VOLUME        = 0x100;

    static final int ERR_NONE          = 0;
    static final int ERR_PARAM         = 1;
    static final int ERR_TOO_MANY      = 2;
    static final int ERR_ALREADY_REG   = 3;

    static final int STATUS_ERROR      = -1;
    static final int STATUS_STOPPED    = 0;
    static final int STATUS_PLAYING    = 1;
    static final int STATUS_PAUSED     = 2;

    private class Sample implements MediaPlayer.OnCompletionListener, MediaPlayer.OnErrorListener
    {
        private final int   m_SampleId;
        private int         m_Status       = STATUS_STOPPED;
        private MediaPlayer m_MediaPlayer;
        private int         m_Volume;
        private int         m_LoopCount;
        private int         m_LoopFrom;
        private boolean     m_WasPlayingOnSuspend;

        Sample(int sampleId)
        {
            m_SampleId = sampleId;
        }

        boolean setVolume(int volume)
        {
            m_Volume = volume;
            float mediaVolume = calculateVolume(m_Volume);
            m_MediaPlayer.setVolume(mediaVolume, mediaVolume);

            return true;
        }

        boolean load(String path)
        {
            try
            {
                AssetFileDescriptor afd = 
                    LoaderAPI.getActivity().getAssets().openFd(path);
                MediaPlayer mediaPlayer = new MediaPlayer();
                mediaPlayer.setOnCompletionListener(this);
                mediaPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
                mediaPlayer.setDataSource(
                    afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
                mediaPlayer.prepare();

                m_MediaPlayer = mediaPlayer;

                return true;
            }
            catch (IOException e)
            {
                return false;
            }
        }

        void unload()
        {
            m_MediaPlayer.release();
        }

        boolean play(int repeat, int loopfrom)
        {
            if (m_Status == STATUS_ERROR)
                return false;
            
            if (m_Status != STATUS_STOPPED)
                m_MediaPlayer.seekTo(0);

            m_Status = STATUS_PLAYING;

            boolean looping = false;
            if (repeat <= 0)
            {
                looping = true;
                m_LoopCount = -1;
            }
            else
            {
                m_LoopCount = repeat - 1;
            }
            m_LoopFrom = loopfrom;
            
            m_MediaPlayer.setLooping(looping);
            m_MediaPlayer.start();

            return true;
        }

        boolean pause()
        {
            if (m_Status == STATUS_PLAYING)
            {
                m_MediaPlayer.pause();
                m_Status = STATUS_PAUSED;
                m_WasPlayingOnSuspend = false;
                return true;
            }
            return false;
        }

        boolean resume()
        {
            if (m_Status == STATUS_PAUSED)
            {
                m_MediaPlayer.start();
                m_Status = STATUS_PLAYING;
                m_WasPlayingOnSuspend = false;
                return true;
            }
            return false;
        }

        boolean stop()
        {
            if (m_Status == STATUS_PLAYING ||
                m_Status == STATUS_PAUSED)
            {
                m_MediaPlayer.pause();
                m_MediaPlayer.seekTo(0);
                m_Status = STATUS_STOPPED;
                return true;
            }
            return false;
        }

        void suspendEvent()
        {
            boolean isPlaying = m_Status == STATUS_PLAYING;
            if (isPlaying)
                pause();
            m_WasPlayingOnSuspend = isPlaying;
        }

        void resumeEvent()
        {
            if (m_WasPlayingOnSuspend)
                resume();
        }

        @Override
        public void onCompletion(MediaPlayer mp)
        {
            if (m_LoopCount == 0)
            {
                m_MediaPlayer.seekTo(0);
                m_Status = STATUS_STOPPED;
                onPlayComplete(m_SampleId);
            }
            else
            {
                m_MediaPlayer.seekTo(m_LoopFrom);
                m_LoopCount -= 1;
                m_MediaPlayer.start();
            }
        }

        @Override
        public boolean onError (MediaPlayer mp, int what, int extra)
        {
            LoaderAPI.trace("!!!! s3eSoundPool onError what: " + what);

            m_Status = STATUS_ERROR;

            return true;
        }
    }

    private native void SetError(int code);
    private native void SampleEnded(int sampleId);

    public s3eSoundPool()
    {
        m_MasterVolume = MAX_VOLUME;

        LoaderAPI.addSuspendResumeListener(this);
    }

    public void Terminate()
    {
        LoaderAPI.trace("**** s3eSoundPool Terminate");

        for (Map.Entry<Integer, Sample> entry : m_Samples.entrySet())
        {
            Sample sample = entry.getValue();
            sample.unload();
        }
        m_Samples.clear();

        LoaderAPI.removeSuspendResumeListener(this);
    }

    public int GetMasterVolume()
    {
        return m_MasterVolume;
    }

    public synchronized int GetSampleVolume(int sampleId)
    {
        int volume = -1;
        Sample sample = m_Samples.get(sampleId);
        if (sample != null)
        {
            volume = sample.m_Volume;
        }

        return volume;
    }

    public synchronized int SetSampleVolume(int sampleId, int volume)
    {
        int result = LoaderAPI.S3E_RESULT_ERROR;
        Sample sample = m_Samples.get(sampleId);
        if (sample != null)
        {
            if (sample.setVolume(volume))
                result = LoaderAPI.S3E_RESULT_SUCCESS;
        }

        return result;
    }

    public synchronized int GetSampleStatus(int sampleId)
    {
        int status = -1;
        Sample sample = m_Samples.get(sampleId);
        if (sample != null)
        {
            status = sample.m_Status;
        }

        return status;
    }

    public void SetMasterVolume(int volume)
    {
        m_MasterVolume = volume;

        for (Map.Entry<Integer, Sample> entry : m_Samples.entrySet())
        {
            Sample sample = entry.getValue();
            sample.setVolume(sample.m_Volume);
        }
    }

    public synchronized int PauseAllSamples()
    {
        for (Map.Entry<Integer, Sample> entry : m_Samples.entrySet())
        {
            Sample sample = entry.getValue();
            sample.pause();
        }

        return LoaderAPI.S3E_RESULT_SUCCESS;
    }

    public synchronized int ResumeAllSamples()
    {
        for (Map.Entry<Integer, Sample> entry : m_Samples.entrySet())
        {
            Sample sample = entry.getValue();
            sample.resume();
        }

        return LoaderAPI.S3E_RESULT_SUCCESS;
    }

    public synchronized int StopAllSamples()
    {
        for (Map.Entry<Integer, Sample> entry : m_Samples.entrySet())
        {
            Sample sample = entry.getValue();
            sample.stop();
        }

        return LoaderAPI.S3E_RESULT_SUCCESS;
    }

    public synchronized int SampleLoad(String path)
    {
        int sampleId = -1;
        
        Sample sample = new Sample(m_NextSampleId);
        if (sample.load(path))
        {
            sample.setVolume(MAX_VOLUME);

            sampleId = m_NextSampleId;
            ++m_NextSampleId;
            m_Samples.put(sampleId, sample);
        }

        return sampleId;
    }

    public synchronized int SampleUnload(int sampleId)
    {
        int result = LoaderAPI.S3E_RESULT_ERROR;
        Sample sample = m_Samples.get(sampleId);
        if (sample != null)
        {
            m_Samples.remove(sampleId);
            sample.unload();
            result = LoaderAPI.S3E_RESULT_SUCCESS;
        }

        return result;
    }

    public synchronized int SamplePlay(int sampleId, int repeat, int loopfrom)
    {
        int result = LoaderAPI.S3E_RESULT_ERROR;
        Sample sample = m_Samples.get(sampleId);
        if (sample != null)
        {
            if (sample.play(repeat, loopfrom))
                result = LoaderAPI.S3E_RESULT_SUCCESS;
        }

        return result;
    }

    public synchronized int SampleStop(int sampleId)
    {
        int result = LoaderAPI.S3E_RESULT_ERROR;
        Sample sample = m_Samples.get(sampleId);
        if (sample != null)
        {
            if (sample.stop())
                result = LoaderAPI.S3E_RESULT_SUCCESS;
        }

        return result;
    }

    public synchronized int SamplePause(int sampleId)
    {
        int result = LoaderAPI.S3E_RESULT_ERROR;
        Sample sample = m_Samples.get(sampleId);
        if (sample != null)
        {
            if (sample.pause())
                result = LoaderAPI.S3E_RESULT_SUCCESS;
        }

        return result;
    }

    public synchronized int SampleResume(int sampleId)
    {
        int result = LoaderAPI.S3E_RESULT_ERROR;
        Sample sample = m_Samples.get(sampleId);
        if (sample != null)
        {
            if (sample.resume())
                result = LoaderAPI.S3E_RESULT_SUCCESS;
        }

        return result;
    }

    @Override
    public synchronized void onSuspendResumeEvent(SuspendResumeEvent e)
    {
        if (e.eventType == SuspendResumeEvent.EventType.SUSPEND)
        {
            LoaderAPI.trace("<<<< s3eSoundPool SUSPEND");

            for (Map.Entry<Integer, Sample> entry : m_Samples.entrySet())
            {
                Sample sample = entry.getValue();
                sample.suspendEvent();
            }
        }
        else if (e.eventType == SuspendResumeEvent.EventType.RESUME)
        {
            LoaderAPI.trace(">>>> s3eSoundPool RESUME");

            for (Map.Entry<Integer, Sample> entry : m_Samples.entrySet())
            {
                Sample sample = entry.getValue();
                sample.resumeEvent();
            }
        }
        else if (e.eventType == SuspendResumeEvent.EventType.SHUTDOWN)
        {
            // Should have terminated by now
            LoaderAPI.trace("!!!! s3eSoundPool SHUTDOWN");
        }
    }

    private synchronized void onPlayComplete(int sampleId)
    {
        LoaderAPI.trace("**** onPlayComplete sampleId: " + sampleId);

        SampleEnded(sampleId);
    }

    private static float convertVolume(int fixedPointVolume)
    {
        return (float) fixedPointVolume / MAX_VOLUME;
    }

    private float calculateVolume(int sampleVolume)
    {
        return convertVolume(m_MasterVolume) * convertVolume(sampleVolume);
    }

    private int                 m_NextSampleId;
    private int                 m_MasterVolume;
    HashMap<Integer, Sample>    m_Samples = new HashMap<Integer, Sample>();
}
