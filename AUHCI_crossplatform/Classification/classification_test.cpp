#include "classification_test.h"

// 当前图片以及对应的人脸参数
extern Mat frame;
extern DetPar frame_detpar;

static const int kAuNum = 16;

void classify_test()
{
    char pic_path[SLEN];
    char pic_warehouse[SLEN] = ".\\Pic_warehouse\\";
    char pic_name[SLEN];

    Mat gabor_img;
    Mat m[10];

    int* output;
    output = (int*)malloc(kAuNum*2*sizeof(int));
    for (int i = 0; i < 10; ++i)
    {
        memset(pic_path, 0, sizeof(pic_path));
        sprintf(pic_name, "%d.png", i);
        strcat(pic_path, pic_warehouse);
        strcat(pic_path, pic_name);

        InitPic(pic_path);

        //PrintFaceToFrame();
        DetectEyes();
        DetectMouth();
        DetectNose();
        cv::imwrite("g.jpg", frame);
        frame = cv::imread("g.jpg", CV_LOAD_IMAGE_GRAYSCALE);
        qDebug("frame type: %d, channels: %d", frame.type(), frame.channels());
        gabor_img = printGabor();
        cv::imwrite("g.jpg", gabor_img);

        gabor_img = cv::imread("g.jpg", CV_LOAD_IMAGE_GRAYSCALE);
        getROI(gabor_img, 15, 15, 35, 10, frame_detpar, EYE,   m[0], m[1]);
        getROI(gabor_img, 15, 15, 15, 15, frame_detpar, EYE,   m[2], m[3]);
        getROI(gabor_img, 20, 20, 10, 50, frame_detpar, EYE,   m[4], m[5]);
        getROI(gabor_img,  0, 30, 15, 15, frame_detpar, EYE,   m[6], m[7]);
        getROI(gabor_img, 30, 30, 20, 10, frame_detpar, MOUTH, m[8], m[9]);
//        PrintToFile(m[8]);

        predict(m, output);
    }
    free(output);
}
