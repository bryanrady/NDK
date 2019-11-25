#include <jni.h>
#include <string>
using namespace std;
#include <android/log.h>
#include <pthread.h>
//定义宏
#define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"JNI",__VA_ARGS__);    //  __VA_ARGS__ 代表... 可变参数

//因为test()方法是c文件写的，并且在native-lib这个文件里没有声明，所以先进行声明
extern "C"{
    extern int test();
}

//  Java类型	签名
//  boolean	    Z
//  short	    S
//  float	    F
//  byte	    B
//  int	        I
//  double	    D
//  char	    C
//  long	    J
//  void	    V
//  引用类型	L + 全限定名 + ;
//  数组	    [+类型签名

extern "C"
JNIEXPORT jstring JNICALL
Java_com_bryanrady_example_MainActivity_stringFromJNI(JNIEnv *env, jobject /* this */) {

    //测试ndk17交叉编译的共享动态库
    LOGE("libTest.so 里面的 test() 方法:%d",test());

    //instnce参数就是this，就是MainActivity对象
    string hello = "Hello from C++";

    //这是c++的调用方式
    //将c/c++的字符串hello转换成jni的字符串jstr
    jstring jstr = env->NewStringUTF(hello.c_str());

    //c的调用方式
    //(*env)->NewStringUTF(env,hello.c_str());
    //JNINativeInterface **a; 因为env在c中是一个二级指针

    //#if defined(__cplusplus)
    //    typedef _JNIEnv JNIEnv;
    //    typedef _JavaVM JavaVM;
    //#else
    //    typedef const struct JNINativeInterface* JNIEnv;
    //typedef const struct JNIInvokeInterface* JavaVM;
    //#endif

//    const char *hello = "Hello from C++";
//    jstring jstr = env->NewStringUTF(hello);
    return jstr;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_example_MainActivity_transferStringToJNI(JNIEnv *env, jobject instance, jstring str_) {
    //将jni的字符串str_转换成c/c++的字符串str
    const char *str = env->GetStringUTFChars(str_, 0);
    LOGE("通过JNI从java中获取字符串: %s", str);
    //记得释放 养成好习惯 虽然这些局部引用出了方法后就会释放，但是还是自己释放一遍
    env->ReleaseStringUTFChars(str_, str);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_example_MainActivity_transferArrayToJni(JNIEnv *env, jobject instance,
                                                           jintArray intArr_,
                                                           jobjectArray stringArr) {
    //这里的第二个参数是一个指针：指向内存地址
    //如果是TRUE: 就是拷贝一份数据 然后重新申请一个数组内存,如果是FALSE：就是用java数组的内存地址
    jint *intArr = env->GetIntArrayElements(intArr_, NULL);
    //获取int数组的大小，jsize实际上就是jint   因为  typedef jint  jsize;
    jsize intArrSize = env->GetArrayLength(intArr_);
    for(size_t i = 0; i < intArrSize; i++){
        //这两种方式都可以获取数组中的元素
       // LOGE("1遍历从java中获取的Int数组: %d", intArr[i]);
        LOGE("2遍历从java中获取的Int数组: %d", *(intArr + i));

        *(intArr + i) = 100;
    }
    jint stringArrSize = env->GetArrayLength(stringArr);
    for(size_t j = 0; j < stringArrSize; j++){
        //获取stringArr数组和下标获取数组中的元素，因为是字符串，这里可以直接转换
//        jobject jobj = env->GetObjectArrayElement(stringArr,j);
//        jstring jstr = (jstring) jobj;
        jstring jstr = (jstring) env->GetObjectArrayElement(stringArr, j);
        //将Jni的字符串转换成c/c++字符串
        const char *str = (char *) env->GetStringUTFChars(jstr, NULL);
        LOGE("遍历从java中获取的String数组: %s",str);

        env->ReleaseStringUTFChars(jstr,str);
    }
    //这里的第3个参数 mode：
    //  0: 刷新java数组并释放c/c+数组
    //  1 = JAVA_COMMIT: 只刷新java数组
    //  2 = JAVA_ABORT:  只释放c/c+数组
    env->ReleaseIntArrayElements(intArr_, intArr, 0);
   // env->ReleaseIntArrayElements(intArr_, intArr, 1);
   // env->ReleaseIntArrayElements(intArr_, intArr, 2);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_example_MainActivity_transferObjectToJni(JNIEnv *env, jobject instance, jobject girl) {

    //获取java中的girl实例
    jclass girlCls = env->GetObjectClass(girl);

    //1.通过反射获取girl实例中的字段 name 和 age
    //获取name属性的id   第二个参数：属性字段，第三个参数：属性类型的签名  "L+全类名;"
    jfieldID nameFieldId = env->GetFieldID(girlCls,"name","Ljava/lang/String;"); //java.lang.String
    jstring jnameStr = (jstring) env->GetObjectField(girl, nameFieldId);
    char *nameStr = (char *)env->GetStringUTFChars(jnameStr,NULL);
    LOGE("从Girl对象获取到的name字段: %s",nameStr);

    jfieldID  ageFieldId = env->GetFieldID(girlCls,"age","I");
    jint jageInt = env->GetIntField(girl,ageFieldId);
    LOGE("从Girl对象获取到的age字段: %d",jageInt);

    //2.获取属性值后修改属性字段值,通过Java打印gril对象后发现属性值已经改变
    const char *updateNameStr = "悦悦";
    jstring jupdateNameStr = env->NewStringUTF(updateNameStr);
    env->SetObjectField(girl,nameFieldId,jupdateNameStr);

    env->SetIntField(girl,ageFieldId,23);

    //3.这里也可以通过反射girl的getName()和getAge()方法查看属性值是否发生改变
    //方法签名 "()Ljava/lang/String;"，括号里面的代表参数，没有参数就不写，后面的代表返回值，返回值是String，所以是 Ljava/lang/String;
    jmethodID getNameMethodId = env->GetMethodID(girlCls,"getName","()Ljava/lang/String;");
    jstring jgetNameStr = (jstring)env->CallObjectMethod(girl,getNameMethodId);
    char *getNameStr = (char *)env->GetStringUTFChars(jgetNameStr,NULL);
    LOGE("通过对属性值name进行修改后，通过反射调用Girl的getName()获取到的name字段: %s",getNameStr);

    //方法签名 "()I"，括号里面的代表参数，没有参数就不写，返回值是int，所以是I
    jmethodID getAgeMethodId = env->GetMethodID(girlCls,"getAge","()I");
    jint jgetAge = env->CallIntMethod(girl,getAgeMethodId);
    LOGE("通过对属性值age进行修改后，通过反射调用Girl的getAge()获取到的age字段: %d",jgetAge);

    //4.通过反射Girl的setName()和setAge()方法修改值
    //方法签名 (L/java/lang/String;)V ，括号里面的代表参数，String类型所以是L/java/lang/String;，返回值是void，所以是V
    jmethodID setNameMethodId = env->GetMethodID(girlCls,"setName","(Ljava/lang/String;)V");
    const char *updateNameStr2 = "思思";
    jstring jupdateNameStr2 = env->NewStringUTF(updateNameStr2);
    env->CallVoidMethod(girl,setNameMethodId,jupdateNameStr2);

    jmethodID setAgeMethodId = env->GetMethodID(girlCls,"setAge","(I)V");
    env->CallVoidMethod(girl,setAgeMethodId,25);

    //释放jstring 和 char* 和 DeleteLocalRef释放局部引用
    env->ReleaseStringUTFChars(jnameStr,nameStr);
    env->DeleteLocalRef(jupdateNameStr);
    env->ReleaseStringUTFChars(jgetNameStr,getNameStr);
    env->DeleteLocalRef(jupdateNameStr2);
    env->DeleteLocalRef(girlCls);
    env->DeleteLocalRef(girl);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_bryanrady_example_MainActivity_getGirlFromJni(JNIEnv *env, jobject instance) {
    //找到具体的Girl类
    jclass girlCls = env->FindClass("com/bryanrady/example/Girl");
    if(girlCls == NULL){
        return NULL;
    }

    //获取Girl两个参数构造函数的方法Id,方法名字是"<init>","(Ljava/lang/String;I)V" 参数是String和int，返回值是Void
    jmethodID constructMethodId = env->GetMethodID(girlCls,"<init>","(Ljava/lang/String;I)V");
    jstring jnameStr = env->NewStringUTF("小美");
    //根据构造函数创建girlObj对象
    jobject girlObj = env->NewObject(girlCls,constructMethodId,jnameStr,18);

    env->DeleteLocalRef(jnameStr);
    env->DeleteLocalRef(girlCls);

    return girlObj;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_example_MainActivity_testGirlStatic(JNIEnv *env, jobject instance) {

    jclass girlCls = env->FindClass("com/bryanrady/example/Girl");
    if(girlCls == NULL){
        return;
    }

    jfieldID jTAGFieldId = env->GetStaticFieldID(girlCls,"TAG","Ljava/lang/String;");
    jstring jTAGStr = (jstring) env->GetStaticObjectField(girlCls,jTAGFieldId);
    const char *tagStr = env->GetStringUTFChars(jTAGStr,NULL);
    LOGE("Girl的静态属性TAG字段值为: %s",tagStr);

    //调用Girl里面的静态方法printMsg(String msg),方法签名没什么区别
    jmethodID printMsgMethodId = env->GetStaticMethodID(girlCls,"printMsg","(Ljava/lang/String;)V");
    jstring jmsgStr = env->NewStringUTF("在C/C++里面打印的信息");
    //这里调用static方法，第一个参数是jclass,而不是jobject
    env->CallStaticVoidMethod(girlCls,printMsgMethodId,jmsgStr);

    jmethodID getMessageCodeMethodId = env->GetStaticMethodID(girlCls,"getMessageCode","()I");
    env->CallStaticIntMethod(girlCls,getMessageCodeMethodId);

    env->ReleaseStringUTFChars(jTAGStr,tagStr);
    env->DeleteLocalRef(jmsgStr);
    env->DeleteLocalRef(girlCls);
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_bryanrady_example_MainActivity_testGirlInner(JNIEnv *env, jobject instance) {

    jclass jinnerCls = env->FindClass("com/bryanrady/example/Girl$Inner");
    if (jinnerCls == NULL){
        return NULL;
    }
    jmethodID innerConstructMethodId = env->GetMethodID(jinnerCls,"<init>","()V");
    jobject jinnerObj = env->NewObject(jinnerCls,innerConstructMethodId);

    //下面通过字段赋值或者方法赋值都可以
    jfieldID gradeFieldId = env->GetFieldID(jinnerCls,"grade","F");
    env->SetFloatField(jinnerObj,gradeFieldId,98.6);
    jmethodID setGirlMethodId = env->GetMethodID(jinnerCls,"setGirl","(Z)V");
    env->CallVoidMethod(jinnerObj,setGirlMethodId,1);

    env->DeleteLocalRef(jinnerCls);

    return jinnerObj;
}

/*
 * 在 JNI 规范中定义了三种引用：局部引用（Local Reference）、全局引用（Global Reference）、弱全局引用（Weak Global Reference）。
 *
 * 局部引用:
 *  大多数JNI函数会创建局部引用。NewObject/FindClass/NewStringUTF 等等都是局部引用。局部引用只有在创建它的本地方法返回前有效,
 *  本地方法返回后，局部引用会被自动释放。因此无法跨线程、跨方法使用。
 *
 *  释放一个局部引用有两种方式:
 *      1、本地方法执行完毕后VM自动释放;
 *      2、通过DeleteLocalRef手动释放；
 *
 *      VM会自动释放局部引用，为什么还需要手动释放呢？
 *          因为局部引用会阻止它所引用的对象被GC回收。
 *
 *
 * 全局引用
 *      由 NewGlobalRef  函数创建, 全局引用可以跨方法、跨线程使用，直到它被手动释放才会失效 。
 *
 *
 * 弱全局引用
 *      与全局引用类似，弱引用可以跨方法、线程使用。与全局引用不同的是，弱引用不会阻止GC回收它所指向的VM内部的对象 。
 *      在对Class进行弱引用是非常合适（FindClass），因为Class一般直到程序进程结束才会卸载。
 *      在使用弱引用时，必须先检查缓存过的弱引用是指向活动的对象，还是指向一个已经被GC的对象
 */

jclass jlocalCls;
extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_example_MainActivity_testJniLocalRef(JNIEnv *env, jobject instance) {
    //调用2次以上都会有问题，调用1次不会有问题
    //局部引用,有问题 出了方法后jlocalcls会被释放，但是jlocalcls指针依然有值，但是指向的地址数据被释放了  悬空指针
    if(jlocalCls == NULL){
        jlocalCls = env->FindClass("com/bryanrady/example/Girl");   //jlocalcls是一个局部引用,虽然是一个全局变量，但实际上还是一个局部引用
    }
    //下面是用了jlocalcls后，当到底2次调用的时候就出现了问题
    jmethodID printMsgMethodId = env->GetStaticMethodID(jlocalCls,"printMsg","(Ljava/lang/String;)V");
    jstring jmsgStr = env->NewStringUTF("测试局部引用");
    env->CallStaticVoidMethod(jlocalCls,printMsgMethodId,jmsgStr);

    env->DeleteLocalRef(jmsgStr);
//    env->DeleteLocalRef(jlocalCls);
}

jclass jglobalCls;

void test(JNIEnv *env){
    jmethodID printMsgMethodId = env->GetStaticMethodID(jglobalCls,"printMsg","(Ljava/lang/String;)V");
    jstring jmsgStr = env->NewStringUTF("测试全局引用");
    env->CallStaticVoidMethod(jglobalCls,printMsgMethodId,jmsgStr);
    env->DeleteLocalRef(jmsgStr);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_example_MainActivity_testJniGlobalRef(JNIEnv *env, jobject instance) {
    if(jglobalCls == NULL){
        jclass jcls= env->FindClass("com/bryanrady/example/Girl");   //jcls是一个局部引用
        jglobalCls = static_cast<jclass>(env->NewGlobalRef(jcls));  //将jcls变成一个全局引用就可以了，调用多少次都不会有问题
        env->DeleteLocalRef(jcls);
    }
    //全局引用可以跨方法跨进程进行使用
    test(env);

    //不需要的时候就去释放全局引用
    //env->DeleteGlobalRef(jlocalCls);
}

jclass jweakglobalCls;

int test2(JNIEnv *env){
    //弱引用 ：不会阻止回收
    //问题： 当我们使用弱引用的时候  弱引用 引用的对象可能被回收了
    //每次使用的时候判断是否被回收了,对一个弱引用 与NULL相比较 IsSameObject
    // true： 释放了
    // false: 还可以使用这个弱引用

    jboolean isEqual = env->IsSameObject(jweakglobalCls, NULL);
    if (!isEqual){
        jmethodID printMsgMethodId = env->GetStaticMethodID(jweakglobalCls,"printMsg","(Ljava/lang/String;)V");
        jstring jmsgStr = env->NewStringUTF("测试弱全局引用");
        env->CallStaticVoidMethod(jweakglobalCls,printMsgMethodId,jmsgStr);
        env->DeleteLocalRef(jmsgStr);
        return 1;
    }else{
        LOGE("弱全局引用jweakglobalCls已经被释放了!")
        return 0;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_example_MainActivity_testJniWeakGlobalRef(JNIEnv *env, jobject instance) {
    jclass jcls =  env->FindClass("com/bryanrady/example/Girl");
    jweakglobalCls = static_cast<jclass>(env->NewWeakGlobalRef(jcls));  //将jcls变成一个弱全局引用

    env->DeleteLocalRef(jcls);

    //这里不好测试出来，但是记住每次使用之前作判断即可
    test2(env);

    //不需要的时候可以手动释放弱全局引用
    //env->DeleteWeakGlobalRef(jweakglobalCls);

}

/***
 *
 * JNI_OnLoad
 *      在调用System.loadLibrary()函数时，内部就会去查找so中的 JNI_OnLoad 函数，如果存在此函数则自动调用。
 *      所以so库中我们可控的第一个JNI函数就是   JNI_OnLoad(); 可以在这个函数中做一些初始化操作
 *
 *      JNI_OnLoad会告诉 VM 此 native 组件使用的 JNI 版本。
 *      ​对应了Java版本，android中只支持JNI_VERSION_1_2 、JNI_VERSION_1_4、JNI_VERSION_1_6,
 *      在JDK1.8有 JNI_VERSION_1_8。
 *
 *       c++
 *      int JNI_OnLoad(JavaVM* vm, void* reserved){
 *          // 2、4、6都可以
 *          return JNI_VERSION_1_4;
 *      }
 *
 *  动态注册
 *  在此之前我们一直在jni中使用的 Java_PACKAGENAME_CLASSNAME_METHODNAME 来进行与java方法的匹配，这种方式我们称之为静态注册。
 *  而动态注册则意味着方法名可以不用这么长了，在android aosp源码中就大量的使用了动态注册的形式
 *
 */

//第一个参数是Java jvm虚拟机，第二个参数具体是什么不知道，反正JVM传递过来的也是NULL,所以不用管

JavaVM *_vm;    //定义全局变量JavaVM

void testDynamicJni1(){
    LOGE("JNI 动态注册 testDynamicJni1");
}

//如果有参数传递的话，要把JNIEnv *env, jobject instance这两个参数也添加上
jstring testDynamicJni2(JNIEnv *env, jobject instance, jint _int){
    LOGE("JNI 动态注册 testDynamicJni2: %d",_int);
    return env->NewStringUTF("调用成功");
}

//typedef struct {
//    const char* name;         代表方法名
//    const char* signature;    方法签名
//    void*       fnPtr;        代表对应的jni方法
//} JNINativeMethod;

//定义一个不可变数组，里面存储需要动态注册的java native方法
static const JNINativeMethod nativeMethods[] = {
        {"testDynamicJava1","()V",(void*)testDynamicJni1},
        {"testDynamicJava2","(I)Ljava/lang/String;",(int*)testDynamicJni2}
};
//需要动态注册native方法的类名
static const char* className = "com/bryanrady/example/MainActivity";

int JNI_OnLoad(JavaVM* vm, void* reserved){
    //将JavaVM使用全局变量_vm保存起来
    _vm = vm;

    JNIEnv *env = NULL ;
    //可以通过vm来获得JNIEnv对象
    int r = vm->GetEnv((void**)&env,JNI_VERSION_1_6);
    // 小于0 失败 ，等于0 成功
    if (r != JNI_OK){
        return -1;
    }
    //获得要动态注册的类的实例
    jclass jcls = env->FindClass(className);
    //进行注册,第3个参数是要注册方法数组的大小
    env->RegisterNatives(jcls, nativeMethods, sizeof(nativeMethods)/ sizeof(JNINativeMethod));

    //这里返回1.2、1.4、1.6都没有太大的影响
    return JNI_VERSION_1_6;
}

/**
 *  native线程调用Java
 *      native调用java需要使用JNIEnv这个结构体，而JNIEnv是由Jvm传入与线程相关的变量。
 *      但是可以通过JavaVM的AttachCurrentThread方法来获取到当前线程中的JNIEnv指针。
 */

struct Context{
    jobject instance;
    JNIEnv *env;
};

void* testThread(void *args){

//    Context *context = static_cast<Context*>(args);
//    //这里就报错了,因为JNIEnv不能跨线程使用，传进来的这个JNIEnv是主线程中的，不能使用在子线程中
//    jclass jcls = context->env->GetObjectClass(context->instance);
//    jmethodID updateUIMethodId = context->env->GetMethodID(jcls,"updateUI","()V");
//    context->env->CallVoidMethod(context->instance,updateUIMethodId);
//    context->env->DeleteLocalRef(jcls);

    //把当前native线程附加到java 虚拟机中
    JNIEnv *env;
    jint r = _vm->AttachCurrentThread(&env,0);
    if (r != JNI_OK){
        return 0;
    }
    Context *context = static_cast<Context*>(args);
    jclass jcls = env->GetObjectClass(context->instance);
    jmethodID updateUIMethodId = env->GetMethodID(jcls,"updateUI","()V");
    env->CallVoidMethod(context->instance,updateUIMethodId);

    env->DeleteGlobalRef(context->instance);
    delete(context);
    env->DeleteLocalRef(jcls);

    //附加之后要记得分离
    _vm->DetachCurrentThread();

    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_bryanrady_example_MainActivity_testNativeThread(JNIEnv *env, jobject instance) {
    //在jni创建一个线程，然后在线程中调用java方法
    pthread_t pid;
    Context *context = new Context;
    context->env = env; //这句话是错误的，不能直接将这个env传递到线程中进行使用
    context->instance = env->NewGlobalRef(instance);    //这里弄成全局的为了让instance可以跨方法跨线程进行使用
    pthread_create(&pid,0,testThread,context);
}