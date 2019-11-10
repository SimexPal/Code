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


#include "../headers/swapHeuristicModule.h"

uint findNeighbor(uint middlePoint, uint dist)
{
  uint result;
  if (dist%2) {             // If @dist is odd:
    result = middlePoint - dist;  // Move @dist to the left
  }
  else {                // If @dist is even
    result = middlePoint + dist;  // Move @dist to the right
  }
  return result;
}

double calcSparsity(uint* vec, uint vecSize)
{
    // @vec must be sorted in ascending order
    // Sparsity is here defined as the inverse of the variance of the difference
    //   between two consecutive elements
    // This has nothing to do with the number of zero elements
    double meanOfDiffs = 0;
    for(uint j=0; j < vecSize-1; ++j) {
        meanOfDiffs += (vec[j+1] - vec[j]);
    }
    meanOfDiffs /= (vecSize-1);

    double varOfDiffs = 0;
    for(uint j=0; j < vecSize-1; ++j) {
        varOfDiffs += ((vec[j+1] - vec[j]) - meanOfDiffs) * ((vec[j+1] - vec[j]) - meanOfDiffs);
    }
    varOfDiffs /= (vecSize-2);

    double sparsity = 1./varOfDiffs;
    return sparsity;
}


void findMostSparseArray(uint* set, uint setSize,
                         uint* subset, uint subsetSize,
                         uint firstIdx)
{
    // @set must be sorted in ascending order
    // Finds the most sparse subset (@subset) of length @subsetSize
    // in the set @set of length @setSize
    // given that the first element of @subset is @set[@firstIdx]

    #if PRINT_EVENT_SELECTION_SWAP_HEURISTIC
    {
      FILE* swapHeuristicInfo = fopen("swapHeuristicInfo.dbg", "a");
      fprintf(swapHeuristicInfo, "\nBefore init:\n");
      fprintf(swapHeuristicInfo, "Set: { ");
      for (uint i=0; i < setSize; ++i) {
        fprintf(swapHeuristicInfo, "%u ", set[i]);
      }
      fprintf(swapHeuristicInfo, "}\n");
      fprintf(swapHeuristicInfo, "Subset: { ");
      for (uint j=0; j < subsetSize; ++j) {
        fprintf(swapHeuristicInfo, "%u ", subset[j]);
      }
      fprintf(swapHeuristicInfo, "}\n");
      fclose(swapHeuristicInfo);
    }
    #endif

    uint subsetIdxsInSet[subsetSize]; // set[subsetIdxsInSet[i]] == subset[i]
    for(uint j=0; j < subsetSize-1; ++j) {
        subsetIdxsInSet[j] = firstIdx;
        subset[j] = set[firstIdx];
    }
    subset[subsetSize-1] = set[setSize-1];
    subsetIdxsInSet[subsetSize-1] = setSize-1;

    #if PRINT_EVENT_SELECTION_SWAP_HEURISTIC
    {
      FILE* swapHeuristicInfo = fopen("swapHeuristicInfo.dbg", "a");
      fprintf(swapHeuristicInfo, "\nAfter init:\n");
      fprintf(swapHeuristicInfo, "Set: { ");
      for (uint i=0; i < setSize; ++i) {
        fprintf(swapHeuristicInfo, "%u ", set[i]);
      }
      fprintf(swapHeuristicInfo, "}\n");
      fprintf(swapHeuristicInfo, "Subset: { ");
      for (uint j=0; j < subsetSize; ++j) {
        fprintf(swapHeuristicInfo, "%u ", subset[j]);
      }
      fprintf(swapHeuristicInfo, "}\n");
      fclose(swapHeuristicInfo);
    }
    #endif

    double S = calcSparsity(subset, subsetSize);
    double lastInnerS = -1;
    double lastOuterS = -1;
    while ( lastOuterS < S ) {
        lastOuterS = S;
        for (uint j=subsetSize-2; j > 0; --j) { // From subsetSize-2 until 1
            while ( lastInnerS <= S ) {
                lastInnerS = S;
                ++subsetIdxsInSet[j];
                subset[j] = set[subsetIdxsInSet[j]];
                S = calcSparsity(subset, subsetSize);
            }
            --subsetIdxsInSet[j];
            subset[j] = set[subsetIdxsInSet[j]];
            S = calcSparsity(subset, subsetSize);
        }
    }

    #if PRINT_EVENT_SELECTION_SWAP_HEURISTIC
    {
      FILE* swapHeuristicInfo = fopen("swapHeuristicInfo.dbg", "a");
      fprintf(swapHeuristicInfo, "\nAfter sparsing:\n");
      fprintf(swapHeuristicInfo, "Set: { ");
      for (uint i=0; i < setSize; ++i) {
        fprintf(swapHeuristicInfo, "%u ", set[i]);
      }
      fprintf(swapHeuristicInfo, "}\n");
      fprintf(swapHeuristicInfo, "Subset: { ");
      for (uint j=0; j < subsetSize; ++j) {
        fprintf(swapHeuristicInfo, "%u ", subset[j]);
      }
      fprintf(swapHeuristicInfo, "}\n");
      fclose(swapHeuristicInfo);
    }
    #endif
}

