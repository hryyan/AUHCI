// haarclassifier.cpp：haar分类器的通用方法
// 
// Created by Vincent yan in 2014/04/21

#include "haarclassifier.h"

// 以下参数是OpenCV的haar默认参数，效果较好速度较慢
static const double SCALE_FACTOR    = 1.1;
static const int    MIN_NEIGHBORS   = 3;
static const int    DETECTOR_FLAGS  = 0;

static const char   haar_classifier_directory[SLEN] = "../AUHCI_crossplatform/CacadeClassifier";

/**
 * 打开haar分类器
 * @param cascade  in/out: 具体要打开的分类器的指针
 * @param filename in: 分类器文件的名字
 * @param datadir  in: 分类器文件夹的名字
 */
void OpenDetector(cv::CascadeClassifier *cascade, const char *filename)
{
    if (cascade->empty())
    {
        char dir[SLEN];
        strcpy(dir, haar_classifier_directory);
        ConvertBackslashesToForwardAndStripFinalSlash(dir);

        char path[SLEN];
        sprintf(path, "%s/%s", dir, filename);
        qDebug("Open CascadeClassifier %s \n", path);

        if (!cascade->load(path))
            qDebug("Cannot load CascadeClassifier %s", path);
    }
}

/**
 * 打开gpu_haar分类器
 * @param cascade  in/out: 具体要打开的分类器的指针
 * @param filename in: 分类器文件的名字
 * @param datadir  in: 分类器文件夹的名字
 */
void OpenDetector(cv::gpu::CascadeClassifier_GPU *cascade,
                  const char *filename)
{
	if (cascade->empty())
	{
		char dir[SLEN];
		strcpy(dir, haar_classifier_directory);
		ConvertBackslashesToForwardAndStripFinalSlash(dir);

		char path[SLEN];
		sprintf(path, "%s/%s", dir, filename);
		qDebug("Open CascadeClassifier %s \n", path);

		if (!cascade->load(path))
			qDebug("Cannot load CascadeClassifier %s", path);
	}
}

/**
 * rect是否在enclosing中
 * @param  rect      in：
 * @param  enclosing in：
 * @return
 */
bool InRect(const Rect& rect, const Rect& enclosing)
{
    int x = rect.x + rect.width / 2;
    int y = rect.y + rect.height / 2;

    return x >= enclosing.x &&
           x <= enclosing.x + enclosing.width &&
           y >= enclosing.y &&
           y <= enclosing.y + enclosing.height;
}

/*
 * 强制将Rect收敛在img内
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
    CV_Assert(iy + nrows >= 0 && iy + nrows <= img.rows);
}

/**
 * 强制将Rect收敛在img内
 * @param rect io: 调整前后的Rect
 * @param img  in: 源图像
 */
void ForceRectIntoImg(Rect &rect, const Image &img)
{
    ForceRectIntoImg(rect.x, rect.y, rect.width, rect.height, img);
}

/**
 * 获得Rect的中心
 * @param x        in/out：
 * @param y        in/out：
 * @param featrect in：人脸
 */
void RectToImgFrame(double &x, double &y, const Rect &featrect)
{
    x = featrect.x + featrect.width / 2;
    y = featrect.y + featrect.height / 2;
}

/**
 * 将ROI的坐标映射到源图像中
 * @param feats      io: 人脸参数
 * @param searchrect io: ROI
 */
void DiscountSearchRegion(vec_Rect &feats, const Rect &searchrect)
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
    static const int MAX_NUMS_IN_IMG = int(1e4);
    vec_Rect feats(MAX_NUMS_IN_IMG);

	QTime time1 = QTime::currentTime();
    cascade->detectMultiScale(roi, feats, SCALE_FACTOR, MIN_NEIGHBORS, DETECTOR_FLAGS, cvSize(minwidth_pixels, minwidth_pixels));
	QTime time2 = QTime::currentTime();
	qDebug() << QString("cpu_detectMultiScale: ") << time1.msecsTo(time2) << QString("ms");

    if (!feats.empty() && searchrect1.width)
        DiscountSearchRegion(feats, searchrect1);

    return feats;
}

/**
 * 将ROI覆盖到IMAGE上，并对其中人脸进行检测
 * @param  img             in: 源图像
 * @param  cascade         in: 分类器
 * @param  searchrect      in: roi
 * @param  minwidth_pixels in: 最小人脸像素
 * @return                 每个人脸一个矩阵
 */
vec_Rect Detect(const Image &img, cv::gpu::CascadeClassifier_GPU *cascade, const Rect *searchrect, int minwidth_pixels)
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

	cv::gpu::GpuMat img_gpu(roi);
	cv::gpu::GpuMat objectsBuf;
	QTime time1 = QTime::currentTime();
	cascade->detectMultiScale(img_gpu, objectsBuf, SCALE_FACTOR, MIN_NEIGHBORS, cvSize(minwidth_pixels, minwidth_pixels));
	QTime time2 = QTime::currentTime();
	qDebug() << QString("gpu_detectMultiScale: ") << time1.msecsTo(time2) << QString("ms");

    Mat obj_host;
	objectsBuf.colRange(0, 5).download(obj_host);

	vec_Rect feats;
	Rect* faces = obj_host.ptr<Rect>();
	for (int i = 0; i < 5; i++)
	{
		if (faces[i].x >= 0 && faces[i].x < roi.cols && faces[i].y >= 0 && faces[i].y < roi.rows)
			feats.push_back(faces[i]);
	}
	
	if (!feats.empty() && searchrect1.width)
		DiscountSearchRegion(feats, searchrect1);

    return feats;
}
