//GLSL OpenGL着色器语言 这里安装了GLSL Support 插件  高亮
//着色器文件的后缀名是不讲究的，随便都行，但是为了匹配 GLSL Support 插件，就定义成vert或者frag或者glsl

//数据类型
//      float       浮点型

//      vec2        包含2个浮点型数据的向量

//      vec4        包含4个浮点型数据的向量   (xyzw, rgba, stpq)

//      sampler2D   2D纹理采样器(代表一层纹理)

//修饰符
//      attribute   属性变量    只能用于顶点着色器中，一般该变量用来表示一些顶点数据。如顶点坐标 纹理标配 颜色等

//      uniforms    一致变量(常量)    在着色器执行期间一致变量的值是不变的。与c++ const变量不同的是，这个值在编译时期是未知的，它是由着色器外部初始化的

//      varying     易变变量    它是从顶点着色器的变量传递给片元着色器的变量


//顶点着色器     处理顶点,法线等数据的小程序

//接收顶点坐标 确定要画画的形状
attribute vec4 vPosition;

//接收纹理坐标，接收采样器采样图片的坐标
attribute vec4 vCoord;

//变换矩阵， 需要将原本的vCoord（01,11,00,10） 与变换矩阵相乘 才能够得到 SurfaceTexture(特殊)的正确的采样坐标
uniform mat4 vMatrix;

//传给片元着色器采样点坐标的像素值
varying vec2 aCoord;

void main(){
    //gl_Position是一个内置变量 我们把顶点坐标数据的变量赋值给这个变量，OpenGL就知道它要画什么形状了
    gl_Position = vPosition;
    //取出vec4的前两个数据（采样点坐标的像素值）  这个aCoord最终会传递给片元着色器  这两种写法都可以
    aCoord = (vMatrix * vCoord).xy;
    //aCoord =  vec2((vMatrix*vCoord).x,(vMatrix*vCoord).y);
}

