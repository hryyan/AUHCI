// Preprocessor.h: 用于生成训练样本
//
// Created by Vincent Yan in 2014/03/25
// Modified by Vincent Yan in 2014/06/15

#ifndef GENERATE_H
#define GENERATE_H

#include "Util/util.h"
#include "Feature/gabor.h"
#include "Util/source.h"
#include "Detection/facedet.h"
#include "Detection/eyedet.h"
#include "Detection/otherdet.h"
#include "Util/tinyxml2.h"

static const char kPositionInformation[SLEN] = "CK_database_information.xml";
static const char kFACSInformation[SLEN] = "FACS_information.xml";

/**
 * CK数据库的预处理程序
 */
class CK_Preprocessor
{
public:
    explicit CK_Preprocessor(QString path);
    ~CK_Preprocessor();

    // 输出40张Gabor滤波图，用来做解释，除了这个压根没用
    void Output40Gabor();

    // 对之前生成的40张Gabor滤波图取L2范数，成一张图，也没用
    void Merge40Gabor();

    // 把上述两个过程都在内存中完成，输出的图精度有保证
    void OutputGabor();

    // TODO: 得到所有Gabor滤波后数据的方差
    void OutputVariance();

    // 根据人脸的XML信息，存储为Information的数据结构
    vector<Information_Face> OutputInformationFromXML();

    // 原先的FACS信息要使用python脚本转换为XML格式
    // 然后在本程序中，切片的时候查找信息，进行Label
    vector<FACS_Face> OutputFACSFromXML();

    // 输出数据库中所有符合某AU的切片合适的片，并输出带Label的文件
    void OutputAULabelSlice(vector<Information_Face>& vecInfo, vector<FACS_Face>& vecFACS, FACESECTION section, int left, int right, int top, int bottom, int au);

private:
    QString src_path;
    QString dst_path;

    // 禁止复制ctor和assign
    DISALLOW_COPY_AND_ASSIGN(CK_Preprocessor);
};

void generator_samples();
#endif
