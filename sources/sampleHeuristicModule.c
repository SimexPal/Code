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


#include "../headers/sampleHeuristicModule.h"

bool initGtList(GTLIST* gtList, uint nElements)
{
  // Each element will be represented as a bit
  // within a block of bits (which lenght is defined at compileTimeOptions.h)
  gtList->nBlocks = (uint)ceil( (float)nElements / BITS_PER_BLOCK );

  gtList->blocks = (BLOCK*) calloc(gtList->nBlocks, sizeof(BLOCK));
  if (gtList->blocks == NULL) { MEM_ERROR; }

  return SUCCESS;
}

bool initGT(GROUNDTRUTH* gt, bool isInternalGt)
{
  gt->nGTPairs = 0;
  gt->gtFilteredSetLength = 0;
  gt->isInternalGt = isInternalGt;

  if ( initGtList(&gt->gtEventsList, graphInfo.nEvents) == FAILURE ) {
    FORWARD_ERROR;
  }

  if ( initGtList(&gt->gtPairsList, graphInfo.nRelevantPairs) == FAILURE ) {
    FORWARD_ERROR;
  }

  return SUCCESS;
}

bool initExternalGT(GROUNDTRUTH* gt, char* externalGtFileName)
{
  FILE* externalGtFile = fopen(externalGtFileName,"r");
  if( externalGtFile == NULL ) {
    STDERR_INFO( "%s: could not read.\n", externalGtFileName);
    FORWARD_ERROR;
  }
  gt->nGTPairs = 0;
  char c = fgetc(externalGtFile);
  while( c != EOF ) {
    if ( c == '\n' ) {
      ++gt->nGTPairs;
    }
    c = fgetc(externalGtFile);
  }

  fprintf(stdout, "Number of lines in external gt: %u", gt->nGTPairs);
  if ( initGtList(&gt->gtEventsList, graphInfo.nEvents) == FAILURE ) {
    STDERR_INFO("Failed to allocate memory for ground truth EVENTS list!\n");
    FORWARD_ERROR;
  }

  if ( initGtList(&gt->gtPairsList, graphInfo.nRelevantPairs) == FAILURE ) {
    STDERR_INFO("Failed to allocate memory for ground truth PAIRS list!\n");
    FORWARD_ERROR;
  }

  return SUCCESS;
}

void freeGT(GROUNDTRUTH* gt)
{
  free(gt->gtEventsList.blocks);
  gt->gtEventsList.blocks = NULL;

  free(gt->gtPairsList.blocks);
  gt->gtPairsList.blocks = NULL;
}

void filterResultByGT(GROUNDTRUTH* gt, PAIR* pairs,
                      TMPRESULT* tmpResult, uint nSamples)
{
  // The "filtered set" refers to the subset of result pairs
  //  in which at least one of the nodes of each pair
  //  is element of the set of nodes of the ground truth
  // !!! Better nomenclature needed !!!
  gt->gtFilteredSetLength = 0;
  uint relevantPairIt = 0;
  uint filteredSetPairIt = 0;
  for (uint row=0; row<(graphInfo.nEvents-1); row++) {
    for (uint col=0; col<(graphInfo.nEvents-1-row); col++) {
      if( graphInfo.originalCooc[row][col] >= settings.minRelevantCooc ) {
        if(    checkBitOnBlockArray( gt->gtEventsList.blocks, row, 0 )
            || checkBitOnBlockArray( gt->gtEventsList.blocks, row + col + 1, 0)
          ) { // <- Choose the ones from which at least one of the nodes is a node of the groud truth
          // Store event pair info
          pairs[filteredSetPairIt].eventId1 = row;
          pairs[filteredSetPairIt].eventId2 = row + col + 1;
          pairs[filteredSetPairIt].relevantPairId = relevantPairIt;
          pairs[filteredSetPairIt].pValue = tmpResult->pValue[row][col];
          pairs[filteredSetPairIt].zScore = zScore_uint(graphInfo.originalCooc[row][col],
                                                      tmpResult->coocSum[row][col],
                                                      tmpResult->coocSquareSum[row][col],
                                                      nSamples);
          // Increment the iterator of the filtered set
          filteredSetPairIt++;
        }
        // Increment the iterator of the result set
        relevantPairIt++;
      }
    }
  }
  // Store the number of pairs in the full set
  gt->gtFilteredSetLength = filteredSetPairIt;


  #if PRINT_PAIR_LISTS_FOR_SAMPLE_HEURISTIC
  if ( gt->isInternalGt ) {
  FILE* gtFullSet = fopen("gtFilteredSetInternalGT.dbg", "a");
  fprintf(gtFullSet,"___________________Sample: %u_________________________\n", nSamples);
  for (uint gtFullSetPairIt=0; gtFullSetPairIt < gt->gtFilteredSetLength; gtFullSetPairIt++) {
  fprintf(gtFullSet,"Gt filtered set: pair: (%u, %u):%u pvalue: %u zscore: %f\n",
        pairs[gtFullSetPairIt].eventId1,
        pairs[gtFullSetPairIt].eventId2,
        pairs[gtFullSetPairIt].relevantPairId,
        pairs[gtFullSetPairIt].pValue,
        pairs[gtFullSetPairIt].zScore);
  }
  fclose(gtFullSet);
  }
  #endif
  #if PRINT_PAIR_LISTS_FOR_EXTERNAL_GROUNTTRUTH
  if ( !gt->isInternalGt ) {
  FILE* gtFullSet = fopen("gtFilteredSetExternalGT.dbg", "a");
  fprintf(gtFullSet,"___________________Sample: %u_________________________\n", nSamples);
  for (uint gtFullSetPairIt=0; gtFullSetPairIt < gt->gtFilteredSetLength; gtFullSetPairIt++) {
  fprintf(gtFullSet,"Gt filtered set: pair: (%u, %u):%u pvalue: %u zscore: %f\n",
        pairs[gtFullSetPairIt].eventId1,
        pairs[gtFullSetPairIt].eventId2,
        pairs[gtFullSetPairIt].relevantPairId,
        pairs[gtFullSetPairIt].pValue,
        pairs[gtFullSetPairIt].zScore);
  }
  fclose(gtFullSet);
  }
  #endif

}