int selectEvents(GRAPH* g, uint** seletedNodes)
{
  uint degreeFrequencyIt;
  
  uint eventDegrees[graphInfo.nEvents];
  uint maxDegree=0;

  for(uint eventIt=0; eventIt < graphInfo.nEvents; eventIt++) {
    eventDegrees[eventIt] = getNodeDegreeViaAdjMatrix(g, eventIt, "event");
    if ( eventDegrees[eventIt] == 0 ) { FORWARD_ERROR; }
    if ( eventDegrees[eventIt] > maxDegree ) {
      maxDegree = eventDegrees[eventIt];
    }
  }

  #if PRINT_EVENT_SELECTION_SWAP_HEURISTIC
  {
    FILE* swapHeuristicInfo = fopen("swapHeuristicInfo.dbg", "a");
    fprintf(swapHeuristicInfo, "\nMax degree: %u\n", maxDegree);
    fclose(swapHeuristicInfo);
  }
  #endif

  uint degreeFrequencies[ maxDegree + 1 ];
  for (degreeFrequencyIt = 0; degreeFrequencyIt < (maxDegree + 1); degreeFrequencyIt++) {
    degreeFrequencies[degreeFrequencyIt] = 0;
  }
  for(uint eventIt = 0; eventIt < graphInfo.nEvents; eventIt++) {
    degreeFrequencies[eventDegrees[eventIt]]++;
  }

  #if PRINT_EVENT_SELECTION_SWAP_HEURISTIC
  {
    FILE* swapHeuristicInfo = fopen("swapHeuristicInfo.dbg", "a");
    fprintf(swapHeuristicInfo, "\n");
    for (uint degreeFrequencyIt = 0; degreeFrequencyIt < (maxDegree + 1); degreeFrequencyIt++) {
      fprintf(swapHeuristicInfo, "Number of events with degree %5u: %5u\n",
                                 degreeFrequencyIt, degreeFrequencies[degreeFrequencyIt]);
    }
    fprintf(swapHeuristicInfo, "\n");
    fclose(swapHeuristicInfo);
  }
  #endif  

  uint maxDegreeWithFrequencyAboveThreshold = 0;
  uint nDegreesWithFrequencyAboveThreshold = 0;
  for (uint degreeFrequencyIt = 0; degreeFrequencyIt < (maxDegree + 1); degreeFrequencyIt++) {
    if( degreeFrequencies[degreeFrequencyIt] >= settings.nEventsPerDegreeSwapHeuristic ) {
      maxDegreeWithFrequencyAboveThreshold = degreeFrequencyIt;
      ++nDegreesWithFrequencyAboveThreshold;
    }
  }

  if ( nDegreesWithFrequencyAboveThreshold < settings.nEventsPerDegreeSwapHeuristic ) {
    STDERR_INFO("The number of degrees (%u) with frequency greater than"
                "the threshold (%u) is smaller than the number of degrees"
                "degrees set to the swap heuristic (%u)."
                "Try using a larger data set;\n"
                "a smaller frequency threshold;\n"
                "a smaller number of degrees;\n"
                "or even set a fixed number of swaps.\n\n",
                nDegreesWithFrequencyAboveThreshold,
                settings.nEventsPerDegreeSwapHeuristic,
                settings.nDegreesSwapHeuristic
                );
    FORWARD_ERROR;
  }

  #if PRINT_EVENT_SELECTION_SWAP_HEURISTIC
  {
    FILE* swapHeuristicInfo = fopen("swapHeuristicInfo.dbg", "a");
    fprintf(swapHeuristicInfo, "Maximum degree with frequency above threshold (%u): %u\n",
                               settings.nEventsPerDegreeSwapHeuristic,
                               maxDegreeWithFrequencyAboveThreshold);
    fprintf(swapHeuristicInfo, "Number of degrees with frequency above threshold (%u): %u\n",
                               settings.nEventsPerDegreeSwapHeuristic,
                               nDegreesWithFrequencyAboveThreshold);
    fprintf(swapHeuristicInfo, "\nSelecting %u degrees with frequency >= %u...\n",
                               settings.nDegreesSwapHeuristic,
                               settings.nEventsPerDegreeSwapHeuristic);
    fclose(swapHeuristicInfo);
  }
  #endif

  uint degreesAboveThreshold[nDegreesWithFrequencyAboveThreshold];
  uint degreesAboveThresholdIt = 0;
  for (uint degreeFrequencyIt = 0; degreeFrequencyIt <= maxDegreeWithFrequencyAboveThreshold; degreeFrequencyIt++) {
    if( degreeFrequencies[degreeFrequencyIt] >= settings.nEventsPerDegreeSwapHeuristic ) {
      degreesAboveThreshold[degreesAboveThresholdIt] = degreeFrequencyIt;
      ++degreesAboveThresholdIt;
    }
  }

  uint selectedDegrees[settings.nDegreesSwapHeuristic];
  uint minSelectedDegree = maxDegreeWithFrequencyAboveThreshold / settings.nDegreesSwapHeuristic + 1;
  // Check if have enough degrees >= the min and decrease min until we have enough
  uint firstDegreeIdx = 0;
  uint nDegreesGreaterThanMin = 0;
  while ( nDegreesGreaterThanMin < settings.nEventsPerDegreeSwapHeuristic ) {
    --minSelectedDegree;
    for(uint degreeIt=0; degreeIt < nDegreesWithFrequencyAboveThreshold; degreeIt++) {
      if ( degreesAboveThreshold[degreeIt] >= minSelectedDegree ) {
        firstDegreeIdx = degreeIt;
        nDegreesGreaterThanMin = nDegreesWithFrequencyAboveThreshold - firstDegreeIdx;
        break; // Must break!
      }
    }
  }

  #if PRINT_EVENT_SELECTION_SWAP_HEURISTIC
  {
    FILE* swapHeuristicInfo = fopen("swapHeuristicInfo.dbg", "a");
    fprintf(swapHeuristicInfo, "Lowest degree set to %u\n",
                               minSelectedDegree);
    fprintf(swapHeuristicInfo, "Number of degrees >= the lowest: %u\n",
                               nDegreesGreaterThanMin);
    fclose(swapHeuristicInfo);
  }
  #endif

  findMostSparseArray(degreesAboveThreshold, nDegreesWithFrequencyAboveThreshold,
                      selectedDegrees, settings.nDegreesSwapHeuristic,
                      firstDegreeIdx);

  #if PRINT_EVENT_SELECTION_SWAP_HEURISTIC
  {
    FILE* swapHeuristicInfo = fopen("swapHeuristicInfo.dbg", "a");
    fprintf(swapHeuristicInfo, "Selected degrees\nDegree Frequency\n");
    for(uint degreeIt = 0; degreeIt < settings.nDegreesSwapHeuristic; degreeIt++) {
      fprintf(swapHeuristicInfo, "%6u %9u\n",
              selectedDegrees[degreeIt],
              degreeFrequencies[ selectedDegrees[degreeIt] ]);
    }
    fclose(swapHeuristicInfo);
  }
  #endif

  uint count[settings.nDegreesSwapHeuristic];
  for (degreeFrequencyIt=0; degreeFrequencyIt < settings.nDegreesSwapHeuristic; degreeFrequencyIt++) {
    count[degreeFrequencyIt] = 0;
  }
  
  // Final step - select events (nodes) to be used in the swap heuristic
  for(uint eventIt=0; eventIt < graphInfo.nEvents; eventIt++) {
    for(uint degreeIt=0; degreeIt < settings.nDegreesSwapHeuristic; degreeIt++) {
      if (eventDegrees[eventIt] == selectedDegrees[degreeIt]) {
        if (count[degreeIt] < settings.nEventsPerDegreeSwapHeuristic) {
          // If event has an appropriate degree, select it
          seletedNodes[degreeIt][count[degreeIt]] = eventIt;
          count[degreeIt]++;
        }
        break;
      }
    }
  }

  #if PRINT_EVENT_SELECTION_SWAP_HEURISTIC
  {
    FILE* swapHeuristicInfo = fopen("swapHeuristicInfo.dbg", "a");
    fprintf(swapHeuristicInfo, "\nSelected events (nodes)\nNodes degree - Node IDs");
    for(uint degreeIt = 0; degreeIt < settings.nDegreesSwapHeuristic; degreeIt++) {
      fprintf(swapHeuristicInfo, "\n%12u - ", selectedDegrees[degreeIt]);
      for(uint eventIt = 0; eventIt < settings.nEventsPerDegreeSwapHeuristic; eventIt++) {
        fprintf(swapHeuristicInfo, "%6u ", seletedNodes[degreeIt][eventIt]);
      }
    }
    fprintf(swapHeuristicInfo, "\n");
    fclose(swapHeuristicInfo);
  }
  #endif

  return SUCCESS;

}

