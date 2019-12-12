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

    private ImageView mImageView1,mImageView2;
    private Bitmap mBitmap1,mBitmap2;
    private GifHandler mGifHandler1,mGifHandler2;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mImageView1 = findViewById(R.id.imageView1);
        mImageView2 = findViewById(R.id.imageView2);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {//如果 API level 是大于等于 23(Android 6.0) 时
            //判断是否具有权限
            if (ContextCompat.checkSelfPermission(this,
                    Manifest.permission.READ_EXTERNAL_STORAGE) != PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,Manifest.permission.WRITE_EXTERNAL_STORAGE},
                        0);
            }
        }

        ndkLoadGif1();
        ndkLoadGif2();
    }

    //https://blog.csdn.net/a756213932/article/details/101853198    android ndk使用giflib高性能显示gif
    public void ndkLoadGif1() {
        File file = new File(Environment.getExternalStorageDirectory(),"demo.gif");
        if(file.exists()){
            mGifHandler1 = new GifHandler(file.getAbsolutePath());
            mBitmap1 = Bitmap.createBitmap(mGifHandler1.getWidth(),mGifHandler1.getHeight(), Bitmap.Config.ARGB_8888);
            int delay = mGifHandler1.updateFrame(mBitmap1);
            handler.sendEmptyMessageDelayed(1,delay);
        }
    }

    public void ndkLoadGif2() {
        File file = new File(Environment.getExternalStorageDirectory(),"demo2.gif");
        if(file.exists()){
            mGifHandler2 = new GifHandler(file.getAbsolutePath());
            mBitmap2 = Bitmap.createBitmap(mGifHandler2.getWidth(),mGifHandler2.getHeight(), Bitmap.Config.ARGB_8888);
            int delay = mGifHandler2.updateFrame(mBitmap2);
            handler.sendEmptyMessageDelayed(2,delay);
        }
    }

    @SuppressLint("HandlerLeak")
    private Handler handler = new Handler(){
        @Override
        public void handleMessage(@NonNull Message msg) {
            switch (msg.what){
                case 1:
                    int delay = mGifHandler1.updateFrame(mBitmap1);
                    mImageView1.setImageBitmap(mBitmap1);
                    handler.sendEmptyMessageDelayed(1,delay);
                    break;
                case 2:
                    int delay2 = mGifHandler2.updateFrame(mBitmap2);
                    mImageView2.setImageBitmap(mBitmap2);
                    handler.sendEmptyMessageDelayed(2,delay2);
                    break;
            }
        }
    };


    @Override
    protected void onDestroy() {
        super.onDestroy();
        mGifHandler1.release();
        mGifHandler2.release();
    }
}
