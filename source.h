// source.h: 来源获取（摄像头、视频、图片）
// 
// Created by Vincent Yan in 2014/03/18

#ifndef SOURCE_H
#define SOURCE_H

#include "util.h"

QImage* Mat2QImage(Mat& T);

bool initCamera();
bool initVideo(std::string);
bool initPic(std::string);

QImage* printScreen();

#endif // SOURCE_H