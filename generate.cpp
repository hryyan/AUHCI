// CK_Preprocessor.cpp: 用于生成训练样本
// 
// Created by Vincent Yan in 2014/03/17
// Modified by Vincent Yan in 2014/06/16

#include "generate.h"
#include "gabor.h"
#include "source.h"
#include "facedet.h"
#include "eyedet.h"
#include "otherdet.h"
#include "tinyxml2.h"
#include "QDir"
// #include "VO_Features.h"
// #include "VO_LBPFeatures.h"

// source中的每一帧
extern Mat frame;
extern DetPar frame_detpar;

/**
 * 将文字的FACS信息解码为数字
 * @param facs 某张图片的FACS信息
 * @param AU   解码后的FACS信息
 */
void decode(char* facs, int* AU)
{
    char direction = ' ', magnitude = ' ';
    int a = 0, tmp = 0;
    for (int i = 0; facs[i] != 0; )
    {
        a = 0; tmp = 0;
        if (facs[i] == 'R')
        {
            a += 2000; i++;
        }
        else if (facs[i] == 'L')
        {
            a += 1000; i++;
        }
        else if (facs[i] == 'B')
        {
            i++;
        }
        sscanf(facs+i, "%d+", &tmp);
        if (tmp < 10) i++;
        else i += 2;
        switch (facs[i])
        {
        case 'a': i+=2; a += 100; break;
        case 'b': i+=2; a += 200; break;
        case 'c': i+=2; a += 300; break;
        case 'd': i+=2; a += 400; break;
        case 'e': i+=2; a += 500; break;
        case '+': i++; a += 600; break;
        case 0: i++; a += 600; break;
        default: break;
        }
        a += tmp;
        AU[a%100] = a / 100;
    }
}

CK_Preprocessor::CK_Preprocessor(QString p)
{
    // 该数据库位于的位置
    src_path = p;

    // 输出的文件保存的位置
    dst_path = QDir::currentPath() + "/AfterPrecess_Pre";
}

CK_Preprocessor::~CK_Preprocessor()
{

}

/**
 * 输出40张Gabor滤波图，用来做解释，除了这个压根没用
 */
