// CK_Preprocessor.cpp: 用于生成训练样本
// 
// Created by Vincent Yan in 2014/03/17

#include "generate.h"
#include "QDir"
#include "QtDebug"
//#include "VO_Features.h"
//#include "VO_LBPFeatures.h"
#include "ctime"
#include "iostream"

//#define OUTPUT_XML
#define CALCULATE_GABOR

// source中每一帧的Mat
extern Mat frame;

// 数据流中的人脸信息
extern DetPar frame_detpar;

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

Information_Face* getResponseFrameOne(Information_Face* a, vector<Information_Face>& vecInfo)
{
	for (vector<Information_Face>::iterator b = vecInfo.begin(); b != vecInfo.end(); b++)
	{
		if (!b->id.compare(a->id)  && !b->expression.compare(a->expression) && b->frame == 1)
			return &(*b);
	}
	return NULL;
}

FACS_Face getResponseFACS(vector<FACS_Face>& vtorff, Information_Face& inff)
{
	for (vector<FACS_Face>::iterator b = vtorff.begin(); b != vtorff.end(); b++)
	{
		if (!b->id.compare(inff.id)  && !b->expression.compare(inff.expression))
			return *b;
	}
	return FACS_Face();
}

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
 * ctr，设定数据库的地址
 * @param  src_path 数据库地址
 */
CK_Preprocessor::CK_Preprocessor(QString p)
{
	src_path = p;
	dst_path = QDir::currentPath()+"/AfterPreprocess_Pre/";
	strcpy(positions_information_name, "CK_database_information.xml");
	strcpy(FACS_information_name, "FACS_information.xml");
	gabor.Init(Size(iSize, iSize), sqrt(2.0), 1, CV_32F);
	//gabor.Init(Size(iSize, iSize), CV_PI*2, 1, CV_32F);
}

/**
 * dtr
 */
CK_Preprocessor::~CK_Preprocessor()
{

}

/**
 * 根据CK数据库的地址得到预处理后的图像，并且根据需求输出人脸信息的XML
 * @param src_path CK数据库的地址
 */
