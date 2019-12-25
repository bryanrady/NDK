package com.bryanrady.douyin;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.os.Build;
import android.os.Bundle;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RadioGroup;

import com.bryanrady.douyin.widget.DouYinView;
import com.bryanrady.douyin.widget.RecordButton;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class MainActivity extends AppCompatActivity {

    DouYinView mDouYinView;
    RecordButton mRecordButton;
    RadioGroup mRadioGroup;
    CheckBox mBigEye;
    CheckBox mSticker;
    CheckBox mBeauty;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mDouYinView = findViewById(R.id.douYinView);
        mRecordButton = findViewById(R.id.btn_record);
        mRadioGroup = findViewById(R.id.rg_speed);
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

        mRecordButton.setOnRecordListener(new RecordButton.OnRecordListener() {
            @Override
            public void onRecordStart() {
                mDouYinView.startRecord();
            }

            @Override
            public void onRecordStop() {
                mDouYinView.stopRecord();
            }
        });

        mRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                switch (checkedId){
                    case R.id.rb_extra_slow: //极慢
                        mDouYinView.setSpeed(DouYinView.Speed.MODE_EXTRA_SLOW);
                        break;
                    case R.id.rb_slow:      //慢
                        mDouYinView.setSpeed(DouYinView.Speed.MODE_SLOW);
                        break;
                    case R.id.rb_normal:    //标准
                        mDouYinView.setSpeed(DouYinView.Speed.MODE_NORMAL);
                        break;
                    case R.id.rb_fast:      //快
                        mDouYinView.setSpeed(DouYinView.Speed.MODE_FAST);
                        break;
                    case R.id.rb_extra_fast: //极快
                        mDouYinView.setSpeed(DouYinView.Speed.MODE_EXTRA_FAST);
                        break;
                }
            }
        });

        mBigEye.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mDouYinView.enableBigEye(isChecked);
            }
        });

        mSticker.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mDouYinView.enableSticker(isChecked);
            }
        });

        mBeauty.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mDouYinView.enableBeauty(isChecked);
            }
        });
    }

}
