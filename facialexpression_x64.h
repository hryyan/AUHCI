#ifndef FACIALEXPRESSION_X64_H
#define FACIALEXPRESSION_X64_H

#include <QtGui/QMainWindow>
#include "ui_facialexpression_x64.h"

class FacialExpression_x64 : public QMainWindow
{
	Q_OBJECT

public:
	FacialExpression_x64(QWidget *parent = 0, Qt::WFlags flags = 0);
	~FacialExpression_x64();

private:
	Ui::FacialExpression_x64Class ui;
};

#endif // FACIALEXPRESSION_X64_H
