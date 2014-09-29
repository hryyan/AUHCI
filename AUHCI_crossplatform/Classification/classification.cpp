// classification.h: SVM分类
// 
// Created by Vincent Yan in 2014/06/5

#include "classification.h"

// 当前图片以及对应的人脸参数
extern Mat frame;
extern DetPar frame_detpar;

// AU数量及具体信息，以及AU模型的信息
static const int kAuNum = 16;
static const int kAuIndex[kAuNum] = { 1,  2,  4,  5,  6,  7,  9, 10, 12, 15, 
							  16, 18, 20, 22, 23, 24};
static const char kDirPath[SLEN] = "../AUHCI_crossplatform/AU_MODEL/";

// 是否被初始化
static bool is_inited = false;

// SVM模板及最大最小特征值
svm_model* au_models[kAuNum];
ScaleFactor* au_scales[kAuNum];

// scale的范围
double _lower = -1.0;
double _upper = 1.0;

/**
 * 加载所有SVM模板
 */
void LoadAllModels()
{
    char model_name[40], model_path[SLEN];

    for (int i = 0; i < kAuNum; i++)
    {
        memset(model_path, 0, sizeof(model_path));

        int index = kAuIndex[i];
        sprintf(model_name, "AU_%d.model", index);
        strcat(model_path, kDirPath);
        strcat(model_path, model_name);
        au_models[i] = svm_load_model(model_path);
    }
}

/**
 * 读取行
 * @param  input in:   文件
 * @return       out:  返回该行内容
 */
char* readline(FILE *input)
{
    int len;
	char *line = NULL;
	int max_line_len = 1024;
    line = (char *) malloc(max_line_len*sizeof(char));
    if(fgets(line,max_line_len,input) == NULL)
        return NULL;

    while(strrchr(line,'\n') == NULL)
    {
        max_line_len *= 2;
        line = (char *) realloc(line, max_line_len);
        len = (int) strlen(line);
        if(fgets(line+len,max_line_len-len,input) == NULL)
            break;
    }
    return line;
}

/**
 * 加载比例文件
 * @param  path in:   比例文件地址
 * @return      out:  比例信息
 */
ScaleFactor* LoadScale(char *path)
{
    int max_index = 0;
    int idx;
    double fmin, fmax;
    ScaleFactor *scale_factor_ptr = new ScaleFactor();
	memset(scale_factor_ptr->feature_max, 0, sizeof(scale_factor_ptr->feature_max));
	memset(scale_factor_ptr->feature_min, 0, sizeof(scale_factor_ptr->feature_min));
    double *feature_max_ptr = scale_factor_ptr->feature_max;
    double *feature_min_ptr = scale_factor_ptr->feature_min;

    // step1: 找出特征的数量
    FILE* fp = fopen(path, "r");
    readline(fp);
    readline(fp);

    while (fscanf(fp, "%d %*f %*f\n", &idx) == 1)
        max_index = std::max(idx, max_index);
    rewind(fp);

    // step2: 把所有特征的值进行预定义
    for (int i = 0; i <= max_index; i++)
    {
        feature_max_ptr[i] = -DBL_MAX;
        feature_min_ptr[i] = DBL_MAX;
    }

    // step3: 读取range文件，对特征值的范围进行预定义
    if (fgetc(fp) == 'x')
    {
        if (fscanf(fp, "%lf %lf\n", &_lower, &_upper) != 2)
        {
            qDebug("Range Error!");
            exit(1);            
        }
        while (fscanf(fp, "%d %lf %lf\n", &idx, &fmin, &fmax)==3)
        {
            feature_max_ptr[idx] = fmax;
            feature_min_ptr[idx] = fmin;
        }
    }
    fclose(fp);
    return scale_factor_ptr;
}

/**
 * 加载所有比例
 */
void LoadAllScales()
{
    char scale_name[40], scale_path[SLEN];

    for (int i = 0; i <kAuNum; i++)
    {
        memset(scale_path, 0, sizeof(scale_path));

        int index = kAuIndex[i];
        sprintf(scale_name, "AU_%d.range", index);
        strcat(scale_path, kDirPath);
        strcat(scale_path, scale_name);
        au_scales[i] = LoadScale(scale_path);
    }
}

