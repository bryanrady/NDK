#include <jni.h>
#include <string>
using namespace std;
#include <android/log.h>
#define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"JNI",__VA_ARGS__);    //  __VA_ARGS__ 代表... 可变参数

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
    char *updateNameStr = "悦悦";
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
    char *updateNameStr2 = "思思";
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
   // env->DeleteLocalRef(jlocalCls);
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