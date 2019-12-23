package com.bryanrady.douyin.filter;

import android.content.Context;

import com.bryanrady.douyin.R;

/**
 * 负责将摄像头的数据渲染到屏幕上
 */
public class ScreenFilter extends AbstractFilter {

    /**
     * 使用自己的构造函数 然后传值调用父类的构造函数
     * @param context
     */
    public ScreenFilter(Context context) {
        super(context, R.raw.base_vertex_shader, R.raw.base_fragment_shader);
    }
}
