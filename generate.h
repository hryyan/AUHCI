// Preprocessor.h: 用于生成训练样本
// 
// Created by Vincent Yan in 2014/03/25

#ifndef GENERATE_H
#define GENERATE_H

#include "util.h"
#include "gabor.h"
#include "source.h"
#include "facedet.h"

class Preprocesor
{
public:
    /**
     * ctr，设定数据库的地址
     * @param  path 数据库地址
     */
    explicit Preprocesor(QString path);

    /**
     * dtr
     */
    ~Preprocesor();

    /**
     * 预处理path地址中的CK数据库
     */
    void generator();

    /* data */
private:
    Gabor gabor;

    QString path;

    // 禁止复制ctor和assign
    DISALLOW_COPY_AND_ASSIGN(Preprocesor);
};

#endif //GENERATE_H