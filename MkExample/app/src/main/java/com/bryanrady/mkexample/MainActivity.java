package com.bryanrady.mkexample;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

public class MainActivity extends AppCompatActivity {

    static {
        //在4.4上 如果load一个动态库 ，需要先将这个动态库的依赖的其他动态库load进来,因为 native-lib 依赖 了 Test库

        //从6.0开始 使用Android.mk 如果来引入一个预编译动态库 有问题
        //在6.0以下  System.loadLibrary 不会自动为我们加载依赖的动态库
        // 6.0以上  System.loadLibrary 会自动为我们加载依赖的动态库

        //这句话只能加载动态库 静态库加载不了
        //System.loadLibrary("Test");

        //目前不知道为什么会报错,jni方法感觉是对的，没什么错误，但是提示找不到,按道理说预编译静态库的话 这样就可以了
        //java.lang.UnsatisfiedLinkError: No implementation found for void com.bryanrady.mkexample.MainActivity.nativeTest() (tried Java_com_bryanrady_mkexample_MainActivity_nativeTest
        // and Java_com_bryanrady_mkexample_MainActivity_nativeTest__)
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        nativeTest();
    }

    private native void nativeTest();

}
