package com.bryanrady.example;

import android.util.Log;

/**
 * @author: wangqingbin
 * @date: 2019/10/30 10:53
 */

public class Girl {

    private String name;
    private int age;

    private static final String TAG = Girl.class.getSimpleName();

    public Girl(){

    }

    public Girl(String name, int age) {
        this.name = name;
        this.age = age;
    }

    public String getName() {
        Log.e("wangqingbin","getName()被调用了");
        return name;
    }

    public void setName(String name) {
        Log.e("wangqingbin","setName()被调用了");
        this.name = name;
    }

    public int getAge() {
        Log.e("wangqingbin","getAge()被调用了");
        return age;
    }

    public void setAge(int age) {
        Log.e("wangqingbin","setAge()被调用了");
        this.age = age;
    }

    public static void printMsg(String msg){
        Log.e("wangqingbin","msg "+msg);
    }

    public static int getMessageCode(){
        Log.e("wangqingbin","getMessageCode被调用");
        return 0;
    }

    static class Inner{
        private float grade;
        private boolean isGirl;

        public float getGrade() {
            return grade;
        }

        public void setGrade(float grade) {
            this.grade = grade;
        }

        public boolean isGirl() {
            return isGirl;
        }

        public void setGirl(boolean girl) {
            isGirl = girl;
        }
    }

}
