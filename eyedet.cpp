// eyedet.cpp: 使用Haar人眼检测
// 
// Created by Vincent Yan in 2014/04/04
// Modefied from stasm

#include "eyedet.h"

// 数据流中的人脸信息
extern DetPar frame_detpar;

/**
 * 左右眼的两个级联分类器
 */
static cv::CascadeClassifier leye_det_g;    // left eye detector
static cv::CascadeClassifier reye_det_g;    // right eye detector

//static cv::gpu::CascadeClassifier_GPU leye_det_gpu_g;   // left eye gpu detector
//static cv::gpu::CascadeClassifier_GPU reye_det_gpu_g;   // right eye gpu detector

/**
 * 检测左右眼是否水平交叉10%，或者左右眼间隔大于left.width
 * @param  left  in：左眼
 * @param  right in：右眼
 * @return
 */
static bool IsEyeHorizOk(const Rect& left, const Rect& right)
{
    return left.x + left.width - right .x   <= .1 * left.width &&
           right.x - (left.x + left.width)  <= left.width; 
}

/**
 * 检测左右眼是否垂直上交叉
 * @param  left  in：左眼
 * @param  right in：右眼
 * @return
 */
static bool VerticalOverlap(const Rect& left, const Rect& right)
{
    const int topleft = left.y + left.height;
    const int topright = left.y + right.height;

    return (left.y   >= right.y && left.y   <= right.y + right.height) ||
           (topleft  >= right.y && topleft  <= right.y + right.height) ||
           (right.y  >= left.y  && right.y  <= left.y  + left.height)  ||
           (topright >= left.y  && topright <= left.y  + left.height);
}

/**
 * 编译单元内部函数，用来检测所有左右眼
 * @param leyes    out: 检测到的可能左眼的列表
 * @param reyes    out：检测到的可能右眼的列表
 * @param img      in：源图像
 * @param facerect in：脸部区域
 */
static void DetectAllEyes(vec_Rect& leyes, vec_Rect& reyes, const Image& img, const Rect& facerect)
{
    CV_Assert(!leye_det_g.empty());
    CV_Assert(!reye_det_g.empty());
    //CV_Assert(!leye_det_gpu_g.empty());
    //CV_Assert(!reye_det_gpu_g.empty());

    // 对于左脸，选取脸部左边2/3的区域进行检测
    int width = facerect.width;
    Rect leftrect = facerect;
    leftrect.width -= width / 3;
    leftrect.height = cvRound(.6 * facerect.height);
    leftrect.width = MAX(0, leftrect.width);

    if (leftrect.width)
    {
        //#ifdef USE_OPENCV_GPU_DETECTION
        //leyes = Detect(img, &leye_det_gpu_g, &leftrect, facerect.width/10);
        //#else
        leyes = Detect(img, &leye_det_g, &leftrect, facerect.width/10);
        //#endif
    }

    // 对于右脸，选取脸部右边2/3的区域进行检测
    Rect rightrect = facerect;
    rightrect.x += width / 3;
    rightrect.width -= width / 3;
    rightrect.height = cvRound(.6 * facerect.height);
    rightrect.width = MAX(0, rightrect.width);

    if (rightrect.width)
    {
        //#ifdef USE_OPENCV_GPU_DETECTION
        //reyes = Detect(img, &reye_det_gpu_g, &rightrect, facerect.width/10);
        //#else
        reyes = Detect(img, &reye_det_g, &rightrect, facerect.width/10);
        //#endif
    }

}

/**
 * 编译单元内部函数，用来选择最合适的左右眼
 * @param ileft_best     out：最合适的左眼的索引
 * @param iright_best    out：最合适的右眼的索引
 * @param leyes          in：所有可能左眼的列表
 * @param reyes          in：所有可能右眼的列表
 * @param eye_inner_rect in：脸上部的小块区域
 */
