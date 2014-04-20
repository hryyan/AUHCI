// gabor.h: Gabor滤波
//
// Created by Vincent Yan in 2014/03/25

#ifndef GABOR_H
#define GABOR_H

#include "util.h"

/**
* Gabor是用来进行Gabor滤波
*/
class Gabor
{
    public:
    /**
    * ctr
    */
    Gabor();
    
    // 获得虚部的Gabor核
	Mat getImagGaborKernel(Size ksize, double sigma, double theta, double nu, double gamma=1, int ktype=CV_32F);
    
    // 获得实部的Gabor核
    Mat getRealGaborKernel(Size ksize, double sigma, double theta, double nu, double gamma=1, int ktype=CV_32F);
    
    // 获得相位
    Mat getPhase(Mat &real, Mat &imag);
    
    // 获得实部与虚部的L2范式
    Mat getMagnitude(Mat &real, Mat &imag);
    
    // 获得实部与虚部
    void getFilterRealImagPart(Mat &src, Mat &real, Mat &imag, Mat &outReal, Mat &outImag);
    
    // 获得实部
    Mat getFilterRealPart(Mat &src, Mat &real);
    
    // 获得虚部
    Mat getFilterImagPart(Mat &sra, Mat &imag);
    
    // 进行初始化
    void Init(Size ksize=Size(19, 19), double sigma=2*CV_PI, double gamma=1, int ktype=CV_32FC1);
    
    // 初始化的flag
    bool isInited;
    
    // 实部核的vector
    vector<Mat>gaborRealKernels;
    
    // 虚部核的vector
    vector<Mat>gaborImagKernels;
    
    private:
    // 禁止复制ctor和assign
    DISALLOW_COPY_AND_ASSIGN(Gabor);
};

/**
* 输出对应相位和方向的Gabor特征的实部与虚部的L2范式
* @param  方向
* @param  频率
* @return
*/
Mat printGabor(Gabor& gabor, int mu, int nu);

/**
* 输出对应相位和方向的Gabor特征的实部与虚部的L2范式
* @param  方向
* @param  频率
* @return
*/
Mat printGabor_(Mat m, Gabor& gabor, int mu, int nu);

/**
 * 输出相关参数
 */
void printGaborPara();

#endif //GABOR_H