void CK_Preprocessor::generator()
{
	QFileInfo dirinfo_outter, dirinfo_inner, fileinfo;
	QFileInfoList middle_list, inner_list;
	QDir outter_dir, middle_dir, inner_dir, dir;
	QImage* tmp_jpg;
	QString filepath, filename;
	Mat face, gabor_result;
	cv::gpu::GpuMat g_src, g_dst;
	int index;

	#ifdef OUTPUT_XML
	FILE *fp = fopen(positions_information_name, "w+");
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
				// imwrite("imread.jpg", frame);
				qDebug() << fileinfo.absoluteFilePath();
				
				index++;
				#ifdef OUTPUT_XML
				printer.OpenElement("Frame");
				printer.PushAttribute("Num", index);
				printer.PushAttribute("jpg", fileinfo.baseName().toStdString().c_str());
				#endif

				// 识别人脸
				face = printFace();
				cv::resize(face, face, Size(RESIZE_WIDTH, RESIZE_HEIGHT));
				frame_detpar.width = RESIZE_WIDTH;				frame_detpar.height = RESIZE_HEIGHT;
				frame_detpar.x	   = frame_detpar.width / 2;	frame_detpar.y		= frame_detpar.height / 2;
				filepath = dst_path+store_path+dirinfo_outter.fileName()+"/"+dirinfo_inner.fileName();
				filename = "/"+fileinfo.baseName()+"face.jpg";
				qDebug() << filepath;

				// 识别人眼，嘴和鼻尖
				DetectEyes(face);
				if (index == 1)
				{
					DetectMouth(face);
					DetectNose(face);
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

				tmp_jpg = Mat2QImage(face);
				if (!dir.exists(filepath))
					dir.mkpath(filepath);
				bool on = tmp_jpg->save(filepath+filename);
				delete tmp_jpg;

				if (on)
					qDebug("save face OK");

				#ifdef CALCULATE_GABOR
					#ifdef USE_OPENCV_GPU
					int top_buttom = face.rows * BORDER_FRAC;
					int left_right = face.cols * BORDER_FRAC;
					copyMakeBorder(face, face, 0, top_buttom, 0, left_right, cv::BORDER_REPLICATE);
					#else
					int top_buttom = face.rows * BORDER_FRAC * 0.335;
					int left_right = face.cols * BORDER_FRAC * 0.335;
					copyMakeBorder(face, face, top_buttom, 0, left_right, 0, cv::BORDER_REPLICATE);
					#endif
				// imwrite("imread_makeborder.jpg", face);

				for (int i = 0; i < 5; i++) // 尺度
				{
					for (int j = 0; j < 8; j++) // 方向
					{
						// Begin: 衡量时间性能的QTime
						QTime time1 = QTime::currentTime();
						gabor_result = printGabor_(face, this->gabor, j, i);
						// End: 衡量时间性能的QTime
						QTime time2 = QTime::currentTime();
						qDebug() << time1.msecsTo(time2);
						Mat gabor_cropped = Mat(gabor_result, Rect(Point(150-101+1, 150-101+1), Point(gabor_result.cols, gabor_result.rows)));
						tmp_jpg = Mat2QImage(gabor_cropped);

						filepath = dst_path+store_path+dirinfo_outter.fileName()+"/"+dirinfo_inner.fileName()+"/"+fileinfo.baseName();
						filename = QString("gabor_%1_%2.jpg").arg(i).arg(j);
						qDebug() << filepath; 
						if (!dir.exists(filepath))
							dir.mkpath(filepath);
						tmp_jpg->save(filepath+"/"+filename);
						delete tmp_jpg;
					}
				}
				#endif
			}
			#ifdef OUTPUT_XML 
			printer.CloseElement();
			#endif
		}
		#ifdef OUTPUT_XML 
		printer.CloseElement();
		#endif
	}
	#ifdef OUTPUT_XML 
	printer.CloseElement();
	#endif
}

/**
 * 求所有Gabor滤波后数据的L2范数
 */
void CK_Preprocessor::getL2()
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

vector<Information_Face> CK_Preprocessor::getInformationFromXML()
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(positions_information_name);

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
					coord.left_eye_x	= atoi(coord_x->GetText());
					tinyxml2::XMLElement *coord_y = coord_x->NextSiblingElement();
					coord.left_eye_y	= atoi(coord_y->GetText());

					coords = coords->NextSiblingElement();
					coord_x = coords->FirstChildElement("X");
					coord.right_eye_x	= atoi(coord_x->GetText());
					coord_y = coord_x->NextSiblingElement();
					coord.right_eye_y	= atoi(coord_y->GetText());

					coords = coords->NextSiblingElement();
					coord_x = coords->FirstChildElement("X");
					coord.mouth_x		= atoi(coord_x->GetText());
					coord_y = coord_x->NextSiblingElement();
					coord.mouth_y		= atoi(coord_y->GetText());

					coords = coords->NextSiblingElement();
					coord_x = coords->FirstChildElement("X");
					coord.nose_x		= atoi(coord_x->GetText());
					coord_y = coord_x->NextSiblingElement();
					coord.nose_y		= atoi(coord_y->GetText());

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
 * 原先的FACS信息由python脚本转换成XML格式
 * 在本程序中，把这些信息加入到图片信息中
 */
vector<FACS_Face> CK_Preprocessor::getFACSInformation()
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(FACS_information_name);

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

// 对每个Information进行检测，1,在范围内，2，是第一张或者最后一张图，若成立，则返回true、
bool sliceOk(Information_Face& info, int x_coord, int y_coord, int left, int right, int top, int bottom)
{
	// 这是第一张图或者是最后一张图
	if (info.frame == 1 || info.final)
		if (x_coord != 99999 && y_coord != 99999 && x_coord >= left && x_coord <= RESIZE_WIDTH-right && y_coord >= top && y_coord <= RESIZE_HEIGHT-bottom)
			return true;
	return false;
}

