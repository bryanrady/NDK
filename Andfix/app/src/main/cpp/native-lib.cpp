#include <jni.h>
#include "art_method.h"
//https://github.com/alibaba/AndFix
//这个头文件是从阿里的andfix里面直接拷贝过来的,我们自己去找的话很复杂，这些是阿里的人员从dalvik中抽取出来的，所以直接用即可
//dalvik.h是参考method.h来的，是阿里采集出来的
#include "dalvik.h"

//我们要hook到这两个函数，所以我们在这里进行声明
typedef Object *(*FindObject)(void *thread, jobject jobject1);
typedef  void* (*FindThread)();

FindObject  findObject;
FindThread  findThread;

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_andfix_DexManager_replaceArt(JNIEnv *env, jobject thiz, jint sdk,
                                                jobject wrong_method, jobject right_method) {
    //找到art虚拟机对应的 ArtMethod 结构体 ArtMethod只需要申明的头文件就行，实现在art虚拟机里面
    art::mirror::ArtMethod *wrongMethod = reinterpret_cast<art::mirror::ArtMethod *>(env->FromReflectedMethod(wrong_method));
    art::mirror::ArtMethod *rightMethod = reinterpret_cast<art::mirror::ArtMethod *>(env->FromReflectedMethod(right_method));

    //不是方法直接替换
    //wrongMethod = rightMethod;

    //而是ArtMethod里面的成员变量进行替换
    wrongMethod->declaring_class_ = rightMethod->declaring_class_;
    wrongMethod->dex_cache_resolved_methods_ = rightMethod->dex_cache_resolved_methods_;
    wrongMethod->access_flags_ = rightMethod->access_flags_;
    wrongMethod->dex_cache_resolved_types_ = rightMethod->dex_cache_resolved_types_;
    wrongMethod->dex_code_item_offset_ = rightMethod->dex_code_item_offset_;
    wrongMethod->dex_method_index_ = rightMethod->dex_method_index_;
    wrongMethod->method_index_ = rightMethod->method_index_;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_andfix_DexManager_replaceDalvik(JNIEnv *env, jobject thiz, jint sdk,
                                                   jobject wrong_method, jobject right_method) {
    //dalvik 虚拟机里面叫 Method 结构体 art虚拟机叫 ArtMethod
    //找到dalvik虚拟机对应的Method 结构体
    Method *wrongMethod = reinterpret_cast<Method *>(env->FromReflectedMethod(wrong_method));
    Method *rightMethod = reinterpret_cast<Method *>(env->FromReflectedMethod(right_method));

    //底层的Hook 直接打开一个so库，使用里面的方法
    //通过dlopen()函数得到一个动态链接库的操作句柄
    void *dvm_hand = dlopen("libdvm.so",RTLD_NOW);
//下一步  把right 对应Object   第一个成员变量ClassObject   status

//    sdk  10    以前是这样   10会发生变化
    findObject= (FindObject) dlsym(dvm_hand, sdk > 10 ? "_Z20dvmDecodeIndirectRefP6ThreadP8_jobject" :"dvmDecodeIndirectRef");
    findThread = (FindThread) dlsym(dvm_hand, sdk > 10 ? "_Z13dvmThreadSelfv" : "dvmThreadSelf");
// method   所声明的Class

    jclass methodClaz = env->FindClass("java/lang/reflect/Method");
    jmethodID rightMethodId = env->GetMethodID(methodClaz, "getDeclaringClass","()Ljava/lang/Class;");
//dalvik  odex   机器码
//    firstFiled->status=CLASS_INITIALIZED
//    art不需要    dalvik适配
    jobject ndkObject = env->CallObjectMethod(right_method, rightMethodId);
//    kclass
//    davik   ------>ClassObject
//  这里就相当于art虚拟机的kclass，一个空白的class
    ClassObject *firstFiled = (ClassObject *) findObject(findThread(), ndkObject);
    //这里设置为初始化完成的，让虚拟机认为类已经加载进来
    firstFiled->status = CLASS_INITIALIZED;

    //这下面的操作就和art虚拟机一样，对Method结构体里面的成员变量进行赋值
    wrongMethod->accessFlags |= ACC_PUBLIC;
//对于具体已经实现了的虚方法来说，这个是该方法在类虚函数表（vtable）中的偏移；对于未实现的纯接口方法来说，
// 这个是该方法在对应的接口表（假设这个方法定义在类继承的第n+1个接口中，则表示iftable[n]->methodIndexArray）中的偏移；
    wrongMethod->methodIndex=rightMethod->methodIndex;
//    这个变量记录了一些预先计算好的信息，从而不需要在调用的时候再通过方法的参数和返回值实时计算了，方便了JNI的调用，提高了调用的速度。
// 如果第一位为1（即0x80000000），则Dalvik虚拟机会忽略后面的所有信息，强制在调用时实时计算；
    wrongMethod->jniArgInfo=rightMethod->jniArgInfo;
    wrongMethod->registersSize=rightMethod->registersSize;
    wrongMethod->outsSize=rightMethod->outsSize;
//    方法参数 原型
    wrongMethod->prototype=rightMethod->prototype;
//
    wrongMethod->insns=rightMethod->insns;
//    如果这个方法是一个Dalvik虚拟机自带的Native函数（Internal Native）的话，
// 则这里存放了指向JNI实际函数机器码的首地址。如果这个方法是一个普通的Native函数的话，
// 则这里将指向一个中间的跳转JNI桥（Bridge）代码；
    wrongMethod->nativeFunc=rightMethod->nativeFunc;
}