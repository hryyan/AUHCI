#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include CUDA runtime and CUFFT
#include <cuda_runtime.h>
#include <cufft.h>

// Helper functions for CUDA
#include <helper_functions.h>
#include <helper_cuda.h>

#include "convolutionFFT2D_common.h"

int snapTransformSize(int dataSize);
float getRand(void);
bool test0(void);
bool test1(void);
bool test2(void);