// 如果SliceOK，则把该区域转换为一维，并且根据是否是第一张或者最后一张，push到指定的vector中。
void outputSliceHelper(vector<Mat>& primeMatV, vector<Mat>& finalMatV, vector<FACS_Face>& finalFACS, Information_Face& info, vector<FACS_Face>& vecFACS, Mat img, char* dstpath, int x_coord, int y_coord, int left, int right, int top, int bottom, bool needFlip)
{
	if (sliceOk(info, x_coord, y_coord, left, right, top, bottom))
	{
		Mat roi, result;
		int width = right + left;						// Slice的宽度
		int height = bottom + top;						// Slice的高度

		Rect r = Rect(Point(x_coord-left, y_coord-top), Point(x_coord+right, y_coord+bottom));

		roi.create(width, height, CV_8UC1);
		if (!needFlip)
			roi = Mat(img, r).clone();
		else
		{
			flip(Mat(img, r), img, 1);
			roi = img.clone();
		}
		cv::normalize(roi, roi, 0, 255, CV_MINMAX, CV_8UC1);
		if (roi.isContinuous())
		{
			result = roi.reshape(0, 1);
			if (info.frame == 1)
			{
				primeMatV.push_back(result);
				imwrite(dstpath, roi);
			}
			else
			{
				finalMatV.push_back(result);
				finalFACS.push_back(getResponseFACS(vecFACS, info));
				imwrite(dstpath, roi);
			}
		}
		else
			qDebug() << "Not continuous, error";
	}
}

int outputSlice(vector<Mat>& primeMatV, vector<Mat>& finalMatV, vector<FACS_Face>& finalFACS, vector<Information_Face>& vecInfo, vector<FACS_Face>& vecFACS, int left, int right, int top, int bottom, FACESECTION section)
{
	char srcpath[SLEN], dstpath[SLEN];
	Mat img;
	int sp = 0;		// 在左右脸的检测AU中，由于需要Flip归一化，但是Label中并没有这个信息，所以需要用Sp来区隔

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

		sprintf(srcpath, "C:\\Users\\vincent\\Documents\\Visual Studio 2010\\Projects\\CV_64bit\\FacialExpression_x64\\AfterPreprocess_Pre\\%s\\%s\\%s_merged_normalized.jpg", \
			b->id.c_str(), b->expression.c_str(), b->filename.c_str());
		// Slice的位置
		sprintf(dstpath, "Slices\\%s_left.jpg", b->filename.c_str());
		img = cv::imread(srcpath);
		if (section == EYE)
			outputSliceHelper(primeMatV, finalMatV, finalFACS, *b, vecFACS, img, dstpath, x_coord, y_coord, left, right, top, bottom, false);
		else
			outputSliceHelper(primeMatV, finalMatV, finalFACS, *b, vecFACS, img, dstpath, x_coord, y_coord, left, 0, top, bottom, false);
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
		}
		else if (section == NOSE)
			getOffsetNose(&(*b), x_coord, y_coord, vecInfo);
		else if (section == MOUTH)
			getOffsetMouth(&(*b), x_coord, y_coord, vecInfo);

		sprintf(srcpath, "C:\\Users\\vincent\\Documents\\Visual Studio 2010\\Projects\\CV_64bit\\FacialExpression_x64\\AfterPreprocess_Pre\\%s\\%s\\%s_merged_normalized.jpg", \
			b->id.c_str(), b->expression.c_str(), b->filename.c_str());
		// Slice的位置
		sprintf(dstpath, "Slices\\%s_right.jpg", b->filename.c_str());
		img = cv::imread(srcpath);
		if (section == EYE)
			outputSliceHelper(primeMatV, finalMatV, finalFACS, *b, vecFACS, img, dstpath, x_coord, y_coord, right, left, top, bottom, true);
		else
			outputSliceHelper(primeMatV, finalMatV, finalFACS, *b, vecFACS, img, dstpath, x_coord, y_coord, 0, right, top, bottom, true);
	}
	return sp;
}

