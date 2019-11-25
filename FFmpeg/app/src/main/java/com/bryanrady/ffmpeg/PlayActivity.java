package com.bryanrady.ffmpeg;

import android.content.Intent;
import android.content.res.Configuration;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Toast;

public class PlayActivity extends AppCompatActivity implements DNPlayer.OnPreparedListener, DNPlayer.OnErrorListener,
        DNPlayer.OnProgressListener,SeekBar.OnSeekBarChangeListener,View.OnClickListener {

    private SurfaceView mSurfaceView;
    private SeekBar mSeekBar;
    private Button mBtnPause;
    private Button mBtnJump;
    private DNPlayer mDnPlayer;

    private boolean mIsTouch;
    private boolean mIsSeek;
    private boolean mIsPause = false;

    private int mCurSeekProgress = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.e("wangqingbin","onCreate()............");
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_play);
        mSurfaceView = findViewById(R.id.surfaceView);
        mSeekBar = findViewById(R.id.seekBar);
        mSeekBar.setOnSeekBarChangeListener(this);
        mBtnPause = findViewById(R.id.btn_pause_play);
        mBtnPause.setOnClickListener(this);
        mBtnJump = findViewById(R.id.btn_jump_other);
        mBtnJump.setOnClickListener(this);

        mDnPlayer = new DNPlayer();
        mDnPlayer.setOnPreparedListener(this);
        mDnPlayer.setOnErrorListener(this);
        mDnPlayer.setOnProgressListener(this);
        mDnPlayer.setSurfaceView(mSurfaceView);
        mDnPlayer.setDataSource("rtmp://192.168.232.129:1935/myapp/123");
        //mDnPlayer.setDataSource("rtmp://58.200.131.2:1935/livetv/hunantv");
        //mDnPlayer.setDataSource("/sdcard/aa5.mp4");

        if(mIsPause){
            mBtnPause.setText("播放");
        }else{
            mBtnPause.setText("暂停");
        }

        mDnPlayer.prepare();

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
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.btn_pause_play:
                mIsPause = !mIsPause;
                if(mIsPause){
                    mDnPlayer.pause();
                    mBtnPause.setText("播放");
                }else{
                    mDnPlayer.pause();
                    mBtnPause.setText("暂停");
                }
                break;
            case R.id.btn_jump_other:
                Intent intent = new Intent(PlayActivity.this, OtherActivity.class);
                startActivity(intent);
                break;
        }
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
                        mCurSeekProgress = progress * 100 / duration;
                        mSeekBar.setProgress(mCurSeekProgress);
                    }
                }
            });
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.e("wangqingbin","onStart()............");
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        mDnPlayer.pause();
        Log.e("wangqingbin","onRestart()............");
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.e("wangqingbin","onResume()............");
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.e("wangqingbin","onPause()............");
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.e("wangqingbin","onStop()............");
        mDnPlayer.pause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.e("wangqingbin","onDestroy()............");
        mDnPlayer.stopPlay();
        mDnPlayer.release();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        Log.e("wangqingbin","onSaveInstanceState()............");
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        Log.e("wangqingbin","onRestoreInstanceState()............");
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.e("wangqingbin","onConfigurationChanged()............");
        setContentView(R.layout.activity_play);
        if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
            getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        } else {
            getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
            mBtnJump = findViewById(R.id.btn_jump_other);
            mBtnJump.setOnClickListener(this);
        }
        mSurfaceView = findViewById(R.id.surfaceView);
        mSeekBar = findViewById(R.id.seekBar);
        mSeekBar.setVisibility(View.VISIBLE);
        mSeekBar.setOnSeekBarChangeListener(this);
        Log.e("wangqingbin","mCurSeekProgress =="+mCurSeekProgress);
        mSeekBar.setProgress(mCurSeekProgress);
        mBtnPause = findViewById(R.id.btn_pause_play);
        mBtnPause.setOnClickListener(this);
        mDnPlayer.setSurfaceView(mSurfaceView);
        if(mIsPause){
            mBtnPause.setText("播放");
        }else{
            mBtnPause.setText("暂停");
        }
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
        int progress = mDnPlayer.getDuration() * seekBar.getProgress() / 100;
        //进度调整
        mDnPlayer.seek(progress);
    }

}
