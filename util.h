// util.h: ����Ķ���
// 
// Created by Vincent Yan in 2014/03/17

#ifndef UTIL_H
#define UTIL_H

// ��׼C��ͷ�ļ�
#include <cstring>

// STL��ͷ�ļ�
#include <vector>
#include <string>
#include <utility>
using std::string;
using std::vector;
using std::pair;

// QT��ͷ�ļ�
#include <QImage>
#include <QtDebug>
#include <QTime>
#include <QString>

// OpenCV��ͷ�ļ�
#include "opencv2\opencv.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\gpu\gpu.hpp"
#include "opencv2\core\internal.hpp"
using cv::Rect;
using cv::Mat;
using cv::VideoCapture;
using cv::Point;
using cv::Scalar;
using cv::Size;
using cv::sqrt;

#include "source.h"

// �Ƿ�ʹ��GPU����
#define USE_GPU

// ����һЩ����ʹ�õ�����
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
         rot(INVALID),
         yaw(INVALID),
         eyaw(EYAW(INVALID))
         {

         };
};

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
 * ��windows��\ת����ͨ�õ�/
 * @param s in: ��Ҫת�����ַ���
 */
void ConvertBackslashesToForwardAndStripFinalSlash(char* s);

#endif //UTIL_H