/**
 * 根据比例获得该值对应的输出
 * @param  idx         in:   索引
 * @param  value       in:   入值
 * @param  scale_index in:   比例的索引
 * @return             out:  按比例缩放后的输出
 */
double Output(int idx, uchar value, int scale_index)
{
	double v = value;
    double* feature_max_ptr = au_scales[scale_index]->feature_max;
    double* feature_min_ptr = au_scales[scale_index]->feature_min;

    if (feature_max_ptr[idx+1] == feature_min_ptr[idx+1])
        return 0;

    if (v == feature_min_ptr[idx+1])
        v = _lower;
    else if (v == feature_max_ptr[idx+1])
        v = _upper;
    else
        v = _lower + (_upper-_lower) *
                (v-feature_min_ptr[idx+1]) /
                (feature_max_ptr[idx+1]-feature_min_ptr[idx+1]);
    return v;
}

/**
 * 把图像矩阵按比例缩放之后得到的SVM节点
 * @param  source      in:   图像矩阵
 * @param  scale_index in:   比例索引
 * @return             out:  返回的SVM节点
 */
svm_node* Scale(Mat &source, int scale_index)
{
    int idx;
	svm_node* nodes = (svm_node*)malloc((source.cols*source.rows+1)*sizeof(svm_node));
    FILE* fp = fopen("scaled", "w+");
    char buff[50];
    for (int i = 0; i < source.rows; i++)
    {
        uchar* it = source.ptr<uchar>(i);
        for (int j = 0; j < source.cols; j++)
        {
            idx = i*source.cols+j;
			//qDebug("value: %d, index: %d", it[j], idx);
            nodes[idx].value = Output(i*source.cols+j, it[j], scale_index);
            nodes[idx].index = idx+1;
			//qDebug("value: %g, index: %d", nodes[idx].value, idx);
            fprintf(fp, "%d:%g ", nodes[idx].index, nodes[idx].value);
        }
        fflush(fp);
    }
    nodes[source.rows*source.cols].index = -1;
    fflush(fp);
    fclose(fp);
	return nodes;
}

/**
 * 分类初始化
 */
void ClassifyInit()
{
	LoadAllModels();
	LoadAllScales();
	is_inited = true;
}

/**
 * 预测辅助函数
 * @param  x     in:   图像矩阵
 * @param  index in:   比例矩阵
 * @return       out:  该AU是否出现
 */
int PredictHelper(Mat& x, int index)
{
	svm_node* nodes = Scale(x, index);
	int on = ceil(svm_predict(au_models[index], nodes));
	free(nodes);
	return on;
}

/**
 * 预测函数
 * @param x      in:   图像矩阵
 * @param output out:  所有AU的信息
 */
void predict(Mat *x, int *output)
{
    if (!is_inited)
        ClassifyInit();
    
    // AU1、AU2
    if (!x[0].empty())
    {
		output[0] = PredictHelper(x[0], 0);
		output[1] = PredictHelper(x[1], 0);

		output[2] = PredictHelper(x[0], 1);
		output[3] = PredictHelper(x[1], 1);
    }

    // AU4、AU5
    if (!x[1].empty())
    {
		output[4] = PredictHelper(x[2], 2);
		output[5] = PredictHelper(x[3], 2);

		output[6] = PredictHelper(x[2], 3);
		output[7] = PredictHelper(x[3], 3);
    }
    // AU6、AU7
    if (!x[2].empty())
    {
		output[8] = PredictHelper(x[4], 4);
		output[9] = PredictHelper(x[5], 4);

		output[10] = PredictHelper(x[4], 5);
		output[11] = PredictHelper(x[5], 5);
    }

    // AU9
    if (!x[3].empty())
    {
		output[12] = PredictHelper(x[6], 6);
		output[13] = PredictHelper(x[7], 6);
    }

    // AU10、AU12、AU15、AU16、AU18、AU20、AU22、AU23、AU24
    if (!x[4].empty())
    {
		output[14] = PredictHelper(x[8], 7);
		output[15] = PredictHelper(x[9], 7);

		output[16] = PredictHelper(x[8], 8);
		output[17] = PredictHelper(x[9], 8);

		output[18] = PredictHelper(x[8], 9);
		output[19] = PredictHelper(x[9], 9);

		output[20] = PredictHelper(x[8], 10);
		output[21] = PredictHelper(x[9], 10);

		output[22] = PredictHelper(x[8], 11);
		output[23] = PredictHelper(x[9], 11);

		output[24] = PredictHelper(x[8], 12);
		output[25] = PredictHelper(x[9], 12);

		output[26] = PredictHelper(x[8], 13);
		output[27] = PredictHelper(x[9], 13);

		output[28] = PredictHelper(x[8], 14);
		output[29] = PredictHelper(x[9], 14);

		output[30] = PredictHelper(x[8], 15);
		output[31] = PredictHelper(x[9], 15);
    }
	for (int i = 0; i < kAuNum; i++)
	{
		if (output[i*2] || output[i*2+1])
			qDebug("%d, AU_%d; true\n", i, kAuIndex[i]);
	}
}

