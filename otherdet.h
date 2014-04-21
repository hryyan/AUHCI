// otherdet.h：使用Haar检测其他
//
// Created by Vincent yan in 2014/04/20

#ifndef OTHERDET_H
#define OTHERDET_H

#include "util.h"

/**
 * 检测Mouth
 * @param frame in: 源图像
 */
void DetectMouth(Mat &frame);

/**
 * 检测Nose
 * @param frame in: 源图像
 */
void DetectNose(Mat &frame);

#endif // OTHERDET_H