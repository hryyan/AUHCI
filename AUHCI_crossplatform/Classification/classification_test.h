#ifndef CLASSIFICATION_TEST_H
#define CLASSIFICATION_TEST_H

#include "classification.h"
#include "Detection/eyedet.h"
#include "Detection/facedet.h"
#include "Detection/otherdet.h"
#include "Util/source.h"
#include "Util/util.h"
#include "Feature/gabor.h"
#include "Feature/haarclassifier.h"

//有时候需要生成测试数据
void classify_test();

#endif // CLASSIFICATION_TEST_H