/**
 * 根据图像中的人脸信息选择兴趣区域
 * @param src       in:   源图像矩阵
 * @param left      in:   左边界
 * @param right     in:   右边界
 * @param top       in:   上边界
 * @param bottom    in:   下边界
 * @param det       in:   人脸信息
 * @param section   in:   枚举值，可以为瞳孔、鼻尖、嘴尖
 * @param left_mat  out:  输出脸左侧的兴趣区域
 * @param right_mat out:  输出脸右侧的兴趣区域
 */
void getROI(Mat &src, int left, int right, int top, int bottom, DetPar det, FACESECTION section, Mat& left_mat, Mat& right_mat)
{
	qDebug("Width: %d, Height: %d", src.cols, src.rows);
	qDebug("left: %d, right: %d, top: %d, bottom: %d", left, right, top, bottom);
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
            right_tl = Point(det.rex-right  , det.rey-top);
            right_br = Point(det.rex+left   , det.rey+bottom);
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
            right_br = Point(det.mouthx+left, det.mouthy+bottom);
        }
    }

    if (IsSliceOk(left_tl, left_br))
    {
        left_mat = GetSlice(src, left_tl, left_br, false);
        // qDebug("lefttop_x: %d; lefttop_y: %d; rightbottom_x: %d; rightbottom_y: %d", left_tl.x, left_tl.y, left_br.x, left_br.y);
        cv::imwrite("left_mat.jpg", left_mat);
		PrintToFile(left_mat, "left");
    }
    else
        left_mat  = Mat();

    if (IsSliceOk(right_tl, right_br))
    {
        right_mat = GetSlice(src, right_tl, right_br, true);
        cv::imwrite("right_mat.jpg", right_mat);
        PrintToFile(right_mat, "right");
    }
    else
        right_mat = Mat();
}

/**
 * 给定图片，获得该图片的AU信息
 * @param au        in/out:  AU信息
 * @param gabor_img in:      图像矩阵
 */
void getAU(bool* au_bool, Mat& gabor_img)
{
	Mat m[10];
	int *output = (int*)malloc(kAuNum*2*sizeof(int));
	getROI(gabor_img, 15, 15, 35, 10, frame_detpar, EYE,   m[0], m[1]);
	getROI(gabor_img, 15, 15, 15, 15, frame_detpar, EYE,   m[2], m[3]);
	getROI(gabor_img, 20, 20, 10, 50, frame_detpar, EYE,   m[4], m[5]);
	getROI(gabor_img,  0, 30, 15, 15, frame_detpar, EYE,   m[6], m[7]);
	getROI(gabor_img, 30, 30, 20, 10, frame_detpar, MOUTH, m[8], m[9]);
	
	predict(m, output);
	
	for (int i = 0; i < kAuNum; i++)
    {
        if (output[i*2] || output[i*2+1])
            au_bool[i] = true;
        else
            au_bool[i] = false;
    }
    free(output);
}

