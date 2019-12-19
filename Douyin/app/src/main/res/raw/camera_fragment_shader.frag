//片元着色器 处理光，阴影，遮挡，环境等对物体表面的影响，最终生成一副图像的小程序

//SurfaceTexture比较特殊 这里因为使用的是SurfaceTexture 所以要加这一句 其他的情况可以不用加
#extension GL_OES_EGL_image_external : require

//指定float数据是什么精度（高 中 低 ）   mediump 中等
precision mediump float;

//采样点坐标的像素值
varying vec2 aCoord;

//采样器 纹理坐标
uniform samplerExternalOES vTexture;

void main(){
    //内置变量 接收像素值
    // texture2D：内置函数 通过采样器vTexture来采集aCoord这个位置的像素 然后赋值给gl_FragColor就可以了
    gl_FragColor = texture2D(vTexture,aCoord);
}

