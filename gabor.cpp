// gabor.cpp: Gabor滤波
//
// Created by Vincent Yan in 2014/03/25

#include "gabor.h"
#include "conv2d.h"

// source中的每一帧
extern Mat frame;

static const int iSize = 101;

float *h_Data, *h_Kernel, *h_ResultGPU;
float *d_Data, *d_PaddedData, *d_Kernel, *d_PaddedKernel;
fComplex *d_DataSpectrum, *d_KernelSpectrum;
cufftHandle fftPlanFwd, fftPlanInv;

int kernelH;
int kernelW;
int kernelY;
int kernelX;
int dataH;
int dataW;
int fftH;
int fftW;

Mat conv2d_Cuda(Mat& kernel, Mat& img)
{
	StopWatchInterface *hTimer = NULL;
	sdkCreateTimer(&hTimer);
    sdkStartTimer(&hTimer);
    
	for (int i = 0; i < kernelH; i++)
	{
		float *Mi = kernel.ptr<float>(i);
		for (int j = 0; j < kernelW; j++)
			h_Kernel[i*kernelW+j] = Mi[j];
	}
	
	for (int i = 0; i < dataH; i++)
	{
		float *Mi = img.ptr<float>(i);
		for (int j = 0; j < dataW; j++)
			h_Data[i*dataW+j] = Mi[j];
	}

	checkCudaErrors(cudaMemcpy(d_Kernel, h_Kernel, kernelH * kernelW * sizeof(float), cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy(d_Data,   h_Data,   dataH   * dataW *   sizeof(float), cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemset(d_PaddedKernel, 0, fftH * fftW * sizeof(float)));
	checkCudaErrors(cudaMemset(d_PaddedData,   0, fftH * fftW * sizeof(float)));

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

	checkCudaErrors(cufftExecR2C(fftPlanFwd, (cufftReal *)d_PaddedKernel, (cufftComplex *)d_KernelSpectrum));
	checkCudaErrors(cudaDeviceSynchronize());
	// sdkResetTimer(&hTimer);
	// sdkStartTimer(&hTimer);
	checkCudaErrors(cufftExecR2C(fftPlanFwd, (cufftReal *)d_PaddedData, (cufftComplex *)d_DataSpectrum));
	modulateAndNormalize(d_DataSpectrum, d_KernelSpectrum, fftH, fftW, 1);
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


/**
* ctr
*/
Gabor::Gabor()
{
    isInited = false;
}

Gabor::~Gabor()
{
    checkCudaErrors(cufftDestroy(fftPlanInv));
    checkCudaErrors(cufftDestroy(fftPlanFwd));

    checkCudaErrors(cudaFree(d_DataSpectrum));
    checkCudaErrors(cudaFree(d_KernelSpectrum));
    checkCudaErrors(cudaFree(d_PaddedData));
    checkCudaErrors(cudaFree(d_PaddedKernel));
    checkCudaErrors(cudaFree(d_Data));
    checkCudaErrors(cudaFree(d_Kernel));

    free(h_ResultGPU);
    free(h_Data);
    free(h_Kernel);
}

// 进行初始化
void Gabor::Init(Size ksize, double sigma, double gamma, int ktype)
{
    #ifdef USE_CUDA
    kernelH = ksize.height;
    kernelW = ksize.width;
    kernelY = (ksize.height - 1) / 2;
    kernelX = (ksize.width - 1) / 2;
      dataH = 200;
      dataW = 200;
       fftH = snapTransformSize(dataH + kernelH - 1);
       fftW = snapTransformSize(dataW + kernelW - 1);

    h_Data      = (float *)malloc(dataH   * dataW * sizeof(float));
    h_Kernel    = (float *)malloc(kernelH * kernelW * sizeof(float));
    h_ResultGPU = (float *)malloc(fftH    * fftW * sizeof(float));

    checkCudaErrors(cudaMalloc((void **)&d_Data,   dataH   * dataW   * sizeof(float)));
    checkCudaErrors(cudaMalloc((void **)&d_Kernel, kernelH * kernelW * sizeof(float)));

    checkCudaErrors(cudaMalloc((void **)&d_PaddedData,   fftH * fftW * sizeof(float)));
    checkCudaErrors(cudaMalloc((void **)&d_PaddedKernel, fftH * fftW * sizeof(float)));

    checkCudaErrors(cudaMalloc((void **)&d_DataSpectrum,   fftH * (fftW / 2 + 1) * sizeof(fComplex)));
    checkCudaErrors(cudaMalloc((void **)&d_KernelSpectrum, fftH * (fftW / 2 + 1) * sizeof(fComplex)));

    checkCudaErrors(cufftPlan2d(&fftPlanFwd, fftH, fftW, CUFFT_R2C));
    checkCudaErrors(cufftPlan2d(&fftPlanInv, fftH, fftW, CUFFT_C2R));

    #endif
    gaborRealKernels.clear();
    gaborImagKernels.clear();
    double mu[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    double nu[5] = {0, 1, 2, 3, 4};
    
    for (int i = 0; i < 5; ++i)	// 尺度
    {
        for (int j = 0; j < 8; ++j) // 方向
        {
			// mu是方向，nu是尺度
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

// 获得实部
Mat Gabor::getFilterRealPart(Mat& src,Mat& real)
{
    CV_Assert(real.type()==src.type());
    Mat dst;
    Mat kernel;

	#ifdef USE_CUDA
    QTime time5 = QTime::currentTime();
	dst = conv2d_Cuda(real, src);
    QTime time6 = QTime::currentTime();
    qDebug() << QString("CUDA: ") << time5.msecsTo(time6) << QString("ms for ImagPart in one frame");

	#else
		flip(real,kernel,-1);//中心镜面
		//Mat tmp;
		//  filter2D(src,dst,CV_32F,kernel,Point(-1,-1),0,BORDER_CONSTANT);
		//normalize(real, tmp, 255, 0, CV_MINMAX, CV_8U);
		//cv::imwrite("filter.jpg", tmp);
		#ifdef USE_OPENCV_GPU
		QTime time3 = QTime::currentTime();
		cv::gpu::GpuMat g_src(src);
		cv::gpu::GpuMat g_tmp(kernel);
		cv::gpu::GpuMat g_dst;
		
		cv::gpu::convolve(g_src, g_tmp, g_dst);
		QTime time4 = QTime::currentTime();
		qDebug() << QString("Gpu: ") << time3.msecsTo(time4) << QString("ms for ImagPart in one frame");
		g_dst.download(dst);
		//normalize(dst, tmp, 255, 0, CV_MINMAX, CV_8UC1);
		//cv::imwrite("result.jpg", tmp);
    
		#else

		QTime time1 = QTime::currentTime();
		filter2D(src,dst,CV_32F,kernel,Point(-1,-1),0,cv::BORDER_REPLICATE);
		QTime time2 = QTime::currentTime();
		qDebug() << QString("Cpu: ") << time1.msecsTo(time2) << QString("ms for ImagPart in one frame");

		#endif
    
	#endif
    return dst;
}

// 获得虚部
Mat Gabor::getFilterImagPart(Mat& src,Mat& imag)
{
    CV_Assert(imag.type()==src.type());
    Mat dst;
    Mat kernel;
    
	#ifdef USE_CUDA
    QTime time5 = QTime::currentTime();
	dst = conv2d_Cuda(imag, src);
    QTime time6 = QTime::currentTime();
    qDebug() << QString("CUDA: ") << time5.msecsTo(time6) << QString("ms for ImagPart in one frame");

	#else
		flip(imag,kernel,-1);//中心镜面
		//  filter2D(src,dst,CV_32F,kernel,Point(-1,-1),0,BORDER_CONSTANT);
		#ifdef USE_OPENCV_GPU

		QTime time3 = QTime::currentTime();
		cv::gpu::GpuMat g_src(src);
		cv::gpu::GpuMat g_tmp(kernel);
		cv::gpu::GpuMat g_dst;
		
		cv::gpu::convolve(g_src, g_tmp, g_dst);
		QTime time4 = QTime::currentTime();
		qDebug() << QString("Gpu: ") << time3.msecsTo(time4) << QString("ms for ImagPart in one frame");
		g_dst.download(dst);
    
		#else
    
		QTime time1 = QTime::currentTime();
		filter2D(src,dst,CV_32F,kernel,Point(-1,-1),0,cv::BORDER_REPLICATE);
		QTime time2 = QTime::currentTime();
		qDebug() << QString("Cpu: ") << time1.msecsTo(time2) << QString("ms for ImagPart in one frame");

		#endif
	#endif
    
    return dst;
}

// 获得实部与虚部
void Gabor::getFilterRealImagPart(Mat& src,Mat& real,Mat& imag,Mat &outReal,Mat &outImag)
{
    outReal=getFilterRealPart(src,real);
    outImag=getFilterImagPart(src,imag);
}

/**
* 输出对应相位和方向的Gabor特征的实部与虚部的L2范式
* @param  方向
* @param  尺度
* @return
*/
Mat printGabor(Gabor& gabor, int mu, int nu)
{
    if (frame.rows == 0 || frame.cols == 0)
    {
        return frame;
    }
    
    if (!gabor.isInited)
    {
        gabor.Init(Size(iSize, iSize), CV_PI*2, 1, CV_32F);
    }
    
    //imwrite("before normalize.jpg", frame);
    frame.convertTo(frame, CV_32F);
    normalize(frame, frame, 1, 0, CV_MINMAX, CV_32F);
    
    Mat out_real, out_imag, output;
    gabor.getFilterRealImagPart(frame, gabor.gaborRealKernels[nu*mu], gabor.gaborImagKernels[nu*mu], out_real, out_imag);
    
    magnitude(out_real, out_imag, output);
    
    normalize(output, output, 255, 0, CV_MINMAX, CV_8UC1);
    return output;
}

/**
* 输出对应相位和方向的Gabor特征的实部与虚部的L2范式
* @param  方向
* @param  尺度
* @return
*/
Mat printGabor_(Mat& m, Gabor& gabor, int mu, int nu)
{
	if (m.rows == 0 || m.cols == 0)
	{
		return m;
	}

	if (!gabor.isInited)
	{
		gabor.Init(Size(iSize, iSize), CV_PI*2, 1, CV_32F);
	}

	////imwrite("before normalize.jpg", m);
	m.convertTo(m, CV_32F);
	normalize(m, m, 1, 0, CV_MINMAX, CV_32F);

	Mat out_real, out_imag, output;
	gabor.getFilterRealImagPart(m, gabor.gaborRealKernels[nu*8+mu], gabor.gaborImagKernels[nu*8+mu], out_real, out_imag);

	magnitude(out_real, out_imag, output);

	normalize(output, output, 255, 0, CV_MINMAX, CV_8UC1);
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