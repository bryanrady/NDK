package com.bryanrady.rtmp;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.hardware.Camera;
import android.os.Build;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.TextView;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class MainActivity extends AppCompatActivity {

    private LivePusher mLivePusher;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {//如果 API level 是大于等于 23(Android 6.0) 时
            //判断是否具有权限
            if (ContextCompat.checkSelfPermission(this,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE) != PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA,Manifest.permission.RECORD_AUDIO},
                        0);
            }
        }

        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        mLivePusher = new LivePusher(this, 800, 480, 800_000, 10, Camera.CameraInfo.CAMERA_FACING_BACK);
        //设置摄像头预览的界面
        mLivePusher.setPreviewDisplay(surfaceView.getHolder());
    }

    public void switchCamera(View view) {
        mLivePusher.switchCamera();
    }

    public void startLive(View view) {
        mLivePusher.startLive("rtmp://192.168.1.103:1935/live/wqb");
    }

    public void stopLive(View view) {
        mLivePusher.stopLive();
    }

}
