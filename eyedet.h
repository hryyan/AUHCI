// eyedet.h: 人眼检测
// 
// Created by Vincent Yan in 2014/04/04

#ifndef EYEDET_H
#define EYEDET_H

#include "util.h"

void OpenEyeDetectors(const char* datadir);
void DetectEyes(DetPar& detpar,const Mat& frame);

#endif // EYEDET_H