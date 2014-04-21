// haarclassifier.h：haar分类器的通用方法
//
// Created by Vincent yan in 2014/04/21

#ifndef HAARCLASSIFIER_H
#define HAARCLASSIFIER_H

#include "util.h"

/**
 * 打开haar分类器
 * @param cascade  in/out: 具体要打开的分类器的指针
 * @param filename in: 分类器文件的名字
 * @param datadir  in: 分类器文件夹的名字
 */
void OpenDetector(cv::CascadeClassifier *cascade,
                  const char *filename);

/**
 * rect是否在enclosing中
 * @param  rect      in：
 * @param  enclosing in：
 * @return
 */
bool InRect(const Rect& rect, const Rect& enclosing);

/*
 * 强制将Rect收敛在img内
 * @param ix    io: 调整前后Rect的坐标x
 * @param iy    io: 调整前后Rect的坐标y
 * @param ncols io: 调整前后Rect的宽度
 * @param nrows io: 调整前后Rect的高度
 * @param img   in: 源图像
 */
void ForceRectIntoImg(int &ix,
                      int &iy,
                      int &ncols,
                      int &nrows,
                      const Image &img);

/**
 * 强制将Rect收敛在img内
 * @param rect io: 调整前后的Rect
 * @param img  in: 源图像
 */
void ForceRectIntoImg(Rect &rect, const Image &img);

/**
 * 获得Rect的中心
 * @param x        in/out：
 * @param y        in/out：
 * @param featrect in：人脸
 */
void RectToImgFrame(double &x, double &y, const Rect &featrect);

/**
 * 将ROI的坐标映射到源图像中
 * @param feats      io: 人脸参数
 * @param searchrect io: ROI
 */
void DiscountSearchRegion(vec_Rect &feats, const Rect &searchrect);

/**
 * 将ROI覆盖到IMAGE上，并对其中人脸进行检测
 * @param  img             in: 源图像
 * @param  cascade         in: 分类器
 * @param  searchrect      in: roi
 * @param  minwidth_pixels in: 最小人脸像素
 * @return                 每个人脸一个矩阵
 */
vec_Rect Detect(const Image &img,
                cv::CascadeClassifier *cascade,
                const Rect *searchrect,
                int minwidth_pixels);

#endif // HAARCLASSIFIER_H