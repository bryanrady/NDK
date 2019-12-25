//画肉体 base_vertex就可以了
//但是还要画灵魂 我们需要个矩阵

attribute vec4 vPosition;

attribute vec2 vCoord;

uniform mat4 vMatrix;

varying vec2 aCoord;


void main(){
    gl_Position = vMatrix * vPosition;

    aCoord =  vCoord;
}