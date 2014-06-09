// classification.h: SVM分类
// 
// Created by Vincent Yan in 2014/06/5

#include "classification.h"
#include "util.h"

const int AU_NUM = 30;
const int AU_INDEX[AU_NUM] = {1, 2, 4, 5, 6, 7, 9, 10, 11,
                          12, 14, 15, 16, 17, 18, 20,
						  21, 22, 23, 24, 25, 26};
const char DIRPATH[SLEN] = ".\\AU_MODEL\\";
svm_model* au_models[AU_NUM];

void Mat2SvmNode(Mat &source, svm_node *x)
{
    int height = source.rows;
    int width  = source.cols;
    x = (svm_node*)malloc((height*width+1)*sizeof(svm_node));

    uchar *it = source.ptr<uchar>(0);
    int i = 0;
    for (i = 0; i < height * width; i++)
    {
        x[i].index = i+1;
        x[i].value = it[i];
        if (it[i] == 0)
            x[i].value = -1.;
        else if (it[i] == 255)
            x[i].value = 1;
        else
            x[i].value = -1 + 2 * it[i] / 255;
    }
    x[i].index = -1;
}

void load_all_model()
{
    char modelname[40], modelpath[SLEN];
    strcat(modelpath, DIRPATH);

    for (int i = 0; i < AU_NUM; i++)
    {
        int index = AU_INDEX[i];
        sprintf(modelname, "AU_%d.model", index);
        strcat(modelpath, modelname);
        au_models[index] = svm_load_model(modelpath);
    }
}

void predict(Mat *x, int *output)
{
    output = (int*)malloc(AU_NUM*sizeof(int));
    // AU1、AU2
    if (!x[0].empty())
    {
        svm_node *nodes;
        Mat2SvmNode(x[0], nodes);
        output[0] = ceil(svm_predict(au_models[0], nodes));
        output[1] = ceil(svm_predict(au_models[1], nodes));
    }

    // AU4、AU5
    if (!x[1].empty())
    {
        svm_node *nodes;
        Mat2SvmNode(x[1], nodes);
        output[2] = ceil(svm_predict(au_models[2], nodes));
        output[3] = ceil(svm_predict(au_models[3], nodes));
    }
    // AU6、AU7
    if (!x[2].empty())
    {
        svm_node *nodes;
        Mat2SvmNode(x[2], nodes);
        output[4] = ceil(svm_predict(au_models[4], nodes));
        output[5] = ceil(svm_predict(au_models[5], nodes));
    }

    // AU9
    if (!x[3].empty())
    {
        svm_node *nodes;
        Mat2SvmNode(x[3], nodes);
        output[6] = ceil(svm_predict(au_models[6], nodes));
    }

    // AU10、AU12、AU15、AU16、AU18、AU20、AU22、AU23、AU24
    if (!x[4].empty())
    {
        svm_node *nodes;
        Mat2SvmNode(x[4], nodes);
        output[7] = ceil(svm_predict(au_models[7], nodes));
        output[8] = ceil(svm_predict(au_models[8], nodes));
        output[9] = ceil(svm_predict(au_models[9], nodes));
        output[10] = ceil(svm_predict(au_models[10], nodes));
        output[11] = ceil(svm_predict(au_models[11], nodes));
        output[12] = ceil(svm_predict(au_models[12], nodes));
        output[13] = ceil(svm_predict(au_models[13], nodes));
        output[14] = ceil(svm_predict(au_models[14], nodes));
        output[15] = ceil(svm_predict(au_models[15], nodes));
    }
}

