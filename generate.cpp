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
const double BORDER_FRAC = 1;

// 由于gpu::convolve函数的限制，需要对识别好的人眼进行一个校准
const double X_OFFSET = 51.;
const double Y_OFFSET = 50.;

// 把采集到的图像进行缩放，到统一尺寸
const int RESIZE_WIDTH  = 150;
const int RESIZE_HEIGHT = 150;

/**
 * ctr，设定数据库的地址
 * @param  src_path 数据库地址
 */
Preprocessor::Preprocessor(QString p)
{
	src_path = p;
	dst_path = QDir::currentPath()+"/AfterPreprocess/";
	gabor.Init(Size(iSize, iSize), sqrt(2.0), 1, CV_32F);
}

/**
 * dtr
 */
Preprocessor::~Preprocessor()
{

}

/**
 * 根据CK数据库的地址得到预处理后的图像
 * @param src_path CK数据库的地址
 */
void Preprocessor::generator()
{
	QFileInfo dirinfo_outter, dirinfo_inner, fileinfo;
	QFileInfoList middle_list, inner_list;
	QDir outter_dir, middle_dir, inner_dir, dir;
	QImage* tmp_jpg;
	QString filepath, filename;
	Mat face, gabor_result;
	cv::gpu::GpuMat g_src, g_dst;
	int index;

	FILE *fp = fopen("CK_database_information.xml", "w+");
	tinyxml2::XMLPrinter printer(fp);
	printer.OpenElement("Main");

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
		printer.OpenElement("People");
		printer.PushAttribute("name", dirinfo_outter.baseName().toStdString().c_str());

		foreach(dirinfo_inner, middle_list)
		{
			inner_dir.cd(dirinfo_inner.absoluteFilePath());
			qDebug() << dirinfo_inner.absoluteFilePath();
			inner_list = inner_dir.entryInfoList();

			printer.OpenElement("Expression");
			printer.PushAttribute("class", dirinfo_inner.baseName().toStdString().c_str());

			// 显示这张图片在这个表情中的帧数，从1开始
			index = 0;
			foreach(fileinfo, inner_list)
			{
				frame = cv::imread(fileinfo.absoluteFilePath().toStdString());
				imwrite("imread.jpg", frame);
				qDebug() << fileinfo.absoluteFilePath();
				
				index++;
				printer.OpenElement("Frame");
				printer.PushAttribute("Num", index);
				printer.PushAttribute("jpg", fileinfo.baseName().toStdString().c_str());

				// 识别人脸
				face = printFace();
				cv::resize(face, face, Size(RESIZE_WIDTH, RESIZE_HEIGHT));
				tmp_jpg = Mat2QImage(face);
				filepath = QDir::currentPath()+"/AfterPreprocess/"+store_path+dirinfo_outter.fileName()+"/"+dirinfo_inner.fileName();
				filename = "/"+fileinfo.baseName()+"face.jpg";
				qDebug() << filepath;

				// 识别人眼
				DetPar detPar;
				detPar.x = face.rows / 2; detPar.y = face.cols / 2;
				detPar.width = face.rows; detPar.height = face.cols;
				DetectEyes(detPar, face);

				printer.OpenElement("Eye_Position");
				printer.OpenElement("Left");
				printer.OpenElement("X");
				printer.PushText(detPar.lex);
				printer.CloseElement();
				printer.OpenElement("Y");
				printer.PushText(detPar.ley);
				printer.CloseElement();
				printer.CloseElement();
				printer.OpenElement("Right");
				printer.OpenElement("X");
				printer.PushText(detPar.rex);
				printer.CloseElement();
				printer.OpenElement("Y");
				printer.PushText(detPar.rey);
				printer.CloseElement();
				printer.CloseElement();
				printer.CloseElement();

				printer.CloseElement();

				if (!dir.exists(filepath))
					dir.mkpath(filepath);
				bool on = tmp_jpg->save(filepath+filename);
				delete tmp_jpg;

				if (on)
					qDebug("save face OK");

				int top_buttom = face.rows * BORDER_FRAC;
				int left_right = face.cols * BORDER_FRAC;
				copyMakeBorder(face, face, 0, top_buttom, 0, left_right, cv::BORDER_REPLICATE);
				imwrite("imread_makeborder.jpg", face);

				for (int i = 0; i < 5; i++) // 尺度
				{
					for (int j = 0; j < 8; j++) // 方向
					{
						// Begin: 衡量时间性能的QTime
						QTime time1 = QTime::currentTime();

						gabor_result = printGabor_(face, this->gabor, j, i);
						Mat gabor_cropped = Mat(gabor_result, Rect(Point(150-101+1, 150-101+1), Point(gabor_result.cols, gabor_result.rows)));
						tmp_jpg = Mat2QImage(gabor_cropped);

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
			printer.CloseElement();
		}
		printer.CloseElement();
	}
	printer.CloseElement();
}

/**
 * 求所有Gabor滤波后数据的L2范数
 */
void Preprocessor::getL2()
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
				QString p = dirinfo_innest.absoluteFilePath() +"_merged.jpg";
				qDebug() << p;
				imwrite(p.toStdString(), mat_tmp);
			}
		}
	}
}

int main()
{
	OpenEyeDetectors("C:\\Users\\vincent\\Documents\\Visual Studio 2010\\Projects\\CV_64bit\\FacialExpression_x64\\CacadeClassifier");
	Preprocessor preprocessor(QString("D:\\ck\\cohn-kanade\\cohn-kanade"));
	preprocessor.generator();

	preprocessor.getL2();
	return 0;
}