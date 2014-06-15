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

void classiftInit();
void getAU(bool* au, Mat& gabor_img);