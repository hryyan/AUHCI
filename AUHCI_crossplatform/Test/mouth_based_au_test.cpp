#include "mouth_based_au_test.h"

static const int kAuNum = 16;
static const int kAuIndex[kAuNum] = { 1,  2,  4,  5,  6,  7,  9, 10, 12, 15,
                              16, 18, 20, 22, 23, 24};
extern Mat frame;

int test(int r)
{
    tinyxml2::XMLDocument pos_doc;
    FILE* fp = fopen("../AUHCI_crossplatform/CK_database_information.xml", "r");
    pos_doc.LoadFile(fp);

    tinyxml2::XMLElement *m = pos_doc.RootElement();
    tinyxml2::XMLElement *people = m->FirstChildElement("People");
    Path1 p;
    char filename[50];
    sprintf(filename, "gaussian_kernel_%d", r);
    FILE* fp1 = fopen(filename, "wb+");
    while (people)
    {
        p.id = people->Attribute("name");
        tinyxml2::XMLElement *expression = people->FirstChildElement("Expression");
        while (expression)
        {
            p.expression = expression->Attribute("class");
            tinyxml2::XMLElement *f = expression->FirstChildElement("Frame");
            while (f)
            {
                p.frame = atoi(f->Attribute("Num"));
                p.filename = f->Attribute("jpg");
                f = f->NextSiblingElement();
            }
            p.path = directory + p.id + "/" + p.expression + "/" + p.filename + ".png";
            InitPic(p.path.toStdString());

            if (frame.cols == 0 || frame.rows == 0)
            {
                expression = expression->NextSiblingElement();
                continue;
            }
            PrintFaceToFrame();
            DetectEyes();
            DetectNose();
            DetectMouth();
            Mat gabor = printGabor();
            bool* au_appear = (bool*)malloc(sizeof(bool)*16);
            getAU2(au_appear, gabor, r);

            int on_off = 0;
            int on_on  = 0;
            int off_off = 0;
            int off_on  = 0;

            CK_Preprocessor ck_pre("");
            vector<FACS_Face> vec = ck_pre.OutputFACSFromXML();

            for (int i = 0; i < vec.size(); i++)
            {
//                QString tmp, tmp2;
//                tmp  = QString::fromStdString(vec[i].id);
//                tmp2 = QString::fromStdString(p.id.toStdString().substr(1));
//                qDebug() << tmp << tmp2;
//                tmp  = QString::fromStdString(vec[i].expression);
//                tmp2 = QString::fromStdString(p.expression.toStdString());
//                qDebug() << tmp << tmp2;
                std::vector<int> off_on_vec, on_off_vec, on_on_vec;
                if (vec[i].id == p.id.toStdString() && \
                    vec[i].expression == p.expression.toStdString())
                {
                    for (int j = 0; j < kAuNum; j++)
                    {
                        if (au_appear[j])
                        {
                            if (vec[i].AU[kAuIndex[j]] != 0)
                            {
                                on_on++;
                                on_on_vec.push_back(kAuIndex[j]);
                            }
                            else
                            {
                                on_off++;
                                on_off_vec.push_back(kAuIndex[j]);
                            }
                        }
                        else
                        {
                            if (vec[i].AU[kAuIndex[j]] != 0)
                            {
                                off_on++;
                                off_on_vec.push_back(kAuIndex[j]);
                            }
                        }
                    }
                    char str[60];
                    sprintf(str, "Num: %s, Expression: %s, correct: %d, miss: %d, wrong: %d",\
                            vec[i].id.c_str(), vec[i].expression.c_str(), on_on, on_off, off_on);
                    fputs(str, fp1);

                    fputs("\n\tcorrect: ", fp1);
                    for (int k = 0; k < on_on_vec.size(); k++)
                    {
                        char str1[60];
                        sprintf(str1, "%d ", on_on_vec[k]);
                        fputs(str1, fp1);
                    }

                    fputs("\n\tmiss: ", fp1);
                    for (int k = 0; k < off_on_vec.size(); k++)
                    {
                        char str1[60];
                        sprintf(str1, "%d ", off_on_vec[k]);
                        fputs(str1, fp1);
                    }

                    fputs("\n\twrong: ", fp1);
                    for (int k = 0; k < on_off_vec.size(); k++)
                    {
                        char str1[60];
                        sprintf(str1, "%d ", on_off_vec[k]);
                        fputs(str1, fp1);
                    }
                    fputs("\n", fp1);
                    fflush(fp1);
                }
            }
            expression = expression->NextSiblingElement();
        }
        people = people->NextSiblingElement();
    }
    fclose(fp1);
    return 0;
}
