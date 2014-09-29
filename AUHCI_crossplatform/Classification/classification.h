// classification.h: SVM分类
// 
// Created by Vincent Yan in 2014/06/05

#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

// Classification Methods
#include "svm.h"
// Util
#include "Util/source.h"
#include "Util/util.h"

// 目前最大的Slice有2400个特征，目前5000个特征已经够了
static const int kFeatureNum = 5000;

/**
 * 最大特征值和最小特征值
 */
struct ScaleFactor
{
    double feature_min[kFeatureNum];
    double feature_max[kFeatureNum];
};

/**
 * 分类初始化
 */
void ClassifyInit();

/**
 * 预测函数
 * @param x      in:   图像矩阵
 * @param output out:  所有AU的信息
 */
void predict(Mat *x, int *output);


/**
 * 根据图像中的人脸信息选择兴趣区域
 * @param src       in:   源图像矩阵
 * @param left      in:   左边界
 * @param right     in:   右边界
 * @param top       in:   上边界
 * @param bottom    in:   下边界
 * @param det       in:   人脸信息
 * @param section   in:   枚举值，可以为瞳孔、鼻尖、嘴尖
 * @param left_mat  out:  输出脸左侧的兴趣区域
 * @param right_mat out:  输出脸右侧的兴趣区域
 */
void getROI(Mat &src, int left, int right, int top, int bottom, DetPar det, FACESECTION section, Mat& left_mat, Mat& right_mat);

/**
 * 给定图片，获得该图片的AU信息
 * @param au        in/out:  AU信息
 * @param gabor_img in:      Gabor滤波后的图片
 */
void getAU(bool* au, Mat& gabor_img);

/**
 * Another verion for getting au from radius points
 * @param au        in/out: AU Information
 * @param gabor_img in:     img matrix
 */
void getAU2(bool* au_bool, Mat& gabor_img, int radius);

#endif
