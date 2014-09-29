#ifndef MOUTH_BASED_AU_TEST_H
#define MOUTH_BASED_AU_TEST_H

#include <Classification/classification.h>
#include <Detection/eyedet.h>
#include <Detection/facedet.h>
#include <Detection/otherdet.h>
#include <Feature/gabor.h>
#include <Feature/haarclassifier.h>
#include <Source/generate.h>
#include <Util/util.h>
#include <Util/tinyxml2.h>
#include <Util/source.h>
#include <Util/predefine.h>

static const QString directory = "../ck/cohn-kanade/cohn-kanade/";

struct Path1
{
    QString id, expression, filename, path;
    int frame;
};

int test(int r);

#endif // MOUTH_BASED_AU_TEST_H
