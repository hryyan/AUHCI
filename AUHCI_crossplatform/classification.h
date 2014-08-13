// classification.h: SVM分类
// 
// Created by Vincent Yan in 2014/06/05

#include "svm.h"
#include "source.h"
#include "facedet.h"
#include "eyedet.h"
#include "otherdet.h"
#include "gabor.h"
#include "util.h"
#include "generate.h"

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
 * 给定图片，获得该图片的AU信息
 * @param au        in/out:  AU信息
 * @param gabor_img in:      Gabor滤波后的图片
 */
void getAU(bool* au, Mat& gabor_img);