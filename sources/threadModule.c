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


#include "../headers/threadModule.h"

// Helper functions

void threadInit(OPENMP* openMP)
{
  openMP->threadId = omp_get_thread_num();
  openMP->nThreads = omp_get_num_threads();
  openMP->nProcs = omp_get_num_procs();

  #if FORCE_THREADWISE_SEQUENTIAL_RUN
  omp_set_lock(&forceSequentialLock); //Lock at start of parallel region
  #if PRINT_THREADWISE_SEQUENTIAL_RUN_VERBOSE
  fprintf(stdout, "Thread %u started!\n", omp_get_thread_num());
  #endif
  #endif
  // Forcing sequencial run may help debugging
  // It is accomplished by an openMP lock which is SET BEFORE ANY WORK block
  //  and UNSET BEFORE ANY BARRIER
  //   After one thread sets the lock, only that thread can proceed to work
  //    until itself unsets the lock, and reaches a barrier, where it waits each
  //    other thread until all of them reach the barrier, and they can proceed
  //   Explicit barriers may be needed inside each parallel block, because of
  //    data dependencies between proceedures. These barriers are what divide
  //    the parallel region into work blocks.
  //   One MUST be carefull NOT TO INTRODUCE implicit barriers
  //    inside any work block, such as:
  //    1. barriers after omp for loops - USE nowait to exclude it
  //    2. "barriers" during reduction in omp for reduction() loops
  //        use simple private local var inside the loop and merge them into the
  //        shared var after the loop using either omp critical or omp atomic
  //    ?. Please, add here any other case that you may find
  //   Failing to properly avoid barriers inside the work block,
  //    or not unsetting and [re]setting the lock around an explicit barrier,
  //    may case a DEADLOCK
  // It is also important to keep in mind that
  //  THE END RESULT SHOULD NOT BE AFFECTED -- this is the point of debugging

}

void threadBarrier()
{
  #if FORCE_THREADWISE_SEQUENTIAL_RUN
  #if PRINT_THREADWISE_SEQUENTIAL_RUN_VERBOSE
  fprintf(stdout, "Thread %u reached barrier!\n", omp_get_thread_num());
  #endif
  omp_unset_lock(&forceSequentialLock);// Unlock before any omp barrier
  #endif

  #pragma omp barrier // Enclosed by forceSequentialLock un- and relock

  #if FORCE_THREADWISE_SEQUENTIAL_RUN
  omp_set_lock(&forceSequentialLock); // Relock after any omp barrier
  #if PRINT_THREADWISE_SEQUENTIAL_RUN_VERBOSE
  fprintf(stdout, "Thread %u restarted!\n", omp_get_thread_num());
  #endif
  #endif
}

void threadEnd()
{
  #if FORCE_THREADWISE_SEQUENTIAL_RUN
  #if PRINT_THREADWISE_SEQUENTIAL_RUN_VERBOSE
  fprintf(stdout, "Thread %u ended!\n", omp_get_thread_num());
  #endif
  omp_unset_lock(&forceSequentialLock); //Unlock at parallel region end
  #endif
}


// Interface functions
gsl_rng** threadRandInit(ulint seed)
{
  gsl_rng **randG;
  arrayCalloc(randG, omp_get_max_threads());
  if ( randG == NULL ) { MEM_ERROR_V(NULL); }

  #pragma omp parallel
  {
    OPENMP openMP;
    threadInit(&openMP);

    randG[openMP.threadId] = gsl_rng_alloc(gsl_rng_mt19937);
    gsl_rng_set(randG[openMP.threadId], openMP.threadId + seed);

    #pragma omp master
    {
      randC = gsl_rng_alloc(gsl_rng_mt19937);
      gsl_rng_set(randC, openMP.nThreads + seed);
    }

    threadEnd();
  }

  return randG;
}

bool threadCopyGraph(GRAPH* g)
{
  bool returnFlag = SUCCESS; // Avoid use of expensive omp cancel
  #pragma omp parallel
  {
    OPENMP openMP;
    threadInit(&openMP);

    if (openMP.threadId != 0) {
      if ( copyGraph(&g[openMP.threadId], &g[0]) == FAILURE ) {
        STDERR_INFO("Thread %u: error while copying graph.",
                    openMP.threadId);
        returnFlag = FAILURE;
      }
    }

    threadEnd();
  }

  return returnFlag;
}

