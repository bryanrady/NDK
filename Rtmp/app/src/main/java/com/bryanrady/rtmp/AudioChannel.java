package com.bryanrady.rtmp;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioChannel {

    private int mInputSamples;
    private ExecutorService mExecutor;
    private AudioRecord mAudioRecord;
    private LivePusher mLivePusher;
    private int mChannels = 1;
    private final int SAMELE_RATE_44100 = 44100;
    private boolean mIsLiving;

    public AudioChannel(LivePusher livePusher) {
        mLivePusher = livePusher;
        mExecutor = Executors.newSingleThreadExecutor();
        //准备录音机 采集pcm 数据
        int channelConfig;
        if (mChannels == 2) {
            channelConfig = AudioFormat.CHANNEL_IN_STEREO;
        } else {
            channelConfig = AudioFormat.CHANNEL_IN_MONO;
        }

        mLivePusher.native_setAudioEncInfo(SAMELE_RATE_44100, mChannels);
        //16 位 2个字节
        mInputSamples = mLivePusher.getInputSamples() * 2;

        //最小需要的缓冲区
        int minBufferSize = AudioRecord.getMinBufferSize(SAMELE_RATE_44100, channelConfig, AudioFormat.ENCODING_PCM_16BIT) * 2;
        //1、麦克风 2、采样率 3、声道数 4、采样位
        mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, SAMELE_RATE_44100, channelConfig, AudioFormat.ENCODING_PCM_16BIT, minBufferSize > mInputSamples ? minBufferSize : mInputSamples);
    }


    public void startLive() {
        mIsLiving = true;
        mExecutor.submit(new AudioTask());
    }

    public void stopLive() {
        mIsLiving = false;
    }


    public void release() {
        mAudioRecord.release();
    }

    class AudioTask implements Runnable {

        @Override
        public void run() {
            //启动录音机
            mAudioRecord.startRecording();
            byte[] bytes = new byte[mInputSamples];
            while (mIsLiving) {
                int len = mAudioRecord.read(bytes, 0, bytes.length);
                if (len > 0) {
                    //送去编码
                    mLivePusher.native_pushAudio(bytes);
                }
            }
            //停止录音机
            mAudioRecord.stop();
        }
    }

}
