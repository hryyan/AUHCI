// facedet.h: 来源获取（摄像头、视频、图片）
//
// Created by Vincent Yan in 2014/03/18

#include "source.h"

// 全局变量，整个程序中数据流的核心，将frame提供给图像处理的单元使用
Mat frame;

// 数据流中的人脸信息
DetPar frame_detpar;

// 视频流的传递
VideoCapture cap;

/**
* 检测摄像头是否被正确开启
* @return         是否已经正确开启
*/
bool isCameraInited()
{
    cap = VideoCapture(0);
    if (!cap.isOpened())
    return false;
    return true; 
}

/**
* 检测视频是否被正确开启
* @param  in:    视频的地址
* @return        是否被正确开启
*/
bool isVideoInited(string path)
{
    cap = VideoCapture(path);
    if (!cap.isOpened())
    return false;
    return true;
}

/**
* 照片是否已经被打开
* @param  in:  照片的地址
* @return        是否已经被打开
*/
bool isPicInited(string path)
{
    frame = cv::imread(path, -1);
    if (frame.rows == 0 || frame.cols == 0)
        return false;
    return true;
}

/**
* TODO: 打印视频流与打印照片没有被正确的区分开
*/

/**
* 如果Capture被打开，则认为打开摄像头或者视频，如果Capture没被打开，则认为打开照片
* @return        返回的是frame
*/
Mat printScreen()
{
    if (cap.isOpened())
    cap >> frame;
    
    return frame;
}
 