// gabor.cpp: Gabor滤波
//
// Modified by Vincent Yan in 2014/06/09

#include "gabor.h"

// source中的每一帧
extern Mat frame;

// gabor的全局函数，用于printGabor调用
Gabor gabor_g;
CUDA_Gabor gabor_cuda_g;

// 默认的核大小
static const int iSize = 21;

// 滤波前需要进行边界扩充，这个代码以前在generator.cpp中使用过
const double BORDER_FRAC = 1;

// 把采集到的图像进行缩放，到统一尺寸
const int RESIZE_WIDTH  = 150;
const int RESIZE_HEIGHT = 150;

Gabor::Gabor()
{
    isInited = false;
}

Gabor::~Gabor()
{

}

void Gabor::Init(Size ksize, double sigma, double gamma, int ktype)
{
    gaborRealKernels.clear();
    gaborImagKernels.clear();
    double mu[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    double nu[5] = {0, 1, 2, 3, 4};

    for (int i = 0; i < 5; ++i)
    {
       for (int j = 0; j < 8; ++j)
       {
        gaborRealKernels.push_back(getRealGaborKernel(ksize, sigma, mu[j]*CV_PI/8+CV_PI/2, nu[i], gamma, ktype));
        gaborImagKernels.push_back(getImagGaborKernel(ksize, sigma, mu[j]*CV_PI/8+CV_PI/2, nu[i], gamma, ktype));
       } 
    }

    isInited = true;
}

// 获得虚部的Gabor核
Mat Gabor::getImagGaborKernel(Size ksize, double sigma, double theta, double nu, double gamma, int ktype)
{
    double  sigma_x     = sigma;
    double  sigma_y     = sigma/gamma;
    int     nstds       = 3;
    double  kmax        = CV_PI/2;
    double  f           = sqrt(2.0);
    int xmin, xmax, ymin, ymax;
    double c = cos(theta), s = sin(theta);
    if( ksize.width > 0 )
    {
        xmax = ksize.width/2;
    }
    else//这个和matlab中的结果一样，默认都是19 !
    {
        xmax = cvRound(std::max(fabs(nstds*sigma_x*c), fabs(nstds*sigma_y*s)));
    }
    if( ksize.height > 0 )
    {
        ymax = ksize.height/2;
    }
    else
    {
        ymax = cvRound(std::max(fabs(nstds*sigma_x*s), fabs(nstds*sigma_y*c)));
    }
    xmin = -xmax;
    ymin = -ymax;
    CV_Assert( ktype == CV_32F || ktype == CV_64F );
    float*  pFloat;
    double* pDouble;
    Mat kernel(ymax - ymin + 1, xmax - xmin + 1, ktype);
    double k        =   kmax/pow(f,nu);
    double scaleReal=   k*k/sigma_x/sigma_y;
    for( int y = ymin; y <= ymax; y++ )
    {
        if( ktype == CV_32F )
        {
            pFloat = kernel.ptr<float>(ymax-y);
        }
        else
        {
            pDouble = kernel.ptr<double>(ymax-y);
        }
        for( int x = xmin; x <= xmax; x++ )
        {
            double xr = x*c + y*s;
            double v = scaleReal*exp(-(x*x+y*y)*scaleReal/2);
            double temp=sin(k*xr);
            v   =  temp*v;
            if( ktype == CV_32F )
            {
                pFloat[xmax - x]= (float)v;
            }
            else
            {
                pDouble[xmax - x] = v;
            }
        }
    }
    return kernel;
}

// 获得实部的Gabor核
Mat Gabor::getRealGaborKernel(Size ksize, double sigma, double theta, double nu, double gamma, int ktype)
{
    double  sigma_x     = sigma;
    double  sigma_y     = sigma/gamma;
    int     nstds       = 3;
    double  kmax        = CV_PI/2;
    double  f           = sqrt(2.0);
    int xmin, xmax, ymin, ymax;
    double c = cos(theta), s = sin(theta);
    if( ksize.width > 0 )
    xmax = ksize.width/2;
    else//这个和matlab中的结果一样，默认都是19 !
    xmax = cvRound(std::max(fabs(nstds*sigma_x*c), fabs(nstds*sigma_y*s)));
    
    if( ksize.height > 0 )
    ymax = ksize.height/2;
    else
    ymax = cvRound(std::max(fabs(nstds*sigma_x*s), fabs(nstds*sigma_y*c)));
    
    xmin = -xmax;
    ymin = -ymax;
    CV_Assert( ktype == CV_32F || ktype == CV_64F );
    float*  pFloat;
    double* pDouble;
    Mat kernel(ymax - ymin + 1, xmax - xmin + 1, ktype);
    double k        =   kmax/pow(f,nu);
    double exy      =   sigma_x*sigma_y/2;
    double scaleReal=   k*k/sigma_x/sigma_y;
    int    x,y;
    for( y = ymin; y <= ymax; y++ )
    {
        if( ktype == CV_32F )
        {
            pFloat = kernel.ptr<float>(ymax-y);
        }
        else
        {
            pDouble = kernel.ptr<double>(ymax-y);
        }
        for( x = xmin; x <= xmax; x++ )
        {
            double xr = x*c + y*s;
            double v = scaleReal*exp(-(x*x+y*y)*scaleReal/2);
            double temp=cos(k*xr) - exp(-exy);
            v   =   temp*v;
            if( ktype == CV_32F )
            {
                pFloat[xmax - x]= (float)v;
            }
            else
            {
                pDouble[xmax - x] = v;
            }
        }
    }
    return kernel;
}

// 获得实部与虚部的L2范式
Mat Gabor::getMagnitude(Mat& real, Mat &imag)
{
    CV_Assert(real.type()==imag.type());
    CV_Assert(real.size()==imag.size());
    int ktype=real.type();
    int row = real.rows,col = real.cols;
    int i,j;
    float*  pFloat,*pFloatR,*pFloatI;
    double* pDouble,*pDoubleR,*pDoubleI;
    Mat     kernel(row, col, real.type());
    for(i=0;i<row;i++)
    {
        if( ktype == CV_32FC1 )
        {
            pFloat = kernel.ptr<float>(i);
            pFloatR= real.ptr<float>(i);
            pFloatI= imag.ptr<float>(i);
        }
        else
        {
            pDouble = kernel.ptr<double>(i);
            pDoubleR= real.ptr<double>(i);
            pDoubleI= imag.ptr<double>(i);
        }
        for(j=0;j<col;j++)
        {
            if( ktype == CV_32FC1 )
            {
                pFloat[j]= sqrt(pFloatI[j]*pFloatI[j]+pFloatR[j]*pFloatR[j]);
            }
            else
            {
                pDouble[j] = sqrt(pDoubleI[j]*pDoubleI[j]+pDoubleR[j]*pDoubleR[j]);
            }
        }
    }
    return kernel;
}

// 获得相位
Mat Gabor::getPhase(Mat &real, Mat &imag)
{
    CV_Assert(real.type()==imag.type());
    CV_Assert(real.size()==imag.size());
    int ktype=real.type();
    int row = real.rows,col = real.cols;
    int i,j;
    float*  pFloat,*pFloatR,*pFloatI;
    double* pDouble,*pDoubleR,*pDoubleI;
    Mat     kernel(row, col, real.type());
    for(i=0;i<row;i++)
    {
        if( ktype == CV_32FC1 )
        {
            pFloat = kernel.ptr<float>(i);
            pFloatR= real.ptr<float>(i);
            pFloatI= imag.ptr<float>(i);
        }
        else
        {
            pDouble = kernel.ptr<double>(i);
            pDoubleR= real.ptr<double>(i);
            pDoubleI= imag.ptr<double>(i);
        }
        for(j=0;j<col;j++)
        {
            if( ktype == CV_32FC1 )
            {
                //              if(pFloatI[j]/(pFloatR[j]+pFloatI[j]) > 0.99)
                //              {
                    //                  pFloat[j]=CV_PI/2;
                //              }
                //              else
                //              {
                    //                  pFloat[j] = atan(pFloatI[j]/pFloatR[j]);
                    pFloat[j] = asin(pFloatI[j]/sqrt(pFloatR[j]*pFloatR[j]+pFloatI[j]*pFloatI[j]));
                /*              }*/
                //              pFloat[j] = atan2(pFloatI[j],pFloatR[j]);
            }//CV_32F
            else
            {
                if(pDoubleI[j]/(pDoubleR[j]+pDoubleI[j]) > 0.99)
                {
                    pDouble[j]=CV_PI/2;
                }
                else
                {
                    pDouble[j] = atan(pDoubleI[j]/pDoubleR[j]);
                }
                //              pDouble[j]=atan2(pDoubleI[j],pDoubleR[j]);
            }//CV_64F
        }
    }
    return kernel;
}

// 获得实部和虚部
void Gabor::getFilterRealImagPart(Mat &src, Mat &real, Mat &imag, Mat &outReal, Mat &outImag)
{
    outReal = getFilter(src, real);
    outImag = getFilter(src, imag);
}

// 获得实部与虚部的L2范数
void Gabor::getFilterMagnitude(Mat &src, Mat &real, Mat &imag, Mat &output)
{
    Mat outReal, outImag;
    getFilterRealImagPart(src, real, imag, outReal, outImag);
    magnitude(outReal, outImag, output);
}

// 调用卷积函数获得结果
Mat Gabor::getFilter(Mat &src, Mat &ker)
{
    CV_Assert(ker.type() == src.type());
    Mat dst, kernel;
    flip(ker, kernel, -1);

    //Mat tmp;
    //filter2D(src, dst, CV_32F, kernel, Point(-1,-1), 0, BORDER_CONSTANT);
    //normalize(ker, tmp, 255, 0, CV_MINMAX, CV_8U);
    //cv::imwrite("filter.jpg", tmp);
    //normalize(dst, tmp, 255, 0, CV_MINUX, CV_8UC1);

    QTime time1 = QTime::currentTime();

#ifdef USE_OPENCV_GPU
    cv::gpu::GpuMat g_src(src);
    cv::gpu::GpuMat g_kernel(kernel);
    cv::gpu::GpuMat g_dst;

    cv::gpu::convolue(g_src, g_kernel, g_dst);
    g_dst.download(dst);
#else
    filter2D(src, dst, CV_32F, kernel, Point(-1,-1), 0, cv::BORDER_REPLICATE);
#endif
    QTime time2 = QTime::currentTime();
    qDebug() << QString("Gpu: ") << time1.msecsTo(time2) << QString("ms for filter");

    return dst;
}

CUDA_Gabor::CUDA_Gabor()
{

}

CUDA_Gabor::~CUDA_Gabor()
{
    checkCudaErrors(cudaFree(d_DataSpectrum));
    checkCudaErrors(cudaFree(d_PaddedData));
    checkCudaErrors(cudaFree(d_PaddedKernel));
    checkCudaErrors(cudaFree(d_Data));
    checkCudaErrors(cudaFree(d_Kernel));

    for (int i = 0; i < 40; i++)
    {
        checkCudaErrors(cudaFree(d_VecRealKernelSpectrum[i]));
        checkCudaErrors(cudaFree(d_VecImagKernelSpectrum[i]));
    }

    free(h_ResultGPU);
    free(h_Data);
    free(h_Kernel);
}

void CUDA_Gabor::Init(Size ksize, double sigma, double gamma, int ktype)
{
    gaborRealKernels.clear();
    gaborImagKernels.clear();
    d_VecRealKernelSpectrum.clear();
    d_VecImagKernelSpectrum.clear();
    double mu[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    double nu[5] = {0, 1, 2, 3, 4};

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            gaborRealKernels.push_back(getRealGaborKernel(ksize, sigma, mu[j]*CV_PI/8+CV_PI/2, nu[i], gamma, ktype));
            gaborImagKernels.push_back(getImagGaborKernel(ksize, sigma, mu[j]*CV_PI/8+CV_PI/2, nu[i], gamma, ktype));
        }
    }
    
    kernelH = ksize.height;
    kernelW = ksize.width;
    kernelY = (ksize.height - 1) / 2;
    kernelX = (ksize.width  - 1) / 2;
    dataH = 200;
    dataW = 200;
    fftH = snapTransformSize(dataH + kernelH - 1);
    fftW = snapTransformSize(dataW + kernelW - 1);

	qDebug("kernelH: %d, kernelW: %d, kernelY: %d, kernelX: %d, fftH: %d, fftW: %d", kernelH, kernelW, kernelY, kernelX, fftH, fftW);

    h_Data      = (float *)malloc(dataH   * dataW * sizeof(float));
    h_Kernel    = (float *)malloc(kernelH * kernelW * sizeof(float));
    h_ResultGPU = (float *)malloc(fftH    * fftW * sizeof(float));

    // GPU中的kernel以及PadderKernel初始化
    checkCudaErrors(cudaMalloc((void **)&d_Kernel, kernelH * kernelW * sizeof(float)));
    checkCudaErrors(cudaMalloc((void **)&d_PaddedKernel,   fftH * fftW * sizeof(float)));

    // GPU中的DATA以及PadderData初始化
    checkCudaErrors(cudaMalloc((void **)&d_Data,   dataH   * dataW   * sizeof(float)));
    checkCudaErrors(cudaMalloc((void **)&d_PaddedData,   fftH * fftW * sizeof(float)));
    checkCudaErrors(cudaMalloc((void **)&d_DataSpectrum,   fftH * (fftW / 2 + 1) * sizeof(fComplex)));

    // 对FFT变换初始化
    checkCudaErrors(cufftPlan2d(&fftPlanFwd, fftH, fftW, CUFFT_R2C));
    checkCudaErrors(cufftPlan2d(&fftPlanInv, fftH, fftW, CUFFT_C2R));

    // 对每一个kernel计算出一个k_KernelSpectrum来
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            Prepare_GPU_PadderKernel(i*8+j, true);
            Prepare_GPU_PadderKernel(i*8+j, false);            
        }
    }
    isInited = true;
}