void CK_Preprocessor::outputTxt(vector<Information_Face> vecInfo, vector<FACS_Face> vecFACS, FACESECTION section, int left, int right, int top, int bottom, int au)
{
	// 一些通用的定义
	vector<Mat> primeMatV;					// 每个sequence的第一张的Slice
	vector<Mat> finalMatV;					// 每个sequence的最后一张的Slice
	vector<FACS_Face> finalFACS;			// 每个sequence的AU Label，即最后一张的Slice呈现的AU Label

	int width = right + left;						// Slice的宽度
	int height = bottom + top;						// Slice的高度

	char filename[20];
	sprintf(filename, "AU_%d.txt", au);

	// 对指定区域进行Slice
	int sp = outputSlice(primeMatV, finalMatV, finalFACS, vecInfo, vecFACS, left, right, top, bottom, section);

	FILE* fp = fopen(filename, "w+");
	for (vector<Mat>::iterator b = primeMatV.begin(); b != primeMatV.end(); b++)
	{
		// 无表情的label为0
		fputs("0 ", fp);
		char* it = b->ptr<char>(0);
		for (int i = 0; i < width * height; i++)
		{
			char buff_[50];
			int int_tmp = it[i] + 128;
			sprintf(buff_, "%d:%d ", i+1, int_tmp);
			fputs(buff_, fp);
			if (i == 1000)
				fflush(fp);
		}
		fputs("\n", fp);
		fflush(fp);
	}

	int i = 0;
	char label[20];
	for (vector<Mat>::iterator b = finalMatV.begin(); b != finalMatV.end(); b++, i++)
	{
		memset(label, 0, sizeof(label));

		// 左眼
		if (i < sp)
		{
			if (finalFACS.at(i).AU[au] != 0 && finalFACS.at(i).AU[au]/1000!=2)
			 	strcat(label, "1 ");
			else
				strcat(label, "0 ");
		}
		// 右眼
		else
		{
			if (finalFACS.at(i).AU[au] != 0 && finalFACS.at(i).AU[au]/1000!=1)
			 	strcat(label, "1 ");
			else
				strcat(label, "0 ");
		}

		fputs(label, fp);
		char* it = b->ptr<char>(0);
		for (int i = 0; i < width * height; i++)
		{
			char buff_[50];
			int int_tmp = it[i] + 128;
			sprintf(buff_, "%d:%d ", i+1, int_tmp);
			fputs(buff_, fp);
			if (i == 1000)
				fflush(fp);
		}
		fputs("\n", fp);
		fflush(fp);
	}
	fclose(fp);
}

//int main()
//{
//	CK_Preprocessor CK_preprocessor(QString("D:\\ck\\cohn-kanade\\cohn-kanade"));
//	CK_preprocessor.generator();
	//vector<Information_Face> a = CK_preprocessor.getInformationFromXML();
	//vector<FACS_Face> b = CK_preprocessor.getFACSInformation();

	//CK_preprocessor.outputTxt(a, b, EYE, 15, 15, 35, 10, 1);   // AU1、AU2
	//CK_preprocessor.outputTxt(a, b, EYE, 15, 15, 15, 15);	// AU4、AU5
	//CK_preprocessor.outputTxt(a, b, EYE, 20, 20, 10, 50, 7);	// AU6、AU7
	//CK_preprocessor.outputTxt(a, b, EYE, 0, 30, 15, 15, 9);	// AU9
	//CK_preprocessor.outputTxt(a, b, MOUTH, 30, 30, 20, 10, 24);	// AU10、AU12、AU15、AU16、AU18、AU20、AU22、AU23、AU24

//	return 0;
//}