void CK_Preprocessor::Output40Gabor()
{
    QFileInfo dirinfo_outter, dirinfo_inner, fileinfo;
    QFileInfoList middle_list, inner_list;
    QDir outter_dir, middle_dir, inner_dir, dir;
    QImage* tmp_jpg;
    QString filepath, filename;
    Mat face, gabor_result;
    cv::gpu::GpuMat g_src, g_dst;
    int index;

	// 初始化检测器
	InitFaceDet();

    #ifdef OUTPUT_XML
    FILE *fp = fopen(kPositionInformation, "w+");
    tinyxml2::XMLPrinter printer(fp);
    printer.OpenElement("Main");
    #endif

    QStringList filters;
    filters << "*.png";

    QString store_path("");

    inner_dir.setFilter(QDir::Files | QDir::Readable);
    inner_dir.setNameFilters(filters);

    outter_dir.cd(this->src_path);
    outter_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    middle_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList list = outter_dir.entryInfoList();
    foreach(dirinfo_outter, list)
    {
        middle_dir.cd(dirinfo_outter.absoluteFilePath());
        qDebug() << dirinfo_outter.absoluteFilePath();
        middle_list = middle_dir.entryInfoList();

        #ifdef OUTPUT_XML
        printer.OpenElement("People");
        printer.PushAttribute("name", dirinfo_outter.baseName().toStdString().c_str());
        #endif

        foreach(dirinfo_inner, middle_list)
        {
            inner_dir.cd(dirinfo_inner.absoluteFilePath());
            qDebug() << dirinfo_inner.absoluteFilePath();
            inner_list = inner_dir.entryInfoList();

            #ifdef OUTPUT_XML
            printer.OpenElement("Expression");
            printer.PushAttribute("class", dirinfo_inner.baseName().toStdString().c_str());
            #endif

            // 显示这张图片在这个表情中的帧数，从1开始
            index = 0;
            foreach(fileinfo, inner_list)
            {
                frame = cv::imread(fileinfo.absoluteFilePath().toStdString());
                qDebug() << fileinfo.absoluteFilePath();
                
                index++;
                #ifdef OUTPUT_XML
                printer.OpenElement("Frame");
                printer.PushAttribute("Num", index);
                printer.PushAttribute("jpg", fileinfo.baseName().toStdString().c_str());
                #endif

                // 识别人脸
                PrintFaceToFrame();
                filepath = dst_path+store_path+dirinfo_outter.fileName()+"/"+dirinfo_inner.fileName();
                filename = "/"+fileinfo.baseName()+"face.jpg";
                qDebug() << filename;

                // 识别人眼，嘴和鼻尖
                DetectEyes();
                if (index == 1)
                {
                    DetectMouth();
                    DetectNose();
                }

                #ifdef OUTPUT_XML 
                printer.OpenElement("Left_Eye_Position");
                printer.OpenElement("X");
                printer.PushText(frame_detpar.lex);
                printer.CloseElement();
                printer.OpenElement("Y");
                printer.PushText(frame_detpar.ley);
                printer.CloseElement();
                printer.CloseElement();

                printer.OpenElement("Right_Eye_Position");
                printer.OpenElement("X");
                printer.PushText(frame_detpar.rex);
                printer.CloseElement();
                printer.OpenElement("Y");
                printer.PushText(frame_detpar.rey);
                printer.CloseElement();
                printer.CloseElement();
                
                printer.OpenElement("Mouth_Position");
                printer.OpenElement("X");
                printer.PushText(frame_detpar.mouthx);
                printer.CloseElement();
                printer.OpenElement("Y");
                printer.PushText(frame_detpar.mouthy);
                printer.CloseElement();
                printer.CloseElement();

                printer.OpenElement("Nose_Position");
                printer.OpenElement("X");
                printer.PushText(frame_detpar.nosex);
                printer.CloseElement();
                printer.OpenElement("Y");
                printer.PushText(frame_detpar.nosey);
                printer.CloseElement();
                printer.CloseElement();

                printer.CloseElement();
                #endif //OUTPUT_XML

                #ifdef SAVE_FACE
                QString p = filepath + filename;
                qDebug() << p;
                cv::imwrite(p.toStdString(), face);
                #endif // SAVE_FACE

                #ifdef CALCULATE_GABOR
                //  #ifdef USE_OPENCV_GPU
                //  int top_buttom = face.rows * BORDER_FRAC;
                //  int left_right = face.cols * BORDER_FRAC;
                //  copyMakeBorder(face, face, 0, top_buttom, 0, left_right, cv::BORDER_REPLICATE);
                //  #else
                //  int top_buttom = face.rows * BORDER_FRAC * 0.335;
                //  int left_right = face.cols * BORDER_FRAC * 0.335;
                //  copyMakeBorder(face, face, top_buttom, 0, left_right, 0, cv::BORDER_REPLICATE);
                //  #endif

                for (int i = 0; i < 5; i++) // 尺度
                {
                    for (int j = 0; j < 8; j++) // 方向
                    {
                        // Begin: 衡量时间性能的QTime
                        QTime time1 = QTime::currentTime();
                        gabor_result = printGabor_(face, i, j);
                        // End: 衡量时间性能的QTime
                        QTime time2 = QTime::currentTime();
                        qDebug() << time1.msecsTo(time2);
                        //Mat gabor_cropped = Mat(gabor_result, Rect(Point(150-101+1, 150-101+1), Point(gabor_result.cols, gabor_result.rows)));
                        tmp_jpg = Mat2QImage(gabor_result);

                        filepath = dst_path+store_path+dirinfo_outter.fileName()+"/"+dirinfo_inner.fileName()+"/"+fileinfo.baseName();
                        filename = QString("gabor_%1_%2.jpg").arg(i).arg(j);
                        qDebug() << filepath; 
                        if (!dir.exists(filepath))
                            dir.mkpath(filepath);
                        tmp_jpg->save(filepath+"/"+filename);
                        delete tmp_jpg;
                    }
                }
                #endif // CALCULATE_GABOR
            }
            #ifdef OUTPUT_XML 
            printer.CloseElement();
            #endif // OUTPUT_XML
        }
        #ifdef OUTPUT_XML 
        printer.CloseElement();
        #endif // OUTPUT_XML
    }
    #ifdef OUTPUT_XML 
    printer.CloseElement();
    #endif // OUTPUT_XML
}

