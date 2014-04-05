// eyedet.cpp: 人眼检测
// 
// Created by Vincent Yan in 2014/04/04
// Modefied from stasm

#include "eyedet.h"

static cv::CascadeClassifier leye_det_g;    // left eye detector
static cv::CascadeClassifier reye_det_g;    // right eye detector
static cv::CascadeClassifier mouth_det_g;   // mouth detector

static const double EYE_SCALE_FACTOR    = 1.2;
static const int    EYE_MIN_NEIGHBORS   = 3;
static const int    EYE_DETECTOR_FLAGS  = 0;

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

        if (!facedet_g.load(path))
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

        if (!facedet_g.load(path))
            qDebug("Cannot load %s/%s \n", dir, rigth_eye_filename);
    }
}

void DetectEyes(DetPar& detpar, const Mat& img)
{
    Rect facerect(cvRound(detpar.x - detpar.width/2),
                  cvRound(detpar.y - detpar.height/2),
                  cvRound(detpar.width),
                  cvRound(detpar.height));

    detpar.lex = detpar.ley = INVALID;
    detpar.rex = detpar.rey = INVALID;
    vec_Rect leyes, reyes;
    int ileft_best = -1, iright_best = -1;
    if (!leye_det_g.empty())
    {
        DetectAllEyes(leyes, reyes, img, facerect);

        Rect eyesRect;
        eyesRect.x      += cvRound(.1 * facerect.width);
        eyesRect.width  += cvRound(.8 * facerect.width);
        eyesRect.y      += cvRound(.2 * facerect.height);
        eyesRect.height += cvRound(.28 * facerect.height);

        SelectEyes(ileft_best, iright_best, leyes, reyes, eyesRect);

        if (ileft_best >= 0)
            ForceRectIntoImg(detpar.lex, detpar.ley, leyes[ileft_best]);

        if (iright_best >= 0)
            ForceRectIntoImg(detpar.rex, detpar.rey, reyes[iright_best]);
    }
}

static void DetectAllEyes(vec_Rect& leyes, vec_Rect& reyes, const Image& img, const Rect& facerect)
{
    CV_Assert(!leye_det_g.empty());
    CV_Assert(!reye_det_g.empty());

    int width = facerect.width;
    Rect leftrect = facerect;
    leftrect.width -= width / 3;
    leftrect.height = cvRound(.6 * facerect.height);
    leftrect.width = MAX(0, leftrect.width);

    if (leftrect.width)
        leyes = Detect(img, &leye_det_g, &leftrect, facerect.width /10);

    Rect rightrect = facerect;
    rightrect.x += width / 3;
    rightrect.width -= width / 3;
    rightrect.height = cvRound(.6 * facerect.height);
    rightrect.width = MAX(0, rightrect.width);

    if (rightrect.width)
        reyes = Detect(img, &reye_det_g, &rightrect, facerect.width /10);
}

static void SelectEyes(int &ileft_best, int &iright_best, const vec_Rect &leyes, const vec_Rect &reyes, const Rect &eye_inner_rect)
{
    ileft_best = iright_best = -1;
    int min_intereye = cvRound(.25 * eye_inner_rect.width);
    int maxwidth = 0;
    int ileft, iright;
    Rect left, right;

    for (ileft = 0; ileft_best < NSIZE(leyes); ileft++)
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

static bool IsEyeHorizOk(const Rect& left, const Rect& right)
{
    return left.x + left.width - right .x   <= .1 * left.width &&
           right.x - (left.x + left.width)  <= left.width; 
}

static bool VerticalOverlap(const Rect& left, const Rect& right)
{
    const int topleft = left.y + left.height;
    const int topright = left.y + right.height;

    return (left.y   >= right.y && left.y   <= right.y + right.height) ||
           (topleft  >= right.y && topleft  <= right.y + right.height) ||
           (right.y  >= left.y  && right.y  <= left.y  + left.height)  ||
           (topright >= left.y  && topright <= left.y  + left.height);
}

static bool InRect(const Rect& rect, const Rect& enclosing)
{
    int x = rect.x + rect.width / 2;
    int y = rect.y + rect.height / 2;

    return x >= enclosing.x &&
           x <= enclosing + enclosing.width &&
           y >= enclosing.y &&
           y <= enclosing.y + enclosing.height;
}

static void ForceRectIntoImg(double &x, double &y, const Rect &featrect)
{
    x = featrect.x + featrect.width / 2;
    y = featrect.y + featrect.height / 2;
}

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

    cascade->detectMultiScale(roi, feats, SCALE_FACTOR, MIN_NEIGHBORS, DETECTOR_FLAGS, cvSize(minwidth_pixels, minwidth_pixels));

    if (!feats.empty() && searchrect1.width)
        DiscountSearchRegion(feats, searchrect1);

    return feats;
}