// source.h: 来源获取（摄像头、视频、图片）
// 
// Created by Vincent Yan in 2014/03/18

#ifndef SOURCE_H
#define SOURCE_H

#include "util.h"

/**
 * 检测摄像头是否被正确开启
 * @return 		是否已经正确开启
 */
bool isCameraInited();

/**
 * 检测视频是否被正确开启
 * @param  in:	视频的地址
 * @return		是否被正确开启
 */
bool isVideoInited(std::string);

/**
 * 照片是否已经被打开
 * @param  in:  照片的地址
 * @return		是否已经被打开
 */
bool isPicInited(std::string);

/**
 * TODO: 打印视频流与打印照片没有被正确的区分开
 */

/**
 * 如果Capture被打开，则认为打开摄像头或者视频，如果Capture没被打开，则认为打开照片
 * @return		返回的是frame
 */
Mat printScreen();

#endif // SOURCE_H