void CUDA_Gabor::Prepare_GPU_PadderKernel(int index, bool isReal)
{
    Mat kernel;
    if (isReal)
        kernel = gaborRealKernels[index];
    else
        kernel = gaborImagKernels[index];

    // 在CPU中取出指定kernel
    for (int i = 0; i < kernelH; i++)
    {
        float *Mi = kernel.ptr<float>(i);
        for (int j = 0; j < kernelW; j++)
            h_Kernel[i*kernelW+j] = Mi[j];
    }

    checkCudaErrors(cudaMalloc((void **)&d_KernelSpectrum, fftH * (fftW / 2 + 1) * sizeof(fComplex)));
    checkCudaErrors(cudaMemcpy(d_Kernel, h_Kernel, kernelH * kernelW * sizeof(float), cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemset(d_PaddedKernel, 0, fftH * fftW * sizeof(float)));

    // 在GPU中操作PadderKernel
    padKernel(
        d_PaddedKernel,
        d_Kernel,
        fftH,
        fftW,
        kernelH,
        kernelW,
        kernelY,
        kernelX
        );

    checkCudaErrors(cufftExecR2C(fftPlanFwd, (cufftReal *)d_PaddedKernel, (cufftComplex *)d_KernelSpectrum));
    checkCudaErrors(cudaDeviceSynchronize());

    if (isReal)
        d_VecRealKernelSpectrum.push_back(d_KernelSpectrum);
    else
        d_VecImagKernelSpectrum.push_back(d_KernelSpectrum);
}

void CUDA_Gabor::Prepare_GPU_PadderData(Mat &src)
{
    for (int i = 0; i < dataH; i++)
    {
        float *Mi = src.ptr<float>(i);
        for (int j = 0; j < dataW; j++)
            h_Data[i*dataW+j] = Mi[j];
    }

    checkCudaErrors(cudaMemcpy(d_Data,   h_Data,   dataH   * dataW *   sizeof(float), cudaMemcpyHostToDevice));
    checkCudaErrors(cudaMemset(d_PaddedData,   0, fftH * fftW * sizeof(float)));

    padDataClampToBorder(
        d_PaddedData,
        d_Data,
        fftH,
        fftW,
        dataH,
        dataW,
        kernelH,
        kernelW,
        kernelY,
        kernelX
        );

    checkCudaErrors(cufftExecR2C(fftPlanFwd, (cufftReal *)d_PaddedData, (cufftComplex *)d_DataSpectrum));
    checkCudaErrors(cudaDeviceSynchronize());
}

Mat CUDA_Gabor::getFilter(Mat &src, int index, bool isReal)
{
    if (!src.empty())
        Prepare_GPU_PadderData(src);

    StopWatchInterface *hTimer = NULL;
    sdkCreateTimer(&hTimer);
    sdkStartTimer(&hTimer);

    if (isReal)
        modulateAndNormalize(d_DataSpectrum, d_VecRealKernelSpectrum[index], fftH, fftW, 1);
    else
        modulateAndNormalize(d_DataSpectrum, d_VecImagKernelSpectrum[index], fftH, fftW, 1);
    checkCudaErrors(cufftExecC2R(fftPlanInv, (cufftComplex *)d_DataSpectrum, (cufftReal *)d_PaddedData));
    checkCudaErrors(cudaDeviceSynchronize());
    checkCudaErrors(cudaMemcpy(h_ResultGPU, d_PaddedData, fftH * fftW * sizeof(float), cudaMemcpyDeviceToHost));

    Mat result = Mat(dataH, dataW, CV_32F);
    for (int i = 0; i < dataH; i++)
    {
        float *Mi = result.ptr<float>(i);
        for (int j = 0; j < dataW; j++)
            Mi[j] = h_ResultGPU[i*fftW+j];
    }
    sdkStopTimer(&hTimer);
    double gpuTime = sdkGetTimerValue(&hTimer);
    qDebug("%f", gpuTime);

    return result;
}

// 获得实部与虚部的L2范数
void CUDA_Gabor::getFilterMagnitude(Mat &src, int index, Mat &output)
{
    Mat outReal, outImag;
    outReal = getFilter(src, index, true);
    outImag = getFilter(src, index, false);
    magnitude(outReal, outImag, output);
}

Mat printGabor()
{
    qDebug("Starting GaborFilter...");
    if (frame.cols == 0 || frame.rows == 0)
    {
        qDebug("No face detected");
        return Mat();
    }
    qDebug("rows: %d, cols: %d", frame.rows, frame.cols);
    Mat mat_tmp = frame.clone();
    cv::resize(mat_tmp, mat_tmp, Size(RESIZE_WIDTH, RESIZE_HEIGHT));

    int top_buttom = mat_tmp.rows * BORDER_FRAC * 0.335;
    int left_right = mat_tmp.cols * BORDER_FRAC * 0.335;
    copyMakeBorder(mat_tmp, mat_tmp, top_buttom, 0, left_right, 0, cv::BORDER_REPLICATE);

    mat_tmp.convertTo(mat_tmp, CV_32F);
    normalize(mat_tmp, mat_tmp, 1, 0, CV_MINMAX, CV_32F);
    
    if (!gabor_cuda_g.isInited)
        gabor_cuda_g.Init(Size(iSize, iSize), sqrt(2.0), 1, CV_32F);
    gabor_cuda_g.Prepare_GPU_PadderData(mat_tmp);

    Mat feats[40];
    Mat blank;
    int index = 0;
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            index = i*8+j;
            gabor_cuda_g.getFilterMagnitude(blank, index, feats[index]);
            cv::pow(feats[index], 2, feats[index]);
        }
    }

    qDebug("Starting add...");
    mat_tmp = feats[0].clone();
    for (int i = 1; i < 40; ++i)
        cv::add(mat_tmp, feats[i], mat_tmp);

    qDebug("Starting pow and normalize...");
    cv::pow(mat_tmp, 0.5, mat_tmp);
    cv::normalize(mat_tmp, mat_tmp, 0, 255, CV_MINMAX, CV_8UC1);
    cv::resize(mat_tmp, mat_tmp, Size(RESIZE_WIDTH, RESIZE_HEIGHT));
    cv::imwrite("out.jpg", mat_tmp);
    return mat_tmp;
}

