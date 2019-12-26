package com.bryanrady.douyin.play;

public interface ISurface {

    /**
     * 把数据加入队列
     * @param data
     */
    void offer(byte[] data);

    /**
     * 从队列中取出数据
     * @return
     */
    byte[] poll();

    /**
     * 把视频的宽 高 码率 帧率 回调出去
     * @param width
     * @param height
     * @param bitrate
     * @param fps
     */
    void getVideoParameters(int width, int height, int bitrate, int fps);

}
