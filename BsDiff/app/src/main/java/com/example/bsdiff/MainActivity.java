package com.example.bsdiff;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.content.FileProvider;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import java.io.File;

import static android.content.pm.PackageManager.PERMISSION_GRANTED;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        TextView version = findViewById(R.id.version);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {//如果 API level 是大于等于 23(Android 6.0) 时
            //判断是否具有权限
            if (ContextCompat.checkSelfPermission(this,
                    Manifest.permission.READ_EXTERNAL_STORAGE) != PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE,Manifest.permission.WRITE_EXTERNAL_STORAGE},
                        0);
            }
        }

        version.setText(BuildConfig.VERSION_NAME);
    }

    @SuppressLint("StaticFieldLeak")
    public void update(View view) {
        //1.从服务器上下载差分包，然后进行合成
        new AsyncTask<Void, Void, File>() {
            @Override
            protected void onPreExecute() {
                super.onPreExecute();
            }

            @Override
            protected File doInBackground(Void... voids) {
                String old = getApplication().getApplicationInfo().sourceDir;
                native_bspatch(old,"/sdcard/patch.diff","/sdcard/new.apk");
                return new File("/sdcard/new.apk");
            }

            @Override
            protected void onPostExecute(File file) {
                super.onPostExecute(file);
                //在这里进行安装
                Intent intent = new Intent(Intent.ACTION_VIEW);
                if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N) {
                    intent.setDataAndType(Uri.fromFile(file), "application/vnd.android.package-archive");
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                } else {
                    // 声明需要的临时权限
                    intent.setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                    // 第二个参数，即第一步中配置的authorities
                    String packageName = getApplication().getPackageName();
                    Uri contentUri = FileProvider.getUriForFile(MainActivity.this, packageName + ".fileProvider", file);
                    intent.setDataAndType(contentUri, "application/vnd.android.package-archive");
                }
                startActivity(intent);
            }

            @Override
            protected void onProgressUpdate(Void... values) {
                super.onProgressUpdate(values);
            }

            @Override
            protected void onCancelled() {
                super.onCancelled();
            }
        }.execute();
    }

    /**
     * 用来合成apk
     * @param oldApk 当前运行的apk
     * @param patch  差分包
     * @param output 合成后的新的apk输出到
     */
    native void native_bspatch(String oldApk,String patch,String output);
}