/**
 * 对之前生成的40张Gabor滤波图取L2范数，成一张图，也没用
 */
void CK_Preprocessor::Merge40Gabor()
{
    QFileInfo dirinfo_outter, dirinfo_inner, dirinfo_innest, fileinfo;
    QFileInfoList middle_list, inner_list, innest_list;
    QDir outter_dir, middle_dir, inner_dir, innest_dir, dir;
    QImage* tmp_jpg;
    QString filepath, filename;
    Mat mat_restore[40], mat_tmp;

    QStringList filters;
    filters << "*.jpg";

    innest_dir.setFilter(QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);
    innest_dir.setNameFilters(filters);

    outter_dir.cd(this->dst_path);
    outter_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    middle_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    inner_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    QFileInfoList list = outter_dir.entryInfoList();
    foreach(dirinfo_outter, list) //S010, S011
    {
        middle_dir.cd(dirinfo_outter.absoluteFilePath());
        qDebug() << dirinfo_outter.absoluteFilePath();
        middle_list = middle_dir.entryInfoList();

        foreach(dirinfo_inner, middle_list) //001, 002
        {
            inner_dir.cd(dirinfo_inner.absoluteFilePath());
            qDebug() << dirinfo_inner.absoluteFilePath();
            inner_list = inner_dir.entryInfoList();

            foreach(dirinfo_innest, inner_list) //S010_001_01594215
            {
                innest_dir.cd(dirinfo_innest.absoluteFilePath());
                qDebug() << dirinfo_innest.absoluteFilePath();
                innest_list = innest_dir.entryInfoList();

                int index = 0;
                foreach(fileinfo, innest_list)
                {
                    qDebug() << fileinfo.absoluteFilePath();
                    mat_restore[index] = cv::imread(fileinfo.absoluteFilePath().toStdString());
                    mat_restore[index].convertTo(mat_restore[index], CV_32F);
                    cv::pow(mat_restore[index], 2, mat_restore[index]);
                    index++;
                }

                mat_tmp = mat_restore[0].clone();
                for (int i = 1; i < 40; ++i)
                {
                    cv::add(mat_tmp, mat_restore[i], mat_tmp);
                }
                cv::pow(mat_tmp, 0.5, mat_tmp);
                cv::normalize(mat_tmp, mat_tmp, 0, 255, CV_MINMAX, CV_8UC1);
                QString p = dirinfo_innest.absoluteFilePath() +"_merged_normalized.jpg";
                qDebug() << p;
                imwrite(p.toStdString(), mat_tmp);
            }
        }
    }
}

/**
 * 把上述两个过程都在内存中完成，输出的图精度有保证
 */
void CK_Preprocessor::OutputGabor()
{
    QFileInfo dirinfo_outter, dirinfo_inner, fileinfo;
    QFileInfoList middle_list, inner_list;
    QDir outter_dir, middle_dir, inner_dir;

    outter_dir.cd(this->dst_path);
    outter_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    middle_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    inner_dir.setFilter(QDir::Files);

    Mat img;

    QFileInfoList list = outter_dir.entryInfoList();
    foreach(dirinfo_outter, list) //S010, S011
    {
        middle_dir.cd(dirinfo_outter.absoluteFilePath());
        qDebug() << dirinfo_outter.absoluteFilePath();
        middle_list = middle_dir.entryInfoList();

        foreach(dirinfo_inner, middle_list) //001, 002
        {
            inner_dir.cd(dirinfo_inner.absoluteFilePath());
            qDebug() << dirinfo_inner.absoluteFilePath();
            inner_list = inner_dir.entryInfoList();

            foreach(fileinfo, inner_list)
            {
                qDebug() << fileinfo.absoluteFilePath();
                int len = fileinfo.baseName().size();
                if (fileinfo.baseName()[len-4] == 'f' && fileinfo.baseName()[len-3] == 'a' &&
                    fileinfo.baseName()[len-2] == 'c' && fileinfo.baseName()[len-1] == 'e')
                {
                    frame = cv::imread(fileinfo.absoluteFilePath().toStdString(), CV_LOAD_IMAGE_GRAYSCALE);
                    if (frame.channels() == 3)
                        cvtColor(frame, frame, CV_BGR2GRAY);
                    img = printGabor();
                    QString p = dirinfo_inner.absoluteFilePath() + "/" + fileinfo.baseName() + "_l2.jpg";
                    qDebug() << p;
                    imwrite(p.toStdString(), img);
                    // p = dirinfo_inner.absoluteFilePath() + "/" + fileinfo.baseName() + "_l2";
                    // Mat2Txt(p, img);
                }
            }
        }
    }
}

