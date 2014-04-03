// util.h: ����Ķ���
// 
// Created by Vincent Yan in 2014/03/17

#include "util.h"

/**
 * ��windows��\ת����ͨ�õ�/
 * @param s in: ��Ҫת�����ַ���
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