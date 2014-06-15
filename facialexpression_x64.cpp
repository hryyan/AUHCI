// facialexpression.cpp: 主程序，用于识别表情与互动
//
// Created by Vincent Yan in 2014/3/17
// Modifid by Vincent Yan in 2014/6/15

#include "facialexpression_x64.h"

extern Mat frame;
extern DetPar frame_detpar;

static int face_tmp_name  = 1;
static int gabor_tmp_name = 1;

FacialExpressionX64::FacialExpressionX64(QWidget *parent, Qt::WFlags flags)
{
    ui.setupUi(this);

    // 绑定UI中的控件
    input_group     = ui.groupBox;
    face_group      = ui.groupBox_2;
    gabor_group     = ui.groupBox_3;

    video_input_button      = ui.pushButton;
    camera_input_button     = ui.pushButton_2;
    picture_input_button    = ui.pushButton_5;
    sequence_input_button   = ui.pushButton_8;
    capture_face_button     = ui.pushButton_3;
    capture_gabor_button    = ui.pushButton_4;
    change_directory_button = ui.pushButton_6;
    stop_button             = ui.pushButton_7;

    input_label     = ui.label;
    face_label      = ui.label_2;
    gabor_label     = ui.label_3;

    show_face_checkbox      = ui.checkBox;
    show_gabor_checkbox     = ui.checkBox_2;
    classify_checkbox       = ui.checkBox_3;

    frames_spinbox   = ui.spinBox;
    frames_spinbox->setRange(1, 30);
    frames_spinbox->setValue(24);
    interval = 1000 / frames_spinbox->value();

    // 设置Label大小以及名称
    input_label->resize(640, 480);
    input_label->setObjectName("Input");
    face_label->resize(640, 480);
    face_label->setObjectName("Face");
    gabor_label->resize(530, 220);
    gabor_label->setObjectName("Gabor");


    // 设置显示AU状态的小灯泡
    on_pixmap   = new QPixmap("./Resources/on.png");
    off_pixmap  = new QPixmap("./Resources/off.png");

    for (int i = 0; i < kBulbNum; i++)
    {
        label_vec[i] = new QLabel(this);
        label_vec[i]->setFixedSize(32, 32);
        label_vec[i]->setGeometry(50+i%8*65, 750+i/8*65, 50, 50);
        label_vec[i]->setPixmap(*off_pixmap);
        label_vec[i+kBulbNum] = new QLabel(this);
        label_vec[i+kBulbNum]->setGeometry(50+i%8*65, 750+i/8*65+20, 50, 50);
        QString t = QString("AU %1").arg(kAU[i]);
        label_vec[i+kBulbNum]->setText(t);
    }

    // 设置截图保存的路径
    screen_shot_directory = "./";

    // 初始化CV环境以及SVM环境
    if (InitCV())
    {
        qDebug("CV environment inited success!");        
    }
    else
    {
        qDebug("CV environment inited fail!");
        this->close();
    }
	if (InitSVM())
	{
		qDebug("SVM environment inited success!");
	}
	else
	{
		qDebug("SVM environment inited fail!");
		this->close();
	}

    can_process = true;

    // 按键的signal/slot
    connect(video_input_button,      SIGNAL(released()),    this,    SLOT(ProcessVideo()));
    connect(camera_input_button,     SIGNAL(released()),    this,    SLOT(ProcessCamera()));
    connect(picture_input_button,    SIGNAL(released()),    this,    SLOT(ProcessPic()));
    connect(sequence_input_button,   SIGNAL(released()),    this,    SLOT(ProcessSequence()));
    connect(capture_face_button,     SIGNAL(released()),    this,    SLOT(CaptureFace()));
    connect(capture_gabor_button,    SIGNAL(released()),    this,    SLOT(CaptureGabor()));
    connect(change_directory_button, SIGNAL(released()),    this,    SLOT(ChangeDirectory()));
    connect(stop_button,             SIGNAL(released()),    this,    SLOT(StopAll()));

    // TODO: 可以控制帧率
    connect(frames_spinbox,    SIGNAL(valueChanged(int)),    this,    SLOT(ChangeInterval(int)));
}

FacialExpressionX64::~FacialExpressionX64()
{

}

/**
 * 初始化CV环境
 * @return 正常加载则返回true
 */
bool FacialExpressionX64::InitCV()
{
    InitFaceDet();
    InitGabor();
    return true;
}

/**
 * 初始化SVM环境
 * @return 正常加载则返回ture
 */
bool FacialExpressionX64::InitSVM()
{
    classiftInit();
    return true;
}

/**
 * 根据Gabor图像分类表情，并在小灯泡上显示
 * @param gabor_img 分类的图像
 */
