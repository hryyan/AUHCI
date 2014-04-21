// facedet.h: 人脸检测
// 
// Created by Vincent Yan in 2014/03/17

#include "util.h"

#ifndef FACEDET_H
#define FACEDET_H

/**
 * 检测人脸
 * @param img       in: 灰度图
 * @param multiface in: 是否检测多张人脸
 * @param minwidth  in: 检测的最小人脸
 */
vector<DetPar> DetectFaces_(const Image& img, bool multiface, int minwidth);

/**
 * 打印人脸
 * @return QImage
 */
Mat printFace();

#endif //FACEDET_H