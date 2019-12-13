#include <jni.h>
#include <string>

extern "C" {
//extern int p_main(int argc,char * argv[]);
extern int main(int argc,char * argv[]);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_bsdiff_MainActivity_native_1bspatch(JNIEnv *env, jobject instance, jstring old_apk,
                                                     jstring patch_, jstring output_) {
    const char *oldapk = env->GetStringUTFChars(old_apk, 0);
    const char *patch = env->GetStringUTFChars(patch_, 0);
    const char *output = env->GetStringUTFChars(output_, 0);
    char * argv[4] = {"", const_cast<char *>(oldapk), const_cast<char *>(output),const_cast<char *>(patch)};
    //p_main(4,argv);
    main(4,argv);

    env->ReleaseStringUTFChars(old_apk, oldapk);
    env->ReleaseStringUTFChars(patch_, patch);
    env->ReleaseStringUTFChars(output_, output);
}