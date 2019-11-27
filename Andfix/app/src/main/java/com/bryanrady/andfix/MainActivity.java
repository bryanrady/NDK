package com.bryanrady.andfix;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatViewInflater;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import android.Manifest;
import android.os.Build;
import android.os.Bundle;
import android.os.Looper;
import android.view.View;
import android.widget.TextView;

import java.io.File;
import java.lang.annotation.Native;

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

//    java.lang.IllegalStateException: Could not execute method for android:onClick
//    at androidx.appcompat.app.AppCompatViewInflater$DeclaredOnClickListener.onClick(AppCompatViewInflater.java:390)
//    at android.view.View.performClick(View.java:5716)
//    at android.widget.TextView.performClick(TextView.java:10926)
//    at android.view.View$PerformClick.run(View.java:22596)
//    at android.os.Handler.handleCallback(Handler.java:739)
//    at android.os.Handler.dispatchMessage(Handler.java:95)
//    at android.os.Looper.loop(Looper.java:148)
//    at android.app.ActivityThread.main(ActivityThread.java:7329)
//    at java.lang.reflect.Method.invoke(Native Method)
//    at com.android.internal.os.ZygoteInit$MethodAndArgsCaller.run(ZygoteInit.java:1230)
//    at com.android.internal.os.ZygoteInit.main(ZygoteInit.java:1120)
//    Caused by: java.lang.reflect.InvocationTargetException
//    at java.lang.reflect.Method.invoke(Native Method)
//    at androidx.appcompat.app.AppCompatViewInflater$DeclaredOnClickListener.onClick(AppCompatViewInflater.java:385)
//    at android.view.View.performClick(View.java:5716) 
//    at android.widget.TextView.performClick(TextView.java:10926) 
//    at android.view.View$PerformClick.run(View.java:22596) 
//    at android.os.Handler.handleCallback(Handler.java:739) 
//    at android.os.Handler.dispatchMessage(Handler.java:95) 
//    at android.os.Looper.loop(Looper.java:148) 
//    at android.app.ActivityThread.main(ActivityThread.java:7329) 
//    at java.lang.reflect.Method.invoke(Native Method) 
//    at com.android.internal.os.ZygoteInit$MethodAndArgsCaller.run(ZygoteInit.java:1230) 
//    at com.android.internal.os.ZygoteInit.main(ZygoteInit.java:1120) 


//    Caused by: java.lang.ArrayIndexOutOfBoundsException: length=8; index=519
//    at android.accessibilityservice.AccessibilityServiceInfo.loadDescription(Caculator.java:6)
//    at com.bryanrady.andfix.MainActivity.test(MainActivity.java:38)


//    at java.lang.reflect.Method.invoke(
//    Native Method) 
//    at androidx.appcompat.app.AppCompatViewInflater$DeclaredOnClickListener.onClick(AppCompatViewInflater.java:385) 
//    at android.view.View.performClick(View.java:5716) 
//    at android.widget.TextView.performClick(TextView.java:10926) 
//    at android.view.View$PerformClick.run(View.java:22596) 
//    at android.os.Handler.handleCallback(Handler.java:739) 
//    at android.os.Handler.dispatchMessage(Handler.java:95) 
//    at android.os.Looper.loop(Looper.java:148) 
//    at android.app.ActivityThread.main(ActivityThread.java:7329) 
//    at java.lang.reflect.Method.invoke(Native Method) 
//    at com.android.internal.os.ZygoteInit$MethodAndArgsCaller.run(ZygoteInit.java:1230) 
//    at com.android.internal.os.ZygoteInit.main(ZygoteInit.java:1120) 

    //修复后一直报错，不知是兼容问题还是什么问题?
    public void test(View view) {
        Caculator caculator = new Caculator();
        caculator.caculat(this);
    }

    /**
     * 这里只是模拟修复，正常的流程是在Application里面进行修复，而不是手动点击按钮进行修复，这里为了看效果才这样做
     * @param view
     */
    public void fix(View view) {
        DexManager dexManager = new DexManager();
        dexManager.loadDex(this, new File("/sdcard/out3.dex"));
    }
}
