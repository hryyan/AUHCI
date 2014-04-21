// Preprocessor.h: 用于生成训练样本
// 
// Created by Vincent Yan in 2014/03/25

#ifndef GENERATE_H
#define GENERATE_H

#include "util.h"
#include "gabor.h"
#include "source.h"
#include "facedet.h"
#include "eyedet.h"
#include "otherdet.h"
#include "tinyxml2.h"

class Preprocessor
{
public:
    /**
     * ctr，设定数据库的地址
     * @param  path 数据库地址
     */
    explicit Preprocessor(QString path);

    /**
     * dtr
     */
    ~Preprocessor();

    /**
     * 预处理path地址中的CK数据库
     */
    void generator();

    /**
     * 求所有Gabor滤波后数据的L2范数
     */
    void getL2();

    /**
     * 得到所有Gabor滤波后数据的方差
     */
    void getVariance();

    /* data */
private:
    Gabor gabor;

    QString src_path;
    QString dst_path;

    // 禁止复制ctor和assign
    DISALLOW_COPY_AND_ASSIGN(Preprocessor);
};

#endif //GENERATE_H