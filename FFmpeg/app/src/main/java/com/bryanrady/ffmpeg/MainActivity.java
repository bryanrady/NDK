package com.bryanrady.ffmpeg;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity implements View.OnClickListener, DNPlayer.OnPreparedListener {

    private SurfaceView mSurfaceView;
    private Button mBtnStart;
  //  private Button mBtnStop;

    private DNPlayer mDnPlayer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mSurfaceView = findViewById(R.id.surfaceView);
        mBtnStart = findViewById(R.id.btn_start);
    //    mBtnStop = findViewById(R.id.btn_stop);
        mBtnStart.setOnClickListener(this);
    //    mBtnStop.setOnClickListener(this);

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
    //    mDnPlayer.setDataSource("rtmp://192.168.1.100:1935/live/wqb");
        mDnPlayer.setDataSource("rtmp://58.200.131.2:1935/livetv/hunantv");
        mDnPlayer.setSurfaceView(mSurfaceView);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()){
            case R.id.btn_start:
                mDnPlayer.prepare();
                break;
//            case R.id.btn_stop:
//                break;
        }
    }

    @Override
    public void onPrepared() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(MainActivity.this,"准备工作成功!",Toast.LENGTH_SHORT).show();
            }
        });
        mDnPlayer.startPlay();
    }

    @Override
    public void onError(int errorCode) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(MainActivity.this,"准备工作失败!",Toast.LENGTH_SHORT).show();
            }
        });
    }
}
