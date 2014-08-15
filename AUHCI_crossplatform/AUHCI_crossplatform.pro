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
    util.py \
    Accelerator/convolutionFFT2D.cuh \
    Util/README.md \
    Util/CodeAmount.py \
    Util/util.py

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
    util.h \
    Accelerator/conv2d.h \
    Accelerator/convolutionFFT2D_common.h \
    Accelerator/svm.h \
    Classification/classification.h \
    Detection/eyedet.h \
    Detection/facedet.h \
    Detection/otherdet.h \
    Feature/haarclassifier.h \
    Source/generate.h \
    Source/source.h \
    Util/predefine.h \
    Util/tinyxml2.h \
    Util/util.h \
    Feature/gabor.h

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
    svm-train.c \
    Accelerator/conv2d.cpp \
    Accelerator/convolutionFFT2D_gold.cpp \
    Accelerator/svm.cpp \
    Accelerator/convolutionFFT2D.cu \
    Accelerator/svm-predict.c \
    Accelerator/svm-scale.c \
    Accelerator/svm-train.c \
    Classification/classification.cpp \
    Detection/eyedet.cpp \
    Detection/facedet.cpp \
    Detection/otherdet.cpp \
    Feature/gabor.cpp \
    Feature/haarclassifier.cpp \
    Source/generate.cpp \
    Source/source.cpp \
    Util/tinyxml2.cpp \
    Util/util.cpp
