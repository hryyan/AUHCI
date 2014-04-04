// facialexpression.h: 主程序，用于识别表情与互动
//
// Created by Vincent Yan in 2014/03/17

#ifndef FACIALEXPRESSION_H
#define FACIALEXPRESSION_H

#include <QtGui/QMainWindow>
#include <QFileDialog>
#include "util.h"
#include "source.h"
#include "facedet.h"
#include "gabor.h"
#include "ui_facialexpression_x64.h"

// 定义函数指针
typedef Mat (*pf)(void);

class FacialExpression_x64 : public QMainWindow
{
    Q_OBJECT
    
    public:
    FacialExpression_x64(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~FacialExpression_x64();
    
    // 后台play函数，用于在label上绑定pf函数
    void play_(QLabel* label, Mat(*pf)(void));
    
    // 初始化CV部分
    bool initCV();
    
    public slots:
    
    // 使用timer激活的slot，用于同步播放所有label上的pf
    void play();
    
    // 按钮slot，把播放Camera的label和pf放进vector中
    void play_camera();
    
    // 按钮slot，把播放Video的label和pf放进vector中
    void play_video();
    
    // 按钮slot，把播放Signal的label和pf放进vector中
    void play_face();
    
    // 按钮slot，把播放Frequency的label和pf放进vector中
    void play_gabor();
    
    // 按钮slot，把播放Picture的label和pf放进vector中
    void play_pic();
    
    // 按钮slot，对Face label进行截图
    void capture_face();
    
    // 按钮slot，对Gabor label进行截图
    void capture_gabor();
    
    // 按钮slot，对截图保存路径进行修改
    void change_directory();
    
    // 按钮slot，停止一切播放，并且清除label
    void stop_all();
    
    // Spin Slot，更改帧率
    void change_interval(int);
    
    private:
    Ui::FacialExpression_x64Class ui;
    
    // UI控件的指针
    QGroupBox *input_group;
    QGroupBox *face_group;
    QGroupBox *gabor_group;
    
    QPushButton *video_input_button;
    QPushButton *camera_input_button;
    QPushButton *picture_input_button;
    QPushButton *capture_face_button;
    QPushButton *capture_gabor_button;
    QPushButton *change_directory_button;
    QPushButton *stop_button;
    
    QLabel *input_label;
    QLabel *face_label;
    QLabel *gabor_label;
    
    QCheckBox *show_face_checkbox;
    QCheckBox *show_gabor_checkbox;
    
    QSpinBox *frames_spinbox;
    
    // 存储label和pf的vector，在play中调用
    vector<pair<QLabel*, pf> > lpfv;
    
    // 截图保存的默认路径
    QString screen_shot_directory;
};

#endif // FACIALEXPRESSION_H