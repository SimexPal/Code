/*
 * Copyright (c) 2019, University of Kaiserslautern
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Andre Lucas Chinazzo
 */


#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>   /* gets */
#include <stdlib.h>  /* atoi, malloc */
#include <string.h>  /* strcpy */
#include <math.h>
#include <float.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "compileTimeOptions.h"
#include <sys/resource.h>

#define STDERR_INFO(format, ...) {\
    fflush(stdout); \
    fprintf(stderr, "[ERROR]: "); \
    fprintf(stderr, format, ##__VA_ARGS__); \
    fprintf(stderr, "\n    > File: %s \n", __FILE__); \
    fprintf(stderr, "    > Function: %s \n", __func__); \
    fprintf(stderr, "    > Line: %d \n", __LINE__); }

#define MPI_INFO(format, ...) {\
    fflush(stdout); \
    fprintf(stdout, "[MPI MODULE] Rank %2u: ", mpiModule.procId); \
    fprintf(stdout, format, ##__VA_ARGS__); }

#define DEBUG_PRINT(format, ...) {\
    fflush(stdout); \
    fprintf(stderr, "[DEBUG]: "); \
    fprintf(stderr, format, ##__VA_ARGS__); \
    fprintf(stderr, "\n    > File: %s \n", __FILE__); \
    fprintf(stderr, "    > Function: %s \n", __func__); \
    fprintf(stderr, "    > Line: %d \n", __LINE__); }

#define RETURN_ERROR { return FAILURE; }
#define RETURN_ERROR_V(errorValue) { return errorValue; }

#define FORWARD_ERROR {\
  STDERR_INFO("Called from:");\
  RETURN_ERROR; }
#define FORWARD_ERROR_V(errorValue) {\
  STDERR_INFO("Called from:");\
  RETURN_ERROR_V(errorValue); }

#define MEM_ERROR {\
  STDERR_INFO("Unable to allocate memory.");\
  RETURN_ERROR; }

#define MEM_ERROR_V(errorValue) {\
  STDERR_INFO("Unable to allocate memory.");\
  RETURN_ERROR_V(errorValue); }

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })

#define max(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b; })

#define arrayCalloc(pointer, size)                                      \
  do {                                                                  \
    size_t _size = (size);                                              \
    (pointer) = calloc(_size, sizeof (*pointer));                       \
}while(0)

#define halfMatrixCalloc(pointer, nRows)                                \
  do {                                                                  \
    size_t _nRows = (nRows);                                            \
    arrayCalloc((pointer), _nRows);                                     \
    if ( (pointer) != NULL ) {                                          \
      for (size_t _row = 0; _row < _nRows; ++_row) {                    \
        arrayCalloc((pointer)[_row], _nRows - _row);                    \
        if ( (pointer)[_row] == NULL ) {                                \
          /* TODO: Free already allocated blocks */                     \
          (pointer) = NULL;                                             \
          break;                                                        \
        }                                                               \
      }                                                                 \
    }                                                                   \
}while(0)

#define fullMatrixCalloc(pointer, nRows, nCols)                         \
  do {                                                                  \
    size_t _nRows = (nRows);                                            \
    size_t _nCols = (nCols);                                            \
    arrayCalloc((pointer), _nRows);                                     \
    if ( (pointer) != NULL ) {                                          \
      for (size_t _row = 0; _row < _nRows; ++_row) {                    \
        arrayCalloc((pointer)[_row], _nCols);                           \
        if ( (pointer)[_row] == NULL ) {                                \
          /* TODO: Free already allocated blocks */                     \
          (pointer) = NULL;                                             \
          break;                                                        \
        }                                                               \
      }                                                                 \
    }                                                                   \
}while(0)


typedef unsigned int uint;
typedef unsigned long int ulint;
typedef unsigned char bool;

float getPeakRSS();
float getCurrentRSS();

BLOCK checkBitOnBlockArray(BLOCK* blockArray, size_t bitIndex, size_t blockOffset);
void setBitOnBlockArray(BLOCK* blockArray, size_t bitIndex, size_t blockOffset);
void clearBitOnBlockArray(BLOCK* blockArray, size_t bitIndex, size_t blockOffset);
uint popCount(BLOCK i);

double deviation_uint(ulint oriSumOfElements, ulint sumOfElements, uint sampleSize );
double stddev_uint(ulint sumOfElements, ulint sumOfSquareElements, uint sampleSize);
float zScore_uint(ulint oriSumOfElements,
                  ulint sumOfElements, ulint sumOfSquareElements,
                  uint sampleSize );

char *getFilenameExt(const char *filename);

#endif