/**
 * 根据人脸的XML信息，存储为Information的数据结构
 */
vector<Information_Face> CK_Preprocessor::OutputInformationFromXML()
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(kPositionInformation);

    vector<Information_Face> AllPeople;

    tinyxml2::XMLElement *m = doc.RootElement();
    tinyxml2::XMLElement *people = m->FirstChildElement("People");
    while (people)
    {
        // People
        Information_Face iof;
        iof.id = people->Attribute("name");
        tinyxml2::XMLElement *expression = people->FirstChildElement("Expression");
        while (expression)
        {
            // Expression Class
            iof.expression = expression->Attribute("class");
            tinyxml2::XMLElement *frame = expression->FirstChildElement("Frame");
            while (frame)
            {
                // Frame Num
                iof.frame = atoi(frame->Attribute("Num"));
                iof.filename = frame->Attribute("jpg");
                tinyxml2::XMLElement *coords = frame->FirstChildElement("Left_Eye_Position");
                
                int t = 0;
                while (coords)
                {
                    // Coords
                    Coords coord;
                    tinyxml2::XMLElement *coord_x = coords->FirstChildElement("X");
                    coord.left_eye_x    = atoi(coord_x->GetText());
                    tinyxml2::XMLElement *coord_y = coord_x->NextSiblingElement();
                    coord.left_eye_y    = atoi(coord_y->GetText());

                    coords = coords->NextSiblingElement();
                    coord_x = coords->FirstChildElement("X");
                    coord.right_eye_x   = atoi(coord_x->GetText());
                    coord_y = coord_x->NextSiblingElement();
                    coord.right_eye_y   = atoi(coord_y->GetText());

                    coords = coords->NextSiblingElement();
                    coord_x = coords->FirstChildElement("X");
                    coord.mouth_x       = atoi(coord_x->GetText());
                    coord_y = coord_x->NextSiblingElement();
                    coord.mouth_y       = atoi(coord_y->GetText());

                    coords = coords->NextSiblingElement();
                    coord_x = coords->FirstChildElement("X");
                    coord.nose_x        = atoi(coord_x->GetText());
                    coord_y = coord_x->NextSiblingElement();
                    coord.nose_y        = atoi(coord_y->GetText());

                    iof.coord = coord;
                    coords = coords->NextSiblingElement();
                }
                frame = frame->NextSiblingElement();
                if (!frame)
                    iof.final = true;
                else
                    iof.final = false;
                AllPeople.push_back(iof);
            }
            expression = expression->NextSiblingElement();
        }
        people = people->NextSiblingElement();
    }
    return AllPeople;
}

/**
 * 原先的FACS信息要使用python脚本转换为XML格式
 * 然后在本程序中，切片的时候查找信息，进行Label
 */