double calcPPV(GROUNDTRUTH* gt, PAIR* pairs,
               TMPRESULT* tmpResult, uint nSamples)
{

  if ( gt->nGTPairs == 0 ) { // No GT yet
    if ( gt->isInternalGt ) { // No prior GT to compare current results
      createInternalGT(gt, pairs, tmpResult, nSamples);
      return 2; // Impossible value - used to report first GT creation
    }
    else {
      if ( createExternalGT(gt, pairs, tmpResult, nSamples) == FAILURE ) {
        FORWARD_ERROR_V(-1); // Error value
      }
    }
  }

  filterResultByGT(gt, pairs, tmpResult, nSamples);

  #if PRINT_PAIR_LISTS_FOR_SAMPLE_HEURISTIC
  if ( gt->isInternalGt ) {
  FILE* gtSetTop = fopen("gtSetTopInternalGT.dbg", "a");
  fprintf(gtSetTop,"___________________Sample: %u_________________________\n", nSamples);
  fclose(gtSetTop);
  FILE* gtSetMatch = fopen("gtSetMatchInternalGT.dbg", "a");
  fprintf(gtSetMatch,"___________________Sample: %u_________________________\n", nSamples);
  fclose(gtSetMatch);
  }
  #endif
  #if PRINT_PAIR_LISTS_FOR_EXTERNAL_GROUNTTRUTH
  if ( !gt->isInternalGt ) {
  FILE* gtSetTop = fopen("gtSetTopExternalGT.dbg", "a");
  fprintf(gtSetTop,"___________________Sample: %u_________________________\n", nSamples);
  fclose(gtSetTop);
  FILE* gtSetMatch = fopen("gtSetMatchExternalGT.dbg", "a");
  fprintf(gtSetMatch,"___________________Sample: %u_________________________\n", nSamples);
  fclose(gtSetMatch);
  }
  #endif

  qselect(pairs, gt->gtFilteredSetLength, gt->nGTPairs-1);

  uint nMatchedPairs=0;
  for (uint pairIt=0; pairIt < gt->nGTPairs; pairIt++) {
    if (  checkBitOnBlockArray(gt->gtPairsList.blocks,
                               pairs[pairIt].relevantPairId, 0 )
       ) { // If pair in the top of the gt filtered set is a pair of the previous gt
      // Increment ppv
      nMatchedPairs++;
      #if PRINT_PAIR_LISTS_FOR_SAMPLE_HEURISTIC
      if ( gt->isInternalGt ) {
      FILE* gtSetMatch = fopen("gtSetMatchInternalGT.dbg", "a");
      fprintf(gtSetMatch,"Matched pairs: pair: (%u, %u):%u pvalue: %u zscore: %f\n",
              pairs[pairIt].eventId1,
              pairs[pairIt].eventId2,
              pairs[pairIt].relevantPairId,
              pairs[pairIt].pValue,
              pairs[pairIt].zScore);
      fclose(gtSetMatch);
      }
      #endif
      #if PRINT_PAIR_LISTS_FOR_EXTERNAL_GROUNTTRUTH
      if ( !gt->isInternalGt ) {
      FILE* gtSetMatch = fopen("gtSetMatchExternalGT.dbg", "a");
      fprintf(gtSetMatch,"Matched pairs: pair: (%u, %u):%u pvalue: %u zscore: %f\n",
              pairs[pairIt].eventId1,
              pairs[pairIt].eventId2,
              pairs[pairIt].relevantPairId,
              pairs[pairIt].pValue,
              pairs[pairIt].zScore);
      fclose(gtSetMatch);
      }
      #endif
    }
    #if PRINT_PAIR_LISTS_FOR_SAMPLE_HEURISTIC
    if ( gt->isInternalGt ) {
    FILE* gtSetTop = fopen("gtSetTopInternalGT.dbg", "a");
    fprintf(gtSetTop,"Gt set top %u: pair: (%u, %u):%u pvalue: %u zscore: %f\n",
            gt->nGTPairs,
            pairs[pairIt].eventId1,
            pairs[pairIt].eventId2,
            pairs[pairIt].relevantPairId,
            pairs[pairIt].pValue,
            pairs[pairIt].zScore);
    fclose(gtSetTop);
    }
    #endif
    #if PRINT_PAIR_LISTS_FOR_EXTERNAL_GROUNTTRUTH
    if ( !gt->isInternalGt ) {
    FILE* gtSetTop = fopen("gtSetTopExternalGT.dbg", "a");
    fprintf(gtSetTop,"Gt set top %u: pair: (%u, %u):%u pvalue: %u zscore: %f\n",
            gt->nGTPairs,
            pairs[pairIt].eventId1,
            pairs[pairIt].eventId2,
            pairs[pairIt].relevantPairId,
            pairs[pairIt].pValue,
            pairs[pairIt].zScore);
    fclose(gtSetTop);
    }
    #endif
  }

  if ( gt->isInternalGt ) {
    createInternalGT(gt, pairs, tmpResult, nSamples);
  }

  return (double)nMatchedPairs / gt->nGTPairs;

}

