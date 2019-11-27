package com.bryanrady.andfix;

import android.content.Context;
import android.text.TextUtils;
import android.widget.Toast;

import java.io.File;
import java.lang.reflect.Method;
import java.util.Enumeration;

import dalvik.system.DexFile;

/**
 * @author: wangqingbin
 * @date: 2019/11/26 14:49
 */
public class DexManager {

    static {
        System.loadLibrary("native-lib");
    }

    /**
     * 加载dex文件找到修复类
     * @param context
     * @param file
     */
    public void loadDex(Context context,File file){
        try {
            DexFile dexFile = DexFile.loadDex(
                    file.getAbsolutePath(),
                    new File(context.getCacheDir(),"opt").getAbsolutePath(),
                    Context.MODE_PRIVATE);
            //entries dex里面有多少个类
            Enumeration<String> entries = dexFile.entries();
            //遍历
            while (entries.hasMoreElements()){
                //得到的是一个全类名
                String className = entries.nextElement();
                //这里不能通过反射去拿这个修复类啊，因为都还没加载到虚拟机中来
                //得到dex文件中的修复类
                Class rightClass = dexFile.loadClass(className, context.getClassLoader());
                if(rightClass != null){
                    findWrongClass(context,rightClass);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * 通过修复类找到错误的类和方法
     * @param rightClass
     */
    private void findWrongClass(Context context,Class rightClass) {
        Method[] methods = rightClass.getDeclaredMethods();
        for (Method rightMethod : methods){
            Replace replace = rightMethod.getAnnotation(Replace.class);
            if(replace == null){
                //这里循环找到修复类的修复方法，一个个找，直到找到为止
                continue;
            }
            //全类名和方法名
            String wrongClassName = replace.wrongClassName();
            String wrongMethodName = replace.wrongMethodName();
            try {
                if(!TextUtils.isEmpty(wrongClassName) && !TextUtils.isEmpty(wrongMethodName)){
                    //这里可以通过名字反射拿到出错的类和方法
                    Class wrongClass = Class.forName(wrongClassName);
                    //第2个参数：获取错误方法的参数类型，通过正确方法来获取
                    Method wrongMethod = wrongClass.getDeclaredMethod(wrongMethodName,rightMethod.getParameterTypes());
                    replace(wrongMethod, rightMethod);
                    Toast.makeText(context,"加载dex，替换方法完成",Toast.LENGTH_SHORT).show();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * 替换方法（方法级的替换 ）
     * @param wrongMethod
     * @param rightMethod
     */
    private native void replace(Method wrongMethod,Method rightMethod);

}
