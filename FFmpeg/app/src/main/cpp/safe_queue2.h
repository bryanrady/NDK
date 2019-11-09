//
// Created by Administrator on 2019/11/8.
//

#ifndef FFMPEG_SAFE_QUEUE2_H
#define FFMPEG_SAFE_QUEUE2_H

#include <queue>
#include <pthread.h>
using namespace std;

template <typename T>
class SafeQueue{
    typedef void (*ReleaseCallback)(T&);    //定义函数指针(传递一个引用,修改引用的值) 用于函数调用
public:
    SafeQueue(){
        pthread_mutex_init(&mutex,0);
        pthread_cond_init(&cond,0);
    }
    ~SafeQueue(){
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void push(T value){
        pthread_mutex_lock(&mutex);
        q.push(value);
        //有了新数据加入进来就notify唤醒
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    int pop(T& value){
        int ret = 0;
        pthread_mutex_lock(&mutex);
        while(!q.empty()){
            //没数据就等待有数据加入进来唤醒
            pthread_cond_wait(&cond,&mutex);
        }
        if(!q.empty()){
            value = q.front();
            q.pop();
            ret = 1;
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    void clear(){
        pthread_mutex_lock(&mutex);
        uint32_t size = q.size();
        for (int i = 0; i < size; ++i) {
            //取出队首的数据
            T value = q.front();
            //释放value
            //releaseCallback != NULL
            if (releaseCallback)
                releaseCallback(value);
            q.pop();
        }
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback releaseCallback){
        this->releaseCallback = releaseCallback;
    }

private:
    queue<T> q;             //队列
    pthread_mutex_t mutex;  //互斥锁
    pthread_cond_t cond;    //条件变量
    ReleaseCallback releaseCallback;
};

#endif //FFMPEG_SAFE_QUEUE2_H