void createInternalGT(GROUNDTRUTH* gt, PAIR* pairs,
                      TMPRESULT* tmpResult, uint nSamples)
{

  gt->nGTPairs = (uint) (graphInfo.nRelevantPairs
                          * settings.ratioGtPairsPerResultPair);
  if( gt->nGTPairs < 1 ) {
    gt->nGTPairs = 1;
  }

  // List the relevant pairs to have a consistent indexing in the GT lists
  resultList(pairs, tmpResult,
             graphInfo.nEvents, nSamples);

  #if PRINT_PAIR_LISTS_FOR_SAMPLE_HEURISTIC
    {
    FILE* relevantPairs = fopen("relevantPairs.dbg", "a");
    fprintf(relevantPairs,"___________________Sample: %u_________________________\n", nSamples);
    for (uint pairIt=0; pairIt < graphInfo.nRelevantPairs; pairIt++) {
      fprintf(relevantPairs,"Relevant cooc pairs: pair: (%u, %u):%u pvalue: %u zscore: %f\n",
              pairs[pairIt].eventId1,
              pairs[pairIt].eventId2,
              pairs[pairIt].relevantPairId,
              pairs[pairIt].pValue,
              pairs[pairIt].zScore);
    }
    fclose(relevantPairs);
    }
  #endif

  // Clear last internal ground truth
  for (uint blockIt = 0; blockIt < gt->gtEventsList.nBlocks; blockIt++) {
    gt->gtEventsList.blocks[blockIt] = (BLOCK)0;
  }
  for (uint blockIt = 0; blockIt < gt->gtPairsList.nBlocks; blockIt++) {
    gt->gtPairsList.blocks[blockIt] = (BLOCK)0;
  }

  // Sort top pairs
  qselect(pairs, graphInfo.nRelevantPairs, gt->nGTPairs-1);

  // Fill internal ground truth events and pairs
  for (uint pairIt=0; pairIt < gt->nGTPairs; pairIt++) {
    setBitOnBlockArray(gt->gtEventsList.blocks, pairs[pairIt].eventId1, 0);
    setBitOnBlockArray(gt->gtEventsList.blocks, pairs[pairIt].eventId2, 0);

    setBitOnBlockArray(gt->gtPairsList.blocks, pairs[pairIt].relevantPairId, 0);
  }

  #if PRINT_PAIR_LISTS_FOR_SAMPLE_HEURISTIC
  {
  FILE* eventsBinaryBlocks = fopen("eventsBinaryBlocks.dbg", "a");
  fprintf(eventsBinaryBlocks,"___________________Sample: %u_________________________\n", nSamples);
  fprintf(eventsBinaryBlocks,"Event ID\tSet/Clear(1/0)\tBlock content\n");
  for (uint eventIt=0; eventIt < graphInfo.nEvents; eventIt++) {
    fprintf(eventsBinaryBlocks,
            "%8u\t%14u\t0x%016jX\n",
            eventIt,
            checkBitOnBlockArray(gt->gtEventsList.blocks, eventIt, 0) ? 1:0,
            (uintmax_t)gt->gtEventsList.blocks[ eventIt/BITS_PER_BLOCK ]);
  }
  fclose(eventsBinaryBlocks);

  FILE* pairsBinaryBlocks = fopen("pairsBinaryBlocks.dbg", "a");
  fprintf(pairsBinaryBlocks,"___________________Sample: %u_________________________\n", nSamples);
  fprintf(pairsBinaryBlocks,"Relevant Pair ID\tSet/Clear(1/0)\tBlock content\n");
  for (uint pairIt=0; pairIt < graphInfo.nRelevantPairs; pairIt++) {
    fprintf(pairsBinaryBlocks,
            "%16u\t%14u\t0x%016jX\n",
            pairIt,
            checkBitOnBlockArray(gt->gtPairsList.blocks, pairIt, 0) ? 1:0,
            (uintmax_t)gt->gtPairsList.blocks[ pairIt/BITS_PER_BLOCK ]);
  }
  fclose(pairsBinaryBlocks);

  FILE* relevantPairsTop = fopen("relevantPairsTop.dbg", "a");
  fprintf(relevantPairsTop,"___________________Sample: %u_________________________\n", nSamples);
  for (uint pairIt=0; pairIt < gt->nGTPairs; pairIt++) {
    fprintf(relevantPairsTop,
            "Relevant Cooc pairs top %u: pair: (%u, %u):%u pvalue: %u zscore: %f\n",
            gt->nGTPairs,
            pairs[pairIt].eventId1,
            pairs[pairIt].eventId2,
            pairs[pairIt].relevantPairId,
            pairs[pairIt].pValue,
            pairs[pairIt].zScore);
  }
  fclose(relevantPairsTop);
  }
  #endif
}

