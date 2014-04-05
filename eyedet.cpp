// eyedet.cpp: 使用Haar人眼检测
// 
// Created by Vincent Yan in 2014/04/04
// Modefied from stasm

#include "eyedet.h"

/**
 * 左右眼的两个级联分类器
 */
static cv::CascadeClassifier leye_det_g;    // left eye detector
static cv::CascadeClassifier reye_det_g;    // right eye detector

/**
 * 分类器参数
 */
static const double EYE_SCALE_FACTOR    = 1.2;
static const int    EYE_MIN_NEIGHBORS   = 3;
static const int    EYE_DETECTOR_FLAGS  = 0;

/**
 * 前向申明
 */
static void DetectAllEyes(vec_Rect& leyes, vec_Rect& reyes, const Image& img, const Rect& facerect);
static void SelectEyes(int &ileft_best, int &iright_best, const vec_Rect &leyes, const vec_Rect &reyes, const Rect &eye_inner_rect);
static bool IsEyeHorizOk(const Rect& left, const Rect& right);
static bool VerticalOverlap(const Rect& left, const Rect& right);
static bool InRect(const Rect& rect, const Rect& enclosing);
static void RectToImgFrame(double &x, double &y, const Rect &featrect);
static vec_Rect Detect(const Image &img, cv::CascadeClassifier *cascade, const Rect *searchrect, int minwidth_pixels);
static void ForceRectIntoImg(Rect &rect, const Image &img);
static void ForceRectIntoImg(int &ix, int &iy, int &ncols, int &nrows, const Image &img);
static void DiscountSearchRegion(vec_Rect &feats, Rect &searchrect);

/**
 * 打开两个级联分类器
 * @param datadir 分类器数据所在的地址
 */
void OpenEyeDetectors(const char* datadir)
{
    char left_eye_filename[SLEN] = "haarcascade_mcs_lefteye.xml";

    if (leye_det_g.empty())
    {
        char dir[SLEN];
        strcpy(dir, datadir);
        ConvertBackslashesToForwardAndStripFinalSlash(dir);

        char path[SLEN];
        sprintf(path, "%s/%s", dir, left_eye_filename);
        qDebug("Open %s/%s \n", dir, left_eye_filename);

        if (!leye_det_g.load(path))
            qDebug("Cannot load %s/%s \n", dir, left_eye_filename);
    }

    char rigth_eye_filename[SLEN] = "haarcascade_mcs_righteye.xml";

    if (reye_det_g.empty())
    {
        char dir[SLEN];
        strcpy(dir, datadir);
        ConvertBackslashesToForwardAndStripFinalSlash(dir);

        char path[SLEN];
        sprintf(path, "%s/%s", dir, rigth_eye_filename);
        qDebug("Open %s/%s \n", dir, rigth_eye_filename);

        if (!reye_det_g.load(path))
            qDebug("Cannot load %s/%s \n", dir, rigth_eye_filename);
    }
}

/**
 * 使用两个级联分类器检测人眼
 * @param detpar 参数集
 * @param frame  
 */
void DetectEyes(DetPar& detpar, const Mat& img)
{
    // 把人脸的区域勾出来
    Rect facerect(cvRound(detpar.x - detpar.width/2),
                  cvRound(detpar.y - detpar.height/2),
                  cvRound(detpar.width),
                  cvRound(detpar.height));

    // 初始化人眼的参数
    detpar.lex = detpar.ley = INVALID;
    detpar.rex = detpar.rey = INVALID;
    vec_Rect leyes, reyes;
    int ileft_best = -1, iright_best = -1;
    if (!leye_det_g.empty())
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
            RectToImgFrame(detpar.lex, detpar.ley, leyes[ileft_best]);

        if (iright_best >= 0)
            RectToImgFrame(detpar.rex, detpar.rey, reyes[iright_best]);
    }
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

    // 对于左脸，选取脸部左边2/3的区域进行检测
    int width = facerect.width;
    Rect leftrect = facerect;
    leftrect.width -= width / 3;
    leftrect.height = cvRound(.6 * facerect.height);
    leftrect.width = MAX(0, leftrect.width);

    if (leftrect.width)
        leyes = Detect(img, &leye_det_g, &leftrect, facerect.width /10);

    // 对于右脸，选取脸部右边2/3的区域进行检测
    Rect rightrect = facerect;
    rightrect.x += width / 3;
    rightrect.width -= width / 3;
    rightrect.height = cvRound(.6 * facerect.height);
    rightrect.width = MAX(0, rightrect.width);

    if (rightrect.width)
        reyes = Detect(img, &reye_det_g, &rightrect, facerect.width /10);
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
 * rect是否在enclosing中
 * @param  rect      in：
 * @param  enclosing in：
 * @return
 */
