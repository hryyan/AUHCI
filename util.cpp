// util.h: 零碎的定义
//
// Created by Vincent Yan in 2014/03/17

#include "util.h"

/**
* 把windows的\转换成通用的/
* @param s in: 需要转换的字符串
*/
void ConvertBackslashesToForwardAndStripFinalSlash(char* s)
{
    int i;
    
    for (i = 0; s[i]; i++)       // convert \ to /
    if (s[i] == '\\')
    s[i] = '/';
    
    if (i > 0 && s[i-1] == '/')  // remove final / if any
    s[i-1] = 0;
}

/**
* 把Mat转换成QImage（Mat的大小只能在底层得到）
* @param  in:  Mat
* @return    QImage的指针
*/
QImage* Mat2QImage(Mat& I)
{
    int channelNum = I.channels();
    
    QImage *img = new QImage(I.cols, I.rows, QImage::Format_RGB32);
    for (int i = 0; i < I.rows; i++)
    {
        uchar* Mi = I.ptr<uchar>(i);
        for (int j = 0; j < I.cols; j++)
        {
            if (channelNum == 1)
            {
                int r = Mi[j];
                img->setPixel(j, i, qRgb(r, r, r));
            }
            else if (channelNum == 3)
            {
                int r = Mi[2+j*3];
                int g = Mi[1+j*3];
                int b = Mi[0+j*3];
                img->setPixel(j, i, qRgb(r, g, b));
            }
        }
    }
    return img;
}

/**
 * 把Mat转换成Txt文本（这样精度就没有任何损失）
 * @param in: Mat
 */
void Mat2Txt(string path, Mat_<uchar>& m)
{
    FILE* fp = fopen(path.c_str(), "w+");
    char buff[50];

    uchar *it = m.ptr<uchar>(0);
    for (int i = 0; i < m.cols*m.rows; i++)
    {
        memset(buff, 0, sizeof(buff));
        sprintf(buff, "%d:%d ", i+1, it[i]);
        fputs(buff, fp);
        if (i % 1001 == 0)
            fflush(fp);
    }
    fflush(fp);
    fclose(fp);
}

/**
 * 公用函数，用于判断这个切片没有超出图像范围
 * @param  x_coord 中心的x坐标
 * @param  y_coord 中心的y坐标
 * @param  left    往左延伸多少
 * @param  right   往右延伸多少
 * @param  top     往上延伸多少
 * @param  bottom  往下延伸多少
 * @return         是否能够切出该片
 */
bool IsSliceOk(int x_coord, int y_coord, int left, int right, int top, int bottom)
{
    if (x_coord != 99999 && y_coord != 99999)
        if (x_coord >= left && x_coord <= RESIZE_WIDTH-right &&
            x_coord <= RESIZE_WIDTH-left &&
            y_coord >= top  && y_coord <= RESIZE_HEIGHT-bottom)
            return true;
    return false;
}

/**
 * 重载函数，用于矩形
 * @param  tl 左上角
 * @param  br 右下角
 * @return    能否切出切片
 */
bool IsSliceOk(Point tl, Point br)
{
    if (tl.x>=0 && tl.y>=0 && tl.x<=RESIZE_WIDTH && tl.y<=RESIZE_HEIGHT &&
        br.x>=0 && br.y>=0 && br.x<=RESIZE_WIDTH && br.y<=RESIZE_HEIGHT &&
        tl.x<br.x && tl.y<br.y)
        return true;
    return false;
}

/**
 * 公用函数，用于从图像中切片，并且flip
 * @param  img      源图像
 * @param  x_coord  中心的x坐标
 * @param  y_coord  中心的y坐标
 * @param  left     往左延伸多少
 * @param  right    往右延伸多少
 * @param  top      往上延伸多少
 * @param  bottom   往下延伸多少
 * @param  needflip 是否需要翻转
 * @return          返回切片
 */
Mat GetSlice(const Mat& img, int x_coord, int y_coord, int left, int right, int top, int bottom, bool needflip)
{
    Rect r = Rect(Point(x_coord-left, y_coord-top), Point(x_coord+right, y_coord+bottom));
    Mat roi, tmp;

    if (!needflip)
        roi = Mat(img, r).clone();
    else
    {
        flip(Mat(img, r), tmp, 1);
        roi = tmp.clone();
    }
    return roi;
}

/**
 * 重载函数，用于矩形
 * @param  img      源图像
 * @param  tl       左上角
 * @param  br       右下角
 * @param  needflip 需要翻转
 * @return          返回切片
 */
Mat GetSlice(const Mat& img, Point tl, Point br, bool needflip)
{
    Rect r = Rect(tl, br);
	qDebug("width: %d, height: %d", img.cols, img.rows);
	qDebug("%d, %d, %d, %d", tl.x, tl.y, br.x, br.y);
    Mat roi, tmp;

    if (!needflip)
        roi = Mat(img, r).clone();
    else
    {
        flip(Mat(img, r), tmp, 1);
        roi = tmp.clone();
    }
	qDebug("width: %d, height: %d", roi.cols, roi.rows);
    return roi;
}

/**
 * 把Mat输出到文件中
 * @param m Mat
 */
void PrintToFile(Mat m)
{
    FILE* fp = fopen("mat", "wb+");
    fputs("1 ", fp);
    char s[50];
    double value;
    uchar *it;

    for (int i = 0; i < m.rows; i++)
    {
        it = m.ptr<uchar>(i);
        for (int j = 0; j < m.cols; j++)
        {
            memset(s, 0, sizeof(s));
            sprintf(s, "%d:%d ", i*m.cols+j+1, it[j]);
            fputs(s, fp);
            if (i*m.cols+j == 1000)
                fflush(fp);
        }
    }
   
    fputs("\n", fp);
    fclose(fp);
}