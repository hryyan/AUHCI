/********************************************************************************
** Form generated from reading UI file 'facialexpression_x64.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FACIALEXPRESSION_X64_H
#define UI_FACIALEXPRESSION_X64_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FacialExpression_x64Class
{
public:
    QWidget *centralWidget;
    QGroupBox *groupBox;
    QLabel *label;
    QGroupBox *groupBox_2;
    QLabel *label_2;
    QGroupBox *groupBox_3;
    QLabel *label_3;
    QLabel *label_4;
    QSpinBox *spinBox;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QPushButton *pushButton_5;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton_3;
    QPushButton *pushButton_4;
    QPushButton *pushButton_6;
    QVBoxLayout *verticalLayout;
    QCheckBox *checkBox;
    QCheckBox *checkBox_2;
    QCheckBox *checkBox_3;
    QPushButton *pushButton_7;
    QPushButton *pushButton_8;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *FacialExpression_x64Class)
    {
        if (FacialExpression_x64Class->objectName().isEmpty())
            FacialExpression_x64Class->setObjectName(QString::fromUtf8("FacialExpression_x64Class"));
        FacialExpression_x64Class->resize(1358, 956);
        centralWidget = new QWidget(FacialExpression_x64Class);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 0, 741, 581));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 30, 54, 12));
        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(760, 0, 581, 581));
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 70, 54, 12));
        groupBox_3 = new QGroupBox(centralWidget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(760, 580, 581, 301));
        label_3 = new QLabel(groupBox_3);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(30, 50, 54, 12));
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(430, 620, 54, 12));
        spinBox = new QSpinBox(centralWidget);
        spinBox->setObjectName(QString::fromUtf8("spinBox"));
        spinBox->setGeometry(QRect(490, 610, 42, 22));
        layoutWidget = new QWidget(centralWidget);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(60, 600, 336, 95));
        verticalLayout_3 = new QVBoxLayout(layoutWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton = new QPushButton(layoutWidget);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout->addWidget(pushButton);

        pushButton_2 = new QPushButton(layoutWidget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        horizontalLayout->addWidget(pushButton_2);

        pushButton_5 = new QPushButton(layoutWidget);
        pushButton_5->setObjectName(QString::fromUtf8("pushButton_5"));

        horizontalLayout->addWidget(pushButton_5);


        verticalLayout_2->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        pushButton_3 = new QPushButton(layoutWidget);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));

        horizontalLayout_2->addWidget(pushButton_3);

        pushButton_4 = new QPushButton(layoutWidget);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));

        horizontalLayout_2->addWidget(pushButton_4);

        pushButton_6 = new QPushButton(layoutWidget);
        pushButton_6->setObjectName(QString::fromUtf8("pushButton_6"));

        horizontalLayout_2->addWidget(pushButton_6);


        verticalLayout_2->addLayout(horizontalLayout_2);


        horizontalLayout_3->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        checkBox = new QCheckBox(layoutWidget);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));

        verticalLayout->addWidget(checkBox);

        checkBox_2 = new QCheckBox(layoutWidget);
        checkBox_2->setObjectName(QString::fromUtf8("checkBox_2"));

        verticalLayout->addWidget(checkBox_2);

        checkBox_3 = new QCheckBox(layoutWidget);
        checkBox_3->setObjectName(QString::fromUtf8("checkBox_3"));

        verticalLayout->addWidget(checkBox_3);


        horizontalLayout_3->addLayout(verticalLayout);


        verticalLayout_3->addLayout(horizontalLayout_3);

        pushButton_7 = new QPushButton(layoutWidget);
        pushButton_7->setObjectName(QString::fromUtf8("pushButton_7"));

        verticalLayout_3->addWidget(pushButton_7);

        pushButton_8 = new QPushButton(centralWidget);
        pushButton_8->setObjectName(QString::fromUtf8("pushButton_8"));
        pushButton_8->setGeometry(QRect(430, 670, 75, 23));
        FacialExpression_x64Class->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(FacialExpression_x64Class);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1358, 23));
        FacialExpression_x64Class->setMenuBar(menuBar);
        mainToolBar = new QToolBar(FacialExpression_x64Class);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        FacialExpression_x64Class->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(FacialExpression_x64Class);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        FacialExpression_x64Class->setStatusBar(statusBar);

        retranslateUi(FacialExpression_x64Class);

        QMetaObject::connectSlotsByName(FacialExpression_x64Class);
    } // setupUi

    void retranslateUi(QMainWindow *FacialExpression_x64Class)
    {
        FacialExpression_x64Class->setWindowTitle(QApplication::translate("FacialExpression_x64Class", "FacialExpression_x64", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("FacialExpression_x64Class", "GroupBox", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("FacialExpression_x64Class", "TextLabel", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("FacialExpression_x64Class", "GroupBox", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("FacialExpression_x64Class", "TextLabel", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("FacialExpression_x64Class", "GroupBox", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("FacialExpression_x64Class", "TextLabel", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("FacialExpression_x64Class", "\346\257\217\347\247\222\345\270\247\346\225\260", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("FacialExpression_x64Class", "Video", 0, QApplication::UnicodeUTF8));
        pushButton_2->setText(QApplication::translate("FacialExpression_x64Class", "Camera", 0, QApplication::UnicodeUTF8));
        pushButton_5->setText(QApplication::translate("FacialExpression_x64Class", "Picture", 0, QApplication::UnicodeUTF8));
        pushButton_3->setText(QApplication::translate("FacialExpression_x64Class", "CaptureF", 0, QApplication::UnicodeUTF8));
        pushButton_4->setText(QApplication::translate("FacialExpression_x64Class", "CaptureG", 0, QApplication::UnicodeUTF8));
        pushButton_6->setText(QApplication::translate("FacialExpression_x64Class", "Directory", 0, QApplication::UnicodeUTF8));
        checkBox->setText(QApplication::translate("FacialExpression_x64Class", "Show Face", 0, QApplication::UnicodeUTF8));
        checkBox_2->setText(QApplication::translate("FacialExpression_x64Class", "Show Gabor", 0, QApplication::UnicodeUTF8));
        checkBox_3->setText(QApplication::translate("FacialExpression_x64Class", "Classify", 0, QApplication::UnicodeUTF8));
        pushButton_7->setText(QApplication::translate("FacialExpression_x64Class", "Stop", 0, QApplication::UnicodeUTF8));
        pushButton_8->setText(QApplication::translate("FacialExpression_x64Class", "Sequence", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class FacialExpression_x64Class: public Ui_FacialExpression_x64Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FACIALEXPRESSION_X64_H
