INCLUDEPATH += /usr/local/opencv-2.4.9
LIBS += -L/usr/local/lib
LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_ml
LIBS += -lopencv_gpu
LIBS += -lopencv_video
LIBS += -lopencv_features2d
LIBS += -lopencv_calib3d
LIBS += -lopencv_objdetect
LIBS += -lopencv_contrib
LIBS += -lopencv_legacy
LIBS += -lopencv_flann
LIBS += -lopencv_nonfree

CONFIG += link_pkgconfig
PKGCONFIG += opencv

FORMS += \
    facialexpression_x64.ui

OTHER_FILES += \
    README.md \
    CodeAmount.py \
    util.py \
    Accelerator/convolutionFFT2D.cuh \
    Util/README.md \
    Util/CodeAmount.py \
    Util/util.py

HEADERS += \
    facialexpression_x64.h \
    Accelerator/conv2d.h \
    Accelerator/convolutionFFT2D_common.h \
    Classification/classification.h \
    Detection/eyedet.h \
    Detection/facedet.h \
    Detection/otherdet.h \
    Feature/haarclassifier.h \
    Source/generate.h \
    Util/predefine.h \
    Util/tinyxml2.h \
    Util/util.h \
    Feature/gabor.h \
    Classification/svm.h \
    Util/source.h \
    Classification/classification_test.h

SOURCES += \
    facialexpression_x64.cpp \
    main.cpp \
    Accelerator/conv2d.cpp \
    Accelerator/convolutionFFT2D_gold.cpp \
    Accelerator/convolutionFFT2D.cu \
    Classification/classification.cpp \
    Detection/eyedet.cpp \
    Detection/facedet.cpp \
    Detection/otherdet.cpp \
    Feature/gabor.cpp \
    Feature/haarclassifier.cpp \
    Source/generate.cpp \
    Util/tinyxml2.cpp \
    Util/util.cpp \
    Classification/svm.cpp \
    Classification/svm-predict.c \
    Classification/svm-scale.c \
    Classification/svm-train.c \
    Util/source.cpp \
    Classification/classification_test.cpp
