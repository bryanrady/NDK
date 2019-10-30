package com.bryanrady.example;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import java.util.Arrays;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);

        //1.从jni中获取字符串
        tv.setText(stringFromJNI());

        //2.传递字符串给jni
        transferStringToJNI("我是java中的字符串!");

        //3.传递数组给jni
        int[] intArr = new int[]{11,22,33,44,55};
        String[] stringArr = new String[]{"lucy","jack","lance","david"};
        transferArrayToJni(intArr,stringArr);
        Log.e("wangqingbin","调用native方法后的java int数组:"+ Arrays.toString(intArr));

        //4.传递引用类型给jni
        Girl girl = new Girl("璐璐",21);
        transferObjectToJni(girl);
        Log.e("wangqingbin",girl.getName() + "----111---" + girl.getAge());

        //5.通过JNI创建Gril对象进行返回
        Girl girlFromJni = getGirlFromJni();
        if (girl != null && girlFromJni.getName() != null){
            Log.e("wangqingbin",girlFromJni.getName() + "---222----" + girlFromJni.getAge());
        }

        //6.测试Girl对象中的静态字段和静态方法
        testGirlStatic();

        //7.测试Girl中的内部类
        Girl.Inner inner = testGirlInner();
        if (inner != null && inner.getGrade() != 0){
            Log.e("wangqingbin",inner.getGrade() + "---333----" + inner.isGirl());
        }

        //8.测试局部应用
        testJniLocalRef();
    //    testJniLocalRef();

        //9.测试全局应用
        testJniGlobalRef();
        testJniGlobalRef();
        testJniGlobalRef();

        //10.测试弱全局应用
        testJniWeakGlobalRef();
    }

    /**
     * 从jni中获取字符串
     * @return
     */
    public native String stringFromJNI();

    /**
     * 传递字符串给jni
     * @param str
     */
    public native void transferStringToJNI(String str);

    /**
     * 传递数组给jni
     * @param intArr
     * @param stringArr
     */
    public native void transferArrayToJni(int[] intArr,String[] stringArr);

    /**
     * 传递引用类型给jni
     * @param girl
     */
    public native void transferObjectToJni(Girl girl);

    /**
     * 通过JNI创建Gril对象进行返回
     * @return
     */
    public native Girl getGirlFromJni();

    /**
     * 测试Girl对象中的静态字段和静态方法
     */
    public native void testGirlStatic();

    /**
     * 测试Girl中的内部类
     */
    public native Girl.Inner testGirlInner();

    /**
     * 测试Jni中局部引用
     */
    public native void testJniLocalRef();

    /**
     * 测试Jni中全局引用
     */
    public native void testJniGlobalRef();

    /**
     * 测试Jni中弱全局引用
     */
    public native void testJniWeakGlobalRef();


}
