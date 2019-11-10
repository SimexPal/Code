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


#include "../headers/algorithm.h"

/* **************************************** */
/* Sorting related functions */
int cmpfunc (const void * a, const void * b)
{
  const uint ai = *(const uint *)a;
  const uint bi = *(const uint *)b;
  if ( ai > bi ) {
    return 1;
  }
  else if ( ai < bi ) {
    return -1;
  }
  else {
    return 0;
  }
}

int cmpZStar (PAIR *a, PAIR *b)
{
  if (a->pValue > b->pValue)
    return 1;
  else if ( a->pValue < b->pValue)
    return -1;

  else {
    if ( a->zScore < b->zScore)
      return 1;
    else if (a->zScore > b->zScore)
      return -1;
    else {
      return (2*((int) gsl_rng_uniform_int (randC, 2))) - 1;
    }
  }
}

PAIR qselect(PAIR *v, uint len, uint k)
{
  // Rearrenge array @v, with length @len, in such way that
  // the @k "greatest" elements (according to cmpZStar() )
  // are positioned from index 0 to @(k - 1).
  // Adapted code from source:
  //  https://rosettacode.org/wiki/Quickselect_algorithm#C
# define SWAP(a, b) { tmp = v[a]; v[a] = v[b]; v[b] = tmp; }
  uint st, i;
  PAIR tmp;

  for (st = i = 0; i < len - 1; i++) {
    if (cmpZStar(&v[i], &v[len-1]) > 0) continue;
    SWAP(i, st);
    st++;
  }

  SWAP(len-1, st);
 
  return k == st  ?v[st]
      :st > k ? qselect(v, st, k)
        : qselect(&v[st], len - st, k - st);
}
/* **************************************** */

