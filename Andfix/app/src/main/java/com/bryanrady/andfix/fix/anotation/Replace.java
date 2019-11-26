package com.bryanrady.andfix.fix.anotation;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * 自定义一个注解，用来注解修复方法
 * @author: wangqingbin
 * @date: 2019/11/26 14:50
 */
@Target(ElementType.METHOD)
@Retention(RetentionPolicy.RUNTIME)
public @interface Replace {

    String wrongClassName();

    String wrongMethodName();

}
