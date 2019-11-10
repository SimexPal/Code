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


#ifndef THREADS_H
#define THREADS_H

#include <omp.h>
#include <stdio.h>   /* gets */
#include <stdlib.h>  /* atoi, malloc */
#include <string.h>  /* strcpy */
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <gsl/gsl_rng.h>
#include "compileTimeOptions.h"
#include "utils.h"
#include "timer.h"
#include "argParser.h"
#include "algorithm.h"
#include "inputReader.h"

typedef struct openmp{
  int nProcs;
  int nThreads;
  int threadId;
}OPENMP;

#if FORCE_THREADWISE_SEQUENTIAL_RUN
omp_lock_t forceSequentialLock; // Global lock used to force sequential run
#endif

// Helper functions
void threadInit(OPENMP* openMP);
void threadBarrier();
void threadEnd();


// Interface functions
gsl_rng** threadRandInit(ulint seed);

bool threadCopyGraph(GRAPH* g);

bool threadGetOriginalCooc(GRAPH* g);

bool threadRunSwapsStep(GRAPH* g, gsl_rng **randGenerator, ulint nSwaps);

bool threadUpdateTmpResult(GRAPH* g, TMPRESULT* tmpResult);

void threadDeleteGraph(GRAPH* g);

void threadRandFree(gsl_rng** randG);

// Core functions
bool threadGetCooc(GRAPH* g, uint** coocs);

bool threadRunSingleSwitchesBipartite(GRAPH* g, gsl_rng** randG, ulint nSwaps);
bool threadRunSingleSwitchesGeneral(GRAPH* g, gsl_rng** randG, ulint nSwaps);
bool threadRunCurveballBipartite(GRAPH* g, gsl_rng** randG, ulint nSwaps);


#endif
