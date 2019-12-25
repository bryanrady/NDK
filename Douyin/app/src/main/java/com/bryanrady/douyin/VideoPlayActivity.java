package com.bryanrady.douyin;

import android.os.Bundle;
import android.text.TextUtils;

import com.bryanrady.douyin.widget.VideoPlayView;

import androidx.appcompat.app.AppCompatActivity;

public class VideoPlayActivity extends AppCompatActivity {

    VideoPlayView mVideoPlayView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_video_play);
        String path = getIntent().getStringExtra("path");
        if (TextUtils.isEmpty(path)){
            finish();
        }

        mVideoPlayView = findViewById(R.id.videoPlayView);
        mVideoPlayView.setDataSource(path);
        mVideoPlayView.startPlay();
    }

}
