package com.bryanrady.andfix;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.View;
import android.widget.TextView;

import java.io.File;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {//如果 API level 是大于等于 23(Android 6.0) 时
            //判断是否具有权限
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE) != PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                        new String[]{
                                Manifest.permission.WRITE_EXTERNAL_STORAGE,
                                Manifest.permission.READ_EXTERNAL_STORAGE},
                        0);
            }
        }

    }

    public void test(View view) {
        Caculator caculator = new Caculator(this);
        caculator.caculat();
    }

    /**
     * 这里只是模拟修复，正常的流程是在Application里面进行修复，而不是手动点击按钮进行修复，这里为了看效果才这样做
     * @param view
     */
    public void fix(View view) {
        DexManager dexManager = new DexManager();
        dexManager.loadDex(this,new File("/sdcard/out.dex"));
    }
}
