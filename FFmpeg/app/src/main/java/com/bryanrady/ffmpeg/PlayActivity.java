package com.bryanrady.ffmpeg;

import android.content.res.Configuration;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.Toast;

public class PlayActivity extends AppCompatActivity implements DNPlayer.OnPreparedListener,
        DNPlayer.OnErrorListener,DNPlayer.OnProgressListener,SeekBar.OnSeekBarChangeListener {

    private SurfaceView mSurfaceView;
    private SeekBar mSeekBar;
    private DNPlayer mDnPlayer;

    private int mProgress;
    private boolean mIsTouch;
    private boolean mIsSeek;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_play);
        mSurfaceView = findViewById(R.id.surfaceView);
        mSeekBar = findViewById(R.id.seekBar);
        mSeekBar.setOnSeekBarChangeListener(this);

//        rtmp测试地址（可用）
//        香港卫视: rtmp://live.hkstv.hk.lxdns.com/live/hks
//
//        香港财经 rtmp://202.69.69.180:443/webcast/bshdlive-pc
//
//        韩国GoodTV,rtmp://mobliestream.c3tv.com:554/live/goodtv.sdp
//
//        韩国朝鲜日报,rtmp://live.chosun.gscdn.com/live/tvchosun1.stream
//
//        美国1,rtmp://ns8.indexforce.com/home/mystream
//
//        美国2,rtmp://media3.scctv.net/live/scctv_800
//
//        美国中文电视,rtmp://media3.sinovision.net:1935/live/livestream
//
//        湖南卫视 rtmp://58.200.131.2:1935/livetv/hunantv

        mDnPlayer = new DNPlayer();
        mDnPlayer.setOnPreparedListener(this);
        mDnPlayer.setOnErrorListener(this);
        mDnPlayer.setOnProgressListener(this);
    //    mDnPlayer.setDataSource("rtmp://192.168.1.100:1935/live/wqb");
    //    mDnPlayer.setDataSource("rtmp://58.200.131.2:1935/livetv/hunantv");
        mDnPlayer.setDataSource("/sdcard/aa1.mp4");
        mDnPlayer.setSurfaceView(mSurfaceView);
    }

    @Override
    public void onPrepared() {
        Log.e("wangqingbin","准备工作成功，可以进行播放了!");
        //获得时间 如果是直播： 时间就是0，点播的话可以显示进度条
        int duration = mDnPlayer.getDuration();
        if (duration != 0){
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    //显示进度条
                    mSeekBar.setVisibility(View.VISIBLE);
                }
            });
        }
        mDnPlayer.startPlay();
    }

    @Override
    public void onError(final int errorCode) {
        Log.e("wangqingbin","准备工作失败 错误码："+errorCode);
        runOnUiThread(new Runnable() {

            @Override
            public void run() {
                mDnPlayer.stopPlay();
                Toast.makeText(PlayActivity.this,"准备工作失败 错误码："+errorCode,Toast.LENGTH_SHORT).show();
            }
        });
    }

    @Override
    public void onProgress(final int progress) {
        if (!mIsTouch) {
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    //如果是直播 duration就是0
                    int duration = mDnPlayer.getDuration();
                    if (duration != 0) {
                        if (mIsSeek){
                            mIsSeek = false;
                            return;
                        }
                        //更新进度 计算比例
                        mSeekBar.setProgress(progress * 100 / duration);
                    }
                }
            });
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        mDnPlayer.prepare();
    }

    @Override
    protected void onStop() {
        super.onStop();
        mDnPlayer.stopPlay();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mDnPlayer.release();
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        } else {
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
        setContentView(R.layout.activity_play);
        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        mSeekBar = findViewById(R.id.seekBar);
        mSeekBar.setOnSeekBarChangeListener(this);
        mSeekBar.setProgress(mProgress);
        mDnPlayer.setSurfaceView(surfaceView);
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }

    /**
     * 开始拖动
     * @param seekBar
     */
    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        mIsTouch = true;
    }

    /**
     * 停止拖动
     * @param seekBar
     */
    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        mIsSeek = true;
        mIsTouch = false;
        mProgress = mDnPlayer.getDuration() * seekBar.getProgress() / 100;
        //进度调整
        mDnPlayer.seek(mProgress);
    }

}