/* **************************************** */
/* Shuffle related functions */
void shuffle(unsigned *array, size_t n, gsl_rng* thisRandG)
{
  if (n > 1) {
    for (size_t i = 0; i < n - 1; i++) {
      size_t j = i + (size_t) gsl_rng_uniform_int (thisRandG, n-i);
      unsigned t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}

void shufflePartial(unsigned *array, size_t n, gsl_rng* thisRandG, uint nEvents)
{
  if(n > 1)
  {
    #if PRINT_SHUFFLE_STEPS
      FILE* curveballDebugFile = fopen("curveball.dbg","a");
      fprintf(curveballDebugFile,"%u Elements in the Pool\n", (uint)n);
    #endif
    for (size_t i = 0; i < nEvents; i++) {
      size_t j = i + (size_t) gsl_rng_uniform_int (thisRandG, n-i);
      unsigned t = array[j];
      array[j] = array[i];
      array[i] = t;
    
      #if PRINT_SHUFFLE_STEPS
        fprintf(curveballDebugFile, "\t Shuffle Step %u / %u: ",((uint)i)+1, nEvents);
        fprintf(curveballDebugFile, "Pos %u: %u <-> Pos %u: %u \t",(uint)i,array[j],(uint)j,array[i]);
        fprintf(curveballDebugFile, "New Pool :[");
        for(size_t k = 0; k < i; k++) {
          fprintf(curveballDebugFile," x");
        }
        fprintf(curveballDebugFile, " |%u|",array[i]);
        for(size_t k = i+1; k < n; k++) {
          if(k == j) {
            fprintf(curveballDebugFile, " |%u|",array[k]);
          }
          else {
            fprintf(curveballDebugFile, " %u",array[k]);
          }
        }
        fprintf(curveballDebugFile, "]\n");
      #endif
    }
    #if PRINT_SHUFFLE_STEPS
      fclose(curveballDebugFile);
    #endif
  }
}
/* **************************************** */


/* **************************************** */
/* Edges swapping related functions */
void singleSwapBipartite(GRAPH* g, uint* edgeIds)
{
  uint eventIds[2], actorIds[2];

  eventIds[0] = g->actorAdjLists[ edgeIds[0] ];
  eventIds[1] = g->actorAdjLists[ edgeIds[1] ];

  actorIds[0] = g->actorEdgeMaps[ edgeIds[0] ];
  actorIds[1] = g->actorEdgeMaps[ edgeIds[1] ];
  
  //Check if it is swappable
  if ( checkBitOnBlockArray(g->adjMatrix, actorIds[1], eventIds[0]*graphInfo.nBlocksPerEvent) ) {  //Check Non-Edge
    return;
  }
  else if ( checkBitOnBlockArray(g->adjMatrix, actorIds[0], eventIds[1]*graphInfo.nBlocksPerEvent) ) { //Check Non-Edge
      return;
  }
  else {
    //Swap edges in adjacency matrix
    setBitOnBlockArray(g->adjMatrix, actorIds[1], eventIds[0]*graphInfo.nBlocksPerEvent);
    setBitOnBlockArray(g->adjMatrix, actorIds[0], eventIds[1]*graphInfo.nBlocksPerEvent);

    clearBitOnBlockArray(g->adjMatrix, actorIds[0], eventIds[0]*graphInfo.nBlocksPerEvent);
    clearBitOnBlockArray(g->adjMatrix, actorIds[1], eventIds[1]*graphInfo.nBlocksPerEvent);

    //Swap edges in adjacency list
    uint tmp = g->actorAdjLists[ edgeIds[0] ];
    g->actorAdjLists[ edgeIds[0] ] = g->actorAdjLists[ edgeIds[1] ];
    g->actorAdjLists[ edgeIds[1] ] = tmp;
  }

}

void singleSwapGeneral(GRAPH* g, uint* edgeIds)
{
  uint eventIds[2], actorIds[2], redundantEdgeIds[2];

  eventIds[0] = g->actorAdjLists[ edgeIds[0] ];
  eventIds[1] = g->actorAdjLists[ edgeIds[1] ];

  actorIds[0] = g->actorEdgeMaps[ edgeIds[0] ];
  actorIds[1] = g->actorEdgeMaps[ edgeIds[1] ];

  redundantEdgeIds[0] = g->edgeLinks[ edgeIds[0] ];
  redundantEdgeIds[1] = g->edgeLinks[ edgeIds[1] ];
  
  //Check if it is swappable
  if ( checkBitOnBlockArray(g->adjMatrix, actorIds[1], eventIds[0] * graphInfo.nBlocksPerEvent) ) {
    return;
  } else if ( checkBitOnBlockArray(g->adjMatrix, actorIds[0], eventIds[1] * graphInfo.nBlocksPerEvent) ) {
      return;
  }
  else {
    setBitOnBlockArray(g->adjMatrix, actorIds[1], eventIds[0]*graphInfo.nBlocksPerEvent );
    setBitOnBlockArray(g->adjMatrix, actorIds[0], eventIds[1]*graphInfo.nBlocksPerEvent );
    setBitOnBlockArray(g->adjMatrix, eventIds[1], actorIds[0]*graphInfo.nBlocksPerEvent );
    setBitOnBlockArray(g->adjMatrix, eventIds[0], actorIds[1]*graphInfo.nBlocksPerEvent );

    clearBitOnBlockArray(g->adjMatrix, actorIds[0], eventIds[0]*graphInfo.nBlocksPerEvent );
    clearBitOnBlockArray(g->adjMatrix, actorIds[1], eventIds[1]*graphInfo.nBlocksPerEvent );
    clearBitOnBlockArray(g->adjMatrix, eventIds[0], actorIds[0]*graphInfo.nBlocksPerEvent );
    clearBitOnBlockArray(g->adjMatrix, eventIds[1], actorIds[1]*graphInfo.nBlocksPerEvent );

    uint tmp;
    //Swap edges in adjacency list (randomly selected pair and their's redundants)
    tmp = g->actorAdjLists[ edgeIds[0] ];
    g->actorAdjLists[ edgeIds[0] ] = g->actorAdjLists[ edgeIds[1] ];
    g->actorAdjLists[ edgeIds[1] ] = tmp;

    tmp = g->actorAdjLists[ redundantEdgeIds[1] ];
    g->actorAdjLists[ redundantEdgeIds[1] ] = g->actorAdjLists[ redundantEdgeIds[0] ];
    g->actorAdjLists[ redundantEdgeIds[0] ] = tmp;

    // Correct links between redundant edges
    tmp = g->edgeLinks[ edgeIds[0] ];
    g->edgeLinks[ edgeIds[0] ] = g->edgeLinks[ edgeIds[1] ];
    g->edgeLinks[ edgeIds[1] ] = tmp;

    tmp = g->edgeLinks[ redundantEdgeIds[1] ];
    g->edgeLinks[ redundantEdgeIds[1] ] = g->edgeLinks[ redundantEdgeIds[0] ];
    g->edgeLinks[ redundantEdgeIds[0] ] = tmp;

  }

}

/* **************************************** */

/* **************************************** */
/* Curveball randomization related functions */
bool curveballTradeSortedLists(GRAPH* g, uint* actorIds, gsl_rng* thisRandG)
{
  // 1. Finds the events that are unique to each actor (the swapping pool),
  //      i.e. A1\A2 U A2\A1
  //    ASSUMES: Adjacent lists A1 and A2 are sorted in ascending order
  // 2. Randomly shuffles the pool and reassign event IDs to lists
  // 3. Sort lists

  // "Rename" actor adjacency lists
  uint* A1 = &g->actorAdjLists[ g->actorAccumulatedDegrees[ actorIds[0] ] ];
  uint* A2 = &g->actorAdjLists[ g->actorAccumulatedDegrees[ actorIds[1] ] ];

  // Get lists' length
  uint lengthA1 = g->actorAccumulatedDegrees[ actorIds[0]+1 ];
  lengthA1 -= g->actorAccumulatedDegrees[ actorIds[0] ];
  uint lengthA2 = g->actorAccumulatedDegrees[ actorIds[1]+1 ];
  lengthA2 -= g->actorAccumulatedDegrees[ actorIds[1] ];

  #if PRINT_CURVEBALL_STEPS
  {
    FILE* curveballDebugFile = fopen("curveball.dbg", "a");
    fprintf(curveballDebugFile, "A1 (%u): [ ", actorIds[0]);
    for( uint i=0; i < lengthA1; ++i ) {
      fprintf(curveballDebugFile, "%u ", A1[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fprintf(curveballDebugFile, "A2 (%u): [ ", actorIds[1]);
    for( uint i=0; i < lengthA2; ++i ) {
      fprintf(curveballDebugFile, "%u ", A2[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fclose(curveballDebugFile);
  }
  bool changedPool = FALSE;
  #endif

  // A1\A2 U A2\A1 -> pool
  //  pool contains the event IDs that can end up in either A1 or A2
  uint pool[lengthA1+lengthA2];
  // Ai\Aj -> Ai elements that are in the pool
  //  inPoolAi contains the index of Ai that can be swapped
  uint inPoolA1[lengthA1];
  uint inPoolA2[lengthA2];

  // Array iteration helpers
  uint A1Idx = 0;
  uint A2Idx = 0;
  uint poolIdx = 0;
  uint inPoolA1Idx = 0;
  uint inPoolA2Idx = 0;

  // Build pool
  startTimer(&poolTimer);
  // Ordered search of different eventIds in adj. lists
  while ( A1Idx < lengthA1 && A2Idx < lengthA2 ) {
    if ( A1[A1Idx] < A2[A2Idx] ) {
      // If eventId in A1 is lower than eventId in A2, at that to the pool
      #if PRINT_CURVEBALL_STEPS
      {
        FILE* curveballDebugFile = fopen("curveball.dbg", "a");
        fprintf(curveballDebugFile,
                "Adding event %u to the pool from A1.\n", A1[A1Idx]);
        fclose(curveballDebugFile);
        changedPool = TRUE;
      }
      #endif
      pool[poolIdx++] = A1[A1Idx];
      inPoolA1[inPoolA1Idx++] = A1Idx;
      ++A1Idx;
    } else if ( A2[A2Idx] < A1[A1Idx] ) {
      // If eventId in A2 is lower than eventId in A1, at that to the pool
      #if PRINT_CURVEBALL_STEPS
      {
        FILE* curveballDebugFile = fopen("curveball.dbg", "a");
        fprintf(curveballDebugFile,
                "Adding event %u to the pool from A2.\n", A2[A2Idx]);
        fclose(curveballDebugFile);
        changedPool = TRUE;
      }
      #endif
      pool[poolIdx++] = A2[A2Idx];
      inPoolA2[inPoolA2Idx++] = A2Idx;
      ++A2Idx;
    } else {
      // If both eventIds are equal, dont add to the pool
      #if PRINT_CURVEBALL_STEPS
      {
        FILE* curveballDebugFile = fopen("curveball.dbg", "a");
        fprintf(curveballDebugFile,
                "Not adding event %u to the pool.\n", A1[A1Idx]);
        fclose(curveballDebugFile);
        changedPool = FALSE;
      }
      #endif
      ++A1Idx;
      ++A2Idx;
    }
    #if PRINT_CURVEBALL_STEPS
    {
      if ( changedPool ) {
        FILE* curveballDebugFile = fopen("curveball.dbg", "a");
        fprintf(curveballDebugFile, "Pool: [ ");
        for( uint i=0; i < poolIdx; ++i ) {
          fprintf(curveballDebugFile, "%u ", pool[i]);
        }
        fprintf(curveballDebugFile, "]\n");

        fprintf(curveballDebugFile, "inPoolA1: [ ");
        for( uint i=0; i < inPoolA1Idx; ++i ) {
          fprintf(curveballDebugFile, "%u ", inPoolA1[i]);
        }
        fprintf(curveballDebugFile, "]\n");

        fprintf(curveballDebugFile, "inPoolA2: [ ");
        for( uint i=0; i < inPoolA2Idx; ++i ) {
          fprintf(curveballDebugFile, "%u ", inPoolA2[i]);
        }
        fprintf(curveballDebugFile, "]\n");
        fprintf(curveballDebugFile, "\n");
        fclose(curveballDebugFile);
      }
      changedPool = FALSE;
    }
    #endif
  }
  // Add all remaining elements of the unfinished list to the pool
  for( ; A1Idx < lengthA1; ++A1Idx ) {
    #if PRINT_CURVEBALL_STEPS
    {
      FILE* curveballDebugFile = fopen("curveball.dbg", "a");
      fprintf(curveballDebugFile,
              "Adding event %u to the pool from A1 end.\n", A1[A1Idx]);
      fclose(curveballDebugFile);
      changedPool = TRUE;
    }
    #endif
    pool[poolIdx++] = A1[A1Idx];
    inPoolA1[inPoolA1Idx++] = A1Idx;
  }
  for( ; A2Idx < lengthA2; ++A2Idx ) {
    #if PRINT_CURVEBALL_STEPS
    {
      FILE* curveballDebugFile = fopen("curveball.dbg", "a");
      fprintf(curveballDebugFile,
              "Adding event %u to the pool from A2 end.\n", A2[A2Idx]);
      fclose(curveballDebugFile);
      changedPool = TRUE;
    }
    #endif
    pool[poolIdx++] = A2[A2Idx];
    inPoolA2[inPoolA2Idx++] = A2Idx;
  }
  accElapsedTime(&poolTimer);

  uint poolSize = poolIdx;
  uint nA1EventsInPool = inPoolA1Idx;
  uint nA2EventsInPool = inPoolA2Idx;
  // Sanity check
  if ( poolSize != nA1EventsInPool + nA2EventsInPool ) {
    STDERR_INFO("Total size of the swapping pool (%u) should be equal to "
                "the sum of the number of elements in the two actor's "
                "adjacency lists (%u + %u = %u).", poolSize, nA1EventsInPool,
                nA2EventsInPool, nA1EventsInPool + nA2EventsInPool);
    RETURN_ERROR;
  }

  #if PRINT_CURVEBALL_STEPS
  {
    FILE* curveballDebugFile = fopen("curveball.dbg", "a");
    fprintf(curveballDebugFile, "Final Pool: [ ");
    for( uint i=0; i < poolSize; ++i ) {
      fprintf(curveballDebugFile, "%u ", pool[i]);
    }
    fprintf(curveballDebugFile, "]\n");

    fprintf(curveballDebugFile, "inPoolA1: [ ");
    for( uint i=0; i < nA1EventsInPool; ++i ) {
      fprintf(curveballDebugFile, "%u ", inPoolA1[i]);
    }
    fprintf(curveballDebugFile, "]\n");

    fprintf(curveballDebugFile, "inPoolA2: [ ");
    for( uint i=0; i < nA2EventsInPool; ++i ) {
      fprintf(curveballDebugFile, "%u ", inPoolA2[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fclose(curveballDebugFile);
  }
  #endif

  // Shuffling the pool results in the same as randomly selecting each event ID
  startTimer(&shuffleTimer);
//  shuffle(pool, poolSize, thisRandG);

  //LessOrEqual Events of A1 are in the Pool
  if(nA1EventsInPool <= nA2EventsInPool){
    #if PRINT_SHUFFLE_STEPS
      FILE* curveballDebugFile = fopen("curveball.dbg","a");
      fprintf(curveballDebugFile,"Start shuffling with nA1 <= nA2\n");
      fclose(curveballDebugFile);
    #endif
    shufflePartial(pool, poolSize, thisRandG, nA1EventsInPool);
  }
  //Less Events of A2 are in the Pool
  else{
    #if PRINT_SHUFFLE_STEPS
      FILE* curveballDebugFile = fopen("curveball.dbg","a");
      fprintf(curveballDebugFile,"Start shuffling with nA2 < nA1\n");
      fclose(curveballDebugFile);
    #endif
    shufflePartial(pool, poolSize, thisRandG, nA2EventsInPool);
  }
  accElapsedTime(&shuffleTimer);

  #if PRINT_CURVEBALL_STEPS
  {
    FILE* curveballDebugFile = fopen("curveball.dbg", "a");
    fprintf(curveballDebugFile, "Shuffled Pool: [ ");
    for( uint i=0; i < poolSize; ++i ) {
      fprintf(curveballDebugFile, "%u ", pool[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fclose(curveballDebugFile);
  }
  #endif

  // "Refill" adjacency lists
  startTimer(&refillTimer);
  inPoolA1Idx = 0;
  for( uint poolIdx = 0; poolIdx < nA1EventsInPool; ++poolIdx ) {
    A1[ inPoolA1[inPoolA1Idx++] ] = pool[poolIdx];
  }
  inPoolA2Idx = 0;
  for( uint poolIdx = nA1EventsInPool; poolIdx < poolSize; ++poolIdx ) {
    A2[ inPoolA2[inPoolA2Idx++] ] = pool[poolIdx];
  }
  accElapsedTime(&refillTimer);

  startTimer(&sortListsTimer);
  qsort(A1, lengthA1, sizeof(uint), cmpfunc);
  qsort(A2, lengthA2, sizeof(uint), cmpfunc);
  accElapsedTime(&sortListsTimer);

  #if PRINT_CURVEBALL_STEPS
  {
    FILE* curveballDebugFile = fopen("curveball.dbg", "a");
    fprintf(curveballDebugFile, "new A1 (%u): [ ", actorIds[0]);
    for( uint i=0; i < lengthA1; ++i ) {
      fprintf(curveballDebugFile, "%u ", A1[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fprintf(curveballDebugFile, "new A2 (%u): [ ", actorIds[1]);
    for( uint i=0; i < lengthA2; ++i ) {
      fprintf(curveballDebugFile, "%u ", A2[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fclose(curveballDebugFile);
  }
  #endif

  return SUCCESS;

}

POOL_HASH* addEventToPool(POOL_HASH* poolHash, uint idx, uint eventId) {
  POOL_HASH* newItem; // Create and allocate a new item for the @pool table
  newItem = (POOL_HASH*)calloc(1, sizeof(POOL_HASH));
  if ( newItem != NULL ) {
    newItem->eventId = eventId;
    newItem->listIndex = idx;
    HASH_ADD_INT(poolHash, eventId, newItem);
  }
  else {
    STDERR_INFO("Unable to allocate new item to hash table.\n"
                "Intended key: %u, and value: %u.", idx, eventId);
    RETURN_ERROR_V(NULL);
  }
  return poolHash; // Return new pointer to the hash structure
}

POOL_HASH* deleteEventFromPool(POOL_HASH* poolHash, POOL_HASH* eventItem) {
  HASH_DEL(poolHash, eventItem);
  free(eventItem);
  return poolHash; // Return new pointer to the hash structure
}

bool curveballTradeHashedLists(GRAPH* g, uint* actorIds, gsl_rng* thisRandG)
{
  // 1. Finds the events that are unique to each actor (the swapping pool),
  //      i.e. A1\A2 U A2\A1
  // 2. Randomly shuffles the pool and reassign event IDs to lists
  // 3. Sort lists

  // "Rename" actor adjacency lists
  uint* A1 = &g->actorAdjLists[ g->actorAccumulatedDegrees[ actorIds[0] ] ];
  uint* A2 = &g->actorAdjLists[ g->actorAccumulatedDegrees[ actorIds[1] ] ];

  // Get lists' length
  uint lengthA1 = g->actorAccumulatedDegrees[ actorIds[0]+1 ];
  lengthA1 -= g->actorAccumulatedDegrees[ actorIds[0] ];
  uint lengthA2 = g->actorAccumulatedDegrees[ actorIds[1]+1 ];
  lengthA2 -= g->actorAccumulatedDegrees[ actorIds[1] ];

  #if PRINT_CURVEBALL_STEPS
  {
    FILE* curveballDebugFile = fopen("curveball.dbg", "a");
    fprintf(curveballDebugFile, "A1 (%u): [ ", actorIds[0]);
    for( uint i=0; i < lengthA1; ++i ) {
      fprintf(curveballDebugFile, "%u ", A1[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fprintf(curveballDebugFile, "A2 (%u): [ ", actorIds[1]);
    for( uint i=0; i < lengthA2; ++i ) {
      fprintf(curveballDebugFile, "%u ", A2[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fclose(curveballDebugFile);
  }
  #endif

  // Build hashed pool
  startTimer(&hashedPoolTimer);
  POOL_HASH* hashedPool = NULL;
  for (uint A1Idx = 0; A1Idx < lengthA1; ++A1Idx) {
    hashedPool = addEventToPool(hashedPool, A1Idx, A1[A1Idx]);
    if ( hashedPool == NULL ) { FORWARD_ERROR; }
    #if PRINT_CURVEBALL_STEPS
    {
      FILE* curveballDebugFile = fopen("curveball.dbg", "a");
      fprintf(curveballDebugFile, "Hashed pool: [ ");
      POOL_HASH *currentItem, *helperItem;
      HASH_ITER(hh, hashedPool, currentItem, helperItem) {
        fprintf(curveballDebugFile, "%u ", currentItem->eventId);
      }
      fprintf(curveballDebugFile, "]\n");
      fclose(curveballDebugFile);
    }
    #endif
  }
  uint nA1EventsInPool = lengthA1; // Starts with all A1 elements
  uint nA2EventsInPool = 0; // Starts with no A2 elements
  for (uint A2Idx = 0; A2Idx < lengthA2; ++A2Idx) {
    POOL_HASH* foundEvent = NULL;
    uint eventId = A2[A2Idx];
    HASH_FIND_INT(hashedPool, &eventId, foundEvent);
    if ( foundEvent ) { // Is in A1 and A2
      hashedPool = deleteEventFromPool(hashedPool, foundEvent);
      --nA1EventsInPool;
    } else { // Is only in A2
      // Use @lengthA1 as safe offset for index in the pool
      hashedPool = addEventToPool(hashedPool, A2Idx + lengthA1, A2[A2Idx]);
      if ( hashedPool == NULL ) { FORWARD_ERROR; }
      ++nA2EventsInPool;
    }
    #if PRINT_CURVEBALL_STEPS
    {
      FILE* curveballDebugFile = fopen("curveball.dbg", "a");
      fprintf(curveballDebugFile, "Hashed pool: [ ");
      POOL_HASH *currentItem, *helperItem;
      HASH_ITER(hh, hashedPool, currentItem, helperItem) {
        fprintf(curveballDebugFile, "%u ", currentItem->eventId);
      }
      fprintf(curveballDebugFile, "]\n");
      fclose(curveballDebugFile);
    }
    #endif
  }
  uint poolSize = HASH_COUNT(hashedPool);
  // Sanity check
  if ( poolSize != nA1EventsInPool + nA2EventsInPool ) {
    STDERR_INFO("Total size of the swapping pool (%u) should be equal to "
                "the sum of the number of elements in the two actor's "
                "adjacency lists (%u + %u != %u).", poolSize, nA1EventsInPool,
                nA2EventsInPool, poolSize);
    RETURN_ERROR;
  }
  accElapsedTime(&hashedPoolTimer);

  // Build vectorized pool
  startTimer(&poolTimer);
  // A1\A2 U A2\A1 -> pool
  //  pool contains the event IDs that can end up in either A1 or A2
  uint pool[poolSize];
  uint poolIdx = 0;
  // Ai\Aj -> Ai elements that are in the pool
  //  inPoolAi contains the index of Ai that can be swapped
  uint inPoolA1[nA1EventsInPool];
  uint inPoolA1Idx = 0;
  uint inPoolA2[nA2EventsInPool];
  uint inPoolA2Idx = 0;

  // Iterates through the hashed pool to fill in its vectorized structures
  POOL_HASH *currentItem, *helperItem;
  HASH_ITER(hh, hashedPool, currentItem, helperItem) {
    pool[poolIdx] = currentItem->eventId;
    ++poolIdx;
    // Is it from A1 or A2?
    if ( currentItem->listIndex < lengthA1 ) { // From A1
      inPoolA1[inPoolA1Idx] = currentItem->listIndex;
      ++inPoolA1Idx;
    } else { // From A2
      inPoolA2[inPoolA2Idx] = currentItem->listIndex - lengthA1; // Remove offset
      ++inPoolA2Idx;
    }
    // Delete item from hashed pool
    hashedPool = deleteEventFromPool(hashedPool, currentItem);

    #if PRINT_CURVEBALL_STEPS
    {
      FILE* curveballDebugFile = fopen("curveball.dbg", "a");
      fprintf(curveballDebugFile, "Pool: [ ");
      for( uint i=0; i < poolIdx; ++i ) {
        fprintf(curveballDebugFile, "%u ", pool[i]);
      }
      fprintf(curveballDebugFile, "]\n");

      fprintf(curveballDebugFile, "inPoolA1: [ ");
      for( uint i=0; i < inPoolA1Idx; ++i ) {
        fprintf(curveballDebugFile, "%u ", inPoolA1[i]);
      }
      fprintf(curveballDebugFile, "]\n");

      fprintf(curveballDebugFile, "inPoolA2: [ ");
      for( uint i=0; i < inPoolA2Idx; ++i ) {
        fprintf(curveballDebugFile, "%u ", inPoolA2[i]);
      }
      fprintf(curveballDebugFile, "]\n");
      fclose(curveballDebugFile);
    }
    #endif
  }
  accElapsedTime(&poolTimer);

  #if PRINT_CURVEBALL_STEPS
  {
    FILE* curveballDebugFile = fopen("curveball.dbg", "a");
    fprintf(curveballDebugFile, "Final Pool: [ ");
    for( uint i=0; i < poolSize; ++i ) {
      fprintf(curveballDebugFile, "%u ", pool[i]);
    }
    fprintf(curveballDebugFile, "]\n");

    fprintf(curveballDebugFile, "inPoolA1: [ ");
    for( uint i=0; i < nA1EventsInPool; ++i ) {
      fprintf(curveballDebugFile, "%u ", inPoolA1[i]);
    }
    fprintf(curveballDebugFile, "]\n");

    fprintf(curveballDebugFile, "inPoolA2: [ ");
    for( uint i=0; i < nA2EventsInPool; ++i ) {
      fprintf(curveballDebugFile, "%u ", inPoolA2[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fclose(curveballDebugFile);
  }
  #endif

  // Shuffling the pool results in the same as randomly selecting each event ID
  startTimer(&shuffleTimer);
  shuffle(pool, poolSize, thisRandG);
  accElapsedTime(&shuffleTimer);

  #if PRINT_CURVEBALL_STEPS
  {
    FILE* curveballDebugFile = fopen("curveball.dbg", "a");
    fprintf(curveballDebugFile, "Shuffled Pool: [ ");
    for( uint i=0; i < poolSize; ++i ) {
      fprintf(curveballDebugFile, "%u ", pool[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fclose(curveballDebugFile);
  }
  #endif

  // "Refill" adjacency lists
  startTimer(&refillTimer);
  inPoolA1Idx = 0;
  for( uint poolIdx = 0; poolIdx < nA1EventsInPool; ++poolIdx ) {
    A1[ inPoolA1[inPoolA1Idx++] ] = pool[poolIdx];
  }
  inPoolA2Idx = 0;
  for( uint poolIdx = nA1EventsInPool; poolIdx < poolSize; ++poolIdx ) {
    A2[ inPoolA2[inPoolA2Idx++] ] = pool[poolIdx];
  }
  accElapsedTime(&refillTimer);

  #if PRINT_CURVEBALL_STEPS
  {
    FILE* curveballDebugFile = fopen("curveball.dbg", "a");
    fprintf(curveballDebugFile, "new A1 (%u): [ ", actorIds[0]);
    for( uint i=0; i < lengthA1; ++i ) {
      fprintf(curveballDebugFile, "%u ", A1[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fprintf(curveballDebugFile, "new A2 (%u): [ ", actorIds[1]);
    for( uint i=0; i < lengthA2; ++i ) {
      fprintf(curveballDebugFile, "%u ", A2[i]);
    }
    fprintf(curveballDebugFile, "]\n");
    fclose(curveballDebugFile);
  }
  #endif

  return SUCCESS;

}

/* **************************************** */

/* **************************************** */
/* Edge pairs' co-occurrence assessing related functions */
void setIndexesOfSubBlocks (GRAPH* g, uint actorId)
{
  // Sort actor adjacency list
  qsort(&(g->actorAdjLists[ g->actorAccumulatedDegrees[actorId] ]),
        g->actorAccumulatedDegrees[actorId+1] - g->actorAccumulatedDegrees[actorId],
        sizeof(uint),
        cmpfunc);

  // Define maximun event ID value for each sub-block
  // This operation could be done only once
  //  However, for the sake of clarity it is done for each co-occurrence computation
  //  knowing that the overhead of doing so is irrelevant.
  uint subBlockUpperLimit[NUMBER_OF_SUBBLOCKS];
  for (uint subBlockIt = 0; subBlockIt < NUMBER_OF_SUBBLOCKS; subBlockIt++) {
    subBlockUpperLimit[subBlockIt] = (subBlockIt + 1) * graphInfo.nEvents / NUMBER_OF_SUBBLOCKS;
  }

  #if PRINT_SUB_BLOCKS_INDEXING_INFO
    char subBlockIndexingFileName[30 + MAX_INT_STR_SIZE];
    strcpy(subBlockIndexingFileName, "subBlockIndexing_");
    char actorId_str[MAX_INT_STR_SIZE];
    sprintf(actorId_str, "%u", actorId);
    strcat(subBlockIndexingFileName, actorId_str);
    strcat(subBlockIndexingFileName, ".dbg");
    FILE* subBlockIndexingFile = fopen(subBlockIndexingFileName, "a");
    fprintf(subBlockIndexingFile, "\nSub-block indexing for actor %u:\n", actorId);
    fprintf(subBlockIndexingFile, "Total #edges: %u  Actor list length: %u "
                                  "(from %u to %u)\n",
            graphInfo.nEdges,
            g->actorAccumulatedDegrees[actorId+1] - g->actorAccumulatedDegrees[actorId],
            g->actorAccumulatedDegrees[actorId], g->actorAccumulatedDegrees[actorId+1]);
    fprintf(subBlockIndexingFile, "Sub-block   Edge Start Event Start   Edge End Event End   Length\n");
    fclose(subBlockIndexingFile);
  #endif

  // Identify the beginning and end of each sub-block
  //  Iterate through all edges of the given actor
  //  checking whether the ID of the event linked by the edge is greater than the sub-block limit
  //  If so, define the end of the sub-block as the last iterated edge
  //    and the start of the next sub-block as the current edge.
  uint edgeIt;
  uint subBlockIt=0;
  // Start of first sub-block
  g->subBlocksStartIndexes[actorId][0] = g->actorAccumulatedDegrees[actorId];
  #if PRINT_SUB_BLOCKS_INDEXING_INFO
  {
    FILE* subBlockIndexingFile = fopen(subBlockIndexingFileName, "a");
    fprintf(subBlockIndexingFile, "%9u   %10u %11u   ",
            subBlockIt, g->subBlocksStartIndexes[actorId][0], g->actorAdjLists[ g->subBlocksStartIndexes[actorId][0] ]);
    fclose(subBlockIndexingFile);
  }
  #endif
  for ( edgeIt = g->actorAccumulatedDegrees[actorId]; edgeIt < g->actorAccumulatedDegrees[actorId+1]; edgeIt++ ) {
    if ( g->actorAdjLists[edgeIt] >= subBlockUpperLimit[subBlockIt] ) {
      // End of current sub-block
      g->subBlocksEndIndexes[actorId][subBlockIt] = edgeIt;
      // Start of next sub-block
      g->subBlocksStartIndexes[actorId][subBlockIt+1] = edgeIt;
      #if PRINT_SUB_BLOCKS_INDEXING_INFO
      {
        FILE* subBlockIndexingFile = fopen(subBlockIndexingFileName, "a");
        fprintf(subBlockIndexingFile, "%8u %9u   %6u\n",
                g->subBlocksEndIndexes[actorId][subBlockIt], g->actorAdjLists[ g->subBlocksEndIndexes[actorId][subBlockIt] ],
                g->subBlocksEndIndexes[actorId][subBlockIt] - g->subBlocksStartIndexes[actorId][subBlockIt]);
        fprintf(subBlockIndexingFile, "%9u   %10u %11u   ",
                subBlockIt+1, g->subBlocksStartIndexes[actorId][subBlockIt+1], g->actorAdjLists[ g->subBlocksStartIndexes[actorId][subBlockIt+1] ]);
        fclose(subBlockIndexingFile);
      }
      #endif
      // Search for the end of next sub-block
      subBlockIt++;
      // And reevaluate last edge - otherwise would assume next SB to contain it
      edgeIt--;
    }
  }
  // End of current sub-block
  g->subBlocksEndIndexes[actorId][subBlockIt] = edgeIt;
  #if PRINT_SUB_BLOCKS_INDEXING_INFO
  {
    FILE* subBlockIndexingFile = fopen(subBlockIndexingFileName, "a");
    fprintf(subBlockIndexingFile, "%8u ",
            g->subBlocksEndIndexes[actorId][subBlockIt]);
    if ( edgeIt < graphInfo.nEdges ) {
      fprintf(subBlockIndexingFile, "%9u   ",
              g->actorAdjLists[ g->subBlocksEndIndexes[actorId][subBlockIt] ]);
    } else {
      fprintf(subBlockIndexingFile, "      end   ");
    }
    fprintf(subBlockIndexingFile, "%6u\n",
            g->subBlocksEndIndexes[actorId][subBlockIt]
            - g->subBlocksStartIndexes[actorId][subBlockIt]);
    fclose(subBlockIndexingFile);
  }
  #endif

  // Important: make sure all sub-block indexes are updated for each graph
  // If the next sub-block iterator is not greater than NUMBER_OF_SUBBLOCKS at this point,
  //  this means that the actor list does not contain any edges pertaining to the last sub-blocks
  for (subBlockIt = subBlockIt + 1; subBlockIt < NUMBER_OF_SUBBLOCKS; subBlockIt++) {
    // Start of next sub-block
    g->subBlocksStartIndexes[actorId][subBlockIt] = edgeIt;
    // End of current sub-block
    g->subBlocksEndIndexes[actorId][subBlockIt] = edgeIt;
    #if PRINT_SUB_BLOCKS_INDEXING_INFO
    {
      FILE* subBlockIndexingFile = fopen(subBlockIndexingFileName, "a");
      fprintf(subBlockIndexingFile, "%9u   %10u %11u   ",
              subBlockIt, g->subBlocksStartIndexes[actorId][subBlockIt], g->actorAdjLists[ g->subBlocksStartIndexes[actorId][subBlockIt] ]);
      fprintf(subBlockIndexingFile, "%8u ",
              g->subBlocksEndIndexes[actorId][subBlockIt]);
      if ( edgeIt < graphInfo.nEdges ) {
        fprintf(subBlockIndexingFile, "%9u   ",
                g->actorAdjLists[ g->subBlocksEndIndexes[actorId][subBlockIt] ]);
      } else {
        fprintf(subBlockIndexingFile, "      end   ");
      }
      fprintf(subBlockIndexingFile, "%6u\n",
              g->subBlocksEndIndexes[actorId][subBlockIt]
              - g->subBlocksStartIndexes[actorId][subBlockIt]);
      fclose(subBlockIndexingFile);
    }
    #endif
  }

}

void computeCooc(GRAPH* g, uint** cooc, int threadId) 
{
  int i, j;

  #if PRINT_THREADS_COMPUTING_COOCCURRENCE
  char threadCoocFileName[40];
  char threadIdStr[MAX_INT_STR_SIZE];
  strcpy(threadCoocFileName, "thread");
  sprintf(threadIdStr, "%d", threadId);
  strcat(threadCoocFileName, threadIdStr);
  strcat(threadCoocFileName, "CoocFile.dbg");
  FILE* threadCoocFile = fopen(threadCoocFileName, "a");
  fprintf(threadCoocFile, "BEGIN GRAPH\n");
  #endif


  switch (threadId) {
    case 0 : //subBlock 0 with subBlock 0
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][0]; i < g->subBlocksEndIndexes[actorId][0] - 1; i++) {
          for (j = i+1; j < g->subBlocksEndIndexes[actorId][0]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 1 : //subBlock 1 with subBlock 1
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][1]; i < g->subBlocksEndIndexes[actorId][1] - 1; i++) {
          for (j = i+1; j < g->subBlocksEndIndexes[actorId][1]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 2 : //subBlock 2 with subBlock 2
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][2]; i < g->subBlocksEndIndexes[actorId][2] - 1; i++) {
          for (j = i+1; j < g->subBlocksEndIndexes[actorId][2]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 3 : //subBlock 3 with subBlock 3
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][3]; i < g->subBlocksEndIndexes[actorId][3] - 1; i++) {
          for (j = i+1; j < g->subBlocksEndIndexes[actorId][3]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 4 : //subBlock 4 with subBlock 4
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][4]; i < g->subBlocksEndIndexes[actorId][4] - 1; i++) {
          for (j = i+1; j < g->subBlocksEndIndexes[actorId][4]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 5 :  //subBlock 0 with subBlock 1
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][0]; i < g->subBlocksEndIndexes[actorId][0]; i++) {
          for (j = g->subBlocksStartIndexes[actorId][1]; j < g->subBlocksEndIndexes[actorId][1]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 6 :  //subBlock 0 with subBlock 2
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][0]; i < g->subBlocksEndIndexes[actorId][0]; i++) {
          for (j = g->subBlocksStartIndexes[actorId][2]; j < g->subBlocksEndIndexes[actorId][2]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 7 :  //subBlock 0 with subBlock 3
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][0]; i < g->subBlocksEndIndexes[actorId][0]; i++) {
          for (j = g->subBlocksStartIndexes[actorId][3]; j < g->subBlocksEndIndexes[actorId][3]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 8 :  //subBlock 0 with subBlock 4
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][0]; i < g->subBlocksEndIndexes[actorId][0]; i++) {
          for (j = g->subBlocksStartIndexes[actorId][4]; j < g->subBlocksEndIndexes[actorId][4]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 9 :  //subBlock 1 with subBlock 2
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][1]; i < g->subBlocksEndIndexes[actorId][1]; i++) {
          for (j = g->subBlocksStartIndexes[actorId][2]; j < g->subBlocksEndIndexes[actorId][2]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 10 :  //subBlock 1 with subBlock 3
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][1]; i < g->subBlocksEndIndexes[actorId][1]; i++) {
          for (j = g->subBlocksStartIndexes[actorId][3]; j < g->subBlocksEndIndexes[actorId][3]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 11 :  //subBlock 1 with subBlock 4
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][1]; i < g->subBlocksEndIndexes[actorId][1]; i++) {
          for (j = g->subBlocksStartIndexes[actorId][4]; j < g->subBlocksEndIndexes[actorId][4]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 12 :  //subBlock 2 with subBlock 3
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][2]; i < g->subBlocksEndIndexes[actorId][2]; i++) {
          for (j = g->subBlocksStartIndexes[actorId][3]; j < g->subBlocksEndIndexes[actorId][3]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 13 :  //subBlock 2 with subBlock 4
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][2]; i < g->subBlocksEndIndexes[actorId][2]; i++) {
          for (j = g->subBlocksStartIndexes[actorId][4]; j < g->subBlocksEndIndexes[actorId][4]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

    case 14 :  //subBlock 3 with subBlock 4
      for (uint actorId=0; actorId < graphInfo.nActors; actorId++) {
        for (i = g->subBlocksStartIndexes[actorId][3]; i < g->subBlocksEndIndexes[actorId][3]; i++) {
          for (j = g->subBlocksStartIndexes[actorId][4]; j < g->subBlocksEndIndexes[actorId][4]; j++ ) {
            #if PRINT_THREADS_COMPUTING_COOCCURRENCE
            fprintf(threadCoocFile, "Thread %d! Actor %d: Indexes: ( %d %d ) Edges Ids: ( %d %d )\n",
                            threadId, actorId,
                            g->actorAdjLists[i], g->actorAdjLists[j]-g->actorAdjLists[i]-1,
                            i, j);
            #endif
            cooc[g->actorAdjLists[i]][g->actorAdjLists[j]-g->actorAdjLists[i]-1]++;
          }
        }
      }
      break;

  } // End switch

  #if PRINT_THREADS_COMPUTING_COOCCURRENCE
  fprintf(threadCoocFile, "END GRAPH\n\n");
  fclose(threadCoocFile);
  #endif

}

void computeDirectEdgeCooc(GRAPH* g, uint actorId, uint** cooc)
{
  uint actorListStart = g->actorAccumulatedDegrees[actorId];
  uint actorListEnd = g->actorAccumulatedDegrees[actorId+1];
  uint actorListLength = actorListEnd - actorListStart;
  uint* actorsList = &g->actorAdjLists[actorListStart];
  for (uint edgeIt = 0; edgeIt < actorListLength; edgeIt++ ) {
    if (actorId > actorsList[edgeIt]) {
      cooc[ actorsList[edgeIt] ][actorId - actorsList[edgeIt] - 1] +=
          settings.directEdgeCoocValue;
    }
  }
}

uint eventPairCoocCalc (GRAPH* g, uint event1, uint event2)
{
  // Single pair co-occurrence calculation
  // Inputs: graph (adj. matrix) and ID of both nodes
  // Output: co-occurence between the two nodes

  // TODO: Separate into two functions, one for bipartite and one for general
  // Important for non-bipartite graphs:
  //  in the way the adj. matrix is built,
  //  the edge between the two nodes of the pair
  //  counts as two co-occurences.

  // Iterates through blocks of adjacency matrix
  //  performing a bitwise AND operation of the current block from each event row
  uint cooc = 0;
  for(uint blockIt=0; blockIt < graphInfo.nBlocksPerEvent; blockIt++) {
    // The number of set bits ("1's") is the co-occurence between the two nodes
    cooc +=
        popCount(   g->adjMatrix[event1 * graphInfo.nBlocksPerEvent+blockIt]
                  & g->adjMatrix[event2 * graphInfo.nBlocksPerEvent+blockIt] );
  }

  return cooc;
}
/* **************************************** */

void resultList(PAIR* pairs, TMPRESULT* tmpResult,
                uint nEvents, uint nSamples)
{
  uint pairIt=0;
  for (uint row=0; row<(nEvents-1); row++) {
    for (uint col=0; col<(nEvents-1-row); col++) {
      if ( graphInfo.originalCooc[row][col] >= settings.minRelevantCooc ) {
        pairs[pairIt].eventId1 = row;
        pairs[pairIt].eventId2 = row + col + 1;
        pairs[pairIt].relevantPairId = pairIt;
        pairs[pairIt].pValue = tmpResult->pValue[row][col];
        pairs[pairIt].zScore = zScore_uint(graphInfo.originalCooc[row][col],
                                           tmpResult->coocSum[row][col],
                                           tmpResult->coocSquareSum[row][col],
                                           nSamples);
        pairIt++;
      }
    }
  }
}

bool coocHalfMatricesInitialize(TMPRESULT* tmpResult, uint nEvents)
{
  halfMatrixCalloc(tmpResult->pValue, nEvents-1);
  if ( tmpResult->pValue == NULL ) { MEM_ERROR; }

  halfMatrixCalloc(tmpResult->coocSum, nEvents-1);
  if ( tmpResult->coocSum == NULL ) { MEM_ERROR; }

  halfMatrixCalloc(tmpResult->coocSquareSum, nEvents-1);
  if ( tmpResult->coocSquareSum == NULL ) { MEM_ERROR; }

  // TODO: could maybe be done in a clearer way
  tmpResult->lastCooc = (uint**) calloc ((nEvents-1), sizeof(uint*));
  if ( tmpResult->lastCooc == NULL ) { MEM_ERROR; }

  return SUCCESS;
}

/* **************************************** */
/* Graph data helpers */
ulint perturbationMeasure(GRAPH* g, GRAPH* baseG) {
  ulint diffEdges = 0;
  if ( settings.isBipartiteGraph ) {
    if ( settings.runCurveball ) {
      // Using adj. lists -- Much slower than using adj. matrix
      for ( uint actorIt=0; actorIt < graphInfo.nActors; ++actorIt ) {
        uint* baseList = baseG->actorAdjLists;
        uint baseListIdx = baseG->actorAccumulatedDegrees[actorIt];
        uint baseListEnd = baseG->actorAccumulatedDegrees[actorIt+1];
        uint baseListLength = baseListEnd - baseListIdx;
        qsort( &baseList[baseListIdx], baseListLength, sizeof(uint), cmpfunc);

        uint* modList = g->actorAdjLists;
        uint modListIdx = g->actorAccumulatedDegrees[actorIt];
        uint modListEnd = g->actorAccumulatedDegrees[actorIt+1];
        uint modListLength = modListEnd - modListIdx;
        qsort( &modList[modListIdx], modListLength, sizeof(uint), cmpfunc);

        while ( baseListIdx < baseListEnd && modListIdx < modListEnd ) {
          if ( baseList[baseListIdx] == modList[modListIdx] ) {
            // Case both lists have same event
            ++baseListIdx;
            ++modListIdx;
          }
          else {
            // Case events are not the equal
              ++diffEdges;
              if ( baseList[baseListIdx] > modList[modListIdx] ) {
                ++modListIdx;
              } else {
                ++baseListIdx;
              }
          }
        }
        if ( baseListIdx == baseListEnd ) {
          diffEdges += modListEnd - modListIdx;
        } else {
          diffEdges += baseListEnd - baseListIdx;
        }
      }
    } else {
      // Using adj. matrix - XOR everyblock and count 1's
      for ( ulint blockIt=0; blockIt < graphInfo.nBlocksAdjMatrix; ++blockIt) {
        BLOCK xoredBlock =
            g->adjMatrix[blockIt] ^ baseG->adjMatrix[blockIt];
        diffEdges += popCount( xoredBlock );
      }
    }

//    if ( diffEdgesAdjMatrix != diffEdges ) {
//      STDERR_INFO("diffEdgesAdjMatrix: %lu diffEdgesAdjLists: %lu\n",
//                  diffEdgesAdjMatrix, diffEdges);
//      exit(-1);
//    }

  } else {
    STDERR_INFO("Perturbation measure for non-bipartite graph is not yet"
                " implemented!");
    RETURN_ERROR_V(-1);
  }

  return diffEdges;
}

void initGraphInfo(GRAPHINFO* gInfo) {
  // Memset makes sure that even padding bytes of the structure are zeroed
  memset(gInfo, 0, sizeof *gInfo);
}

bool allocGraphInfo(GRAPHINFO* gInfo) {

  // Free graphInfo allocated memory blocks, if existing
  deleteGraphInfoArrays(gInfo);

  // Event names list
  fullMatrixCalloc(gInfo->eventList, gInfo->nEvents, gInfo->maxNodeStrLenght);
  if ( gInfo->eventList == NULL ) { MEM_ERROR; }

  // Original co-occurrence half matrix
  halfMatrixCalloc(gInfo->originalCooc, gInfo->nEvents-1);
  if ( gInfo->originalCooc == NULL ) { MEM_ERROR; }

  // Actors' degree sequence
  arrayCalloc(gInfo->actorDegrees, gInfo->nActors);
  if ( gInfo->actorDegrees == NULL ) { MEM_ERROR; }

  // Events' degree sequence
  arrayCalloc(gInfo->eventDegrees, gInfo->nEvents);
  if ( gInfo->eventDegrees == NULL ) { MEM_ERROR; }

  return SUCCESS;
}

bool copyGraphInfoValues(GRAPHINFO* gInfo, GRAPHINFO* baseGInfo) {
  // Copy values of each (non-pointer) member of the GRAPHINFO structure
  gInfo->nBlocksAdjMatrix = baseGInfo->nBlocksAdjMatrix;
  gInfo->nBlocksPerEvent = baseGInfo->nBlocksPerEvent;

  gInfo->maxNodeStrLenght = baseGInfo->maxNodeStrLenght;

  gInfo->nEvents = baseGInfo->nEvents;
  gInfo->nActors = baseGInfo->nActors;
  gInfo->nEdges = baseGInfo->nEdges;
  gInfo->nPairs = baseGInfo->nPairs;

  gInfo->coocSum = baseGInfo->coocSum;
  gInfo->nRelevantPairs = baseGInfo->nRelevantPairs;

  return SUCCESS;
}

bool copyGraphInfoArrays(GRAPHINFO* gInfo, GRAPHINFO* baseGInfo) {
  // Copy data from arrays

  // Event names list
  if ( baseGInfo->eventList != NULL ) {
    for ( uint eventIt = 0; eventIt < gInfo->nEvents; ++eventIt ) {
      strcpy( gInfo->eventList[eventIt], baseGInfo->eventList[eventIt] );
    }
  }

  // Original co-oc
  if ( baseGInfo->originalCooc != NULL ) {
    for ( uint row = 0; row < gInfo->nEvents - 1; ++row ) {
      for ( uint col = 0; col < gInfo->nEvents - 1 - row; ++col ) {
        gInfo->originalCooc[row][col] = baseGInfo->originalCooc[row][col];
      }
    }
  }

  // Events' degree sequence
  if ( baseGInfo->eventDegrees != NULL ) {
    for ( uint eventIt = 0; eventIt < gInfo->nEvents; ++eventIt ) {
      gInfo->eventDegrees[eventIt] = baseGInfo->eventDegrees[eventIt];
    }
  }

  // Actors' degree sequence
  if ( baseGInfo->actorDegrees != NULL ) {
    for ( uint actorIt = 0; actorIt < gInfo->nActors; ++actorIt ) {
      gInfo->actorDegrees[actorIt] = baseGInfo->actorDegrees[actorIt];
    }
  }

  return SUCCESS;
}

bool copyGraphInfo(GRAPHINFO* gInfo, GRAPHINFO* baseGInfo) {

  // Copy graph infomation single values
  if ( copyGraphInfoValues(gInfo, baseGInfo) == FAILURE ) { FORWARD_ERROR; }

  // (Re-)allocate memory for graph information
  if( allocGraphInfo(gInfo) == FAILURE ) { FORWARD_ERROR; }

  // Copy graph information arrays
  if( copyGraphInfoArrays(gInfo, baseGInfo) == FAILURE ) { FORWARD_ERROR; }

  // Check if copy was successfull
  if ( !areEqualGraphInfos(gInfo, baseGInfo) ) {
    STDERR_INFO("Unexpected behaviour: failed to copy graph information.");
    RETURN_ERROR;
  }

  return SUCCESS;
}

bool areEqualGraphInfos(GRAPHINFO* gInfo1, GRAPHINFO* gInfo2)
{
  // Trivial comparison
  if ( gInfo1 == gInfo2 ) { return TRUE; }

  // Single values
  if ( gInfo1->nBlocksAdjMatrix != gInfo2->nBlocksAdjMatrix ) { return FALSE; }
  if ( gInfo1->nBlocksPerEvent != gInfo2->nBlocksPerEvent ) { return FALSE; }

  if ( gInfo1->nEvents != gInfo2->nEvents ) { return FALSE; }
  if ( gInfo1->nActors != gInfo2->nActors ) { return FALSE; }
  if ( gInfo1->nEdges != gInfo2->nEdges ) { return FALSE; }
  if ( gInfo1->nPairs != gInfo2->nPairs ) { return FALSE; }

  if ( gInfo1->coocSum != gInfo2->coocSum ) { return FALSE; }
  if ( gInfo1->nRelevantPairs != gInfo2->nRelevantPairs ) { return FALSE; }

  // Arrays
  // Event list - array of strings
  if ( gInfo1->eventList != gInfo2->eventList ) { // == case both are NULL
    if ( gInfo1->eventList == NULL || gInfo2->eventList == NULL ) {
      return FALSE;
    }
    // If neither is NULL, both must have the same size
    for (uint eventIt=0; eventIt < gInfo1->nEvents; ++eventIt) {
      if ( strcmp(gInfo1->eventList[eventIt], gInfo2->eventList[eventIt]) ) {
        return FALSE;
      }
    }
  }

  // Co-oc half matrix - each row has one element less then the previous
  if ( gInfo1->originalCooc != gInfo2->originalCooc ) { // == case both are NULL
    if ( gInfo1->originalCooc == NULL || gInfo2->originalCooc == NULL ) {
      return FALSE;
    }
    // If neither is NULL, both must have the same size
    for (uint row=0; row < gInfo1->nEvents-1; ++row) {
      for (uint col=0; col < gInfo1->nEvents-1-row; ++col) {
        if ( gInfo1->originalCooc[row][col] != gInfo2->originalCooc[row][col] ){
          return FALSE;
        }
      }
    }
  }

  // Event degrees - array of ints
  if ( gInfo1->eventDegrees != gInfo2->eventDegrees ) { // == case both are NULL
    if ( gInfo1->eventDegrees == NULL || gInfo2->eventDegrees == NULL ) {
      return FALSE;
    }
    // If neither is NULL, both must have the same size
    for (uint eventIt=0; eventIt < gInfo1->nEvents; ++eventIt) {
      if ( gInfo1->eventDegrees[eventIt] != gInfo2->eventDegrees[eventIt] ) {
        return FALSE;
      }
    }
  }

  // Actor degrees - array of ints
  if ( gInfo1->actorDegrees != gInfo2->actorDegrees ) { // == case both are NULL
    if ( gInfo1->actorDegrees == NULL || gInfo2->actorDegrees == NULL ) {
      return FALSE;
    }
    // If neither is NULL, both must have the same size
    for (uint actorIt=0; actorIt < gInfo1->nActors; ++actorIt) {
      if ( gInfo1->actorDegrees[actorIt] != gInfo2->actorDegrees[actorIt] ) {
        return FALSE;
      }
    }
  }

  return TRUE;
}

void deleteGraphInfoArrays(GRAPHINFO* gInfo)
{
  // This function will result in segmentation fault if
  //  there is no memory allocated to a vector, but it does not point to NULL

  if ( gInfo->eventList != NULL ) {
    for (uint eventIt = 0; eventIt < gInfo->nEvents; ++eventIt) {
      free(gInfo->eventList[eventIt]);
      gInfo->eventList[eventIt] = NULL;
    }
    free(gInfo->eventList);
    gInfo->eventList = NULL;
  }

  if ( gInfo->originalCooc != NULL ) {
    for (uint eventIt = 0; eventIt < gInfo->nEvents-1; ++eventIt) {
      free(gInfo->originalCooc[eventIt]);
      gInfo->originalCooc[eventIt] = NULL;
    }
    free(gInfo->originalCooc);
    gInfo->originalCooc = NULL;
  }

  free(gInfo->eventDegrees);
  gInfo->eventDegrees = NULL;

  free(gInfo->actorDegrees);
  gInfo->actorDegrees = NULL;

}

void deleteGraphInfo(GRAPHINFO* gInfo)
{
  // Free allocated memory blocks
  deleteGraphInfoArrays(gInfo);

  // (Re-)initialize graph infomation
  initGraphInfo(gInfo);

}


void initGraph(GRAPH* g) {
  // Memset makes sure that even padding bytes of the structure are zeroed
  memset(g, 0, sizeof *g);
}

bool allocGraph(GRAPH* g) {

  // Free graph, if existing
  deleteGraph(g);

  // adjMatrix
  arrayCalloc(g->adjMatrix, graphInfo.nBlocksAdjMatrix);
  if ( g->adjMatrix == NULL ) { MEM_ERROR; }

  // actorAdjLists
  arrayCalloc(g->actorAdjLists, graphInfo.nEdges);
  if ( g->actorAdjLists == NULL ) { MEM_ERROR; }

  // actorAccumulatedDegrees
  arrayCalloc(g->actorAccumulatedDegrees, graphInfo.nActors+1);
  if ( g->actorAccumulatedDegrees == NULL ) { MEM_ERROR; }

  // actorEdgeMaps
  arrayCalloc(g->actorEdgeMaps, graphInfo.nEdges);
  if ( g->actorEdgeMaps == NULL ) { MEM_ERROR; }

  if ( !settings.isBipartiteGraph ) {
    // edgeLinks
    arrayCalloc(g->edgeLinks, graphInfo.nEdges);
    if ( g->edgeLinks == NULL ) { MEM_ERROR; }
  }

  // subBlocks Indexes
  fullMatrixCalloc(g->subBlocksStartIndexes,
                   graphInfo.nActors, NUMBER_OF_SUBBLOCKS);
  if ( g->subBlocksStartIndexes == NULL ) { MEM_ERROR; }

  fullMatrixCalloc(g->subBlocksEndIndexes,
                   graphInfo.nActors, NUMBER_OF_SUBBLOCKS);
  if ( g->subBlocksEndIndexes == NULL ) { MEM_ERROR; }

  return SUCCESS;
}

bool copyGraphData(GRAPH* g, GRAPH* baseG) {

  for (ulint blockIt = 0; blockIt < graphInfo.nBlocksAdjMatrix; blockIt++) {
    g->adjMatrix[blockIt] = baseG->adjMatrix[blockIt];
  }

  for (uint edgeIt = 0; edgeIt < graphInfo.nEdges; edgeIt++) {
    g->actorAdjLists[edgeIt] = baseG->actorAdjLists[edgeIt];
  }
  for (uint edgeIt = 0; edgeIt < graphInfo.nEdges; edgeIt++) {
    g->actorEdgeMaps[edgeIt] = baseG->actorEdgeMaps[edgeIt];
  }
  if ( !settings.isBipartiteGraph ) {
    for (uint edgeIt = 0; edgeIt < graphInfo.nEdges; edgeIt++) {
      g->edgeLinks[edgeIt] = baseG->edgeLinks[edgeIt];
    }
  }
  for (uint actorIt = 0; actorIt < (graphInfo.nActors + 1); actorIt++) {
    g->actorAccumulatedDegrees[actorIt] =
        baseG->actorAccumulatedDegrees[actorIt];
  }

  for (uint actorIt=0; actorIt < graphInfo.nActors; ++actorIt) {
    for (uint sbIt=0; sbIt < NUMBER_OF_SUBBLOCKS; ++sbIt) {
      g->subBlocksStartIndexes[actorIt][sbIt] =
        baseG->subBlocksStartIndexes[actorIt][sbIt];

      g->subBlocksEndIndexes[actorIt][sbIt] =
        baseG->subBlocksEndIndexes[actorIt][sbIt];
    }
  }

  return SUCCESS;
}

bool copyGraph(GRAPH* g, GRAPH* baseG) {

  // (Re-)allocate memory for graph
  if( allocGraph(g) == FAILURE ) { FORWARD_ERROR; }

  // Copy graph data
  if( copyGraphData(g, baseG) == FAILURE ) { FORWARD_ERROR; }

  // Check if copy was successfull
  if ( !areEqualGraphs(g, baseG) ) {
    STDERR_INFO("Unexpected behaviour: failed to copy graph.");
    RETURN_ERROR;
  }

  return SUCCESS;
}

bool canonizeGraph(GRAPH* g)
{
  // "Canonical" form of the graph:
  // 1. Adj. Lists in ascending order
  // 2. Updated subblock indexes
  // 3. Updated redundant edge links (non-bipartite graph only)

  // Sort adjacency lists and find subblock indexes
  for (uint actorIt = 0; actorIt < graphInfo.nActors; ++actorIt ) {
    setIndexesOfSubBlocks(g, actorIt);
  }

  // Update redundant edge links -- non-bipartite graph only
  if ( !settings.isBipartiteGraph ) {
    for (uint edgeIt = 0; edgeIt < (graphInfo.nEdges) ; edgeIt++) {
      g->edgeLinks[edgeIt] = findLinkedEgde(g, edgeIt);
      //Handle reported error through impossible value
      if ( g->edgeLinks[edgeIt] == graphInfo.nEdges ) { FORWARD_ERROR; }
    }
  }

  return SUCCESS;
}

bool areEqualGraphs(GRAPH* g1, GRAPH* g2)
{
  // Trivial comparison
  if ( g1 == g2 ) { return TRUE; }

  // Adjacency matrix - array of BLOCKs
  if ( g1->adjMatrix != g2->adjMatrix ) { // == case both are NULL
    if ( g1->adjMatrix == NULL || g2->adjMatrix == NULL ) {
      printf("MATRIX1\n\n\n");
      return FALSE;
    }
    // If neither is NULL, both must have the same size
    for (uint blockIt=0; blockIt < graphInfo.nBlocksAdjMatrix; ++blockIt) {
      if ( g1->adjMatrix[blockIt] != g2->adjMatrix[blockIt] ) {
        printf("MATRIX2\n\n\n");
        return FALSE;
      }
    }
  }


  // Actors adjacency lists  - array of ints
  if ( g1->actorAdjLists != g2->actorAdjLists ) { // == case both are NULL
    if ( g1->actorAdjLists == NULL || g2->actorAdjLists == NULL ) {
      return FALSE;
    }
    // If neither is NULL, both must have the same size
    for (uint edgeIt=0; edgeIt < graphInfo.nEdges; ++edgeIt) {
      if ( g1->actorAdjLists[edgeIt] != g2->actorAdjLists[edgeIt] ) {
        return FALSE;
      }
    }
  }

  // Actors accumulated degrees - array of ints
  if ( g1->actorAccumulatedDegrees != g2->actorAccumulatedDegrees ) {
    if ( g1->actorAccumulatedDegrees == NULL
         || g2->actorAccumulatedDegrees == NULL ) {
      return FALSE;
    }
    // If neither is NULL, both must have the same size
    for (uint actorIt=0; actorIt < graphInfo.nActors; ++actorIt) {
      if ( g1->actorAccumulatedDegrees[actorIt]
           != g2->actorAccumulatedDegrees[actorIt] ) {
        return FALSE;
      }
    }
  }

  // Actors edge maps - array of ints
  if ( g1->actorEdgeMaps != g2->actorEdgeMaps ) { // == case both are NULL
    if ( g1->actorEdgeMaps == NULL || g2->actorEdgeMaps == NULL ) {
      return FALSE;
    }
    // If neither is NULL, both must have the same size
    for (uint edgeIt=0; edgeIt < graphInfo.nEdges; ++edgeIt) {
      if ( g1->actorEdgeMaps[edgeIt] != g2->actorEdgeMaps[edgeIt] ) {
        return FALSE;
      }
    }
  }

  // Edge links - array of ints
  if ( g1->edgeLinks != g2->edgeLinks ) { // == case both are NULL
    if ( g1->edgeLinks == NULL || g2->edgeLinks == NULL ) {
      return FALSE;
    }
    // If neither is NULL, both must have the same size
    for (uint edgeIt = 0; edgeIt < graphInfo.nEdges ; edgeIt++) {
      if ( g1->edgeLinks[edgeIt] != g2->edgeLinks[edgeIt] ) {
        return FALSE;
      }
    }
  }

  // Subblock start indexes - matrix of ints
  if ( g1->subBlocksStartIndexes != g2->subBlocksStartIndexes ) {
    if ( g1->subBlocksStartIndexes==NULL || g2->subBlocksStartIndexes==NULL ) {
      printf("SB11\n\n\n");
      return FALSE;
    }
    // If neither is NULL, both must have the same size
    for (uint actorIt=0; actorIt < graphInfo.nActors; ++actorIt) {
      for (uint sbIt=0; sbIt < NUMBER_OF_SUBBLOCKS; ++sbIt) {
        if ( g1->subBlocksStartIndexes[actorIt][sbIt]
             != g2->subBlocksStartIndexes[actorIt][sbIt] ) {
          printf("SB21\n\n\n");
          return FALSE;
        }
      }
    }
  }

  // Subblock end indexes - matrix of ints
  if ( g1->subBlocksEndIndexes != g2->subBlocksEndIndexes ) {
    if ( g1->subBlocksEndIndexes == NULL || g2->subBlocksEndIndexes == NULL ) {
      printf("SB12\n\n\n");
      return FALSE;
    }
    // If neither is NULL, both must have the same size
    for (uint actorIt=0; actorIt < graphInfo.nActors; ++actorIt) {
      for (uint sbIt=0; sbIt < NUMBER_OF_SUBBLOCKS; ++sbIt) {
        if ( g1->subBlocksEndIndexes[actorIt][sbIt]
             != g2->subBlocksEndIndexes[actorIt][sbIt] ) {
          printf("SB22\n\n\n");
          return FALSE;
        }
      }
    }
  }

  return TRUE;
}

void deleteGraph(GRAPH* g)
{
  // This function will result in segmentation fault if
  //  there is no memory allocated to a vector, but it does not point to NULL

  free(g->adjMatrix);
  g->adjMatrix = NULL;

  free(g->actorAdjLists);
  g->actorAdjLists = NULL;

  free(g->actorEdgeMaps);
  g->actorEdgeMaps = NULL;

  free(g->edgeLinks);
  g->edgeLinks = NULL;

  free(g->actorAccumulatedDegrees);
  g->actorAccumulatedDegrees = NULL;

  if ( g->subBlocksStartIndexes != NULL )  {
    for (uint actorIt = 0; actorIt < graphInfo.nActors; ++actorIt) {
      free(g->subBlocksStartIndexes[actorIt]);
      g->subBlocksStartIndexes[actorIt] = NULL;

      free(g->subBlocksEndIndexes[actorIt]);
      g->subBlocksEndIndexes[actorIt] = NULL;
    }
    free(g->subBlocksStartIndexes);
    g->subBlocksStartIndexes = NULL;
  }

  free(g->subBlocksEndIndexes);
  g->subBlocksEndIndexes = NULL;

}

bool adjMatrixFromAdjLists(GRAPH* g)
{
  // Clear the adj. matrix
  for(uint blockIt = 0; blockIt < graphInfo.nBlocksAdjMatrix; ++blockIt) {
    g->adjMatrix[blockIt] = 0;
  }

  // For each actor, set bits in the matrix representing its edges
  // And for non-bipartite graphs, set the main diagonal (self-loops)
  for(uint actorIt = 0; actorIt < graphInfo.nActors; ++actorIt) {
    uint actorAdjListFirst = g->actorAccumulatedDegrees[actorIt];
    uint actorAdjListLast = g->actorAccumulatedDegrees[actorIt + 1];
    for(uint edgeIt = actorAdjListFirst; edgeIt < actorAdjListLast; ++edgeIt) {
      uint eventId = g->actorAdjLists[ edgeIt ];
      setBitOnBlockArray(g->adjMatrix,
                         actorIt, eventId * graphInfo.nBlocksPerEvent);
    }
    if ( !settings.isBipartiteGraph ) {
      // Main diagonal: (a->a) self-loop. Simplify swap possibility checks
      setBitOnBlockArray(g->adjMatrix,
                         actorIt, actorIt * graphInfo.nBlocksPerEvent);
    }
  }

  return SUCCESS;
}

bool adjListsFromAdjMatrix(GRAPH* g)
{
  uint edgeCnt = 0;
  g->actorAccumulatedDegrees[0] = 0;
  for (uint actorIt=0; actorIt < graphInfo.nActors; ++actorIt) {
    for (uint eventIt=0; eventIt < graphInfo.nEvents; ++eventIt) {
      // Skip self-loops (main diagonal) if is a non-bipartite graph
      if ( !settings.isBipartiteGraph && actorIt == eventIt ) { continue; }
      if ( checkBitOnBlockArray(g->adjMatrix, actorIt,
                                eventIt * graphInfo.nBlocksPerEvent ) ) {
        g->actorAdjLists[edgeCnt] = eventIt;
        g->actorEdgeMaps[edgeCnt] = actorIt;
        ++edgeCnt;
      }
    }
    g->actorAccumulatedDegrees[actorIt+1] = edgeCnt;
  }
  // Sanity check
  if ( edgeCnt != graphInfo.nEdges ) {
    STDERR_INFO("Unexpected behaviour: "
                "Construction of adjacency lists from adjacency matrix resulted"
                " in different number of edges (%u) from the expected (%u).",
                edgeCnt, graphInfo.nEdges);
    RETURN_ERROR;
  }

  return SUCCESS;
}

uint getNodeDegreeViaAdjMatrix(GRAPH* g, uint nodeId, const char* nodeType)
{
  uint nodeDegree = 0;
  if ( !strcmp("event", nodeType) ) { // If getting event degree
    for (uint blockIt = 0;
         blockIt < graphInfo.nBlocksPerEvent; blockIt++) {
      nodeDegree +=
          popCount( g->adjMatrix[ nodeId * graphInfo.nBlocksPerEvent
                                    + blockIt ] );
    }
  }
  else if ( !strcmp("actor", nodeType) ) { // If getting actor degree
    for (uint eventIt = 0; eventIt < graphInfo.nEvents; eventIt++) {
      if ( checkBitOnBlockArray(g->adjMatrix, nodeId,
                                eventIt*graphInfo.nBlocksPerEvent) ) {
        nodeDegree++;
      }
    }
  }
  else {
    STDERR_INFO("Unexpected behavior - could not decide for which node type "
                  "to evaluate node degree.");
    RETURN_ERROR_V(0); // All nodes must have degree of at least 1
  }
  if ( settings.isBipartiteGraph == TRUE ) { // If is a bipartite graph
    return nodeDegree;
  }
  else { // If is a non-bipartite graph
    // Remove "self edge" (main diagonal)
    return nodeDegree - 1;
  }
}

uint getNodeDegreeViaAdjLists(GRAPH* g, uint nodeId, const char* nodeType)
{
  uint nodeDegree = 0;
  if ( !strcmp("event", nodeType) ) { // If getting event degree
    for(uint edgeIt = 0; edgeIt < graphInfo.nEdges; ++edgeIt) {
      // If the current edge links the given node
      if ( g->actorAdjLists[edgeIt] == nodeId ) {
        // Increment its degree
        nodeDegree++;
      }
    }
  }
  else if ( !strcmp("actor", nodeType) ) { // If getting actor degree
    nodeDegree = g->actorAccumulatedDegrees[nodeId+1];
    nodeDegree -= g->actorAccumulatedDegrees[nodeId];
  }
  else {
    STDERR_INFO("Unexpected behavior - could not decide for which node type "
                "to evaluate node degree.");
    RETURN_ERROR_V(0); // All nodes must have degree of at least 1
  }

  return nodeDegree;
}

uint findLinkedEgde(GRAPH* g, uint edgeKnown) {
  // Return the index of the yet unknown edge
  //   which represents the same edge as the known edge,
  // i.e. given here is a->b, return here is b->a

  // Search through the adj. list of the node pointed by @edgeKnown
  for(uint edgeIt = g->actorAccumulatedDegrees[g->actorAdjLists[edgeKnown]];
      edgeIt < g->actorAccumulatedDegrees[g->actorAdjLists[edgeKnown] + 1];
      ++edgeIt) {
    // Stop when find the node whose adj. list contains @edgeKnown
    if ( g->actorAdjLists[edgeIt] == g->actorEdgeMaps[edgeKnown] ) {
      return edgeIt;
    }
  }

  // If the redundant edge is not found,
  //   report error and return an impossible value
  STDERR_INFO("Could not find redundant edge of (%u,%u)! "
              "Index of the know edge is: %u",
              g->actorEdgeMaps[edgeKnown],
              g->actorAdjLists[edgeKnown],
              edgeKnown);
  RETURN_ERROR_V(graphInfo.nEdges);
}
/* **************************************** */

/* **************************************** */
/* Debug print-outs/checking related functions */
void printAdjMatrix(GRAPH* g, const char* adjMatrixFileName)
{
  FILE* adjMatrixFile = fopen(adjMatrixFileName, "a");
  fprintf(adjMatrixFile,"\nAdjacency matrix\n      \\ Actors\nEvents\n\t");
  for (uint eventIt = 0; eventIt < graphInfo.nEvents; eventIt++) {
    for (uint actorIt = 0; actorIt < graphInfo.nActors; actorIt++) {
      fputs( checkBitOnBlockArray(g->adjMatrix, actorIt,
                eventIt*graphInfo.nBlocksPerEvent) ? "1 " : "0 ",
             adjMatrixFile);
    }
    fprintf(adjMatrixFile,"\n\t");
   }
  fprintf(adjMatrixFile,"\n");
  fclose(adjMatrixFile);
}

void printActorAdjLists(GRAPH* g, const char* adjListFileName)
{
  FILE* adjListFile = fopen(adjListFileName, "a");
  fprintf(adjListFile,"\nActor adjacency list\n");
  for (uint actorIt = 0; actorIt < graphInfo.nActors; actorIt++) {
    fprintf(adjListFile,"Actor %u: ", actorIt);
    for (uint edgeIt = g->actorAccumulatedDegrees[actorIt];
         edgeIt < g->actorAccumulatedDegrees[actorIt + 1]; edgeIt++ ) {
      fprintf(adjListFile,"%u ", g->actorAdjLists[edgeIt]);
    }
    fprintf(adjListFile,"\n");
  }
  fprintf(adjListFile,"\n");
  fclose(adjListFile);
}

void printActorAccumulatedDegrees(GRAPH* g, const char* adjListFileName)
{
  FILE* adjListFile = fopen(adjListFileName, "a");
  fprintf(adjListFile,"\nActor degrees\n");
  for (uint actorIt = 0; actorIt < graphInfo.nActors; actorIt++) {
    uint actorDegree = g->actorAccumulatedDegrees[actorIt + 1];
    actorDegree -= g->actorAccumulatedDegrees[actorIt];
    fprintf(adjListFile,"Actor: %u\tDegree: %u\t Accumulated Degree: %u\n",
      actorIt, actorDegree, g->actorAccumulatedDegrees[actorIt + 1]);
  }
  fprintf(adjListFile,"\n");
  fclose(adjListFile);
}

void printActorEdgeMap(GRAPH* g, const char* adjListFileName)
{
  FILE* adjListFile = fopen(adjListFileName, "a");
  fprintf(adjListFile,"\nActor edge map\n");
  for (uint actorIt = 0; actorIt < graphInfo.nActors; actorIt++) {
    fprintf(adjListFile,"Actor %u: ", actorIt);
    for (uint edgeIt = g->actorAccumulatedDegrees[actorIt];
         edgeIt < g->actorAccumulatedDegrees[actorIt + 1]; edgeIt++ ) {
      fprintf(adjListFile,"%u ", g->actorEdgeMaps[edgeIt]);
    }
    fprintf(adjListFile,"\n");
  }
  fprintf(adjListFile,"\n");
  fclose(adjListFile);
}

void printEdgeLinks(GRAPH* g, const char* adjListFileName)
{
  FILE* adjListFile = fopen(adjListFileName, "a");
  fprintf(adjListFile,"\nRedundant edges\n");
  for(uint edgeIt = 0; edgeIt < graphInfo.nEdges; edgeIt++) {
    fprintf(adjListFile,"%u and %u\n", edgeIt, g->edgeLinks[edgeIt]);
  }
}

void printEventList()
{
  FILE* eventListFile = fopen("eventList.dbg", "a");
  fprintf(eventListFile,"Event List:\n");
  fprintf(eventListFile,"ID  Name\n");
  for (uint eventIt=0; eventIt < graphInfo.nEvents; eventIt++) {
    fprintf(eventListFile,"%d  %s\n", eventIt, graphInfo.eventList[eventIt]);
  }
  fprintf(eventListFile,"\n");
  fclose(eventListFile);
}

bool initOriginalNodeDegrees(GRAPH* g)
{
  for(uint actorIt = 0; actorIt < graphInfo.nActors; actorIt++) {
    // Getting original actor degrees from lists, which is more trivial
    graphInfo.actorDegrees[actorIt] =
        getNodeDegreeViaAdjLists(g, actorIt, "actor");
    if ( graphInfo.actorDegrees[actorIt] == 0 ) { FORWARD_ERROR; }
  }

  for(uint eventIt = 0; eventIt < graphInfo.nEvents; eventIt++) {
    // Getting original event degrees from matrix, which is more trivial
    graphInfo.eventDegrees[eventIt] =
        getNodeDegreeViaAdjMatrix(g, eventIt, "event");
    if ( graphInfo.eventDegrees[eventIt] == 0 ) { FORWARD_ERROR; }
  }

  return SUCCESS;
}

bool checkNodesDegrees(GRAPH* g)
{
  // Check all actor degrees
  for(uint actorIt = 0; actorIt < graphInfo.nActors; actorIt++) {
    uint nodeDegreeFromMatrix = getNodeDegreeViaAdjMatrix(g, actorIt, "actor");
    if ( nodeDegreeFromMatrix == 0 ) { FORWARD_ERROR; }
    uint nodeDegreeFromLists = getNodeDegreeViaAdjLists(g, actorIt, "actor");
    if ( nodeDegreeFromLists == 0 ) { FORWARD_ERROR; }
    // If structures "report" different degree values
    if (  nodeDegreeFromMatrix != nodeDegreeFromLists ) {
      STDERR_INFO("Unexpected behavior - in actor's %u degree:\n"
                  "Adj matrix gives degree of %u, "
                  "while adj. lists give degree of %u!\n"
                  "Original degree was %u.", actorIt,
                  nodeDegreeFromMatrix,
                  nodeDegreeFromLists,
                  graphInfo.actorDegrees[actorIt]);
      RETURN_ERROR;
    }
    // If structures "report" the same degree values, but different from the original
    else if (  nodeDegreeFromMatrix != graphInfo.actorDegrees[actorIt] ) {
      STDERR_INFO("Unexpected behavior - in actor's %u degree:\n"
                  "new graph give degree of %u,"
                  "while original graph gave degree of %u!",
                  actorIt,
                  nodeDegreeFromMatrix,
                  graphInfo.actorDegrees[actorIt]);
      RETURN_ERROR;
    }
  }
  // And all event degrees
  for(uint eventIt = 0; eventIt < graphInfo.nEvents; eventIt++) {
    uint nodeDegreeFromMatrix = getNodeDegreeViaAdjMatrix(g, eventIt, "event");
    if ( nodeDegreeFromMatrix == 0 ) { FORWARD_ERROR; }
    uint nodeDegreeFromLists = getNodeDegreeViaAdjLists(g, eventIt, "event");
    if ( nodeDegreeFromLists == 0 ) { FORWARD_ERROR; }
    // If structures "report" different degree values
    if (  nodeDegreeFromMatrix != nodeDegreeFromLists ) {
      STDERR_INFO("Unexpected behavior - in event's %u degree:\n"
                  "Adj matrix gives degree of %u, "
                  "while adj. lists give degree of %u!\n"
                  "Original degree was %u.", eventIt,
                  nodeDegreeFromMatrix,
                  nodeDegreeFromLists,
                  graphInfo.eventDegrees[eventIt]);
      RETURN_ERROR;
    }
    // If structures "report" the same degree values, but different from the original
    else if (  nodeDegreeFromMatrix != graphInfo.eventDegrees[eventIt] ) {
      STDERR_INFO("Unexpected behavior - in event's %u degree:\n"
                  "new graph give degree of %u,"
                  "while original graph gave degree of %u!",
                  eventIt,
                  nodeDegreeFromMatrix,
                  graphInfo.eventDegrees[eventIt]);
      RETURN_ERROR;
    }
  }
  // Report success if everything matches
  return SUCCESS;
}

bool checkAdjListUniqueEvent(GRAPH* g)
{
  for(uint actorIt = 0; actorIt < (graphInfo.nActors); ++actorIt) {
    uint edgeItFirst = g->actorAccumulatedDegrees[actorIt];
    uint edgeItLast = g->actorAccumulatedDegrees[actorIt + 1];;
    for(uint edgeIt1 = edgeItFirst; edgeIt1 < edgeItLast - 1; ++edgeIt1 ) {
      for(uint edgeIt2 = edgeIt1 + 1; edgeIt2 < edgeItLast; ++edgeIt2 ) {
        uint eventId1 = g->actorAdjLists[ edgeIt1 ];
        uint eventId2 = g->actorAdjLists[ edgeIt2 ];
        if ( eventId1 == eventId2 ) {
          STDERR_INFO("Unexpected behavior - in actor's %u degree:\n"
                      "Adj list has repeated event ids:\n"
                      "  Edge with id %u represents event id %u,\n  and also"
                      " the edge with id %u represents event id %u.",
                      actorIt,
                      edgeIt1, eventId1,
                      edgeIt2, eventId2);
          RETURN_ERROR;
        }
      }
    }
  }
  return SUCCESS;
}

bool checkAdjListSort(GRAPH* g)
{
  for(uint actorIt = 0; actorIt < (graphInfo.nActors); ++actorIt) {
    uint edgeItFirst = g->actorAccumulatedDegrees[actorIt];
    uint edgeItLast = g->actorAccumulatedDegrees[actorIt + 1];;
    for(uint edgeIt = edgeItFirst; edgeIt < edgeItLast - 1; ++edgeIt ) {
      uint eventId1 = g->actorAdjLists[ edgeIt ];
      uint eventId2 = g->actorAdjLists[ edgeIt + 1 ];
      if ( eventId1 >= eventId2 ) {
        if ( eventId1 > eventId2 ) {
          STDERR_INFO("Unexpected behavior - in actor's %u degree:\n"
                      "Adj list is not sorted in ascending order:\n"
                      "  Edge with id %u represents event id %u,\n  while"
                      " the next edge (id %u) represents event id %u.",
                      actorIt,
                      edgeIt, eventId1,
                      edgeIt+1, eventId2);
          RETURN_ERROR;
        }
        else if ( eventId1 == eventId2 ) {
          STDERR_INFO("Unexpected behavior - in actor's %u degree:\n"
                      "Adj list is has repeated event ids:\n"
                      "  Edge with id %u represents event id %u,\n  while"
                      " the next edge (id %u) represents event id %u.",
                      actorIt,
                      edgeIt, eventId1,
                      edgeIt+1, eventId2);
          RETURN_ERROR;
        }
      }
    }
  }
  return SUCCESS;
}

bool checkAdjStructs(GRAPH* g)
{
  // ASSUMING NODE DEGREES OF BOTH ADJ. MATRIX AND LISTS MATCHES WITH ORIGINAL
  // Check if edges in adj. lists are in adj. matrix
  // For each actor, check if adj list matches with adj matrix
  for(uint actorIt = 0; actorIt < (graphInfo.nActors); ++actorIt) {
    uint edgeItFirst = g->actorAccumulatedDegrees[actorIt];
    uint edgeItLast = g->actorAccumulatedDegrees[actorIt + 1];;
    for(uint edgeIt = edgeItFirst; edgeIt < edgeItLast; ++edgeIt) {
      uint eventId = g->actorAdjLists[ edgeIt ];
      if ( !checkBitOnBlockArray(g->adjMatrix, actorIt,
                                 eventId * graphInfo.nBlocksPerEvent) )
      {
        STDERR_INFO("Unexpected behavior - in actor's %u degree:\n"
                    "Adj matrix does not contain event id %u, "
                    "but adj list does.\n"
                    "Impossible to know which one is right, if any.",
                    actorIt, eventId);
        RETURN_ERROR;
      }
    }
  }

  return SUCCESS;
}

bool checkEdgeLinks(GRAPH* g)
{
  for(uint edgeIt = 0; edgeIt < graphInfo.nEdges; edgeIt++) {

    uint actorId1 = g->actorEdgeMaps[edgeIt];
    uint eventId1 = g->actorAdjLists[edgeIt];

    uint actorId2 = g->actorEdgeMaps[g->edgeLinks[edgeIt]];
    uint eventId2 = g->actorAdjLists[g->edgeLinks[edgeIt]];

    // If linked edge are not redundant, i.e., E1 = (A,B) and E2 != (B,A)
    if ( (actorId1 != eventId2) || (actorId1 != eventId2) ) {
      STDERR_INFO("Unexpected behavior - linked edges with IDs %u and %u"
                  " seems not to be redundant: "
                  "edge %u = (%u, %u) and edge %u = (%u, %u)!",
                  edgeIt, g->edgeLinks[edgeIt],
                  edgeIt, actorId1, eventId1,
                  g->edgeLinks[edgeIt], actorId2, eventId2);
      RETURN_ERROR;
    }
  }

  return SUCCESS;
}

bool graphTest (GRAPH* g)
{
  // Check node degrees
  if ( checkNodesDegrees(g) == FAILURE ) { FORWARD_ERROR; }

  if ( checkAdjListUniqueEvent(g) == FAILURE ) { FORWARD_ERROR; }

  // Check is all lists are sorted in ascending order
  if ( settings.runCurveball ) {
//    if ( checkAdjListSort(g) == FAILURE ) { FORWARD_ERROR; }
  }

  // Check data consistency between adj lists and matrix
  if ( checkAdjStructs(g) == FAILURE ) { FORWARD_ERROR; }

  // Check links between redundant edge, if using a non-bipartite graph
  if ( settings.isBipartiteGraph == FALSE ) {
    if ( checkEdgeLinks(g) == FAILURE ) { FORWARD_ERROR; }
  }

  return SUCCESS;
}