uint coocIndex(uint degree1, uint degreeFrequencyIt, uint degree2, uint j)
{
  return ( degree1 * settings.nDegreesSwapHeuristic
        - (degree1 * (degree1 + 1)) / 2 ) * settings.nEventsPerDegreeSwapHeuristic * settings.nEventsPerDegreeSwapHeuristic
        + settings.nEventsPerDegreeSwapHeuristic * settings.nEventsPerDegreeSwapHeuristic * (degree2 - degree1 - 1)
        + settings.nEventsPerDegreeSwapHeuristic * degreeFrequencyIt
        + j;
}

void selectiveCoocCalc (GRAPH* g, TMPSWAPHEURESULT* tmpSwapHeuResult, uint** seletedNodes)
{
  uint numberOfSelectedPairs = settings.nEventsPerDegreeSwapHeuristic 
                                     * settings.nEventsPerDegreeSwapHeuristic
                                     * settings.nDegreesSwapHeuristic
                                     * (settings.nDegreesSwapHeuristic-1)
                                     / 2;

  for (uint graphIt = 0; graphIt < NUMBER_OF_THREADS; graphIt++) {
    for(uint degreeIt1 = 0; degreeIt1 < settings.nDegreesSwapHeuristic-1; degreeIt1++) {
      for(uint degreeIt2 = degreeIt1+1; degreeIt2 < settings.nDegreesSwapHeuristic; degreeIt2++) {
        for(uint eventIt1 = 0; eventIt1  < settings.nEventsPerDegreeSwapHeuristic; eventIt1 ++) {
          for(uint eventIt2 = 0; eventIt2  < settings.nEventsPerDegreeSwapHeuristic; eventIt2 ++) {
            tmpSwapHeuResult->lastCooc[ coocIndex(degreeIt1, eventIt1 , degreeIt2, eventIt2 ) ] =
              eventPairCoocCalc (&g[graphIt], seletedNodes[degreeIt1][eventIt1], seletedNodes[degreeIt2][eventIt2]);
          }
        }
      }
    }

    for(uint pairIt=0; pairIt < numberOfSelectedPairs; pairIt++) {
      tmpSwapHeuResult->coocSum[pairIt] += tmpSwapHeuResult->lastCooc[pairIt];
    }
  }
  
}

