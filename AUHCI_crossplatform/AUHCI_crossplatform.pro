INCLUDEPATH += /usr/local/opencv-2.4.9
LIBS += -L/usr/local/lib
LIBS += -lopencv_core
LIBS += -lopencv_imgproc
LIBS += -lopencv_highgui
LIBS += -lopencv_ml
LIBS += -lopencv_video
LIBS += -lopencv_features2d
LIBS += -lopencv_calib3d
LIBS += -lopencv_objdetect
LIBS += -lopencv_contrib
LIBS += -lopencv_legacy
LIBS += -lopencv_flann
LIBS += -lopencv_nonfree

FORMS += \
    facialexpression_x64.ui

OTHER_FILES += \
    convolutionFFT2D.cuh \
    README.md \
    CodeAmount.py \
    util.py

HEADERS += \
    classification.h \
    conv2d.h \
    convolutionFFT2D_common.h \
    eyedet.h \
    facedet.h \
    facialexpression_x64.h \
    gabor.h \
    generate.h \
    haarclassifier.h \
    otherdet.h \
    predefine.h \
    source.h \
    svm.h \
    tinyxml2.h \
    util.h

SOURCES += \
    classification.cpp \
    conv2d.cpp \
    convolutionFFT2D_gold.cpp \
    eyedet.cpp \
    facedet.cpp \
    facialexpression_x64.cpp \
    gabor.cpp \
    generate.cpp \
    haarclassifier.cpp \
    main.cpp \
    otherdet.cpp \
    source.cpp \
    svm.cpp \
    tinyxml2.cpp \
    util.cpp \
    convolutionFFT2D.cu \
    svm-predict.c \
    svm-scale.c \
    svm-train.c