void FacialExpressionX64::ClassifyAndDisplay(Mat &gabor_img)
{
    CV_Assert(gabor_img.type() == 0);
    CV_Assert(gabor_img.channels() == 1);
	bool *au_appear = (bool*)malloc(sizeof(bool)*kBulbNum);
    getAU(au_appear, gabor_img);
    for (int i = 0; i < kBulbNum; i++)
    {
        if (au_appear[i])
            label_vec[i]->setPixmap(*on_pixmap);
        else
            label_vec[i]->setPixmap(*off_pixmap);
    }
	free(au_appear);
}

/**
 * 显示输入的图像
 */
void FacialExpressionX64::DisplayInput()
{
    QImage* img = Mat2QImage(frame);
    input_label->setPixmap(QPixmap::fromImage(*img));
    free(img);
}

/**
 * 处理单帧图像，方便代码重用
 */
void FacialExpressionX64::ProcessOneFrame()
{
    Mat gabor;
    QImage *img_to_be_print;
    // 检测三个checkBox
    // 第一个是是否检测脸部
    if (show_face_checkbox->isChecked())
    {
        PrintFaceToFrame();
		DetectEyes();
		//DetectMouth();
        img_to_be_print = Mat2QImage(frame);
        face_label->setPixmap(QPixmap::fromImage(*img_to_be_print));
        free(img_to_be_print);

        // 第二个是是否获得Gabor滤波
        if (show_gabor_checkbox->isChecked())
        {
            gabor = printGabor();
			img_to_be_print = Mat2QImage(gabor);
            gabor_label->setPixmap(QPixmap::fromImage(*img_to_be_print));
            free(img_to_be_print);

            // 第三个是是否获得AU
            if (classify_checkbox->isChecked())
            {
                ClassifyAndDisplay(gabor);
            }
        }
    }
}

/**
 * 处理一张图片，由于Qt中的SIGNAL/SLOT机制，不方便传值
 */
void FacialExpressionX64::ProcessPic()
{
    can_process = true;
    string path = QFileDialog::getOpenFileName().toStdString();

    // 载入单张图片
    if (InitPic(path))
    {
        DisplayInput();
        qDebug("Pic load success!");
    }
    else
    {
        qDebug("Pic load fail!");
        return;
    }
    ProcessOneFrame();
}

/**
 * 处理一段视频，由于Qt中的SIGNAL/SLOT机制，不方便传值
 */
void FacialExpressionX64::ProcessVideo()
{
    can_process = true;
    string path = QFileDialog::getOpenFileName().toStdString();

    if (InitVideo(path))
    {
        DisplayInput();
        qDebug("Video load success!");
    }
    else
    {
        qDebug("Video load fail!");
        return;
    }
    while (can_process)
    {
        ProcessOneFrame();
    }
}

void FacialExpressionX64::ProcessCamera()
{
    can_process = true;
    if (InitCamera())
    {
        DisplayInput();
        qDebug("Camera load success!");
    }
    else
    {
        qDebug("Camera load fail!");
        return;
    }
    while (can_process)
    {
        ProcessOneFrame();
    }
}

void FacialExpressionX64::ProcessSequence()
{
    can_process = true;
    QStringList paths = QFileDialog::getOpenFileNames();
    string path;
    for (QStringList::Iterator it = paths.begin(); can_process && it != paths.end(); it++)
    {
        path = it->toStdString();
        if (InitPic(path))
        {
            DisplayInput();
            qDebug("Pic load success!");
        }
        else
        {
            qDebug("Pic load fail!");
            return;
        }
        ProcessOneFrame();

        // 让程序睡眠一下
        //QTime t; 
        //t.start(); 
        //while(t.elapsed() < 250) 
        //{ 
        //    QCoreApplication::processEvents(); 
        //    qSleep(10000);//sleep和usleep都已经obsolete，建议使用nanosleep代替 
        //} 
    }
}

void FacialExpressionX64::StopAll()
{
    can_process = false;
}

void FacialExpressionX64::CaptureFace()
{
    QString file_name = QString("%1_face.jpg").arg(face_tmp_name);
    file_name = screen_shot_directory + file_name;
    face_label->pixmap()->save(file_name);
}

void FacialExpressionX64::CaptureGabor()
{
    QString file_name = QString("%1_gabor.jpg").arg(gabor_tmp_name);
    file_name = screen_shot_directory + file_name;
    gabor_label->pixmap()->save(file_name);
}

void FacialExpressionX64::ChangeDirectory()
{
    screen_shot_directory = QFileDialog::getExistingDirectory();
}

void FacialExpressionX64::ChangeInterval(int interval)
{

}