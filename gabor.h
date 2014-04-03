// gabor.h: Gabor滤波
// 
// Created by Vincent Yan in 2014/03/25

#ifndef GABOR_H
#define GABOR_H

#include "util.h"

class Gabor
{
public:
	Gabor();

	static Mat getImagGaborKernel(Size ksize, double sigma, double theta, double nu, double gamma=1, int ktype=CV_32F);
	static Mat getRealGaborKernel(Size ksize, double sigma, double theta, double nu, double gamma=1, int ktype=CV_32F);
	static Mat getPhase(Mat &real, Mat &imag);
	static Mat getMagnitude(Mat &real, Mat &imag);
	static void getFilterRealImagPart(Mat &src, Mat &real, Mat &imag, Mat &outReal, Mat &outImag);
	static Mat getFilterRealPart(Mat &src, Mat &real);
	static Mat getFilterImagPart(Mat &sra, Mat &imag);

	void Init(Size ksize=Size(19, 19), double sigma=2*CV_PI, double gamma=1, int ktype=CV_32FC1);

	bool isInited;

	vector<Mat> gaborRealKernels;
	vector<Mat> gaborImagKernels;

private:
	// 禁止复制ctor和assign
    DISALLOW_COPY_AND_ASSIGN(Gabor);
};

QImage* printGabor();
QImage* printGabor_2(int mu, int nu);

#endif //GABOR_H