bool threadGetOriginalCooc(GRAPH* g)
{

  // Get original co-occurance
  threadGetCooc( g, graphInfo.originalCooc );

  // Check sum and extract the number of relevant pairs
  ulint originalCoocSum = 0;
  graphInfo.nRelevantPairs = 0;
  #pragma omp parallel
  {
    OPENMP openMP;
    threadInit(&openMP);

    ulint localThreadCoocSum = 0; // Simple sum reduction
    ulint localThreadNRelevantPairs = 0; // Simple sum reduction
    #pragma omp for nowait schedule(auto) //See threadInit()
    for (uint row=0; row<(graphInfo.nEvents-1); row++) {
      for (uint col=0; col<(graphInfo.nEvents-1-row); col++) {
        localThreadCoocSum += graphInfo.originalCooc[row][col];
        if(graphInfo.originalCooc[row][col] >= settings.minRelevantCooc) {
          localThreadNRelevantPairs++;
        }
      }
    }
    // Simple sum reduction is used instead of omp reduction
    //  to avoid deadlock when running in forced sequential mode
    #pragma omp atomic
    originalCoocSum += localThreadCoocSum;
    #pragma omp atomic
    graphInfo.nRelevantPairs += localThreadNRelevantPairs;

    threadEnd();
  }

  if ( originalCoocSum != graphInfo.coocSum ) {
    STDERR_INFO("Unexpected behavior - original co-occurrence sum  ( %lu ) is "
                "different from the expected ( %lu ) !",
                originalCoocSum,
                graphInfo.coocSum
                );
    RETURN_ERROR;
  }

  if ( graphInfo.nRelevantPairs == 0 ) {
    STDERR_INFO("No pair has co-occurence greater than the minimum (%u). "
                "No point in continuing!", settings.minRelevantCooc);
    RETURN_ERROR;
  }

  return SUCCESS;

}

bool threadRunSwapsStep(GRAPH* g, gsl_rng **randGenerator, ulint nSwaps)
{
  if (settings.isBipartiteGraph) {
    if ( settings.runCurveball ) {
      if ( threadRunCurveballBipartite(g, randGenerator, nSwaps) == FAILURE ) {
        FORWARD_ERROR;
      }
    } else {
      if( threadRunSingleSwitchesBipartite(g, randGenerator, nSwaps)==FAILURE ){
        FORWARD_ERROR;
      }
    }
  }
  else {
    if ( threadRunSingleSwitchesGeneral(g, randGenerator, nSwaps) == FAILURE ) {
      FORWARD_ERROR;
    }
  }

  return SUCCESS;
}

bool threadUpdateTmpResult(GRAPH* g, TMPRESULT* tmpResult)
{
  // TODO: change to NUMBER_OF_GRAPHS or so
  for ( uint graphIt = 0; graphIt < NUMBER_OF_THREADS; graphIt++) {

    // Get graph cooc
    threadGetCooc( &g[graphIt], tmpResult->lastCooc );

    ulint coocSum = 0;
    #pragma omp parallel
    {
      OPENMP openMP;
      threadInit(&openMP);

      ulint localThreadCoocSum = 0; // Simple sum reduction
      #pragma omp for nowait schedule(auto) //See threadInit()
      for (uint row=0; row<(graphInfo.nEvents-1); row++) {
        for (uint col=0; col<(graphInfo.nEvents-1-row); col++) {
          tmpResult->coocSum[row][col] += tmpResult->lastCooc[row][col];
          tmpResult->coocSquareSum[row][col] +=
              tmpResult->lastCooc[row][col] * tmpResult->lastCooc[row][col];
          if(graphInfo.originalCooc[row][col] < tmpResult->lastCooc[row][col]) {
            tmpResult->pValue[row][col]++;
          } else
          if(graphInfo.originalCooc[row][col] == tmpResult->lastCooc[row][col]){
            // Pseudo .5/.5 rand. Graphs must be independent, so are these cases
            // This also ensures exactly equal results for equal rand. seeds
            tmpResult->pValue[row][col] += (graphIt % 2);
          }
          localThreadCoocSum += tmpResult->lastCooc[row][col];
          tmpResult->lastCooc[row][col] = 0;
        }
      }
      // Simple sum reduction is used instead of omp reduction
      //  to avoid deadlock when running in forced sequential mode
      #pragma omp atomic
      coocSum += localThreadCoocSum;

      threadEnd();
    }
    if ( coocSum != graphInfo.coocSum ) {
      STDERR_INFO("Graph %2u co-occurrence sum  ( %lu ) is "
                  "different from the expected ( %lu ) !\n",
                  graphIt,
                  coocSum,
                  graphInfo.coocSum
                  );
      RETURN_ERROR;
    }
  }

  return SUCCESS;
}

