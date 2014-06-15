// gabor.h: Gabor滤波
//
// Modified by Vincent Yan in 2014/06/09

#ifndef GABOR_H
#define GABOR_H

#include "util.h"
#include "conv2d.h"

/**
 * 进行Gabor滤波
 */
class Gabor
{
public:
    /**
     * ctr
     */
    Gabor();

    /**
     * desctr
     */
    virtual ~Gabor();

    // 获得虚部的Gabor核
    Mat getImagGaborKernel(Size ksize, double sigma, double theta, double nu, double gamma=1, int ktype=CV_32F);

    // 获得实部的Gabor核
    Mat getRealGaborKernel(Size ksize, double sigma, double theta, double nu,
        double gamma=1, int ktype=CV_32F);

    // 获得相位
    Mat getPhase(Mat &real, Mat &imag);

    // 获得实部与虚部的L2范式
    Mat getMagnitude(Mat &real, Mat &imag);

    // 获得实部与虚部
    void getFilterRealImagPart(Mat &src, Mat &real, Mat &imag, Mat &outReal, Mat &outImag);

    // 获得实部与虚部的L2范数
    void getFilterMagnitude(Mat &src, Mat &real, Mat &imag, Mat &output);

    // 调用卷积函数获得结果
    Mat getFilter(Mat &src, Mat &kernel);

    // 初始化
    virtual void Init(Size ksize=Size(19, 19), double sigma=2*CV_PI, double gamma=1, int ktype=CV_32FC1);

    // 初始化的flag
    bool isInited;

    // 实部核和虚部核的vector
    vector<Mat> gaborRealKernels;
    vector<Mat> gaborImagKernels;

private:
    // 禁止复制ctr和assign
    DISALLOW_COPY_AND_ASSIGN(Gabor);
};

class CUDA_Gabor : public Gabor
{
public:
    /**
     * ctr
     */
    CUDA_Gabor();

    /**
     * desctr
     */
    virtual ~CUDA_Gabor();

    // 预先算出kernel的频谱
    void Prepare_GPU_PadderKernel(int index, bool isReal);

    // 把Image放置到GPU中
    void Prepare_GPU_PadderData(Mat &src);

    // 调用卷积函数获得结果
    Mat getFilter(Mat &src, int index, bool isReal);

    // 获得实部与虚部的L2范数
    void getFilterMagnitude(Mat &src, int index, Mat &output);

    // 初始化
    virtual void Init(Size ksize=Size(19, 19), double sigma=2*CV_PI, double gamma=1, int ktype=CV_32FC1);

    // 初始化的flag
    bool isInited;

    // 实部核和虚部核的vector
    vector<Mat> gaborRealKernels;
    vector<Mat> gaborImagKernels;
    // 实部核和虚部核的频谱
    vector<fComplex*> d_VecRealKernelSpectrum, d_VecImagKernelSpectrum;
    fComplex* d_KernelSpectrum, *d_DataSpectrum, *d_StoreSpectrum;
    // CPU端的数据
    float *h_Data, *h_Kernel, *h_ResultGPU;
    // GPU端的数据
    float *d_Data, *d_PaddedData, *d_Kernel, *d_PaddedKernel, *d_Result;
    // FFT变换
    cufftHandle fftPlanFwd, fftPlanInv;
    // 核的数据
    int kernelH, kernelW, kernelY, kernelX;
    // 图像的数据
    int dataH, dataW;
    // fft变换的数据
    int fftW, fftH;
    
private:
    // 禁止复制ctr和assign
    DISALLOW_COPY_AND_ASSIGN(CUDA_Gabor);
};

void InitGabor();
Mat printGabor();
Mat printGabor_(Mat& m, int nu, int mu);

/**
 * 输出相关参数
 */
void printGaborPara();
#endif