vector<FACS_Face> CK_Preprocessor::OutputFACSFromXML()
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(kFACSInformation);

    vector<FACS_Face> AllFACS;

    tinyxml2::XMLElement *root = doc.RootElement();
    tinyxml2::XMLElement *subject = root->FirstChildElement("Subject");
    char facs[100];
    string id;
    while (subject)
    {
        id = subject->Attribute("Num");
        tinyxml2::XMLElement *session = subject->FirstChildElement("Session");

        while (session)
        {
            FACS_Face facs_face;
            memset(facs_face.AU, 0, sizeof(facs_face.AU));
            
            facs_face.id += 'S';
            //facs_face.id[0] = 'S';
            // ID是一个两位数比如10.0
            if (id[2] == '.')
            {
                facs_face.id += '0';
                facs_face.id += id[0];
                facs_face.id += id[1];
            }
            else
            {
                facs_face.id += id[0];
                facs_face.id += id[1];
                facs_face.id += id[2];
            }

            facs_face.expression = session->Attribute("Class");
            memset(facs, 0, sizeof(facs));
            strcpy(facs, session->GetText());
            decode(facs, facs_face.AU);
            session = session->NextSiblingElement();
            AllFACS.push_back(facs_face);
        }
        subject = subject->NextSiblingElement();
    }
    return AllFACS;
}

/**
 * 找出这张脸对应的frame one
 * @param  a       这张脸的信息
 * @param  vecInfo 全部脸的信息
 * @return         frame one
 */
Information_Face* getResponseFrameOne(Information_Face* a, vector<Information_Face>& vecInfo)
{
    for (vector<Information_Face>::iterator b = vecInfo.begin(); b != vecInfo.end(); b++)
    {
        if (!b->id.compare(a->id)  && !b->expression.compare(a->expression) && b->frame == 1)
            return &(*b);
    }
    return NULL;
}

/**
 * 从脸的FACS中查找现在这张脸的
 * @param  vtorff 全部脸的FACS
 * @param  inff   这张脸的信息
 * @return        这张脸的信息
 */
FACS_Face getResponseFACS(vector<FACS_Face>& vtorff, Information_Face& inff)
{
    for (vector<FACS_Face>::iterator b = vtorff.begin(); b != vtorff.end(); b++)
    {
        if (!b->id.compare(inff.id)  && !b->expression.compare(inff.expression))
            return *b;
    }
    return FACS_Face();
}

/**
 * 找出对应的第一帧的鼻子的位置，并对其加上偏移
 * @param b       这张脸的信息
 * @param x       需要返回的鼻子的x坐标
 * @param y       需要返回的鼻子的y坐标
 * @param vecInfo 全部脸的信息
 */
void getOffsetNose(Information_Face* b, int& x, int& y, vector<Information_Face>& vecInfo)
{
    if (b->frame == 1)
    {
        x = b->coord.nose_x;
        y = b->coord.nose_y;        
    }
    else
    {
        if (b->coord.right_eye_x != 99999 &&\
            b->coord.right_eye_y != 99999 &&\
            b->coord.left_eye_x  != 99999 &&\
            b->coord.left_eye_y  != 99999)
        {
            Information_Face* info = getResponseFrameOne(b, vecInfo);
            if (info)
            {
                int offset_x = (b->coord.right_eye_x - info->coord.right_eye_x\
                               + b->coord.left_eye_x - info->coord.left_eye_x) / 2;
                int offset_y = (b->coord.right_eye_y - info->coord.right_eye_y\
                               + b->coord.left_eye_y - info->coord.left_eye_y) / 2;
                x = info->coord.nose_x + offset_x;
                y = info->coord.nose_y + offset_y;
            }
        }
    }
}

/**
 * 找出对应的第一帧的嘴的位置，并对其加上偏移
 * @param b       这张脸的信息
 * @param x       需要返回的嘴的x坐标
 * @param y       需要返回的嘴的y坐标
 * @param vecInfo 全部脸的信息
 */
void getOffsetMouth(Information_Face *b, int& x, int& y, vector<Information_Face>& vecInfo)
{
    if (b->frame == 1)
    {
        x = b->coord.mouth_x;
        y = b->coord.mouth_y;       
    }
    else
    {
        if (b->coord.right_eye_x != 99999 &&\
            b->coord.right_eye_y != 99999 &&\
            b->coord.left_eye_x  != 99999 &&\
            b->coord.left_eye_y  != 99999)
        {
            Information_Face* info = getResponseFrameOne(b, vecInfo);
            if (info)
            {
                int offset_x = (b->coord.right_eye_x - info->coord.right_eye_x\
                               + b->coord.left_eye_x - info->coord.left_eye_x) / 2;
                int offset_y = (b->coord.right_eye_y - info->coord.right_eye_y\
                               + b->coord.left_eye_y - info->coord.left_eye_y) / 2;
                x = info->coord.mouth_x + offset_x;
                y = info->coord.mouth_y + offset_y;
            }
        }
    }
}

