// util.h: 零碎的定义
// 
// Created by Vincent Yan in 2014/03/17

#include "util.h"

/**
 * 把windows的\转换成通用的/
 * @param s in: 需要转换的字符串
 */
void ConvertBackslashesToForwardAndStripFinalSlash(char* s)
{
    int i;

    for (i = 0; s[i]; i++)       // convert \ to /
        if (s[i] == '\\')
            s[i] = '/';

    if (i > 0 && s[i-1] == '/')  // remove final / if any
        s[i-1] = 0;
}