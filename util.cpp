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