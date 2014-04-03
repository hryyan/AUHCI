// facedet.cpp: 人脸检测
// 
// Created by Vincent Yan in 2014/03/17

#include "facedet.h"
#include "source.h"

// source中每一帧的Mat
extern Mat frame;

typedef vector<DetPar> vec_DetPar;

// 级联分类器
static cv::CascadeClassifier facedet_g;

// 图像边缘忽略的比例
static double BORDER_FRAC = 0.1;

// 以下参数是OpenCV的人脸检测默认参数，效果较好速度较慢
static const double SCALE_FACTOR    = 1.1;
static const int    MIN_NEIGHBORS   = 3;
static const int    DETECTOR_FLAGS  = 0;

/**
 * 打开人脸检测器(Haar)
 * @param datadir   in: 分类器的目录地址
 */
void FaceDet::OpenFaceDetector_(const char *datadir)
{
    char filename[SLEN] = "haarcascade_frontalface_alt2.xml";

    if (facedet_g.empty())
    {
        char dir[SLEN];
        strcpy(dir, datadir);
        ConvertBackslashesToForwardAndStripFinalSlash(dir);

        char path[SLEN];
        sprintf(path, "%s/%s", dir, filename);
        qDebug("Open %s/%s \n", dir, filename);

        if (!facedet_g.load(path))
            qDebug("Cannot load %s/%s \n", dir, filename);
    }
}

/**
 * 在图像周围添加边界
 * @param  leftborder out: 左右边界大小
 * @param  topborder  out: 上下边界大小
 * @param  img        io: 源图像
 * @return            添加边界后的图像
 */
static Image EmborderImg(int &leftborder, int &topborder, const Image& img)
{
    Image bordered_img(img);

    // cvRound是OpenCV 1.X的函数，使用C，所以不在命名空间内
    leftborder = cvRound(BORDER_FRAC * bordered_img.cols);
    topborder  = cvRound(BORDER_FRAC * bordered_img.rows);

    copyMakeBorder(bordered_img, bordered_img, topborder, topborder, leftborder, leftborder, cv::BORDER_REPLICATE);
    return bordered_img;
}

/**
 * 强制将Rect映射到Image上
 * @param ix    io: 调整前后Rect的坐标x
 * @param iy    io: 调整前后Rect的坐标y
 * @param ncols io: 调整前后Rect的宽度
 * @param nrows io: 调整前后Rect的高度
 * @param img   in: 源图像
 */
void ForceRectIntoImg(int &ix, int &iy, int &ncols, int &nrows, const Image &img)
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
 * 强制将Rect映射到Image上
 * @param rect io: 调整前后的Rect
 * @param img  in: 源图像
 */
