// classification.h: SVM分类
// 
// Created by Vincent Yan in 2014/06/5

#include "classification.h"
#include "source.h"
#include "facedet.h"
#include "eyedet.h"
#include "otherdet.h"
#include "gabor.h"
#include "util.h"
#include "generate.h"

extern Mat frame;
extern DetPar frame_detpar;

const int RESIZE_WIDTH  = 150;
const int RESIZE_HEIGHT = 150;

const int AU_NUM = 16;
const int AU_INDEX[AU_NUM] = { 1,  2,  4,  5,  6,  7,  9, 10, 12, 15, 
                              16, 18, 20, 22, 23, 24};
const char DIRPATH[SLEN] = ".\\AU_MODEL\\";
bool isInited = false;
svm_model* au_models[AU_NUM];

void Mat2SvmNode(Mat &source, svm_node* &x)
{
    int height = source.rows;
    int width  = source.cols;
    x = (struct svm_node*)malloc((height*width+1)*sizeof(struct svm_node));

    uchar *it = source.ptr<uchar>(0);
    int i = 0;
    for (i = 0; i < height * width; i++)
    {
        x[i].index = i+1;
        if (it[i] == 0)
            x[i].value = -1.;
        else if (it[i] == 255)
            x[i].value = 1.;
        else
            x[i].value = -1. + 2. * (double)it[i] / 255.;
    }
    x[height*width].index = -1;
}

void load_all_model()
{
    char modelname[40], modelpath[SLEN];

    for (int i = 0; i < AU_NUM; i++)
    {
        memset(modelpath, 0, sizeof(modelpath));

        int index = AU_INDEX[i];
        sprintf(modelname, "AU_%d.txt.model", index);
        strcat(modelpath, DIRPATH);
        strcat(modelpath, modelname);
        au_models[i] = svm_load_model(modelpath);
    }
    isInited = true;
}

void predict(Mat *x, int *output)
{
    if (!isInited)
        load_all_model();
    output = (int*)malloc(AU_NUM*2*sizeof(int));
    // AU1、AU2
    if (!x[0].empty())
    {
        svm_node *nodes;
        Mat2SvmNode(x[0], nodes);
		QTime time1 = QTime::currentTime();
        output[0] = ceil(svm_predict(au_models[0], nodes));
		output[1] = ceil(svm_predict(au_models[0], nodes));
        output[2] = ceil(svm_predict(au_models[1], nodes));
		output[3] = ceil(svm_predict(au_models[1], nodes));
		QTime time2 = QTime::currentTime();
		qDebug() << time1.msecsTo(time2);
		free(nodes);
    }

    // AU4、AU5
    if (!x[1].empty())
    {
        svm_node *nodes;
        Mat2SvmNode(x[1], nodes);
        output[4] = ceil(svm_predict(au_models[2], nodes));
		output[5] = ceil(svm_predict(au_models[2], nodes));
		output[6] = ceil(svm_predict(au_models[3], nodes));
        output[7] = ceil(svm_predict(au_models[3], nodes));
		free(nodes);
    }
    // AU6、AU7
    if (!x[2].empty())
    {
        svm_node *nodes;
        Mat2SvmNode(x[2], nodes);
        output[8] = ceil(svm_predict(au_models[4], nodes));
		output[9] = ceil(svm_predict(au_models[4], nodes));
		output[10] = ceil(svm_predict(au_models[5], nodes));
        output[11] = ceil(svm_predict(au_models[5], nodes));
		free(nodes);
    }

    // AU9
    if (!x[3].empty())
    {
        svm_node *nodes;
        Mat2SvmNode(x[3], nodes);
        output[12] = ceil(svm_predict(au_models[6], nodes));
		output[13] = ceil(svm_predict(au_models[6], nodes));
		free(nodes);
    }

    // AU10、AU12、AU15、AU16、AU18、AU20、AU22、AU23、AU24
    if (!x[4].empty())
    {
        svm_node *nodes;
        Mat2SvmNode(x[4], nodes);
        output[14] = ceil(svm_predict(au_models[7], nodes));
		output[15] = ceil(svm_predict(au_models[7], nodes));
		output[16] = ceil(svm_predict(au_models[8], nodes));
		output[17] = ceil(svm_predict(au_models[8], nodes));
		output[18] = ceil(svm_predict(au_models[9], nodes));        
		output[19] = ceil(svm_predict(au_models[9], nodes));
        output[20] = ceil(svm_predict(au_models[10], nodes));
		output[21] = ceil(svm_predict(au_models[10], nodes));
        output[22] = ceil(svm_predict(au_models[11], nodes));
		output[23] = ceil(svm_predict(au_models[11], nodes));
        output[24] = ceil(svm_predict(au_models[12], nodes));
		output[25] = ceil(svm_predict(au_models[12], nodes));
        output[26] = ceil(svm_predict(au_models[13], nodes));
		output[27] = ceil(svm_predict(au_models[13], nodes));
        output[28] = ceil(svm_predict(au_models[14], nodes));
		output[29] = ceil(svm_predict(au_models[14], nodes));
        output[30] = ceil(svm_predict(au_models[15], nodes));
		output[31] = ceil(svm_predict(au_models[15], nodes));
		free(nodes);
    }
	for (int i = 0; i < AU_NUM; i++)
	{
		if (output[i*2] || output[i*2+1])
			qDebug("%d, AU_%d; true\n", i, AU_INDEX[i]);
	}
}

