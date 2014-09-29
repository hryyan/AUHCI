#include <QtGui/QApplication>
#include "facialexpression_x64.h"
#include "Classification/classification_test.h"
#include "Source/generate.h"
#include "Test/mouth_based_au_test.h"

// The programe type
enum Type
{
    GENERATESAMPLES,
    TRAININGSAMPLES,
    CLASSIFICATIONSTEST,
    CLASSIFYSAMPLES,
    TESTMOUTHBASEDAU,
};

int main(int argc, char *argv[])
{
    int program_type = TESTMOUTHBASEDAU;

    // Generate training samples
    if (program_type == GENERATESAMPLES)
    {
        generator_samples();
    }

    // Training samples
    if (program_type == TRAININGSAMPLES)
    {

    }

    // Classification test
    if (program_type == CLASSIFICATIONSTEST)
    {
        classify_test();
    }

    // Classify samples
    if (program_type == CLASSIFYSAMPLES)
    {
        QApplication a(argc, argv);
        FacialExpressionX64 w;
        w.show();
        return a.exec();
    }

    // Test mouth based au
    if (program_type == TESTMOUTHBASEDAU)
    {
        for (int i = 0; i < 4; i++)
            test(i);
    }
}