static bool InRect(const Rect& rect, const Rect& enclosing)
{
    int x = rect.x + rect.width / 2;
    int y = rect.y + rect.height / 2;

    return x >= enclosing.x &&
           x <= enclosing.x + enclosing.width &&
           y >= enclosing.y &&
           y <= enclosing.y + enclosing.height;
}

/**
 * 把人眼的坐标投射到整个脸上
 * @param x        in/out：
 * @param y        in/out：
 * @param featrect in：人脸
 */
static void RectToImgFrame(double &x, double &y, const Rect &featrect)
{
    x = featrect.x + featrect.width / 2;
    y = featrect.y + featrect.height / 2;
}

/**
 * 检测人眼的主函数
 * @param  img             in：源图像
 * @param  cascade         in：级联分类器
 * @param  searchrect      in：搜索区域
 * @param  minwidth_pixels in：最小人眼大小
 * @return                 out: 可能人眼的集合
 */
static vec_Rect Detect(const Image &img, cv::CascadeClassifier *cascade, const Rect *searchrect, int minwidth_pixels)
{
    CV_Assert(!cascade->empty());

    // 对searchrect进行处理
    Rect searchrect1;
    searchrect1.width = 0;

    if (searchrect)
    {
        searchrect1 = *searchrect;
        ForceRectIntoImg(searchrect1, img);
        if (searchrect1.height == 0)
            searchrect1.width = 0;
    }
    Image roi(img, searchrect1.width? searchrect1 : Rect(0, 0, img.cols, img.rows));

    // 设置最大的人脸数量
    static const int MAX_NFACES_IN_IMG = int(1e4);
    vec_Rect feats(MAX_NFACES_IN_IMG);

    cascade->detectMultiScale(roi, feats, EYE_SCALE_FACTOR, EYE_MIN_NEIGHBORS, EYE_DETECTOR_FLAGS, cvSize(minwidth_pixels, minwidth_pixels));

    if (!feats.empty() && searchrect1.width)
        DiscountSearchRegion(feats, searchrect1);

    return feats;
}

/**
 * 强制将Rect映射到Image上
 * @param rect io: 调整前后的Rect
 * @param img  in: 源图像
 */
static void ForceRectIntoImg(Rect &rect, const Image &img)
{
    ForceRectIntoImg(rect.x, rect.y, rect.width, rect.height, img);
}

/**
 * 强制将Rect映射到Image上
 * @param ix    io: 调整前后Rect的坐标x
 * @param iy    io: 调整前后Rect的坐标y
 * @param ncols io: 调整前后Rect的宽度
 * @param nrows io: 调整前后Rect的高度
 * @param img   in: 源图像
 */
static void ForceRectIntoImg(int &ix, int &iy, int &ncols, int &nrows, const Image &img)
{
    ix = Clamp(ix, 0, img.cols-1);

    int ix1 = ix + ncols;
    if (ix1 > img.cols)
        ix1 = img.cols;

    ncols = ix1 - ix;

    CV_Assert(ix >= 0 && ix < img.cols);
    CV_Assert(ix + ncols >= 0 && ix + ncols <= img.cols);

    iy = Clamp(iy, 0, img.rows-1);

    int iy1 = iy + nrows;
    if (iy1 > img.rows)
        iy1 = img.rows;

    nrows = iy1 - iy;

    CV_Assert(iy >= 0 && iy < img.rows);
    CV_Assert(iy + nrows >= 0 && iy + ncols <= img.rows);
}

/**
 * 将ROI的坐标映射到源图像中
 * @param feats      io: 人脸参数
 * @param searchrect io: ROI
 */
static void DiscountSearchRegion(vec_Rect &feats, Rect &searchrect)
{
    for (int ifeat = 0; ifeat < NSIZE(feats); ifeat++)
    {
        feats[ifeat].x += searchrect.x;
        feats[ifeat].y += searchrect.y;
    }
}