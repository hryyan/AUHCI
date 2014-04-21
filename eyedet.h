// eyedet.h: 使用Haar人眼检测
// 
// Created by Vincent Yan in 2014/04/04

#ifndef EYEDET_H
#define EYEDET_H

#include "util.h"
#include "haarclassifier.h"

// TODO：把程序中数据的传递方式改为同时传递detpar和frame
// 否定：因为Qt中不能设置为数据驱动的，所以只能用frame进行数据流动
// 肯定：将detpar作为全局变量，传递一张脸的信息

/**
 * 使用两个级联分类器检测人眼
 * @param detpar 参数集
 * @param frame  
 */
void DetectEyes(DetPar& detpar,const Mat& frame);

#endif // EYEDET_H