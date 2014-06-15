// util.h: 零碎的定义
//
// Created by Vincent Yan in 2014/03/17

#ifndef UTIL_H
#define UTIL_H

// 标准C的头文件
#include <cstring>

// STL的头文件
#include <vector>
#include <string>
#include <utility>
using std::string;
using std::vector;
using std::pair;

// QT的头文件
#include <QImage>
#include <QtDebug>
#include <QTimer>
#include <QTime>
#include <QString>

// OpenCV的头文件
#include "opencv2\opencv.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\gpu\gpu.hpp"
#include "opencv2\core\internal.hpp"
using cv::Rect;
using cv::Mat;
using cv::Mat_;
using cv::VideoCapture;
using cv::Point;
using cv::Scalar;
using cv::Size;
using cv::sqrt;

// 定义的DEFINE
#include "predefine.h"

// 定义一些方便使用的类型
typedef vector<Rect>  vec_Rect;

typedef cv::Mat_<double> MAT;
typedef unsigned char byte;
typedef cv::Mat_<byte> Image;

static const int SLEN    = 260;
static const int INVALID = 99999;

enum EYAW
{
    EYAW_45 = -3,   // yaw -45 degrees (left facing strong three-quarter pose)
    EYAW_22 = -2,   // yaw -22 degrees (left facing mild three-quarter pose)
    EYAW00  =  1,   // yaw 0 degrees   (frontal pose)
    EYAW22  =  2,   // yaw 22 degrees  (right facing mild three-quarter pose)
    EYAW45  =  3    // yaw 45 degrees  (right facing strong three-quarter pose)
};

struct DetPar   // the stucture describing a face detection
{
    double x, y;                // center of detector shape
    double width ,height;       // width and height of detector shape
    double lex, ley;            // center of left eye, left and right are wrt the viewer
    double rex, rey;            // ditto for right eye
    double mouthx, mouthy;      // center of mouth
    double nosex, nosey;        // center of nose
    double rot;                 // in-plane rotation
    double yaw;                 // yaw
    EYAW   eyaw;                // yaw as an enum
    
    DetPar() // constructor sets all fields to INVALID
    :x(INVALID),
    y(INVALID),
    width(INVALID),
    height(INVALID),
    lex(INVALID),
    ley(INVALID),
    rex(INVALID),
    rey(INVALID),
    mouthx(INVALID),
    mouthy(INVALID),
    nosex(INVALID),
    nosey(INVALID),
    rot(INVALID),
    yaw(INVALID),
    eyaw(EYAW(INVALID))
    {
        
    };
};

typedef vector<DetPar> vec_DetPar;

// A macro to disallow the copy constructor and operator= functions.
// This is used in the private declarations for a class where those member
// functions have not been explicitly defined.  This macro prevents use of
// the implicitly defined functions (the compiler will complain if you try
// to use them).
// This is often just paranoia.  The implicit functions may actually be ok
// for the class in question, but shouldn't be used until that is checked.
// For details, See Item 6 Meyers Effective C++ and the Google C++ Style Guide.
#define DISALLOW_COPY_AND_ASSIGN(ClassName) \
ClassName(const ClassName&);      \
void operator=(const ClassName&)

static inline int NSIZE(const MAT &m)
{
    return int((m).total());
}

template <typename T> int NSIZE(const T& x)
{
    return int(x.size());
}

template <typename T> T Clamp(const T x, const T min, const T max)
{
    return MIN(MAX(x, min), max);
}

/**
* 把windows的\转换成通用的/
* @param s in: 需要转换的字符串
*/
void ConvertBackslashesToForwardAndStripFinalSlash(char* s);

/**
* 把Mat转换成QImage（Mat的大小只能在底层得到）
* @param  in:  Mat
* @return      QImage的指针
*/
QImage* Mat2QImage(Mat& T);

/**
 * 把Mat转换成Txt文本（这样精度就没有任何损失）
 * @param in: Mat
 */
void Mat2Txt(string path, Mat_<uchar>& m);

// 这个与DetPar不同，这个关注于同文件相关的信息
// 而DetPar关注于同文件无关的信息
struct Coords
{
    int left_eye_x;
    int left_eye_y;
    int right_eye_x;
    int right_eye_y;
    int mouth_x;
    int mouth_y;
    int nose_x;
    int nose_y;
};

// 这个与DetPar不同，这个关注于同文件相关的信息
// 而DetPar关注于同文件无关的信息
struct Information_Face
{
    string id;
    string expression;
    int frame;
    string filename;
    Coords coord;
    bool final;
};

// 这张图片的FACS编码，由一个int的数组表示
// 每一个AU都由一个四位数组成，第一位为LR标签，L为1，R为2，第二位为abcde强度标签，a为1，e为5，无强度则为6
// 第三四位为AU标签
struct FACS_Face
{
    string id;
    string expression;
    int AU[100];
};

// 人脸区域的enum
enum FACESECTION
{
    EYE, NOSE, MOUTH
};

// 公用函数，用于判断这个切片没有超出图像范围
bool IsSliceOk(int x_coord, int y_coord, int left, int right, int top, int bottom);
bool IsSliceOk(Point tl, Point br);

// 公用函数，用于从图像中切片，并且flip
Mat GetSlice(const Mat& img, int x_coord, int y_coord, int left, int right, int top, int bottom, bool needflip);
Mat GetSlice(const Mat& img, Point tl, Point br, bool needflip);

// 把Mat输出到文件中
void PrintToFile(Mat m);

#endif //UTIL_H