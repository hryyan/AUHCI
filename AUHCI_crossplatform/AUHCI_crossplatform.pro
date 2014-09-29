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
    Classification/classification_test.h \
    Classification/svm.h \
    Detection/eyedet.h \
    Detection/facedet.h \
    Detection/otherdet.h \
    Feature/haarclassifier.h \
    Feature/gabor.h \
    Source/generate.h \
    Util/predefine.h \
    Util/tinyxml2.h \
    Util/util.h \
    Util/source.h \
    Test/mouth_based_au_test.h

SOURCES += \
    facialexpression_x64.cpp \
    main.cpp \
    Accelerator/conv2d.cpp \
    Accelerator/convolutionFFT2D_gold.cpp \
    Accelerator/convolutionFFT2D.cu \
    Classification/classification.cpp \
    Classification/classification_test.cpp \
    Classification/svm.cpp \
    Classification/svm-predict.c \
    Classification/svm-scale.c \
    Classification/svm-train.c \
    Detection/eyedet.cpp \
    Detection/facedet.cpp \
    Detection/otherdet.cpp \
    Feature/gabor.cpp \
    Feature/haarclassifier.cpp \
    Source/generate.cpp \
    Util/tinyxml2.cpp \
    Util/util.cpp \
    Util/source.cpp \
    Test/mouth_based_au_test.cpp

CUDA_HEADERS += \
    Acceleraotr/convolutionFFT2D.cuh

CUDA_SOURCES += \
    Accelerator/convolutionFFT2D.cu

# Path to cuda toolkit install
CUDA_DIR       = /usr/local/cuda-6.0
HELPER_DIR     = /usr/local/cuda-6.0/samples/common
# Path to header and libs files
INCLUDEPATH   += $$CUDA_DIR/include
INCLUDEPATH   += $$HELPER_DIR/inc
QMAKE_LIBDIR  += $$CUDA_DIR/lib64
# libs used in your code
LIBS += -L$$CUDA_DIR/lib64
LIBS += -lcudart -lcuda
# GPU architecture
CUDA_ARCH      = sm_30
# Here are some NVCC flags I've always used by default
NVCCFLAGS     = --compiler-options -fno-strict-aliasing -use_fast_math --ptxas-options=-v

# Prepare the extra compiler configuration (taken from the nvidia forum - i'm not an expert in this part)
CUDA_INC = $$join(INCLUDEPATH,' -I','-I',' ')

cuda.commands = $$CUDA_DIR/bin/nvcc -m64 -O3 -arch=$$CUDA_ARCH -c $$NVCCFLAGS \
                $$CUDA_INC $$LIBS  ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT} \
# nvcc error printout format ever so slightly different from gcc
# http://forums.nvidia.com/index.php?showtopic=171651
                2>&1 | sed -r \"s/\\(([0-9]+)\\)/:\\1/g\" 1>&2

cuda.dependency_type = TYPE_C # there was a typo here. Thanks workmate!
cuda.depend_command = $$CUDA_DIR/bin/nvcc -O3 -M $$CUDA_INC $$NVCCFLAGS   ${QMAKE_FILE_NAME}

cuda.input = CUDA_SOURCES
cuda.output = ${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.o
# Tell Qt that we want add more stuff to the Makefile
QMAKE_EXTRA_COMPILERS += cuda

SOURCES -= Accelerator/convolutionFFT2D.cu