/**
 * 对于每一张图像进行检测
 * 1、若其是首帧或者末帧
 * 2、若SliceOk
 * 3、若其是左脸，则切出该块
 * 4、若其是右脸，则切除该块并flip
 * 5、若其是首帧，则将其放入primeMatV
 * 6、若其是末帧，则将其放入finalMatV，并查找表，将其FACS信息放入finalFACS
 * @param primeMatV 保存首帧Mat
 * @param finalMatV 保存末帧Mat
 * @param finalFACS 保存末帧的FACS
 * @param info      该图像中瞳孔、鼻尖、嘴尖的信息
 * @param vecFACS   需要在这中查找对应的FACS
 * @param img       图像
 * @param dstpath   将该切片保存在Slice文件夹中，以备查看
 * @param x_coord   关键点的x坐标
 * @param y_coord   关键点的y坐标
 * @param left      关键点左侧边界
 * @param right     关键点右侧边界
 * @param top       关键点上方边界
 * @param bottom    关键点下方边界
 * @param needFlip  是否需要翻转，flase为左边，true为右脸
 */
void OutputSliceHelper(vector<Mat_<uchar>>& primeMatV, vector<Mat_<uchar>>& finalMatV, vector<FACS_Face>& finalFACS, Information_Face& info, vector<FACS_Face>& vecFACS, const Mat_<uchar>& img, char* dstpath, int x_coord, int y_coord, int left, int right, int top, int bottom, bool needFlip)
{
    CV_Assert(img.type() == 0);
    CV_Assert(img.channels() == 1);
    Mat roi;

    if (info.frame == 1 || info.final)
    {
        if (IsSliceOk(x_coord, y_coord, left, right, top, bottom))
        {
            roi = GetSlice(img, x_coord, y_coord, left, right, top, bottom, needFlip);
            if (info.frame == 1)
            {
				PrintToFile(roi, "left");
                primeMatV.push_back(roi);
                imwrite(dstpath, roi);
            }
            else
            {
				PrintToFile(roi, "right");
                finalMatV.push_back(roi);
                finalFACS.push_back(getResponseFACS(vecFACS, info));
                imwrite(dstpath, roi);
            }
        }
    }
}

/**
 * 分别对左脸和右脸指定区域进行切片
 * 对于首帧，放入primeMatV，对于末帧，放入finalMatV
 * 末帧中的FACS信息放入finalFACS
 * @param  primeMatV 首帧的Mat
 * @param  finalMatV 末帧的Mat
 * @param  finalFACS 末帧的FACS信息
 * @param  vecInfo   带有人脸信息的数据结构
 * @param  vecFACS   带有FACS信息的数据结构
 * @param  left      关键点左侧边界
 * @param  right     关键点右侧边界
 * @param  top       关键点上方边界
 * @param  bottom    关键点下方边界
 * @param  section   脸部部位
 * @return           其中左脸共有多少个
 */