double thetaCalc(TMPSWAPHEURESULT* tmpSwapHeuResult, uint numsamples)
{
  double theta=0.0;

  uint groupId, i;
  uint numberOfGroups = settings.nDegreesSwapHeuristic 
                              * (settings.nDegreesSwapHeuristic-1)
                              / 2;
  uint groupLength = settings.nEventsPerDegreeSwapHeuristic 
                           * settings.nEventsPerDegreeSwapHeuristic;
  
  double groupSum, groupSquareSum;
  double groupMean, groupSTDEV;
  for (groupId=0; groupId < numberOfGroups; groupId++) {
    groupSum = 0.0;
    groupSquareSum = 0.0;
    for (i=0; i < groupLength; i++) {
      groupSum += (double) tmpSwapHeuResult->coocSum[groupId * groupLength + i] / (double)(numsamples);
      groupSquareSum += (double) tmpSwapHeuResult->coocSum[groupId * groupLength + i] / (double)(numsamples) * (double) tmpSwapHeuResult->coocSum[groupId * groupLength + i] / (double)(numsamples);
    }
    groupMean = (double) groupSum / (double) groupLength;
    groupSTDEV = sqrt((double)(groupSquareSum - ((double)(groupSum*groupSum) / (double)(groupLength))) / (double)(groupLength-1.0));
    
    theta += groupSTDEV / groupMean;  
  }

  theta = theta / numberOfGroups;

  // Handle a not-a-number result
  //  which comes from a null std deviation
  //  meaning not enough swaps and/or samples
  if ( isnan(theta) ) {
    theta = settings.thresholdTheta * 2.;
  }

  return theta;
}
