// facialexpression.h: 主程序，用于识别表情与互动
//
// Created by Vincent Yan in 2014/3/17
// Modifid by Vincent Yan in 2014/6/15

#ifndef FACIALEXPRESSION_H
#define FACIALEXPRESSION_H

#include <QtGui/QMainWindow>
#include <QFileDialog>
#include <QThread>
#include "ui_facialexpression_x64.h"
#include "util.h"
#include "source.h"
#include "facedet.h"
#include "gabor.h"
#include "classification.h"

// 用于显示AU状态的小灯泡的数量
static const int kBulbNum = 16;

// 每帧的间隔，设置为41.6可以实现24帧的实时
static int interval = 100;

// 存储AU的队列
static const int kAU[kBulbNum] = {1, 2, 4, 5, 6, 7, 9, 10, 12, 15, 16, 18, 20 ,22, 23, 24};

class ProcessThread : public QThread
{
	Q_OBJECT
public:
	bool* Classify(Mat &);
	bool show_face;
	bool show_gabor;
	bool classify;

protected:
	virtual void run();

signals:
	void Write(QImage*, QImage *face, QImage *gabor, bool *au_appear);

};

class FacialExpressionX64 : public QMainWindow
{
    Q_OBJECT

public:
    FacialExpressionX64(QWidget *parent=0, Qt::WFlags flags=0);
    ~FacialExpressionX64();

    // 初始化CV环境
    bool InitCV();

    // 初始化SVM环境
    bool InitSVM();

    // 用于进行分类
    void ClassifyAndDisplay(Mat&);

    // 用于在input_label显示
    void DisplayInput();

public slots:
    // 用于处理一帧frame(extern Mat frame)
    void ProcessOneFrame();

    // 用于处理一张图片
    void ProcessPic();

    // 用于处理视频序列
    void ProcessVideo();

    // 用于处理Web Camera
    void ProcessCamera();

    // 用于处理一连串的图像
    void ProcessSequence();

    // 停止所有处理
    void StopAll();

    // 对Face进行截图
    void CaptureFace();

    // 对Gabor进行截图
    void CaptureGabor();

    // 对截图保存的路径进行控制
    void ChangeDirectory();

    // TODO：操作帧率
    void ChangeInterval(int);

	// 读取处理线程得到的数据
	void Read(QImage*, QImage*, QImage*, bool*);

private:
    // 使用Qt Designer设计的ui
    Ui::FacialExpression_x64Class ui;

    // UI控件的指针
    QGroupBox *input_group;
    QGroupBox *face_group;
    QGroupBox *gabor_group;

    QPushButton *video_input_button;
    QPushButton *camera_input_button;
    QPushButton *picture_input_button;
    QPushButton *sequence_input_button;
    QPushButton *capture_face_button;
    QPushButton *capture_gabor_button;
    QPushButton *change_directory_button;
    QPushButton *stop_button;

    QLabel *input_label;
    QLabel *face_label;
    QLabel *gabor_label;

    QCheckBox *show_face_checkbox;
    QCheckBox *show_gabor_checkbox;
    QCheckBox *classify_checkbox;

    QSpinBox *frames_spinbox;

    // 截图保存的路径
    QString screen_shot_directory;

    // 两个显示AU状态的小灯泡
    QPixmap *on_pixmap;
    QPixmap *off_pixmap;

    // 用于显示AU状态和AU名字的Label
    QLabel *label_vec[2*kBulbNum];

    bool can_process;

	ProcessThread process_t;
};

#endif // FACIALEXPRESSION_H