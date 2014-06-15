// otherdet.cpp：使用Haar检测其他人脸部件
//
// Created by Vincent yan in 2014/04/20

#include "otherdet.h"
#include "haarclassifier.h"

/**
 * 嘴和鼻子的两个级联分类器
 */
static cv::CascadeClassifier mouth_det_g;
static cv::CascadeClassifier nose_det_g;

//static cv::gpu::CascadeClassifier_GPU mouth_det_gpu_g;
//static cv::gpu::CascadeClassifier_GPU nose_det_gpu_g;

// 数据流中的人脸信息
extern DetPar frame_detpar;

// 数据流中的Mat
extern Mat frame;

/**
 * 按矩形的大小降序排列
 * @param  rect1 in: Rect1
 * @param  rect2 in: Rect2
 * @return       true  if rect1.width <  rect2.width
 *               false if rect2.width >= rect2.width
 */
static bool DecreasingWidth(const Rect &rect1, const Rect &rect2)
{
    return 1e5 * rect1.width > 1e5 * rect2.width;
}

/**
 * 在一组类似Mouth的Rect中选出最合适的一个
 * @param imouth_best in/out: 最合适的index
 * @param mouths      in: 类似Mouth的Rect组
 * @param mouths_rect in: 检测的范围
 */
static void SelectMouths(int &imouth_best, vec_Rect *mouths, const Rect &mouths_rect)
{
    // 目前来说选取最大的方框，作为嘴的Rect
    sort(mouths->begin(), mouths->end(), DecreasingWidth);

    if (mouths->empty())
        imouth_best = -1;
    else
        imouth_best = 0;
}

/**
 * 检测Mouth的程序
 * @param img in/out: 源图像
 */
void DetectMouth()
{
    OpenDetector(&mouth_det_g, "Mouth.xml");
    //OpenDetector(&mouth_det_gpu_g, "Mouth.xml");

    // 把人脸的区域勾出来
    Rect facerect(cvRound(frame_detpar.x - frame_detpar.width/2),
                  cvRound(frame_detpar.y - frame_detpar.height/2),
                  cvRound(frame_detpar.width),
                  cvRound(frame_detpar.height));

    // 初始化嘴的参数
    frame_detpar.mouthx = frame_detpar.mouthy = INVALID;
    vec_Rect mouths;
    int imouth_best = -1;

    if (!mouth_det_g.empty())
    {
        // 脸下部的区域
        Rect mouths_rect;
        mouths_rect.x       += cvRound(.1 * facerect.width);
        mouths_rect.y       += cvRound(.65 * facerect.height);
        mouths_rect.width   += cvRound(.8 * facerect.width);
        mouths_rect.height  += cvRound(.3 * facerect.height);

        // 输出所有预选的嘴部列表
        //#ifdef USE_OPENCV_GPU_DETECTION
        //mouths = Detect(frame, &mouth_det_gpu_g, &mouths_rect, facerect.width/10);
        //#else
        mouths = Detect(frame, &mouth_det_g, &mouths_rect, facerect.width/10);
        //#endif

        // 在所有嘴部中选取最合适的
        SelectMouths(imouth_best, &mouths, mouths_rect);

        // 把嘴部框的坐标映射到人脸框
        if (imouth_best >= 0)
		{
            RectToImgFrame(frame_detpar.mouthx, frame_detpar.mouthy, mouths[imouth_best]);
			#ifdef ALLOWPOINT
			circle(frame, Point(mouths[0].x+mouths[0].width/2, mouths[0].y+mouths[0].height/2), 1, Scalar(255, 255, 255));
			#endif
		}

        // 画出嘴巴
		#ifdef ALLOWPOINT
         rectangle(frame, mouths[0], Scalar(0, 0, 255));
		 #endif
    }
}

/**
 * 检测Nose
 */
void DetectNose()
{
    OpenDetector(&nose_det_g, "Nose.xml");
    //OpenDetector(&nose_det_gpu_g, "Nose.xml");
    // 把人脸的区域勾出来
    Rect facerect(cvRound(frame_detpar.x - frame_detpar.width/2),
                  cvRound(frame_detpar.y - frame_detpar.height/2),
                  cvRound(frame_detpar.width),
                  cvRound(frame_detpar.height));

    // 初始化嘴的参数
    frame_detpar.nosex = frame_detpar.nosey = INVALID;
    vec_Rect noses;
    int inose_best = -1;
    if (!nose_det_g.empty())
    {
        // 脸中部的区域
        Rect noses_rect;
        noses_rect.x       += cvRound(.2 * facerect.width);
        noses_rect.y       += cvRound(.4 * facerect.height);
        noses_rect.width   += cvRound(.6 * facerect.width);
        noses_rect.height  += cvRound(.4 * facerect.height);

        // 输出所有预选的嘴部列表
        //#ifdef USE_OPENCV_GPU_DETECTION
        //noses = Detect(frame, &nose_det_gpu_g, &noses_rect, facerect.width/10);
        //#else
        noses = Detect(frame, &nose_det_g, &noses_rect, facerect.width/10);
        //#endif
	
        // 在所有嘴部中选取最合适的
        SelectMouths(inose_best, &noses, noses_rect);

		#ifdef ALLOWPOINT
		for (int i = 0; i < noses.size(); i++)
		{
			rectangle(frame, noses[i], Scalar(255, 255, 255));
		}
		#endif
        // 把嘴部框的坐标映射到人脸框
        if (inose_best >= 0)
		{
            RectToImgFrame(frame_detpar.nosex, frame_detpar.nosey, noses[inose_best]);
			#ifdef ALLOWPOINT
			circle(frame, Point(noses[0].x+noses[0].width/2, noses[0].y+noses[0].height/2), 1, Scalar(255, 255, 255));
			#endif
		}
		
    }
}