// eyedet.h: 使用Haar人眼检测
// 
// Created by Vincent Yan in 2014/04/04

#ifndef EYEDET_H
#define EYEDET_H

#include "util.h"

/**
 * 打开两个级联分类器
 * @param datadir 分类器数据所在的地址
 */
void OpenEyeDetectors(const char* datadir);

// TODO：把程序中数据的传递方式改为同时传递detpar和frame

/**
 * 使用两个级联分类器检测人眼
 * @param detpar 参数集
 * @param frame  
 */
void DetectEyes(DetPar& detpar,const Mat& frame);

#endif // EYEDET_H