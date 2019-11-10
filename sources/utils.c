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


#include "../headers/utils.h"

/**
 * Returns the peak (maximum so far) resident set size (physical
 * memory use) measured in Megabytes
 */
float getPeakRSS()
{
  struct rusage rusage;
  getrusage( RUSAGE_SELF, &rusage );
  return (float)rusage.ru_maxrss / 1024 / 1024;
}

/**
 * Returns the current resident set size (physical memory use) measured
 * in Gigabytes, or zero if the value cannot be determined.
 */
float getCurrentRSS()
{
  long rss = 0L;
  FILE* fp = NULL;
  if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
      return (size_t)0L;      /* Can't open? */
  if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
  {
      fclose( fp );
      return (size_t)0L;      /* Can't read? */
  }
  fclose( fp );
  return (float)((size_t)rss * (size_t)sysconf( _SC_PAGESIZE)) / 1024 / 1024 / 1024;
}

BLOCK checkBitOnBlockArray(BLOCK* blockArray, size_t bitIndex, size_t blockOffset)
{
  return ( blockArray[ blockOffset + bitIndex / BITS_PER_BLOCK ]
            & ((BLOCK)1 << (bitIndex % BITS_PER_BLOCK))
         );
}

void setBitOnBlockArray(BLOCK* blockArray, size_t bitIndex, size_t blockOffset)
{
  ( blockArray[ blockOffset + bitIndex / BITS_PER_BLOCK ]
     |= ((BLOCK)1 << (bitIndex % BITS_PER_BLOCK))
  );
}

void clearBitOnBlockArray(BLOCK* blockArray, size_t bitIndex, size_t blockOffset)
{
  ( blockArray[ blockOffset + bitIndex / BITS_PER_BLOCK ]
     &= ~((BLOCK)1 << (bitIndex % BITS_PER_BLOCK))
  );
}

uint popCount(BLOCK _block)
{
// Options of bit counters are abundant and data dependent
// Since we deal with sparse blocks, the most efficient implementation
//  was shown to be the following:

  uint count;
  for (count = 0; _block; count++) {
    _block &= _block - 1;
  }
  return count;

  // Other options can be found at:
  // https://en.wikipedia.org/wiki/Hamming_weight
}


double deviation_uint(ulint oriSumOfElements, ulint sumOfElements, uint sampleSize )
{
  return (double)( (double)oriSumOfElements - (double)sumOfElements / sampleSize );
}

double stddev_uint(ulint sumOfElements, ulint sumOfSquareElements, uint sampleSize)
{
  return (double)( sqrt( ( (double)sumOfSquareElements
                          - (double)sumOfElements / sampleSize * sumOfElements)
                        / ((double)sampleSize-1.0)
                      )
                );
}

float zScore_uint(ulint oriSumOfElements,
                  ulint sumOfElements, ulint sumOfSquareElements,
                  uint sampleSize )
{
  float zScore = (float)
               ( deviation_uint(oriSumOfElements, sumOfElements, sampleSize)
                / stddev_uint(sumOfElements, sumOfSquareElements, sampleSize) );
#if AVOID_NAN_AND_INF_ZSCORE
  if ( isnan(zScore) ) {
    zScore = 0.0;
  } else if ( isinf(zScore) ) {
    zScore = zScore > 0 ? FLT_MAX : -FLT_MAX;
  }
#endif
  return zScore;
}

char *getFilenameExt(const char *filename) {
  // Return either a pointer to the last '.' or the end of the string
  const char *begin = strrchr(filename, '/'); // Split path and filename
  if ( !begin ) { // If there is no path
    begin = filename;
  } else { // If there is a path, go past it
    ++begin;
  }
  char *dot = strrchr(begin, '.'); // Find last '.' occurrence
  if ( !dot || dot == begin ) { // If it is a dot file with no ext (eg. .secretFile)
    return strrchr(begin, '\0'); // Find and return the end point of the string
  }
  return dot; // Return pointer to the last '.' occurrence
}
