
precision mediump float;

varying vec2 aCoord;

//再java中得到的是一个byte[],我们要从byte[]数组中分离出 y u v 数据
//将y u v数据分别传进来
uniform sampler2D sampler_y;
uniform sampler2D sampler_u;
uniform sampler2D sampler_v;

//透明度 由外面传进来
uniform float alpha;

void main(){
    //4个float数据 y、u、v保存在向量中的第一个
    vec4 y = texture2D(sampler_y,aCoord);
    vec4 u = texture2D(sampler_u,aCoord);
    vec4 v = texture2D(sampler_v,aCoord);

    // yuv转rgb的公式
    //R = Y + 1.402 (v-128)
    //G = Y - 0.34414 (u - 128) - 0.71414 (v-128)
    //B = Y + 1.772 (u- 128)

    //注意：
    //1、glsl中 不能直接将int与float进行计算    fv - 128 这样写不行 要 写成 fv - 128.0
    //2、rgba取值都是：0-1 （128是0-255 转成为0-1 128就是0.5）所以 v-128 == v - 0.5
    //取出Y 就是 y.r 或者 y.x

    float fy = y.r;
    float fu = u.r - 0.5;
    float fv = v.r - 0.5;

    vec3 rgb;
    rgb.r = fy + 1.402 * fv;
    rgb.g = fy - 0.34414 * fu - 0.71414 * fv;
    rgb.b = fy + 1.772 * fu;

    //rgba
    gl_FragColor = vec4(rgb,alpha);
}