static void SelectEyes(int &ileft_best, int &iright_best, const vec_Rect &leyes, const vec_Rect &reyes, const Rect &eye_inner_rect)
{
    // 默认左右眼的索引
    ileft_best = iright_best = -1;
    // 最小的左右眼间隔
    int min_intereye = cvRound(.25 * eye_inner_rect.width);
    // 左右眼最大距离，最大为合适
    int maxwidth = 0;
    int ileft, iright;
    Rect left, right;

    // 几点判别：
    // 1，左右眼是否在脸上部的小块区域
    // 2，左右眼是否
    for (ileft = 0; ileft < NSIZE(leyes); ileft++)
    {
        left = leyes[ileft];
        if (InRect(left, eye_inner_rect))
        {
            for (iright = 0; iright < NSIZE(reyes); iright++)
            {
                right = reyes[iright];
                if (InRect(right, eye_inner_rect) &&
                    IsEyeHorizOk(left, right) &&
                    right.x - left.x >= min_intereye &&
                    VerticalOverlap(left, right))
                {
                    int total_width = left.width + right.width;
                    if (total_width > maxwidth)
                    {
                        maxwidth = total_width;
                        ileft_best = ileft;
                        iright_best = iright;
                    }
                }
            }
        }
    }

    if (ileft_best == -1 && iright_best == -1)
    {
        int max_left_width = 0;
        for (ileft = 0; ileft < NSIZE(leyes); ileft++)
        {
            left = leyes[ileft];
            if (InRect(left, eye_inner_rect))
            {
                if (left.width > max_left_width)
                {
                    max_left_width = left.width;
                    ileft_best = ileft;
                }
            }
        }
        int max_right_width = 0;
        for (iright = 0; iright < NSIZE(reyes); iright++)
        {
            right = reyes[iright];
            if (InRect(right, eye_inner_rect))
            {
                if (right.width > max_right_width)
                {
                    max_right_width = right.width;
                    iright_best = iright;
                }
            }
        }

        if (ileft_best != -1 && iright_best != -1)
        {
            left = leyes[ileft_best];
            right = reyes[iright_best];
            if (!IsEyeHorizOk(left, right) || right.x - left.x < min_intereye)
            {
                if (max_right_width > max_left_width)
                    ileft_best = -1;
                else
                    iright_best = -1;
            }
        }
    }
}

/**
 * 使用两个级联分类器检测人眼
 * @param frame_detpar 参数集
 * @param frame  
 */
void DetectEyes(const Mat &img)
{
    if (leye_det_g.empty())
        OpenDetector(&leye_det_g, "haarcascade_mcs_lefteye.xml");

    if (reye_det_g.empty())
        OpenDetector(&reye_det_g, "haarcascade_mcs_righteye.xml");

    //if (leye_det_gpu_g.empty())
        //OpenDetector(&leye_det_gpu_g, "haarcascade_mcs_lefteye.xml");

    //if (reye_det_gpu_g.empty())
        //OpenDetector(&reye_det_gpu_g, "haarcascade_mcs_righteye.xml");

    // 把人脸的区域勾出来
    Rect facerect(cvRound(frame_detpar.x - frame_detpar.width/2),
                  cvRound(frame_detpar.y - frame_detpar.height/2),
                  cvRound(frame_detpar.width),
                  cvRound(frame_detpar.height));

    // 初始化人眼的参数
    frame_detpar.lex = frame_detpar.ley = INVALID;
    frame_detpar.rex = frame_detpar.rey = INVALID;
    vec_Rect leyes, reyes;
    int ileft_best = -1, iright_best = -1;
    if (!leye_det_g.empty() && !reye_det_g.empty())
    {
        // 输出所有预选的左右眼的列表
        DetectAllEyes(leyes, reyes, img, facerect);

        // 脸上部的小块区域
        Rect eyesRect;
        eyesRect.x      += cvRound(.1 * facerect.width);
        eyesRect.width  += cvRound(.8 * facerect.width);
        eyesRect.y      += cvRound(.2 * facerect.height);
        eyesRect.height += cvRound(.28 * facerect.height);

        // 在所有左右眼列表中选取最合适的
        SelectEyes(ileft_best, iright_best, leyes, reyes, eyesRect);

        // 把人眼框的坐标映射到人脸框
        if (ileft_best >= 0)
            RectToImgFrame(frame_detpar.lex,
                frame_detpar.ley, leyes[ileft_best]);

        if (iright_best >= 0)
            RectToImgFrame(frame_detpar.rex,
                frame_detpar.rey, reyes[iright_best]);
    }
}