int OutputSlice(vector<Mat_<uchar>>& primeMatV, vector<Mat_<uchar>>& finalMatV, vector<FACS_Face>& finalFACS, vector<Information_Face>& vecInfo, vector<FACS_Face>& vecFACS, int left, int right, int top, int bottom, FACESECTION section)
{
    char srcpath[SLEN], dstpath[SLEN];
    Mat_<uchar> img;
    int sp = 0;     // 在左右脸的检测AU中，由于需要Flip归一化，但是Label中并没有这个信息，所以需要用Sp来区隔

    int x_coord, y_coord;
    // 先对左脸进行Slice
    for (vector<Information_Face>::iterator b = vecInfo.begin(); b != vecInfo.end(); b++)
    {
        if (section == EYE)
        {
            x_coord = b->coord.left_eye_x;
            y_coord = b->coord.left_eye_y;
        }
        else if (section == NOSE)
            getOffsetNose(&(*b), x_coord, y_coord, vecInfo);
        else if (section == MOUTH)
            getOffsetMouth(&(*b), x_coord, y_coord, vecInfo);

        sprintf(srcpath, "C:\\Users\\vincent\\Documents\\Visual Studio 2010\\Projects\\CV_64bit\\FacialExpression_x64\\AfterPreprocess_Pre\\%s\\%s\\%sface_l2.jpg", \
            b->id.c_str(), b->expression.c_str(), b->filename.c_str());
        // Slice的位置
        sprintf(dstpath, "Slices\\%s_left.jpg", b->filename.c_str());
        img = cv::imread(srcpath, CV_LOAD_IMAGE_GRAYSCALE);
        if (section == EYE)
            OutputSliceHelper(primeMatV, finalMatV, finalFACS, *b, vecFACS, img, dstpath, x_coord, y_coord, left, right, top, bottom, false);
        else
            OutputSliceHelper(primeMatV, finalMatV, finalFACS, *b, vecFACS, img, dstpath, x_coord, y_coord, left, 0, top, bottom, false);
    }

    // 用于标记左脸与右脸的分割
    sp = finalMatV.size();

    // 再对右脸进行Slice
    for (vector<Information_Face>::iterator b = vecInfo.begin(); b != vecInfo.end(); b++)
    {
        if (section == EYE)
        {
            x_coord = b->coord.right_eye_x;
            y_coord = b->coord.right_eye_y;
			qDebug("right_eye_x: %d, right_eye_y: %d", x_coord, y_coord);
        }
        else if (section == NOSE)
            getOffsetNose(&(*b), x_coord, y_coord, vecInfo);
        else if (section == MOUTH)
            getOffsetMouth(&(*b), x_coord, y_coord, vecInfo);

        sprintf(srcpath, "C:\\Users\\vincent\\Documents\\Visual Studio 2010\\Projects\\CV_64bit\\FacialExpression_x64\\AfterPreprocess_Pre\\%s\\%s\\%sface_l2.jpg", \
            b->id.c_str(), b->expression.c_str(), b->filename.c_str());
        // Slice的位置
        sprintf(dstpath, "Slices\\%s_right.jpg", b->filename.c_str());
        img = cv::imread(srcpath, CV_LOAD_IMAGE_GRAYSCALE);
        if (section == EYE)
            OutputSliceHelper(primeMatV, finalMatV, finalFACS, *b, vecFACS, img, dstpath, x_coord, y_coord, right, left, top, bottom, true);
        else
            OutputSliceHelper(primeMatV, finalMatV, finalFACS, *b, vecFACS, img, dstpath, x_coord, y_coord, 0, left, top, bottom, true);
    }
    return sp;
}

/**
 * 输出数据库中所有符合某AU的切片合适的片，并输出带Label的文件
 * @param vecInfo 带有人脸信息的数据结构
 * @param vecFACS 带有FACS信息的数据结构
 * @param section 脸部部位
 * @param left    关键点左侧边界
 * @param right   关键点右侧边界
 * @param top     关键点上方边界
 * @param bottom  关键点下方边界
 * @param au      需要符合哪个AU
 */
