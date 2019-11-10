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


#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <stdio.h>   /* gets */
#include <stdlib.h>  /* atoi, malloc */
#include <string.h>  /* strcpy */
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <inttypes.h>
#include <gsl/gsl_rng.h>
#include <assert.h>
#include "uthash/src/uthash.h"
#include "compileTimeOptions.h"
#include "utils.h"
#include "timer.h"
#include "argParser.h"

// Static information about the graph provided by the user
// This values do not change throughout the LAP solving steps
typedef struct graphInfo {
  ulint  nBlocksAdjMatrix;
  uint  nBlocksPerEvent;

  uint maxNodeStrLenght;
  char**    eventList;

  uint  nEvents;
  uint  nActors;
  uint  nEdges;
  uint  nPairs;

  uint** originalCooc;
  ulint coocSum;
  uint nRelevantPairs;

  uint* eventDegrees;
  uint* actorDegrees;
}GRAPHINFO;
// Since it is static* and used in a good portion of the functions,
// it is currently globally declared
// *static in the sense that each variable is initialized/defined only once,
//  and therefore does not change it's value until the end of the program.
GRAPHINFO graphInfo;

// Graph structure itself, containing:
//  A binary adjacency matrix;
//  A vector of adjacency lists;
//  An edge map, linking the edge index to it's actor;
//  An edge link vector, linking to edges that are actually the same (used for non-bipartite graphs);
//  TODO: EXPLAIN SUB BLOCKS
typedef struct graph {
  // The adj. matrix was chosen to be
  //  an unidimensional vector instead of a real matrix
  //  only for memory concerns. This saves #nodes*sizeof(pointer) bytes.
  // For code understandability, and even performance,
  //  a matrix (2D) design could be a better choise.
  BLOCK*   adjMatrix;

  uint*   actorAdjLists;
  uint*   actorAccumulatedDegrees;
  uint*   actorEdgeMaps;
  uint*   edgeLinks;

  int**     subBlocksStartIndexes;
  int**     subBlocksEndIndexes;

}GRAPH;

// Structure that contain all important variables to caclculate the final result
// These temporary results refer to all possible pairs of nodes of interest
// Not only the ones which the original co-occurrence is higher than zero
typedef struct tmpresult {
  uint** lastCooc;

  uint** pValue;

  ulint** coocSum;
  ulint** coocSquareSum;
  
}TMPRESULT;

typedef struct tmpSwapHeuResult {

  uint* lastCooc;

  ulint* coocSum;

}TMPSWAPHEURESULT;

typedef struct __attribute__((packed)) pair {

  uint eventId1;
  uint eventId2;
  uint relevantPairId;
  uint pValue;
  float zScore;
  
} PAIR;

struct timespec randSorterSeed;

gsl_rng *randC;

/* **************************************** */
/* Sorting related functions */
typedef int (*compfn)(const void*, const void*);
int cmpfunc (const void * a, const void * b);
int cmpZStar (PAIR *a, PAIR *b);
PAIR qselect(PAIR *v, uint len, uint k);
/* **************************************** */

/* **************************************** */
/* Shuffle related functions */
void shuffle(unsigned *array, size_t n, gsl_rng* thisRandG);
void shufflePartial(unsigned *array, size_t n, gsl_rng* thisRandG, uint nEvents);
/* **************************************** */

/* **************************************** */
/* Edges swapping functions */
void singleSwapBipartite (GRAPH* g, uint* edgeIds);

void singleSwapGeneral (GRAPH* g, uint* edgeIds);
void singleSwapParallel (GRAPH* g, uint* edgeIds);
/* **************************************** */

/* **************************************** */
/* Curveball randomization related functions and structs */
bool curveballTradeSortedLists(GRAPH* g, uint* actorIds, gsl_rng* thisRandG);

typedef struct __attribute__((packed)) pool_hash_table {
    uint32_t listIndex;
    uint eventId;
    UT_hash_handle hh;         /* makes this structure hashable */
}POOL_HASH;
POOL_HASH* addEventToPool(POOL_HASH* poolHash, uint idx, uint eventId);
bool curveballTradeHashedLists(GRAPH* g, uint* actorIds, gsl_rng* thisRandG);
/* **************************************** */

/* **************************************** */
/* Edge pairs' co-occurrence assessing functions */
void setIndexesOfSubBlocks (GRAPH* g, uint actorId);
void computeCooc(GRAPH* g, uint** cooc, int threadId); 

void computeDirectEdgeCooc(GRAPH* g, uint actorId, uint** cooc);

uint eventPairCoocCalc (GRAPH* g, uint event1, uint event2);
/* **************************************** */

void resultList(PAIR* pairs, TMPRESULT* tmpResult,
                   uint nEvents, uint nSamples);

bool coocHalfMatricesInitialize(TMPRESULT* tmpResult, uint nEvents);


/* **************************************** */
/* Graph data helpers */
ulint perturbationMeasure(GRAPH* g, GRAPH* baseG);

void initGraphInfo(GRAPHINFO* gInfo);
bool allocGraphInfo(GRAPHINFO* gInfo);
bool copyGraphInfoValues(GRAPHINFO* gInfo, GRAPHINFO* baseGInfo);
bool copyGraphInfoArrays(GRAPHINFO* gInfo, GRAPHINFO* baseGInfo);
bool copyGraphInfo(GRAPHINFO* gInfo, GRAPHINFO* baseGInfo);
bool areEqualGraphInfos(GRAPHINFO* gInfo1, GRAPHINFO* gInfo2);
void deleteGraphInfoArrays(GRAPHINFO* gInfo);
void deleteGraphInfo(GRAPHINFO* gInfo);

void initGraph(GRAPH* g);
bool allocGraph(GRAPH* g);
bool copyGraphData(GRAPH* g, GRAPH* baseG);
bool copyGraph(GRAPH* g, GRAPH* baseG);
bool canonizeGraph(GRAPH* g);
bool areEqualGraphs(GRAPH* g1, GRAPH* g2);
void deleteGraph(GRAPH* g);

bool adjMatrixFromAdjLists(GRAPH* g);
bool adjListsFromAdjMatrix(GRAPH* g);

uint getNodeDegreeViaAdjMatrix(GRAPH* g, uint nodeId, const char* nodeType);
uint getNodeDegreeViaAdjLists(GRAPH* g, uint nodeId, const char* nodeType);

uint findLinkedEgde(GRAPH* g, uint edge);
/* **************************************** */


/* **************************************** */
/* Debug print-outs/checking functions */
void printAdjMatrix(GRAPH* g, const char* adjMatrixFileName);
void printActorAdjLists(GRAPH* g, const char* adjListFileName);
void printActorAccumulatedDegrees(GRAPH* g, const char* adjListFileName);
void printActorEdgeMap(GRAPH* g, const char* adjListFileName);
void printEdgeLinks(GRAPH* g, const char* adjListFileName);
void printEventList();

bool initOriginalNodeDegrees(GRAPH* g);
bool checkNodesDegrees(GRAPH* g);
bool checkAdjListUniqueEvent(GRAPH* g);
bool checkAdjListSort(GRAPH* g);
bool checkAdjStructs(GRAPH* g);
bool checkEdgeLinks(GRAPH* g);
bool graphTest(GRAPH* g);
/* **************************************** */

#endif
