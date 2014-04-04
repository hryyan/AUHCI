// Preprocessor.cpp: 用于生成训练样本
// 
// Created by Vincent Yan in 2014/03/17

#include "generate.h"
#include "QDir"
#include "QtDebug"

// source中每一帧的Mat
extern Mat frame;

// 卷积核大小，101效果比较好，速度挺慢的，使用GPU加速后，一张Gabor特征10ms（一个虚部或者一个实部）
const int iSize = 101;

// 由于gpu::convolve函数的限制（默认会裁剪图像），所以需要在右边和下边添加边框
// http://code.opencv.org/issues/1639
const double BORDER_FRAC = 0.5;

/**
 * ctr，设定数据库的地址
 * @param  path 数据库地址
 */
Preprocesor::Preprocesor(QString p)
{
	path = p;
	gabor.Init(Size(iSize, iSize), sqrt(2.0), 1, CV_32F);
}

/**
 * dtr
 */
Preprocesor::~Preprocesor()
{

}

/**
 * 根据CK数据库的地址得到预处理后的图像
 * @param path CK数据库的地址
 */
void Preprocesor::generator()
{
	QFileInfo dirinfo_outter, dirinfo_inner, fileinfo;
	QFileInfoList middle_list, inner_list;
	QDir outter_dir, middle_dir, inner_dir, dir;
	QImage* tmp_jpg;
	QString filepath, filename;
	cv::gpu::GpuMat g_src, g_dst;

	QStringList filters;
	filters << "*.png";

	QString store_path("");

	inner_dir.setFilter(QDir::Files | QDir::Readable);
	inner_dir.setNameFilters(filters);

	outter_dir.cd(this->path);
	outter_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
	middle_dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

	QFileInfoList list = outter_dir.entryInfoList();
	foreach(dirinfo_outter, list)
	{
		middle_dir.cd(dirinfo_outter.absoluteFilePath());
		qDebug() << dirinfo_outter.absoluteFilePath();
		middle_list = middle_dir.entryInfoList();

		foreach(dirinfo_inner, middle_list)
		{
			inner_dir.cd(dirinfo_inner.absoluteFilePath());
			qDebug() << dirinfo_inner.absoluteFilePath();
			inner_list = inner_dir.entryInfoList();

			foreach(fileinfo, inner_list)
			{
				frame = cv::imread(fileinfo.absoluteFilePath().toStdString());
				imwrite("imread.jpg", frame);
				qDebug() << fileinfo.absoluteFilePath();

				tmp_jpg = Mat2QImage(printFace());
				filepath = QDir::currentPath()+"/AfterPreprocess/"+store_path+dirinfo_outter.fileName()+"/"+dirinfo_inner.fileName();
				filename = "/"+fileinfo.baseName()+"face.jpg";
				qDebug() << filepath;

				if (!dir.exists(filepath))
					dir.mkpath(filepath);
				bool on = tmp_jpg->save(filepath+filename);
				delete tmp_jpg;

				if (on)
					qDebug("save face OK");

				int top_buttom = frame.rows * BORDER_FRAC;
				int left_right = frame.cols * BORDER_FRAC;
				copyMakeBorder(frame, frame, 0, top_buttom, 0, left_right, cv::BORDER_REPLICATE);

				for (int i = 0; i < 5; i++)
				{
					for (int j = 0; j < 8; j++)
					{
						// Begin: 衡量时间性能的QTime
						QTime time1 = QTime::currentTime();

						tmp_jpg = Mat2QImage(printGabor(this->gabor, i, j));

						// End: 衡量时间性能的QTime
						QTime time2 = QTime::currentTime();
						//qDebug() << time1.msecsTo(time2);

						filepath = QDir::currentPath()+"/AfterPreprocess/"+store_path+dirinfo_outter.fileName()+"/"+dirinfo_inner.fileName()+"/"+fileinfo.baseName();
						filename = QString("gabor_%1_%2.jpg").arg(i).arg(j);
						qDebug() << filepath; 
						if (!dir.exists(filepath))
							dir.mkpath(filepath);
						tmp_jpg->save(filepath+"/"+filename);
						delete tmp_jpg;
					}
				}
			}
		}
	}
}

int main()
{
	// Debug gpu::convolve
	//cv::Mat src_host = cv::imread("lena.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	//cv::gpu::GpuMat dst, src;
	//src.upload(src_host);

	//int top_buttom = src.rows * 0.1;
	//int left_right = src.cols * 0.1;

	//cv::gpu::copyMakeBorder(src, dst, top_buttom, top_buttom, left_right, left_right, cv::BORDER_REPLICATE);

	//cv::Mat result_host;
	//dst.download(result_host);
	//cv::imwrite("lena_AfterMakeBorder.jpg", result_host);

	//QImage *tmp_jpg;
	//result_host.copyTo(frame);
	//tmp_jpg = printGabor_2(1, 1);
	//tmp_jpg->save("lena_AfterAll_Cpu.jpg");
	Preprocesor preprocessor(QString("D:\\ck\\cohn-kanade\\cohn-kanade"));
	preprocessor.generator();
	int CudaEnabledDeviceCount = cv::gpu::getCudaEnabledDeviceCount();
	qDebug("%d", CudaEnabledDeviceCount);
}