void ForceRectIntoImg(Rect &rect, const Image &img)
{
    ForceRectIntoImg(rect.x, rect.y, rect.width, rect.height, img);
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

/**
 * 将ROI覆盖到IMAGE上，并对其中人脸进行检测
 * @param  img             in: 源图像
 * @param  cascade         in: 分类器
 * @param  searchrect      in: roi
 * @param  minwidth_pixels in: 最小人脸像素
 * @return                 每个人脸一个矩阵
 */
vec_Rect Detect(const Image &img, cv::CascadeClassifier *cascade, const Rect *searchrect, int minwidth_pixels)
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

/**
 * 使用Haar检测人脸
 * @param detpars  out: 检测出的人脸参数vector
 * @param img      in: 源图像
 * @param minwidth in: 图像宽度的百分比
 */
void DetectFaces(vec_DetPar &detpars, const Image &img, int minwidth)
{
    int leftborder = 0, topborder = 0;
    Image bordered_img(BORDER_FRAC == 0 ? img : EmborderImg(leftborder, topborder, img));

    // 直方均衡后的结果比较好，并且直方均衡比较快
    
    Image equalized_img;
    equalizeHist(bordered_img, equalized_img);
    //cv::namedWindow("Result");
    //imshow("Result", equalized_img);
    //cv::waitKey();

    CV_Assert(minwidth >= 1 && minwidth <= 100);

    // 设置最小人脸占图片的比例
    int minpix = MAX(100, cvRound(img.cols * minwidth / 100.));

    vec_Rect facerects = Detect(equalized_img, &facedet_g, NULL, minpix);

    // 把检测到的人脸参数复制到人脸的参数Vector中
    
    detpars.resize(facerects.size());
    for (int i = 0; i < facerects.size(); i++)
    {
        Rect* facerect = &facerects[i];
        // 构造函数把所有项设为INVALID
        DetPar detpar; 
        // detpar.x 和 detpar.y 是人脸矩阵的中心
        detpar.x = facerect->x + facerect->width / 2.;
        detpar.y = facerect->y + facerect->height / 2.;
        detpar.x -= leftborder;
        detpar.y -= topborder;
        detpar.width  = double(facerect->width);
        detpar.height = double(facerect->height);
        detpar.yaw = 0;
        detpar.eyaw = EYAW00;
        detpars[i] = detpar;
    }
}

/**
 * 把人脸矩阵按离左侧远近排序，由sort调用
 * @param  detpar1 in: 矩阵1
 * @param  detpar2 in: 矩阵2
 * @return         closer or further
 */
static bool IncreasingLeftMargin(const DetPar& detpar1, const DetPar& detpar2)
{
    return 1e5 * detpar2.width + detpar2.x <
           1e5 * detpar1.width + detpar1.x;
}

/**
 * 把人脸矩阵按大到小排列，由sort调用
 * @param  detpar1 in: 矩阵1
 * @param  detpar2 in: 矩阵2
 * @return         bigger or smaller
 */
static bool DecreasingWidth(        // compare predicate for std::sort
    const DetPar& detpar1,          // in
    const DetPar& detpar2)          // in
{
    return 1e5 * detpar2.width - detpar2.x <
           1e5 * detpar1.width - detpar1.x;
}

/**
 * 忽略太大或者太小的脸
 * @param detpars io: 人脸矩阵
 */
static void DiscardMissizedFaces(vec_DetPar &detpars)
{
    // 远离中位数的比例
    static const double MIN_WIDTH = 1.33;
    static const double MAX_WIDTH = 1.33;

    if (NSIZE(detpars) >= 3)
    {
        // 按照人脸矩阵大小排序, 取得中位数
        sort(detpars.begin(), detpars.end(), DecreasingWidth);
        const int median     = cvRound(detpars[NSIZE(detpars) / 2].width);
        const int minallowed = cvRound(median / MIN_WIDTH);
        const int maxallowed = cvRound(median * MAX_WIDTH);

        // 只留下那些不是太大或者太小的人脸矩阵
        vec_DetPar all_detpars(detpars);
        detpars.resize(0);
        for (int iface = 0; iface < NSIZE(all_detpars); iface++)
        {
            DetPar *face = &all_detpars[iface];
            if (face->width >= minallowed && face->width <= maxallowed)
                detpars.push_back(*face);
            else
                qDebug("Discard %d of %d", iface, NSIZE(all_detpars));
        }

    }
}

/**
 * 人脸检测全过程，先检测，后去掉不理想的结果
 * @param img       in: 源图像
 * @param multiface in: 是否检测多张人脸
 * @param minwidth  in: 最小人脸
 */
void FaceDet::DetectFaces_(const Image &img, bool multiface, int minwidth)
{
    CV_Assert(!facedet_g.empty());
    
    // 先检测所有人脸，再把不太好的结果去掉
    DetectFaces(detpars_, img, minwidth);
    DiscardMissizedFaces(detpars_);

    // 如果要求返回多个人脸矩阵，按照离左右距离排序
    // 如果只要返回一个人脸矩阵，按照矩阵大小排序
    if (multiface)
        sort(detpars_.begin(), detpars_.end(), IncreasingLeftMargin);
    else
    {
        sort(detpars_.begin(), detpars_.end(), DecreasingWidth);
        if (NSIZE(detpars_))
            detpars_.resize(1);        
    }
    iface_ = 0;
}

/**
 * 返回下一个人脸参数
 * @return  人脸参数
 */
const DetPar FaceDet::NextFace_(void)
{
    DetPar detpar;

    if (iface_ < NSIZE(detpars_))
        detpar = detpars_[iface_++];

    return detpar;
}

/**
 * 打印人脸
 * @return QImage
 */
QImage* printFace()
{
    Mat face, face_mask, dst;
    
    if(facedet_g.empty())
        faceDet.OpenFaceDetector_("C:\\Users\\vincent\\Documents\\Visual Studio 2010\\Projects\\CV\\FacialExpression");

    if (frame.channels() == 3)
        cvtColor(frame, frame, CV_BGR2GRAY);

    faceDet.DetectFaces_(frame, false, 30);
    DetPar detPar = faceDet.NextFace_();

    if (detPar.x != INVALID)
    {
        double width = static_cast<double>(frame.cols);
        double height = static_cast<double>(frame.rows);
        double topleft_x = cv::max(cv::min(width, detPar.x-detPar.width/2), 0.0);
        double topleft_y = cv::max(cv::min(height, detPar.y-detPar.height/2), 0.0);
        double buttomright_x = cv::max(cv::min(width, detPar.x+detPar.width/2), 0.0);
        double buttomright_y = cv::max(cv::min(height, detPar.y+detPar.height/2), 0.0);
        Point topleft = Point(topleft_x, topleft_y);
        Point bottomright = Point(buttomright_x, buttomright_y);
        face = Mat(frame, Rect(topleft, bottomright));
        face.copyTo(face_mask);
        face_mask.setTo(0);
        ellipse(face_mask, Point(detPar.width/2, detPar.height/2), Size(detPar.width/2.4, detPar.height/2.1), 0, 0, 360, Scalar(255, 0, 0), -1);
        face.copyTo(dst, face_mask);
    }

    dst.copyTo(frame);
    //bilateralFilter(face, face_after_bilateral_filter, 7, 9, 9);
    return Mat2QImage(dst);
}