void getROI(const Mat &src, int left, int right, int top, int bottom, DetPar det, FACESECTION section, Mat& left_mat, Mat& right_mat)
{
    Point left_tl, left_br, right_tl, right_br;
    if (section == EYE)
    {
        if (det.lex != 99999 && det.ley != 99999)
        {
            left_tl  = Point(det.lex-left    , det.ley-top);
            left_br  = Point(det.lex+right   , det.ley+bottom);
        }

        if (det.rex != 99999 && det.rey != 99999)
        {
            right_tl = Point(det.rex-left    , det.rey-top);
            right_br = Point(det.rex+right   , det.rey+bottom);
        }
    }
    else if (section == NOSE)
    {
        if (det.nosex != 99999 && det.nosey != 99999)
        {
            left_tl  = Point(det.nosex-left  , det.nosey-top);
            left_br  = Point(det.nosex       , det.nosey+bottom);

            right_tl = Point(det.nosex       , det.nosey-top);
            right_br = Point(det.nosex+right , det.nosey+bottom);
        }
    }
    else if (section == MOUTH)
    {
        if (det.mouthx != 99999 && det.mouthy != 99999)
        {
            left_tl  = Point(det.mouthx-left , det.mouthy-top);
            left_br  = Point(det.mouthx      , det.mouthy+bottom);

            right_tl = Point(det.mouthx      , det.mouthy-top);
            right_br = Point(det.mouthx+right, det.mouthy+bottom);
        }
    }
    if (left_tl.x > 0 && left_tl.y > 0 &&
        left_br.x > 0 && left_br.y > 0)
	{
        left_mat  = Mat(src, Rect(left_tl, left_br));
		qDebug("lefttop_x: %d; lefttop_y: %d; rightbottom_x: %d; rightbottom_y: %d", left_tl.x, left_tl.y, left_br.x, left_br.y);
		cv::imwrite("left_mat.jpg", left_mat);
	}
    else
        left_mat  = Mat();

    if (right_tl.x > 0 && right_tl.y > 0 &&
        right_br.x > 0 && right_br.y > 0)
	{
        flip(Mat(src, Rect(right_tl, right_br)), right_mat, 1);
		cv::imwrite("right_mat.jpg", right_mat);
	}
    else
        right_mat = Mat();
}

void print_to_file(Mat m)
{
    FILE* fp = fopen("mat", "wb+");
    fputs("1 ", fp);
    char s[50];
    double value;
	uchar *it;

	for (int i = 0; i < m.rows; i++)
	{
		it = m.ptr<uchar>(i);
		for (int j = 0; j < m.cols; j++)
		{
			memset(s, 0, sizeof(s));
			sprintf(s, "%d:%d ", i*m.cols+j+1, it[j]);
			fputs(s, fp);
			if (i*m.cols+j == 1000)
				fflush(fp);
		}
	}
   
    fputs("\n", fp);
    fclose(fp);
}

int mai ()
{
	char pic_path[SLEN];
	char pic_warehouse[SLEN] = ".\\Pic_warehouse\\";
	char pic_name[SLEN];

	Mat gabor_img;
	Mat m[10];

	int* output;

	for (int i = 0; i < 10; ++i)
	{
		memset(pic_path, 0, sizeof(pic_path));
		sprintf(pic_name, "%d.png", i);
		strcat(pic_path, pic_warehouse);
		strcat(pic_path, pic_name);

		isPicInited(pic_path);

		printFace();
		cv::resize(frame, frame, Size(RESIZE_WIDTH, RESIZE_HEIGHT));
		frame_detpar.width = RESIZE_WIDTH;				frame_detpar.height = RESIZE_HEIGHT;
		frame_detpar.x	   = frame_detpar.width / 2;	frame_detpar.y		= frame_detpar.height / 2;
		DetectEyes(frame);
		DetectMouth(frame);
		DetectNose(frame);
		cv::imwrite("g.jpg", frame);
		frame = cv::imread("g.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		qDebug("frame type: %d, channels: %d", frame.type(), frame.channels());
		gabor_img = printGabor();
		cv::imwrite("g.jpg", gabor_img);

		gabor_img = cv::imread("g.jpg", CV_LOAD_IMAGE_GRAYSCALE);
		getROI(gabor_img, 15, 15, 35, 10, frame_detpar, EYE,   m[0], m[1]);
		print_to_file(m[0]);
		getROI(gabor_img, 15, 15, 15, 15, frame_detpar, EYE,   m[2], m[3]);
		getROI(gabor_img, 20, 20, 10, 50, frame_detpar, EYE,   m[4], m[5]);
		getROI(gabor_img,  0, 30, 15, 15, frame_detpar, EYE,   m[6], m[7]);
		getROI(gabor_img, 30, 30, 20, 10, frame_detpar, MOUTH, m[8], m[9]);

		predict(m, output);
	}
	free(output);
}