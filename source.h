// source.h: ��Դ��ȡ������ͷ����Ƶ��ͼƬ��
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