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
// 每一个AU都由一个四位数组成，第一位为LR标签，L为1，R为2，第二位为abcde强度标签，a为1，e为5
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

/**
 * CK数据库的预处理程序
 */
class CK_Preprocessor
{
public:
    /**
     * ctr，设定数据库的地址
     * @param  path 数据库地址
     */
    explicit CK_Preprocessor(QString path);

    /**
     * dtr
     */
    ~CK_Preprocessor();

    /**
     * 预处理path地址中的CK数据库，并且根据需求输出人脸信息的XML
     */
    void generator();

    /**
     * 求所有Gabor滤波后数据的L2范数
     */
    void getL2();

    /**
     * TODO：得到所有Gabor滤波后数据的方差
     */
    void getVariance();

    /**
     * 根据人脸的XML信息，存储为Information的数据结构
     */
    vector<Information_Face> getInformationFromXML();

    /**
     * 根据Information的数据结构, 以及FACESECTION得到那个局部的Slice
     * 输出文件以提供给SVM分类
     * @param vecInfo 人脸图片的信息
     * @param section 需要哪一个局部的Slice
     */
    void outputTxt(vector<Information_Face> vecInfo, vector<FACS_Face> vecFACS, FACESECTION section, int left, int right, int top, int buttom, int au);

    /**
     * 原先的FACS信息由python脚本转换成XML格式
     * 在本程序中，把这些信息加入到图片信息中
     */
    vector<FACS_Face> getFACSInformation();

    /* data */
private:
    Gabor gabor;

    QString src_path;
    QString dst_path;
    char positions_information_name[SLEN];
    char FACS_information_name[SLEN];

    // 禁止复制ctor和assign
    DISALLOW_COPY_AND_ASSIGN(CK_Preprocessor);
};

#endif //GENERATE_H