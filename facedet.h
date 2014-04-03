// facedet.h: 人脸检测
// 
// Created by Vincent Yan in 2014/03/17

#include "util.h"

#ifndef FACEDET_H
#define FACEDET_H

class FaceDet
{
public:
    /**
     * 打开人脸检测器(Haar)
     * @param datadir   in: 分类器的目录地址
     */
    void OpenFaceDetector_(const char* datadir);

    /**
     * 检测人脸
     * @param img       in: 灰度图
     * @param multiface in: 是否检测多张人脸
     * @param minwidth  in: 检测的最小人脸
     */
    void DetectFaces_(const Image& img, bool multiface, int minwidth);

    /**
     * 获得下一张人脸的参数
     * @return  下一张人脸的参数
     */
    const DetPar NextFace_();

    /**
     * ctor
     */
    FaceDet(){};

private:
    // 检测到的所有人脸的参数Vector
    vector<DetPar> detpars_;

    // 现在所定位的人脸的index
    int            iface_;

    // 禁止复制ctor和assign
    DISALLOW_COPY_AND_ASSIGN(FaceDet);
};

static FaceDet faceDet;

/**
 * 打印人脸
 * @return QImage
 */
QImage* printFace();

#endif //FACEDET_H