package com.bryanrady.andfix;

import android.content.Context;

/**
 * 报错的类
 * @author: wangqingbin
 * @date: 2019/11/26 14:41
 */
public class Caculator {

    private Context mContext;

    public Caculator(Context context){
        mContext = context;
    }

    public void caculat() {
        throw new RuntimeException("出异常了");
    }

}