/**
 * Another verion for getting au from radius points
 * @param au        in/out: AU Information
 * @param gabor_img in:     img matrix
 */
void getAU2(bool* au_bool, Mat& gabor_img, int radius)
{
    Mat m[10];
    int *output = (int*)malloc(kAuNum*2*sizeof(int));
    typedef std::vector<double> res;
    res au_percent[kAuNum];

    // gaussian_kernel
    Mat gaussian_kernel = cv::getGaussianKernel(2*radius+1, 0);
    for (int i = 0; i < 2*radius+1; i++)
    {
        qDebug("%f", gaussian_kernel.at<double>(0, i));
    }

    DetPar tmp = frame_detpar;
    for (int x_offset = 0-radius; x_offset <= radius; x_offset++)
    {
        for (int y_offset = abs(x_offset)-radius; y_offset <= radius-abs(x_offset); y_offset++)
        {
            frame_detpar.lex = tmp.lex + x_offset;  frame_detpar.ley = tmp.ley + y_offset;
            frame_detpar.rex = tmp.rex + x_offset;  frame_detpar.rey = tmp.rey + y_offset;
            frame_detpar.mouthx = 75 + x_offset  ;  frame_detpar.mouthy = tmp.nosey + 32 + y_offset;

            getROI(gabor_img, 15, 15, 35, 10, frame_detpar, EYE,   m[0], m[1]);
            getROI(gabor_img, 15, 15, 15, 15, frame_detpar, EYE,   m[2], m[3]);
            getROI(gabor_img, 20, 20, 10, 50, frame_detpar, EYE,   m[4], m[5]);
            getROI(gabor_img,  0, 30, 15, 15, frame_detpar, EYE,   m[6], m[7]);
            getROI(gabor_img, 30, 30, 20, 10, frame_detpar, MOUTH, m[8], m[9]);

            predict(m, output);
            for (int i = 0; i < kAuNum; i++)
            {
                if (output[i*2] || output[i*2+1])
                {
                    // gaussian_kernel
                    au_percent[i].push_back(\
                                gaussian_kernel.at<double>(0, radius-abs(x_offset)) * \
                                gaussian_kernel.at<double>(0, radius-abs(y_offset)));
                    // normal_kernel
//                    au_percent[i].push_back(1);
//                    qDebug("%f", au_percent[i].back());
                }
            }
        }
    }

    for (int i = 0; i < kAuNum; i++)
    {
        double sum = 0;
        for (int j = 0; j < au_percent[i].size(); j++)
            sum += au_percent[i].at(j);

        // normal_kernel
//        int k[7] = {0, 1, 4, 9, 16, 25, 36};
//        int s = k[radius]*2 + (radius+1) * 2 - 1;
//        sum /= s;
        if (sum > 0.6)
        {
            qDebug("AU %d on!", kAuIndex[i]);
            au_bool[i] = true;
        }
        else
            au_bool[i] = false;
    }
}

/**
 * Another verion for getting au from radius points
 * @param au        in/out: AU Information
 * @param gabor_img in:     img matrix
 */
void getAU3(bool* au_bool, Mat& gabor_img, int radius)
{
    Mat m[10];
    int *output = (int*)malloc(kAuNum*2*sizeof(int));
    typedef std::vector<double> res;
    res au_percent[kAuNum];

    Mat gaussian_kernel = cv::getGaussianKernel(2*radius+1, 0);
    for (int i = 0; i < 2*radius+1; i++)
        qDebug("%f", gaussian_kernel.at<double>(0, i));


    for (int y_offset = 0-radius; y_offset <= radius; y_offset++)
    {
        frame_detpar.mouthx = 75;       frame_detpar.mouthy = frame_detpar.nosey+32;
        getROI(gabor_img, 30, 30, 20, 10, frame_detpar, MOUTH, m[8], m[9]);

        predict(m, output);
        for (int i = 7; i < kAuNum; i++)
        {
            if (output[i*2] || output[i*2+1])
            {
                au_percent[i].push_back(1);
            }
        }
    }
}
