void threadDeleteGraph(GRAPH* g)
{
  #pragma omp parallel
  {
    OPENMP openMP;
    threadInit(&openMP);

    deleteGraph(&g[openMP.threadId]);

    threadEnd();
 }
}

void threadRandFree(gsl_rng** randG)
{
  #pragma omp parallel
  {
    OPENMP openMP;
    threadInit(&openMP);

    gsl_rng_free(randG[openMP.threadId]);

    threadEnd();
  }

  free(randG);
  randG = NULL;

  gsl_rng_free(randC);

}


// Core functions
bool threadGetCooc(GRAPH* g, uint** coocs)
{

  bool currentStatus = SUCCESS;
  #pragma omp parallel
  {
    OPENMP openMP;
    threadInit(&openMP);

    if ( settings.isBipartiteGraph ) {
      #pragma omp for nowait schedule(auto) //See threadInit()
      for (uint actorIt=0; actorIt < graphInfo.nActors; actorIt++) {
        setIndexesOfSubBlocks( g, actorIt );
      }
    } else {
      #pragma omp for nowait schedule(auto) //See threadInit()
      for (uint actorIt=0; actorIt < graphInfo.nActors; actorIt++) {
        setIndexesOfSubBlocks( g, actorIt );

        computeDirectEdgeCooc( g, actorIt, coocs);
      }
    }

    threadBarrier();

    computeCooc( g, coocs, openMP.threadId );

    // Update position of redundant edges
    if ( settings.isBipartiteGraph == FALSE ) {
      #pragma omp for nowait schedule(auto) //See threadInit()
      for (uint edgeIt = 0; edgeIt < graphInfo.nEdges; ++edgeIt) {
        if ( currentStatus == SUCCESS ) {
          g->edgeLinks[edgeIt] = findLinkedEgde( g, edgeIt );
          //Handle reported error through impossible value
          if ( g->edgeLinks[edgeIt] == graphInfo.nEdges ) {
            currentStatus = FAILURE; // Avoid use of the expensive omp cancel
          }
        }
      }
    }

    threadEnd();
  }
  if ( currentStatus == FAILURE ) { FORWARD_ERROR; } // Catch error

  return SUCCESS;
}