void CK_Preprocessor::OutputAULabelSlice(vector<Information_Face>& vecInfo, vector<FACS_Face>& vecFACS, FACESECTION section, int left, int right, int top, int bottom, int au)
{
    // 一些通用的定义
    vector<Mat_<uchar>> primeMatV;                  // 每个sequence的第一张的Slice
    vector<Mat_<uchar>> finalMatV;                  // 每个sequence的最后一张的Slice
    vector<FACS_Face> finalFACS;            // 每个sequence的AU Label，即最后一张的Slice呈现的AU Label

    int width = right + left;                       // Slice的宽度
    int height = bottom + top;                      // Slice的高度

    int negative_sample = 0;
    int positive_sample = 0;

    char filename[20];
    sprintf(filename, "AU_RESOURCE/AU_%d.txt", au);

    // 对指定区域进行Slice
    int sp = OutputSlice(primeMatV, finalMatV, finalFACS, vecInfo, vecFACS, left, right, top, bottom, section);

    FILE* fp = fopen(filename, "w+");
    for (vector<Mat_<uchar>>::iterator b = primeMatV.begin(); b != primeMatV.end(); b++)
    {
        // 无表情的label为0
        fputs("0 ", fp);
        char s[50];
        uchar *it;

        for (int i = 0; i < b->rows; i++)
        {
            it = b->ptr<uchar>(i);
            for (int j = 0; j < b->cols; j++)
            {
                memset(s, 0, sizeof(s));
                sprintf(s, "%d:%d ", i*b->cols+j+1, it[j]);
                fputs(s, fp);
                if (i*b->cols+j == 1000)
                    fflush(fp);
            }
        }
        fputs("\n", fp);
        fflush(fp);
        negative_sample++;
    }

    int i = 0;
    char label[20];
    for (vector<Mat_<uchar>>::iterator b = finalMatV.begin(); b != finalMatV.end(); b++, i++)
    {
        memset(label, 0, sizeof(label));
#ifdef WITH_ID_EXPRESSION
		char id_expression[SLEN];
		memset(id_expression, 0, sizeof(SLEN));
		strcat(id_expression, finalFACS[i].id.c_str());
		strcat(id_expression, " ");
		strcat(id_expression, finalFACS[i].expression.c_str());
		strcat(id_expression, " ");
		fputs(id_expression, fp);
#endif

        // 左眼
        if (i < sp)
        {
            if (finalFACS.at(i).AU[au] != 0 && finalFACS.at(i).AU[au]/1000!=2)
            {
                strcat(label, "1 ");
                positive_sample++;
            }
            else
            {
                strcat(label, "0 ");
                negative_sample++;              
            }
        }
        // 右眼
        else
        {
            if (finalFACS.at(i).AU[au] != 0 && finalFACS.at(i).AU[au]/1000!=1)
            {
                strcat(label, "1 ");
                positive_sample++;
            }   
            else
            {
                strcat(label, "0 ");
                negative_sample++;              
            }
        }

        fputs(label, fp);
        char s[50];
        uchar *it;
        for (int i = 0; i < b->rows; i++)
        {
            it = b->ptr<uchar>(i);
            for (int j = 0; j < b->cols; j++)
            {
                memset(s, 0, sizeof(s));
                sprintf(s, "%d:%d ", i*b->cols+j+1, it[j]);
                fputs(s, fp);
                if (i*b->cols+j == 1000)
                    fflush(fp);
            }
        }
        fputs("\n", fp);
        fflush(fp);
    }
    fclose(fp);

    qDebug("Positive sample: %d, Negative sample: %d", positive_sample, negative_sample);
}

// 有时候用于生成训练数据
//int main()
//{
//    CK_Preprocessor CK_preprocessor(QString("D:\\ck\\cohn-kanade\\cohn-kanade"));
//    //CK_preprocessor.Output40Gabor();
//    //CK_preprocessor.Merge40Gabor();
//    //CK_preprocessor.OutputGabor();
//
//     vector<Information_Face> a = CK_preprocessor.OutputInformationFromXML();
//     vector<FACS_Face> b = CK_preprocessor.OutputFACSFromXML();
//
//     //CK_preprocessor.OutputAULabelSlice(a, b, EYE, 15, 15, 35, 10, 2);   // AU1、AU2
//     CK_preprocessor.OutputAULabelSlice(a, b, EYE, 15, 15, 15, 15, 4); // AU4、AU5
//     //CK_preprocessor.OutputAULabelSlice(a, b, EYE, 20, 20, 10, 50, 7); // AU6、AU7
//     //CK_preprocessor.OutputAULabelSlice(a, b, EYE, 0, 30, 15, 15, 9);  // AU9
//     //CK_preprocessor.OutputAULabelSlice(a, b, MOUTH, 30, 30, 20, 10, 20);    // AU10、AU12、AU15、AU16、AU18、AU20、AU22、AU23、AU24
//
//    return 0;
//}