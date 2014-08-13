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
bool InitCamera();

/**
 * 检测视频是否被正确开启
 * @param  in:	视频的地址
 * @return		是否被正确开启
 */
bool InitVideo(std::string);

/**
 * 照片是否已经被打开
 * @param  in:  照片的地址
 * @return		是否已经被打开
 */
bool InitPic(std::string);

/**
 * TODO: 打印视频流与打印照片没有被正确的区分开
 */

#endif // SOURCE_H