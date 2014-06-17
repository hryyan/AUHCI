// classification.h: SVM分类
// 
// Created by Vincent Yan in 2014/06/5

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

struct ScaleFactor
{
    double feature_min[kFeatureNum];
    double feature_max[kFeatureNum];
};

void ClassifyInit();
void getAU(bool* au, Mat& gabor_img);