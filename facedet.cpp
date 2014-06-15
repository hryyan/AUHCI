// facedet.cpp: 人脸检测
// 
// Created by Vincent Yan in 2014/03/17

#include "facedet.h"
#include "haarclassifier.h"

// source中每一帧的Mat
extern Mat frame;

// 数据流中的人脸信息
extern DetPar frame_detpar;

// 级联分类器
static cv::CascadeClassifier facedet_g;

// GPU级联分类器
static cv::gpu::CascadeClassifier_GPU facedet_gpu_g;

// 图像边缘忽略的比例
static double BORDER_FRAC_FACE = 0.1;

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
    leftborder = cvRound(BORDER_FRAC_FACE * bordered_img.cols);
    topborder  = cvRound(BORDER_FRAC_FACE * bordered_img.rows);

    copyMakeBorder(bordered_img, bordered_img, topborder, topborder, leftborder, leftborder, cv::BORDER_REPLICATE);
    return bordered_img;
}

/**
 * 使用Haar检测人脸
 * @param detpars  out: 检测出的人脸参数vector
 * @param img      in: 源图像
 * @param minwidth in: 图像宽度的百分比
 */
static void DetectFaces(vec_DetPar &detpars, const Image &img, int minwidth)
{
    int leftborder = 0, topborder = 0;
    Image bordered_img(BORDER_FRAC_FACE == 0 ? img : EmborderImg(leftborder, topborder, img));

    // 直方均衡后的结果比较好，并且直方均衡比较快
    Image equalized_img;
    equalizeHist(bordered_img, equalized_img);
    //cv::namedWindow("Result");
    //imshow("Result", equalized_img);
    //cv::waitKey();

    CV_Assert(minwidth >= 1 && minwidth <= 100);

    // 设置最小人脸占图片的比例
    int minpix = MAX(100, cvRound(img.cols * minwidth / 100.));
	
#ifdef USE_OPENCV_GPU_DETECTION
	vec_Rect facerects = Detect(equalized_img, &facedet_gpu_g, NULL, minpix);
#else
	vec_Rect facerects = Detect(equalized_img, &facedet_g, NULL, minpix);
#endif//USE_OPENCV_GPU_DETECTION
    

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
vector<DetPar> DetectFaces_(const Image &img, bool multiface, int minwidth)
{
    CV_Assert(!facedet_g.empty());
	CV_Assert(!facedet_gpu_g.empty());

    vector<DetPar> detpars_;

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

    return detpars_;
}

void InitFaceDet()
{
    if(facedet_g.empty())
        OpenDetector(&facedet_g, "haarcascade_frontalface_alt2.xml");

    if (facedet_gpu_g.empty())
        OpenDetector(&facedet_gpu_g, "haarcascade_frontalface_alt2.xml");
}

/**
 * 打印人脸
 * @return QImage
 */
void PrintFaceToFrame()
{
    CV_Assert(!facedet_g.empty());
    CV_Assert(!facedet_gpu_g.empty());

	qDebug("Starting printFace...");
	QTime time1 = QTime::currentTime();
    Mat face, face_mask, dst;

    if (frame.channels() == 3)
        cvtColor(frame, frame, CV_BGR2GRAY);

    vec_DetPar v_detpar = DetectFaces_(frame, false, 30);
	if (!v_detpar.empty())
		frame_detpar = v_detpar.at(0);
	else
		frame_detpar = DetPar();

    if (frame_detpar.x != INVALID)
    {
        double width = static_cast<double>(frame.cols);
        double height = static_cast<double>(frame.rows);
        double topleft_x     = cv::max(cv::min(width,   frame_detpar.x-frame_detpar.width/2),   0.0);
        double topleft_y     = cv::max(cv::min(height,  frame_detpar.y-frame_detpar.height/2),  0.0);
        double buttomright_x = cv::max(cv::min(width,   frame_detpar.x+frame_detpar.width/2),   0.0);
        double buttomright_y = cv::max(cv::min(height,  frame_detpar.y+frame_detpar.height/2),  0.0);
        Point topleft        = Point(topleft_x, topleft_y);
        Point bottomright    = Point(buttomright_x, buttomright_y);
        face = Mat(frame, Rect(topleft, bottomright));
        face.copyTo(face_mask);
        face_mask.setTo(0);
        ellipse(face_mask, Point(frame_detpar.width/2, frame_detpar.height/2), Size(frame_detpar.width/2.4, frame_detpar.height/2.1), 0, 0, 360, Scalar(255, 0, 0), -1);
        face.copyTo(dst, face_mask);

        // 因为已经把人脸部分抠出来了，所以frame_detpar的参数得再调整
        frame_detpar.width  = buttomright_x - topleft_x;
        frame_detpar.height = buttomright_y - topleft_y;
        frame_detpar.x = frame_detpar.width / 2;
        frame_detpar.y = frame_detpar.height / 2;
    }

    // 把图像缩放成一个固定尺寸，并且修改它的大小等参数。
    cv::resize(dst, dst, Size(RESIZE_WIDTH, RESIZE_HEIGHT));
    frame_detpar.width  = RESIZE_WIDTH;
    frame_detpar.height = RESIZE_HEIGHT;
    frame_detpar.x      = frame_detpar.width / 2;
    frame_detpar.y      = frame_detpar.height / 2;
    dst.copyTo(frame);

	QTime time2 = QTime::currentTime();
	qDebug() << QString("face_detection: ") << time1.msecsTo(time2) << QString("ms");
}