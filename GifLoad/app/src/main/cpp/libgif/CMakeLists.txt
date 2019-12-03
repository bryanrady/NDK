cmake_minimum_required(VERSION 3.4.1)

file(GLOB gif_source *.c)

add_library(
        gif

        STATIC

        ${gif_source})
