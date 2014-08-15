#include <QtGui/QApplication>
#include "facialexpression_x64.h"
#include "Classification/classification_test.h"
#include "Source/generate.h"

// The programe type
enum Type
{
    GENERATESAMPLES,
    TRAININGSAMPLES,
    CLASSIFICATIONSTEST,
    CLASSIFYSAMPLES,
};

int main(int argc, char *argv[])
{
    int program_type = CLASSIFYSAMPLES;

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
}
