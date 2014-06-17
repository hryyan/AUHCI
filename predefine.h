// predefine: 用于定义一些全局性的变量
// 
// Created by Vincent Yan in 2014/06/16

#ifndef PREDEFINE_H
#define PREDEFINE_H
// 处理的图像大小
static const int RESIZE_WIDTH   = 150;
static const int RESIZE_HEIGHT  = 150;

// Gabor卷积核大小，101效果好，速度慢，使用21比较快(单张1ms)
static const int kGaborSize = 21;

// 由于gpu::convolve函数的限制（默认会裁剪图像），所以需要在右边和下边添加边框
// http://code.opencv.org/issues/1639
static const double BORDER_FRAC = 1;

// 由于gpu::convolve函数的限制，需要对识别好的人眼进行一个校准
static const double X_OFFSET = 51.;
static const double Y_OFFSET = 50.;

// 是否使用GPU加速卷积
//#define USE_CUDA
//#define USE_OPENCV_GPU

// 是否使用目标识别
//#define USE_OPENCV_GPU_DETECTION

// 在生成数据的时候，是否需要输出XML或者计算Gabor特征或者是否需要输出检测到人脸之后的照片
#define OUTPUT_XML
//#define CALCULATE_GABOR
//#define SAVE_FACE
#define WITH_ID_EXPRESSION

#endif // PREDEFINE_H