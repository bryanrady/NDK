package com.bryanrady.andfix.fix;

import android.content.Context;
import android.widget.Toast;

import com.bryanrady.andfix.fix.anotation.Replace;

/**
 * 修复类
 * @author: wangqingbin
 * @date: 2019/11/26 14:54
 */
public class Caculator {

    private Context mContext;

    public Caculator(Context context){
        mContext = context;
    }

    @Replace(wrongClassName = "com.bryanrady.andfix.Caculator",wrongMethodName = "caculat")
    public void caculat(){
        Toast.makeText(mContext,"修复好了",Toast.LENGTH_SHORT).show();
    }
}