bool createExternalGT(GROUNDTRUTH* gt, PAIR* pairs,
                      TMPRESULT* tmpResult, uint nSamples)
{

  // List the relevant pairs to have a consistent indexing in the GT lists
  resultList(pairs, tmpResult,
             graphInfo.nEvents, nSamples);

  #if PRINT_PAIR_LISTS_FOR_EXTERNAL_GROUNTTRUTH
    {
    FILE* relevantPairs = fopen("relevantPairsExternalGT.dbg", "a");
    for (uint pairIt=0; pairIt < graphInfo.nRelevantPairs; pairIt++) {
      fprintf(relevantPairs,"Relevant cooc pairs: pair: (%u, %u):%u pvalue: %u zscore: %f\n",
              pairs[pairIt].eventId1,
              pairs[pairIt].eventId2,
              pairs[pairIt].relevantPairId,
              pairs[pairIt].pValue,
              pairs[pairIt].zScore);
    }
    fclose(relevantPairs);
    }
  #endif

  // Clear GT list for safity
  for (uint blockIt = 0; blockIt < gt->gtEventsList.nBlocks; blockIt++) {
    gt->gtEventsList.blocks[blockIt] = (BLOCK)0;
  }
  for (uint blockIt = 0; blockIt < gt->gtPairsList.nBlocks; blockIt++) {
    gt->gtPairsList.blocks[blockIt] = (BLOCK)0;
  }

  FILE* externalGtFile = fopen(settings.externalGtFileName,"r");
  if( externalGtFile == NULL ) {
    STDERR_INFO( "%s: could not read.\n", settings.externalGtFileName);
    RETURN_ERROR;
  }
  char rightSideStr[graphInfo.maxNodeStrLenght];
  char leftSideStr[graphInfo.maxNodeStrLenght];
  gt->nGTPairs = 0;
  ulint lineCnt = 0;
  while( fscanf(externalGtFile, "%s %s\n", leftSideStr, rightSideStr) == 2 ) {
    ++lineCnt;
    uint eventIdl = graphInfo.nEvents; // Impossible value
    uint eventIdr = graphInfo.nEvents; // Impossible value
    // Find event ids associated with event string
    bool foundStrr = FALSE;
    bool foundStrl = FALSE;
    for( uint eventIt=0; eventIt < graphInfo.nEvents; ++eventIt ) {
      if( !strcmp(leftSideStr, graphInfo.eventList[eventIt]) ) {
        eventIdl = eventIt;
        foundStrl = TRUE;
        if ( foundStrr ) { break; }
      }
      if( !strcmp(rightSideStr, graphInfo.eventList[eventIt]) ) {
        eventIdr = eventIt;
        foundStrr = TRUE;
        if ( foundStrl ) { break; }
      }
    }
    // If both events are found, i.e. are in the input file,
    //  add them to GT's event list and the pair to GT's pair list (if relevant)
    if ( foundStrl && foundStrr ) {
      ++gt->nGTPairs;
      setBitOnBlockArray(gt->gtEventsList.blocks, eventIdl, 0);
      setBitOnBlockArray(gt->gtEventsList.blocks, eventIdr, 0);
      for( uint pairIt = 0; pairIt < graphInfo.nRelevantPairs; ++pairIt ) {
        if( pairs[pairIt].eventId1 == eventIdl
            && pairs[pairIt].eventId2 == eventIdr ) {
          setBitOnBlockArray(gt->gtPairsList.blocks,
                             pairs[pairIt].relevantPairId, 0);
          break;
        }
        else if( pairs[pairIt].eventId1 == eventIdr
                 && pairs[pairIt].eventId2 == eventIdl ) {
          setBitOnBlockArray(gt->gtPairsList.blocks,
                             pairs[pairIt].relevantPairId, 0);
          break;
        }
      }
    } else if ( settings.includeGTMissingNodes ) {
      ++gt->nGTPairs; // Count this GT line (pair)
      // And include the other event, if any of them is in the input data set
      if ( foundStrl ) {
        setBitOnBlockArray(gt->gtEventsList.blocks, eventIdl, 0);
      }
      if ( foundStrr ) {
        setBitOnBlockArray(gt->gtEventsList.blocks, eventIdr, 0);
      }
    } else if ( !settings.ignoreGTMissingNodes ) {
      // Throw error if event is in GT but not in input file
      STDERR_INFO("Line %lu of external ground truth file has an event ( %s )"
                  " that is not on the input file.\n", lineCnt,
                  foundStrl ? leftSideStr : rightSideStr );
      RETURN_ERROR;
    } else {
      // Just ignore the missing nodes of interest that are in the GT,
      //  but not in the input data set
      // Equavalent to removing this GT line (pair) from the GT file
    }
  }
  fclose(externalGtFile);

  #if PRINT_PAIR_LISTS_FOR_EXTERNAL_GROUNTTRUTH
  {
  FILE* eventsBinaryBlocks = fopen("eventsBinaryBlocksExternalGT.dbg", "a");
  fprintf(eventsBinaryBlocks,"Event ID\tSet/Clear(1/0)\tBlock content\n");
  for (uint eventIt=0; eventIt < graphInfo.nEvents; eventIt++) {
    fprintf(eventsBinaryBlocks,
            "%8u\t%14u\t0x%016jX\n",
            eventIt,
            checkBitOnBlockArray(gt->gtEventsList.blocks, eventIt, 0) ? 1:0,
            (uintmax_t)gt->gtEventsList.blocks[ eventIt/BITS_PER_BLOCK ]);
  }
  fclose(eventsBinaryBlocks);

  FILE* pairsBinaryBlocks = fopen("pairsBinaryBlocksExternalGT.dbg", "a");
  fprintf(pairsBinaryBlocks,"Relevant Pair ID\tSet/Clear(1/0)\tBlock content\n");
  for (uint pairIt=0; pairIt < graphInfo.nRelevantPairs; pairIt++) {
    fprintf(pairsBinaryBlocks,
            "%16u\t%14u\t0x%016jX\n",
            pairIt,
            checkBitOnBlockArray(gt->gtPairsList.blocks, pairIt, 0) ? 1:0,
            (uintmax_t)gt->gtPairsList.blocks[ pairIt/BITS_PER_BLOCK ]);
  }
  fclose(pairsBinaryBlocks);
  }
  #endif

  return SUCCESS;
}
