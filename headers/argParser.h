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


#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <stdio.h>   /* gets */
#include <stdlib.h>  /* atoi, malloc */
#include <string.h>  /* strcpy */
#include "compileTimeOptions.h"
#include "utils.h"
#include "timer.h"

typedef struct settings {
  char inputFilePath[MAX_FILEPATH_SIZE];
  char inputFileName[MAX_FILENAME_SIZE];
  bool isBinaryInput;
  bool writeBinaryGraph;
  char outputFilePath[MAX_FILEPATH_SIZE];
  char outputFileName[MAX_FILENAME_SIZE + MAX_FILEPATH_SIZE];
  bool appendRunInfo : 1;

  char externalGtFileName[MAX_FILENAME_SIZE];
  bool hasExternalGt : 1;
  bool includeGTMissingNodes : 1;
  bool ignoreGTMissingNodes : 1;

  bool gotExternalSeed : 1;
  ulint seed;

  bool isBipartiteGraph : 1;
  char bipartiteSideOfInterest;
  uint directEdgeCoocValue;

  ulint nSwaps;
  bool elneSwaps : 1;
  bool runCurveball : 1;
  bool runSwapHeuristic : 1;
  uint nDegreesSwapHeuristic;
  uint nEventsPerDegreeSwapHeuristic;
  double thresholdTheta;

  uint minRelevantCooc;

  uint nSamples;
  uint nMaxSamples;
  bool runSamplesHeuristic : 1;
  double ratioGtPairsPerResultPair;
  double internalPpvThreshold;

  char dateStr[MAX_DATE_STR_SIZE];
  char runIndex[MAX_INT_STR_SIZE];

  bool isHelpRun : 1;
}SETTINGS;

SETTINGS settings;

int argParser(int argc, char **argv);

#endif
