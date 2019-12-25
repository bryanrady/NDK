package com.bryanrady.douyin;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import com.bryanrady.douyin.record.MediaRecorder;
import com.bryanrady.douyin.widget.VideoRecordView;
import com.bryanrady.douyin.widget.VideoRecordButton;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class VideoRecordActivity extends AppCompatActivity {

    VideoRecordView mVideoRecordView;
    VideoRecordButton mVideoRecordButton;
    RadioGroup mRadioGroup;
    TextView mSwitchCamera;
    CheckBox mBigEye;
    CheckBox mSticker;
    CheckBox mBeauty;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera_preview);
        mVideoRecordView = findViewById(R.id.videoRecordView);
        mVideoRecordButton = findViewById(R.id.btn_record);
        mRadioGroup = findViewById(R.id.rg_speed);
        mSwitchCamera = findViewById(R.id.switch_camera);
        mBigEye = findViewById(R.id.big_eye);
        mSticker = findViewById(R.id.sticker);
        mBeauty = findViewById(R.id.beauty);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {//如果 API level 是大于等于 23(Android 6.0) 时
            //判断是否具有权限
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) != PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA
                                , Manifest.permission.WRITE_EXTERNAL_STORAGE
                                , Manifest.permission.READ_EXTERNAL_STORAGE},
                        0);
            }
        }

        mVideoRecordView.setOnRecordFinishedListener(new MediaRecorder.OnRecordFinishedListener() {
            @Override
            public void onRecordFinished(final String path) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        Intent intent = new Intent(VideoRecordActivity.this, VideoPlayActivity.class);
                        intent.putExtra("path", path);
                        startActivity(intent);
                    }
                });
            }
        });

        mVideoRecordButton.setOnRecordListener(new VideoRecordButton.OnRecordListener() {
            @Override
            public void onRecordStart() {
                mVideoRecordView.startRecord();
            }

            @Override
            public void onRecordStop() {
                mVideoRecordView.stopRecord();
            }
        });

        mRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                switch (checkedId){
                    case R.id.rb_extra_slow: //极慢
                        mVideoRecordView.setSpeed(VideoRecordView.Speed.MODE_EXTRA_SLOW);
                        break;
                    case R.id.rb_slow:      //慢
                        mVideoRecordView.setSpeed(VideoRecordView.Speed.MODE_SLOW);
                        break;
                    case R.id.rb_normal:    //标准
                        mVideoRecordView.setSpeed(VideoRecordView.Speed.MODE_NORMAL);
                        break;
                    case R.id.rb_fast:      //快
                        mVideoRecordView.setSpeed(VideoRecordView.Speed.MODE_FAST);
                        break;
                    case R.id.rb_extra_fast: //极快
                        mVideoRecordView.setSpeed(VideoRecordView.Speed.MODE_EXTRA_FAST);
                        break;
                }
            }
        });

        mSwitchCamera.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mVideoRecordView.switchCamera();
            }
        });

        mBigEye.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mVideoRecordView.enableBigEye(isChecked);
            }
        });

        mSticker.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mVideoRecordView.enableSticker(isChecked);
            }
        });

        mBeauty.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mVideoRecordView.enableBeauty(isChecked);
            }
        });
    }

}