/**
* 输出对应相位和方向的Gabor特征的实部与虚部的L2范式
* @param  方向
* @param  尺度
* @return
*/
Mat printGabor_(Mat& m, Gabor& gabor, int mu, int nu)
{
    Mat tmp, output;
    if (m.rows == 0 || m.cols == 0)
    {
        return m;
    }

    if (!gabor.isInited)
    {
        gabor.Init(Size(iSize, iSize), sqrt(2.0), 1, CV_32F);
    }
    m.convertTo(tmp, CV_32F);
    normalize(tmp, tmp, 1, 0, CV_MINMAX, CV_32F);

    gabor.getFilterMagnitude(tmp, gabor.gaborRealKernels[nu*8+mu], gabor.gaborImagKernels[nu*8+mu], output);

    return output;
}


/**
 * 输出相关参数
 */
void printGaborPara()
 {
    Gabor gabor;
    gabor.Init(Size(iSize, iSize), sqrt(2.0), 1, CV_32F);
    for (int i = 0; i < 5; i++)  // 尺度
    {
        for (int j = 0; j < 8; j++)  // 方向
        {
            Mat tmp;
            char p[SLEN];
            sprintf(p, "real_gabor_kernel_%d_%d.jpg", i, j);
            normalize(gabor.gaborRealKernels[i*8+j], tmp, 255, 0, CV_MINMAX, CV_8U);
            imwrite(p, tmp);

            sprintf(p, "imag_gabor_kernel_%d_%d.jpg", i, j);
            normalize(gabor.gaborImagKernels[i*8+j], tmp, 255, 0, CV_MINMAX, CV_8U);
            imwrite(p,tmp);

            sprintf(p, "magnitude_%d_%d.jpg", i, j);
            normalize(gabor.getMagnitude(gabor.gaborRealKernels[i*8+j], gabor.gaborImagKernels[i*8+j]), tmp, 255, 0, CV_MINMAX, CV_8U);
            imwrite(p, tmp);

            sprintf(p, "phase_%d_%d.jpg", i, j);
            normalize(gabor.getPhase(gabor.gaborRealKernels[i*8+j], gabor.gaborImagKernels[i*8+j]), tmp, 255, 0, CV_MINMAX, CV_8U);
            imwrite(p, tmp);
        }
    }
 }