package com.bryanrady.douyin.filter;

import android.content.Context;

import com.bryanrady.douyin.R;

/**
 * 往虚拟屏幕上绘制图像   和往手机屏幕上绘制是一样的
 * @author: wangqingbin
 * @date: 2019/12/24 15:31
 */
public class VirtualFilter extends AbstractFilter {

    public VirtualFilter(Context context) {
        super(context.getApplicationContext(), R.raw.base_vertex_shader, R.raw.base_fragment_shader);
    }

}
