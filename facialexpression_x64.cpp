// facialexpression.cpp: 主程序，用于识别表情与互动
//
// Created by Vincent Yan in 2014/03/17

#include "facialexpression_x64.h"

extern VideoCapture cap;

// 每帧的间隔，设置为41.6可以实现24帧的实时
static int interval = 100;

// 存储face文件时候的读数
static int face_file_num = 1;

// 存储gabor文件时候的读数
static int gabor_file_num = 1;

FacialExpression_x64::FacialExpression_x64(QWidget *parent, Qt::WFlags flags)
: QMainWindow(parent, flags)
{
    ui.setupUi(this);
    
    // 绑定ui中的控件
    input_group     = ui.groupBox;
    face_group      = ui.groupBox_2;
    gabor_group     = ui.groupBox_3;
    
    video_input_button      = ui.pushButton;
    camera_input_button     = ui.pushButton_2;
    picture_input_button    = ui.pushButton_5;
    capture_face_button     = ui.pushButton_3;
    capture_gabor_button    = ui.pushButton_4;
    change_directory_button = ui.pushButton_6;
    stop_button             = ui.pushButton_7;
    
    input_label     = ui.label;
    face_label      = ui.label_2;
    gabor_label     = ui.label_3;
    
    show_face_checkbox   = ui.checkBox;
    show_gabor_checkbox  = ui.checkBox_2;
    
    frames_spinbox = ui.spinBox;
    frames_spinbox->setRange(1, 30);
    frames_spinbox->setValue(24);
    interval = 1000 / frames_spinbox->value();
    
    // 设置Label大小以及名称（用于DEBUG）
    input_label->resize(640, 480);
    input_label->setObjectName("inputLabel");
    face_label->resize(640, 480);
    face_label->setObjectName("signalLabel");
    gabor_label->resize(530, 220);
    gabor_label->setObjectName("frequencyLabel");
    
    screen_shot_directory = "./";
    
    // 初始化CV环境
    initCV();
    
    // 构建Timer用于激活play slot
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(play()));
    timer->start(interval);
    
    // 按键的signal/slot
    connect(camera_input_button,      SIGNAL(released()),         this, SLOT(play_camera()));
    connect(video_input_button,       SIGNAL(released()),         this, SLOT(play_video()));
    connect(picture_input_button,     SIGNAL(released()),         this, SLOT(play_pic()));
    connect(capture_face_button,      SIGNAL(released()),         this, SLOT(capture_face()));
    connect(capture_gabor_button,     SIGNAL(released()),         this, SLOT(capture_gabor()));
    connect(change_directory_button,  SIGNAL(released()),         this, SLOT(change_directory()));
    connect(stop_button,              SIGNAL(released()),         this, SLOT(stop_all()));
    connect(frames_spinbox,           SIGNAL(valueChanged(int)),  this, SLOT(change_interval(int)));
}

FacialExpression_x64::~FacialExpression_x64()
{
    
}

/**
* 使用timer激活的slot，用于同步播放所有label中的frame
*/
void FacialExpression_x64::play()
{
    // 把所有需要播放的label和pf放进vector中
    for (vector<pair<QLabel*, pf> >::iterator it = lpfv.begin();\
    it != lpfv.end(); it++)
    play_(it->first, it->second);
}

/**
* 后台play函数，用于在label上绑定pf函数
* @param label   in: 为UI中需要播放的QLabel
* @param pf      in: 函数指针，产生QIamge的指针
*/
void FacialExpression_x64::play_(QLabel* label, Mat (*pf)(void))
{
    // 因为Mat的大小只有在CV中才能得到，所以Mat到QImage的转换只能在CV中做。在这里有一个指针接收对象，并且在显示后负责将其释放。
	Mat img = pf();
	QImage* tmp = NULL;
	if (img.cols !=0 && img.rows != 0)
		tmp = Mat2QImage(img);
    
    if (tmp == NULL)
    qDebug("%s play ERROR", qPrintable(label->objectName()));
    
	if (tmp != NULL)
	{
		label->setPixmap(QPixmap::fromImage(*tmp));
		delete tmp;
	}
	else
		qDebug("%s play OK", qPrintable(label->objectName()));
}

/**
* 把播放Camera的label和pf放进vector中
*/
void FacialExpression_x64::play_camera()
{
    lpfv.clear();
    
    if (!isCameraInited())
    {
        qDebug("Camera init ERROR");
        return;
    }
    
    qDebug("Camera init OK");
    lpfv.push_back(pair<QLabel*, pf>(input_label, printScreen));
    
    if (show_face_checkbox->isChecked())
    play_face();
    if (show_gabor_checkbox->isChecked())
    play_gabor();
}

/**
* 把播放Picture的label和pf放进vector中
*/
void FacialExpression_x64::play_pic()
{
    string path = QFileDialog::getOpenFileName().toStdString();
    lpfv.clear();
    
    if (!isPicInited(path))
    {
        qDebug("Pic init ERROR");
        return;
    }
    
    qDebug("Pic init OK");
    lpfv.push_back(pair<QLabel*, pf>(input_label, printScreen));
    
    if (show_face_checkbox->isChecked())
    play_face();
    if (show_gabor_checkbox->isChecked())
    play_gabor();
    
    // TODO: 可以一次选择多幅照片
}

/**
* 对Face label进行截图
*/
void FacialExpression_x64::capture_face()
{
    QString file_name = QString("No%1 face.jpg").arg(face_file_num);
    file_name = screen_shot_directory + file_name;
    face_label->pixmap()->save(file_name);
}


/**
* 对Gabor label进行截图
*/
void FacialExpression_x64::capture_gabor()
{
    QString file_name = QString("No%1 gabor.jpg").arg(face_file_num);
    file_name = screen_shot_directory + file_name;
    gabor_label->pixmap()->save(file_name);
}

/**
* 对Gabor label进行截图
*/
void FacialExpression_x64::change_directory()
{
    screen_shot_directory = QFileDialog::getExistingDirectory();
}

/**
* 停止一切播放，清除label
*/
void FacialExpression_x64::stop_all()
{
    lpfv.clear();
    input_label->clear();
    face_label->clear();
    gabor_label->clear();
    
    cap.release();
}

/**
* 更改帧率
*/
void FacialExpression_x64::change_interval(int value)
{
    interval = 1000 / value;
}

/**
* 把播放Video的label和pf放进vector中
*/
void FacialExpression_x64::play_video()
{
    //string path = QFileDialog::getOpenFileName().toStdString();
    string path = "C:\\Users\\vincent\\Desktop\\yangchunlei_0.avi";
    lpfv.clear();
    
    if (!isVideoInited(path))
    {
        qDebug("Video init ERROR");
        return;
    }
    
    qDebug("Video init OK");
    lpfv.push_back(pair<QLabel*, pf>(input_label, printScreen));
    
    if (show_face_checkbox->isChecked())
    play_face();
    if (show_gabor_checkbox->isChecked())
    play_gabor();
}

/**
* 把播放Signal的label和pf放进vector中
*/
void FacialExpression_x64::play_face()
{
    lpfv.push_back(pair<QLabel*, pf>(face_label, printFace));
}

/**
* 把播放Frequency的label和pf放进vector中
*/
void FacialExpression_x64::play_gabor()
{
    lpfv.push_back(pair<QLabel*, pf>(gabor_label, printGabor));
}

/**
* [initCV description]
* @return [description]
*/
bool FacialExpression_x64::initCV()
{
    return true;
}