bool threadRunCurveballBipartite(GRAPH* g, gsl_rng** randG, ulint nSwaps)
{
  omp_lock_t writeStderrLock;
  omp_init_lock(&writeStderrLock);

  bool returnFlag = SUCCESS;
  #pragma omp parallel
  {
    OPENMP openMP;
    threadInit(&openMP);

    uint randNumber[2];
    for (ulint step=0; step < nSwaps; ++step) {

      randNumber[0] =
          (uint) gsl_rng_uniform_int(randG[openMP.threadId], graphInfo.nActors);
      randNumber[1] =
          (uint) gsl_rng_uniform_int(randG[openMP.threadId], graphInfo.nActors);
      while ( randNumber[1] == randNumber[0] ) {
        // Force actors to be different
        randNumber[1] =
            (uint)gsl_rng_uniform_int(randG[openMP.threadId],graphInfo.nActors);
      }
      curveballTradeHashedLists( &g[openMP.threadId],
                                 randNumber, randG[openMP.threadId]);

      #if PRINT_GRAPH_AS_ADJACENCY_MATRIX_EACH_SWAP
      {
        char adjMatrixFileName[30 + MAX_INT_STR_SIZE];
        strcpy(adjMatrixFileName, SAMPLES_ADJACENCY_MATRICES_FILE_NAME);
        char threadId_str[MAX_INT_STR_SIZE];
        sprintf(threadId_str, "%u", openMP.threadId);
        strcat(adjMatrixFileName, threadId_str);
        strcat(adjMatrixFileName, ".dbg");
        printAdjMatrix(&g[openMP.threadId],  adjMatrixFileName);
      }
      #endif
      #if PRINT_GRAPH_AS_ACTOR_ADJACENCY_LIST_EACH_SWAP
      {
        char adjListFileName[30 + MAX_INT_STR_SIZE];
        strcpy(adjListFileName, SWAPS_ADJACENCY_LISTS_FILE_NAME);
        char threadId_str[MAX_INT_STR_SIZE];
        sprintf(threadId_str, "%u", openMP.threadId);
        strcat(adjListFileName, threadId_str);
        strcat(adjListFileName, ".dbg");
        printActorAdjLists(&g[openMP.threadId], adjListFileName);
      }
      #endif

      #if TEST_GRAPH_EACH_SWAP
      {
        adjMatrixFromAdjListsBipartite( &g[openMP.threadId] );
        if ( graphTest(&g[openMP.threadId]) == FAILURE )
        {
          returnFlag = FAILURE; // Force end of parallel section for all threads
          omp_set_lock(&writeStderrLock);
          // Print graph test error in order
          graphTest(&g[openMP.threadId]);
          STDERR_INFO("Unexpected behavior - graph test failed in thread %u.\n"
                      "Last swap tried edges with id's %u and %u.",
                      openMP.threadId,
                      randNumber[0], randNumber[1]);
          omp_unset_lock(&writeStderrLock);
          break;
        } else if ( returnFlag == FAILURE )
        {
          break;
        }
      }
      #endif
    }

    // Re-build adj. matrix - actually not needed for swapping nor for cooc
    //  currently here for testing the graph
    adjMatrixFromAdjLists( &g[openMP.threadId] );
    #if TEST_GRAPH
    {
      if ( graphTest(&g[openMP.threadId]) == FAILURE )
      {
        returnFlag = FAILURE; // Force end of parallel section for all threads
        omp_set_lock(&writeStderrLock);
        // Print graph test error in order
        graphTest(&g[openMP.threadId]);
        STDERR_INFO("Unexpected behavior - graph test failed in thread %u.\n"
                    "Last swap tried edges of actors with id's %u and %u.",
                    openMP.threadId,
                    randNumber[0], randNumber[1]);
        omp_unset_lock(&writeStderrLock);
      }
    }
    #endif

    threadEnd();
  }

  return returnFlag;
}

bool threadRunSingleSwitchesBipartite(GRAPH* g, gsl_rng** randG, ulint nSwaps)
{
  omp_lock_t writeStderrLock;
  omp_init_lock(&writeStderrLock);

  bool returnFlag = SUCCESS;
  #pragma omp parallel
  {
    OPENMP openMP;
    threadInit(&openMP);

    uint randNumber[2];
    for (ulint swap=0; swap < nSwaps; swap++) {
      randNumber[0] =
          (uint) gsl_rng_uniform_int(randG[openMP.threadId], graphInfo.nEdges);
      randNumber[1] =
          (uint) gsl_rng_uniform_int(randG[openMP.threadId], graphInfo.nEdges);

      singleSwapBipartite( &g[openMP.threadId], randNumber);

      #if PRINT_GRAPH_AS_ADJACENCY_MATRIX_EACH_SWAP
      {
        char adjMatrixFileName[30 + MAX_INT_STR_SIZE];
        strcpy(adjMatrixFileName, SAMPLES_ADJACENCY_MATRICES_FILE_NAME);
        char threadId_str[MAX_INT_STR_SIZE];
        sprintf(threadId_str, "%u", openMP.threadId);
        strcat(adjMatrixFileName, threadId_str);
        strcat(adjMatrixFileName, ".dbg");
        printAdjMatrix(&g[openMP.threadId],  adjMatrixFileName);
      }
      #endif
      #if PRINT_GRAPH_AS_ACTOR_ADJACENCY_LIST_EACH_SWAP
      {
        char adjListFileName[30 + MAX_INT_STR_SIZE];
        strcpy(adjListFileName, SWAPS_ADJACENCY_LISTS_FILE_NAME);
        char threadId_str[MAX_INT_STR_SIZE];
        sprintf(threadId_str, "%u", openMP.threadId);
        strcat(adjListFileName, threadId_str);
        strcat(adjListFileName, ".dbg");
        printActorAdjLists(&g[openMP.threadId], adjListFileName);
      }
      #endif
      #if TEST_GRAPH_EACH_SWAP
      {
        if ( graphTest(&g[openMP.threadId]) == FAILURE ) {
          returnFlag = FAILURE; // Force end of parallel section for all threads
          omp_set_lock(&writeStderrLock);
          graphTest(&g[openMP.threadId]);
          STDERR_INFO("Unexpected behavior - graph test failed in thread %u.\n"
                      "Last swap tried edges with id's %u and %u.",
                      openMP.threadId,
                      randNumber[0], randNumber[1]);
          omp_unset_lock(&writeStderrLock);
          break;
        } else if ( returnFlag == FAILURE ) { break; }
      }
      #endif
    }

    #if TEST_GRAPH
    {
      if ( graphTest(&g[openMP.threadId]) == FAILURE ) {
        returnFlag = FAILURE; // Force end of parallel section for all threads
        omp_set_lock(&writeStderrLock);
        graphTest(&g[openMP.threadId]);
        STDERR_INFO("Unexpected behavior - graph test failed in thread %u.\n"
                    "Last swap tried edges with id's %u and %u.",
                    openMP.threadId,
                    randNumber[0], randNumber[1]);
        omp_unset_lock(&writeStderrLock);
      }
    }
    #endif

    threadEnd();
  }

  return returnFlag;
}

