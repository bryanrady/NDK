package com.bryanrady.gifload;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.annotation.SuppressLint;
import android.graphics.Bitmap;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.File;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class MainActivity extends AppCompatActivity {

    private ImageView mImageView;
    private Bitmap mBitmap;
    private GifHandler mGifHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mImageView = findViewById(R.id.imageView);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {//如果 API level 是大于等于 23(Android 6.0) 时
            //判断是否具有权限
            if (ContextCompat.checkSelfPermission(this,
                    Manifest.permission.READ_EXTERNAL_STORAGE) != PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,Manifest.permission.WRITE_EXTERNAL_STORAGE},
                        0);
            }
        }

        ndkLoadGif();

    }

    public void ndkLoadGif() {
        File file = new File(Environment.getExternalStorageDirectory(),"demo2.gif");
        if(file.exists()){
            mGifHandler = new GifHandler(file.getAbsolutePath());
            mBitmap = Bitmap.createBitmap(mGifHandler.getWidth(),mGifHandler.getHeight(), Bitmap.Config.ARGB_8888);
            int delay = mGifHandler.updateFrame(mBitmap);
            handler.sendEmptyMessageDelayed(1,delay);
        }
    }

    @SuppressLint("HandlerLeak")
    private Handler handler = new Handler(){
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what){
                case 1:
                    int delay = mGifHandler.updateFrame(mBitmap);
                    mImageView.setImageBitmap(mBitmap);
                    handler.sendEmptyMessageDelayed(1,delay);
                    break;
            }
        }
    };


    @Override
    protected void onDestroy() {
        super.onDestroy();
        mGifHandler.release();
    }
}
