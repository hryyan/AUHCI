// facedet.h: 来源获取（摄像头、视频、图片）
// 
// Created by Vincent Yan in 2014/03/18

#include "source.h"

Mat frame;
VideoCapture cap;

QImage* Mat2QImage(Mat& I)
{
	int channelNum = I.channels();

	QImage *img = new QImage(I.cols, I.rows, QImage::Format_RGB32);
	for (int i = 0; i < I.rows; i++)
	{
		uchar* Mi = I.ptr<uchar>(i);
		for (int j = 0; j < I.cols; j++)
		{
			if (channelNum == 1)
			{
				int r = Mi[j];
				img->setPixel(j, i, qRgb(r, r, r));
			}
			else if (channelNum == 3)
			{
				int r = Mi[2+j*3];
				int g = Mi[1+j*3];
				int b = Mi[0+j*3];
				img->setPixel(j, i, qRgb(r, g, b));
			}
		}
	}
	return img;
}

bool initCamera()
{
	cap = VideoCapture(0);
	if (!cap.isOpened())
		return false;
	return true;
}

bool initVideo(string path)
{
	cap = VideoCapture(path);
	if (!cap.isOpened())
		return false;
	return true;
}

bool initPic(string path)
{
	frame = cv::imread(path, -1);
	if (frame.rows == 0 || frame.cols == 0)
		return false;
	return true;
}

QImage* printScreen()
{
	if (cap.isOpened())
		cap >> frame;

	return Mat2QImage(frame);
}