bool threadRunSingleSwitchesGeneral(GRAPH* g, gsl_rng** randG, ulint nSwaps)
{
  omp_lock_t writeStderrLock;
  omp_init_lock(&writeStderrLock);

  bool returnFlag = SUCCESS;
  #pragma omp parallel
  {
    OPENMP openMP;
    threadInit(&openMP);

    uint randNumber[2];
    for (ulint swap=0; swap < nSwaps; swap++) {
      randNumber[0] =
          (uint) gsl_rng_uniform_int(randG[openMP.threadId], graphInfo.nEdges);
      randNumber[1] =
          (uint) gsl_rng_uniform_int(randG[openMP.threadId], graphInfo.nEdges);

      singleSwapGeneral( &g[openMP.threadId], randNumber);

      #if PRINT_GRAPH_AS_ADJACENCY_MATRIX_EACH_SWAP
      {
        char adjMatrixFileName[30 + MAX_INT_STR_SIZE];
        strcpy(adjMatrixFileName, SAMPLES_ADJACENCY_MATRICES_FILE_NAME);
        char threadId_str[MAX_INT_STR_SIZE];
        sprintf(threadId_str, "%u", openMP.threadId);
        strcat(adjMatrixFileName, threadId_str);
        strcat(adjMatrixFileName, ".dbg");
        printAdjMatrix(&g[openMP.threadId],  adjMatrixFileName);
      }
      #endif
      #if PRINT_GRAPH_AS_ACTOR_ADJACENCY_LIST_EACH_SWAP
      {
        char adjListFileName[30 + MAX_INT_STR_SIZE];
        strcpy(adjListFileName, SWAPS_ADJACENCY_LISTS_FILE_NAME);
        char threadId_str[MAX_INT_STR_SIZE];
        sprintf(threadId_str, "%u", openMP.threadId);
        strcat(adjListFileName, threadId_str);
        strcat(adjListFileName, ".dbg");
        printActorAdjLists(&g[openMP.threadId], adjListFileName);
      }
      #endif
      #if TEST_GRAPH_EACH_SWAP
      {
        if ( graphTest(&g[openMP.threadId]) == FAILURE) {
          returnFlag = FAILURE; // Force end of parallel section for all threads
          omp_set_lock(&writeStderrLock);
          graphTest(&g[openMP.threadId]);
          STDERR_INFO("Unexpected behavior - graph test failed in thread %u.\n"
                      "Last swap tried edges with id's %u and %u.",
                      openMP.threadId,
                      randNumber[0], randNumber[1]);
          omp_unset_lock(&writeStderrLock);
          break;
        } else if ( returnFlag == FAILURE ) { break; }
      }
      #endif
    }

    #if TEST_GRAPH
    {
      if ( graphTest(&g[openMP.threadId]) == FAILURE) {
        returnFlag = FAILURE; // Force end of parallel section for all threads
        omp_set_lock(&writeStderrLock);
        graphTest(&g[openMP.threadId]);
        STDERR_INFO("Unexpected behavior - graph test failed in thread %u.\n"
                    "Last swap tried edges with id's %u and %u.",
                    openMP.threadId,
                    randNumber[0], randNumber[1]);
        omp_unset_lock(&writeStderrLock);
      }
    }
    #endif

    threadEnd();
  }

  return returnFlag;
}
