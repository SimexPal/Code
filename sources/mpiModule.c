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


#include "../headers/mpiModule.h"

void mpiInit (int argc, char** argv)
{
  MPI_Init (&argc, &argv);
  /* get current process id */
  MPI_Comm_rank (MPI_COMM_WORLD, &mpiModule.procId);
  /* get number of processes */
  MPI_Comm_size (MPI_COMM_WORLD, &mpiModule.numProcs);
}

void printRunSettings()
{
  MPI_INFO("Run settings:\n");
  MPI_INFO("Input file path: %s\n", settings.inputFilePath);
  MPI_INFO("Input file name: %s\n", settings.inputFileName);
  MPI_INFO("Input file type: %s\n",
           settings.isBinaryInput ? "binary" : "text");
  MPI_INFO("Write binary graph: %s\n",
           settings.writeBinaryGraph ? "true" : "false");
  MPI_INFO("Output file path: %s\n", settings.outputFilePath);
  MPI_INFO("Output file name: %s\n", settings.outputFileName);
  MPI_INFO("Append run info: %s\n", settings.appendRunInfo ? "true" : "false");

  if ( settings.hasExternalGt ) {
    MPI_INFO("External ground truth file: %s\n", settings.externalGtFileName);
    MPI_INFO("Include external GT missing nodes: %s\n",
             settings.includeGTMissingNodes ? "yes" : "no");
    MPI_INFO("Ignore external GT missing nodes: %s\n",
             settings.ignoreGTMissingNodes ? "yes" : "no");
  }
  MPI_INFO("Random seed: %lu\n", settings.seed);
  if ( settings.isBipartiteGraph ) {
    MPI_INFO("Graph type: bipartite\n");
    if ( settings.bipartiteSideOfInterest == 'l' ) {
      MPI_INFO("Side of interest: %s\n", "left");
    } else {
      MPI_INFO("Side of interest: %s\n", "right");
    }

  } else {
    MPI_INFO("Graph type: non-bipartite\n");
    MPI_INFO("Direct edge co-occurrence value: %u\n",
             settings.directEdgeCoocValue);
  }
  if ( settings.runCurveball ) {
    MPI_INFO("Graph randomization algorithm: curveball\n");
  } else {
    MPI_INFO("Graph randomization algorithm: single switch\n");
  }
  if ( settings.runSwapHeuristic ) {
    MPI_INFO("Number of swaps: swap heuristic\n");
  }
  else if ( settings.elneSwaps ) {
    MPI_INFO("Number of swaps: |E| x ln(|E|)\n");
  }
  else {
    MPI_INFO("Number of swaps: %lu\n", settings.nSwaps);
  }
  MPI_INFO("Minimum co-occurrance for relevant pair: %u\n",
           settings.minRelevantCooc);
  if ( settings.runSamplesHeuristic ) {
    MPI_INFO("Number of samples: sample heuristic\n");
    MPI_INFO("Maximum number of samples: %u\n", settings.nMaxSamples);
    MPI_INFO("Ratio of pairs in internal GT: %lf\n",
             settings.ratioGtPairsPerResultPair);
    MPI_INFO("Internal PPV threshold: %lf\n",
             settings.internalPpvThreshold);
  } else {
    MPI_INFO("Number of samples: %u\n", settings.nSamples);
  }
  if ( strcmp(settings.dateStr, "") ) {
    MPI_INFO("Date string: %s\n", settings.dateStr);
  }
  if ( strcmp(settings.runIndex, "") ) {
    MPI_INFO("Run index: %s\n", settings.runIndex);
  }
}

ulint mpiRunSwapHeuristic(GRAPH* g, gsl_rng** randG) {

  // Very small graphs - can afford safe amount of swaps
  if ( graphInfo.nEdges < 100 ) {
    if ( settings.runCurveball ) {
      return (100 * graphInfo.nActors);
    } else { // Single switches
      return (graphInfo.nEdges * log(graphInfo.nEdges));
    }
  }

  FILE* pertFile = fopen("perturbation.dbg", "w");
  fprintf(pertFile, "%20s %20s %20s %20s\n",
          "nSwaps", "Perturbation", "bestNSwaps", "bestPerturbance");
  fclose(pertFile);

  GRAPH* oriG = &g[0];
  GRAPH* testG;
  testG = (GRAPH*) calloc(1, sizeof(GRAPH));
  if ( testG == NULL ) { RETURN_ERROR_V(0); }
  initGraph(testG);
  if ( copyGraph(testG, oriG) == FAILURE ) { RETURN_ERROR_V(0); }


  ulint nSwapsNeeded = 0;  // #Swaps from orig. to fully perturbated graph
  ulint nSwapsStep = 0;   // #Swaps to be done before next pert. evaluation
  if ( settings.runCurveball ) {
    nSwapsStep = graphInfo.nActors;
  } else { // Single switch
    nSwapsStep = graphInfo.nEdges/5;
  }

  TIMER swapTimer, pertTimer;

  ulint perturbation = 0; // Number of edges different from original graph
  ulint bestPert = 0; // Best perturbation (high enough)
  ulint nStepsNeeded = 0; // Number of steps until best perturbation
  //     \/ TODO: Make it user input
  float relevantIncrease = 1.01; // High enough means within x% of est. max
  uint nSteps = 0;
  // Until last half shows no significant increase
  while ( nSteps <= 2*nStepsNeeded ) {

    startTimer(&swapTimer);
    if ( settings.runCurveball ) {
      uint randNumber[2];
      for (ulint swap=0; swap < nSwapsStep; ++swap) {
        randNumber[0] = (uint)gsl_rng_uniform_int(randG[0], graphInfo.nActors);
        randNumber[1] = (uint)gsl_rng_uniform_int(randG[0], graphInfo.nActors);
        while ( randNumber[1] == randNumber[0] ) { // Force different actors
          randNumber[1] = (uint)gsl_rng_uniform_int(randG[0],graphInfo.nActors);
        }
        if( curveballTradeHashedLists( testG, randNumber, randG[0]) == FAILURE )
        {
          FORWARD_ERROR_V(0);
        }
//        adjMatrixFromAdjListsBipartite( testG );
      }
    } else {
      uint randNumber[2];
      for (ulint swap=0; swap < nSwapsStep; ++swap) {
        randNumber[0] = (uint) gsl_rng_uniform_int (randG[0], graphInfo.nEdges);
        randNumber[1] = (uint) gsl_rng_uniform_int (randG[0], graphInfo.nEdges);

        singleSwapBipartite( testG, randNumber);
      }
    }
    ++nSteps;
    if ( accElapsedTime(&swapTimer) == FAILURE ) { FORWARD_ERROR_V(0); }

    startTimer(&pertTimer);
    perturbation = perturbationMeasure(testG, oriG);
    if ( perturbation == -1 ) { FORWARD_ERROR; }
    if ( accElapsedTime(&pertTimer) == FAILURE ) { FORWARD_ERROR_V(0); }

    if ( perturbation > relevantIncrease * bestPert ) {
      bestPert = perturbation;
      nStepsNeeded = nSteps;
      nSwapsNeeded = nStepsNeeded * nSwapsStep;
    }

    pertFile = fopen("perturbation.dbg", "a");
    fprintf(pertFile, "%20lu %20lf %20lu %20lu %20lu\n",
            nSteps*nSwapsStep, swapTimer.totalElapsedTime,
            perturbation, nSwapsNeeded, bestPert);
    fclose(pertFile);

    fprintf(stdout, "Swaps so far: %lu\n", nSteps*nSwapsStep);
    fprintf(stdout, "Perturbation: %lu\n", perturbation);
    fprintf(stdout, "Best nSwaps: %lu\n", nSwapsNeeded);
    fprintf(stdout, "Enough perturbation: %lu\n", bestPert);
    if ( !isValidTimer(&swapTimer) ) { FORWARD_ERROR_V(0); }
    fprintf(stdout, "Total time swapping: %lf\n",
            getTotalElapsedTime(&swapTimer));
    fprintf(stdout, "Mean time swapping: %lf\n",
            getTotalElapsedTime(&swapTimer) / nSteps);
    if ( !isValidTimer(&pertTimer) ) { FORWARD_ERROR_V(0); }
    fprintf(stdout, "Total time meas. pert: %lf\n",
            getTotalElapsedTime(&pertTimer));
    fprintf(stdout, "Mean time meas. pert: %lf\n",
            getTotalElapsedTime(&pertTimer) / nSteps);
    if ( isValidTimer(&hashedPoolTimer) ) {
      fprintf(stdout, "Total time building hashed pool: %lf\n",
              getTotalElapsedTime(&hashedPoolTimer));
    }
    if ( isValidTimer(&poolTimer) ) {
      fprintf(stdout, "Total time building pool: %lf\n",
              getTotalElapsedTime(&poolTimer));
    }
    if ( isValidTimer(&shuffleTimer) ) {
      fprintf(stdout, "Total time shuffling pool: %lf\n",
              getTotalElapsedTime(&shuffleTimer));
    }
    if ( isValidTimer(&refillTimer) ) {
      fprintf(stdout, "Total time refilling lists: %lf\n",
              getTotalElapsedTime(&refillTimer));
    }
    if ( isValidTimer(&sortListsTimer) ) {
      fprintf(stdout, "Total time sorting lists: %lf\n",
              getTotalElapsedTime(&sortListsTimer));
    }

  }
  fprintf(stdout, "Swaps so far: %lu\n", nSteps*nSwapsStep);
  fprintf(stdout, "Perturbation: %lu\n", perturbation);
  fprintf(stdout, "Best nSwaps: %lu\n", nSwapsNeeded);
  fprintf(stdout, "Enough perturbation: %lu\n", bestPert);
  if ( !isValidTimer(&swapTimer) ) { FORWARD_ERROR_V(0); }
  fprintf(stdout, "Total time swapping: %lf\n",
          getTotalElapsedTime(&swapTimer));
  fprintf(stdout, "Mean time swapping: %lf\n",
          getTotalElapsedTime(&swapTimer) / nSteps);
  if ( !isValidTimer(&pertTimer) ) { FORWARD_ERROR_V(0); }
  fprintf(stdout, "Total time meas. pert: %lf\n",
          getTotalElapsedTime(&pertTimer));
  fprintf(stdout, "Mean time meas. pert: %lf\n",
          getTotalElapsedTime(&pertTimer) / nSteps);
  if ( isValidTimer(&hashedPoolTimer) ) {
    fprintf(stdout, "Total time building hashed pool: %lf\n",
            getTotalElapsedTime(&hashedPoolTimer));
  }
  if ( isValidTimer(&poolTimer) ) {
    fprintf(stdout, "Total time building pool: %lf\n",
            getTotalElapsedTime(&poolTimer));
  }
  if ( isValidTimer(&shuffleTimer) ) {
    fprintf(stdout, "Total time shuffling pool: %lf\n",
            getTotalElapsedTime(&shuffleTimer));
  }
  if ( isValidTimer(&refillTimer) ) {
    fprintf(stdout, "Total time refilling lists: %lf\n",
            getTotalElapsedTime(&refillTimer));
  }
  if ( isValidTimer(&sortListsTimer) ) {
    fprintf(stdout, "Total time sorting lists: %lf\n",
            getTotalElapsedTime(&sortListsTimer));
  }

  pertFile = fopen("perturbation.dbg", "a");
  fprintf(pertFile, "\n");
  fclose(pertFile);

  if ( settings.runCurveball ) {
    if ( threadRunCurveballBipartite(g, randG, nSwapsNeeded) == FAILURE ) {
      FORWARD_ERROR_V(0);
    }
  } else {
    if ( threadRunSingleSwitchesBipartite(g, randG, nSwapsNeeded) == FAILURE ) {
      FORWARD_ERROR_V(0);
    }
  }
  for ( uint i=0; i<NUMBER_OF_THREADS-1; ++i) {
    perturbation = perturbationMeasure(&g[i], &g[i+1]);
    if ( perturbation == -1 ) { FORWARD_ERROR_V(0); }
    fprintf(stdout, "Graph %u: Pert: %lu\n",
            i, perturbation);
  }
  perturbation = perturbationMeasure(&g[NUMBER_OF_THREADS-1], &g[0]);
  if ( perturbation == -1 ) { FORWARD_ERROR_V(0); }
  fprintf(stdout, "Graph %u: Pert: %lu\n", NUMBER_OF_THREADS-1, perturbation);

  return nSwapsNeeded;
}

bool mpiRun(int argc, char** argv)
{
  MPI_INFO("I am here!\n");

  // Initialize global lock if is forced sequential run
  #if FORCE_THREADWISE_SEQUENTIAL_RUN
  omp_init_lock(&forceSequentialLock);
  #endif

  /* *********************************************************************** */
  /* DECLARE TIME TRACKING VARIABLES */
  TIMER readTimer, coocTimer, swapTimer, mergeTimer, extGtTimer, sampleHeuTimer;
  /* *********************************************************************** */

  /* *********************************************************************** */
  /* PARSE INPUT ARGUMENTS */
  if ( argParser(argc, argv) == FAILURE ) { FORWARD_ERROR; }
  else if (settings.isHelpRun == TRUE) { return SUCCESS; }
  /* *********************************************************************** */

  /* *********************************************************************** */
  /* GET SAMPLING SEEDS */
  if ( !settings.gotExternalSeed ) {
    struct timespec seed;
    clock_gettime(CLOCK_MONOTONIC, &seed);
    settings.seed = seed.tv_nsec;
  }
  MPI_Bcast(&settings.seed, 1, MPI_LONG, 0, MPI_COMM_WORLD);
  ulint ranksSeed = settings.seed + NUMBER_OF_THREADS * mpiModule.procId;
  gsl_rng** randGenerator = threadRandInit(ranksSeed);
  if ( randGenerator == NULL ) { FORWARD_ERROR; }
  /* *********************************************************************** */


  /* *********************************************************************** */
  /* PRINT OUT SETTINGS */
  if (mpiModule.procId == 0) {
    printRunSettings();
  }
  /* *********************************************************************** */

  /* *********************************************************************** */
  /* READ INPUT GRAPH */
  GRAPH graph[NUMBER_OF_THREADS];
  for ( uint graphIt=0; graphIt < NUMBER_OF_THREADS; ++graphIt ) {
    initGraph( &graph[ graphIt ] );
  }

  startTimer(&readTimer);
  char inputFile[MAX_FILEPATH_SIZE+MAX_FILENAME_SIZE];
  strcpy(inputFile, settings.inputFilePath);
  strcat(inputFile, "/");
  strcat(inputFile, settings.inputFileName);
  MPI_INFO("Input file: %s\n", inputFile);
  if (settings.isBipartiteGraph) {
    if ( readInputBipartite(&graph[0], inputFile) == FAILURE ) {
      FORWARD_ERROR;
    }
    MPI_INFO("Graph info: #events %u, #actors %u, #edges %u, cooc sum %lu\n",
            graphInfo.nEvents, graphInfo.nActors, graphInfo.nEdges,
            graphInfo.coocSum);
  }
  else {
    if ( readInputNonBipartite(&graph[0], inputFile) == FAILURE ) {
      FORWARD_ERROR;
    }
    MPI_INFO("Graph info: #nodes %u, #edges %u (A->B and B->A), cooc sum %lu\n",
            graphInfo.nEvents, graphInfo.nEdges, graphInfo.coocSum);
  }
  MPI_INFO("Copying graphs...\n");
  if ( threadCopyGraph(graph) == FAILURE ) { FORWARD_ERROR; }
  if ( getElapsedTime(&readTimer) < 0 ) { FORWARD_ERROR; }
  MPI_INFO("Reading duration: %lf s\n", readTimer.elapsedTime);
  /* *********************************************************************** */

  /* *********************************************************************** */
  /* WRITE BINARY GRAPH FILE */
  if ( mpiModule.procId == 0 && settings.writeBinaryGraph ) {
    MPI_INFO("Writing binary graph...\n");
    if ( writeBinaryGraph(&graph[0], inputFile) == FAILURE ) { FORWARD_ERROR; }
  }
  /* *********************************************************************** */


  /* *********************************************************************** */
  /* INITIALIZE CO-OCCURENCE RELEATED STRUCTURES */
  TMPRESULT tmpResult;
  if ( coocHalfMatricesInitialize(&tmpResult, graphInfo.nEvents) == FAILURE ) {
    FORWARD_ERROR;
  }
  /* *********************************************************************** */

  /* *********************************************************************** */
  /* ORIGINAL CO-OCCURRENCE COMPUTATION */
  MPI_INFO("Calculating original co-occurrence...\n");
  if ( threadGetOriginalCooc(&graph[0]) == FAILURE ) {FORWARD_ERROR;}
  MPI_INFO("Original co-occurrence done.\n");
  MPI_INFO("Number of relevant pairs: %u\n", graphInfo.nRelevantPairs);
  /* *********************************************************************** */

//  /* *********************************************************************** */
//  /* FIRST (BIGGER) SWAP STEP - BURN IN PHASE  - |E| ln|E| SWAPS */
//  MPI_INFO("Running burn in phase (long swapping step)...\n");
//  if ( threadRunBurnInPhase(graph, randGenerator) == FAILURE ) { FORWARD_ERROR; }
//  MPI_INFO("Burn in phase done.\n");
//  /* *********************************************************************** */

  /* *********************************************************************** */
  /* DECIDE ON NUMBER OF SWAPS PER STEP */
  if ( settings.elneSwaps == TRUE ) {
    settings.nSwaps = (ulint) (graphInfo.nEdges * log((double)graphInfo.nEdges));
  }
  else if ( settings.runSwapHeuristic == TRUE ) {
    MPI_INFO("Running swap heuristic...\n");
    ulint nSwaps = mpiRunSwapHeuristic(graph, randGenerator);
    if ( nSwaps == 0 ) { FORWARD_ERROR; }
    MPI_Reduce(&nSwaps, &settings.nSwaps, 1,
               MPI_LONG, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Bcast(&settings.nSwaps, 1, MPI_LONG, 0, MPI_COMM_WORLD);
  } else {
    settings.nSwaps = settings.nSwaps;
  }
  /* *********************************************************************** */

  /* *********************************************************************** */
  // INITIALIZE INTERNAL GROUND TRUTH AND SET/CLEAR CONTINUE SAMPLING FLAG
  bool continueSamplingByHeuristic;
  GROUNDTRUTH internalGt;
  continueSamplingByHeuristic = TRUE;
  if ( settings.runSamplesHeuristic == TRUE ) {
    if ( initGT(&internalGt, TRUE) == FAILURE ) { FORWARD_ERROR; };
    settings.nSamples = settings.nMaxSamples;
  }
  /* *********************************************************************** */

  /* *********************************************************************** */
  // INITIALIZE EXTERNAL GROUND TRUTH
  GROUNDTRUTH externalGt;
  if ( initGT(&externalGt, FALSE) == FAILURE ) {
    FORWARD_ERROR;
  };
  /* *********************************************************************** */

  uint rankCurrentSample = 0;
  /* *********************************************************************** */
  /* GENERATE SEVERAL RANDOM GRAPHS BY THE FDSM */
  while (// Heuristic says to keep sampling (always TRUE if heuristic is not running)
            continueSamplingByHeuristic
         // Heuristic hard stop (if heuristic is on) or fixed number of samples (if heristic is off)
         && rankCurrentSample*mpiModule.numProcs < settings.nSamples
        ) {

    /* *********************************************************************** */
    /* RE-RANDOMIZE GRAPHS */
    MPI_INFO("Randomizing samples %u"
             " until %u. Swaps: %lu \n",
             rankCurrentSample, (rankCurrentSample + NUMBER_OF_THREADS),
             settings.nSwaps);

    startTimer(&swapTimer);
    // Each thread has its own graph and random seed
    threadRunSwapsStep(graph, randGenerator, settings.nSwaps);
    if ( accElapsedTime(&swapTimer) == FAILURE ) { FORWARD_ERROR; }
    MPI_Barrier(MPI_COMM_WORLD);
    /* *********************************************************************** */


    /* *********************************************************************** */
    /* CALCULATE CO-OCCURRENCES */
    MPI_INFO("Calculating co-occurrence...\n");

    startTimer(&coocTimer);

    // Re-alloc lastCooc for every new set of samples
    // This enables the memory exchange between the lastCooc vector and the vector of pairs
    for(uint row = 0; row < (graphInfo.nEvents-1); row++) {
      tmpResult.lastCooc[row] = (uint*) calloc ((graphInfo.nEvents - 1 - row), sizeof(uint));
      if ( tmpResult.lastCooc[row] == NULL ) { FORWARD_ERROR; }
    }

    // Split co-occurrence calculation among threads
    if ( threadUpdateTmpResult(graph, &tmpResult) == FAILURE ) { FORWARD_ERROR;}

    for (uint row = 0; row < (graphInfo.nEvents - 1); row++) {
      free(tmpResult.lastCooc[row]);
      tmpResult.lastCooc[row] = NULL;
    }

    if ( accElapsedTime(&coocTimer) == FAILURE ) { FORWARD_ERROR; }
    /* *********************************************************************** */

    // Increment the number of samples already perfomed
    rankCurrentSample += NUMBER_OF_THREADS;
    MPI_INFO("%u samples done.\n", rankCurrentSample);

    MPI_Barrier(MPI_COMM_WORLD);

    /* *********************************************************************** */
    /* MERGE RESULTS */
    if (mpiModule.numProcs > 1) {
      MPI_INFO("Merging results...\n");

      startTimer(&mergeTimer);
      if (mpiModule.procId == 0) {
        // PARENT PROCESS
        for (uint row = 0; row < (graphInfo.nEvents-1); row++) {
          MPI_Reduce(MPI_IN_PLACE, tmpResult.coocSum[row], (graphInfo.nEvents - 1 - row),
                     MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

          MPI_Reduce(MPI_IN_PLACE, tmpResult.coocSquareSum[row], (graphInfo.nEvents - 1 - row),
                     MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

          MPI_Reduce(MPI_IN_PLACE, tmpResult.pValue[row], (graphInfo.nEvents - 1 - row),
                     MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        }
      }
      else {
        // CHILD PROCESS
        for (uint row = 0; row < (graphInfo.nEvents - 1); row++) {
          MPI_Reduce(tmpResult.coocSum[row], tmpResult.coocSum[row],
                     graphInfo.nEvents - 1 - row,
                     MPI_LONG, MPI_SUM, 0,
                     MPI_COMM_WORLD);

          MPI_Reduce(tmpResult.coocSquareSum[row], tmpResult.coocSquareSum[row],
                     graphInfo.nEvents - 1 - row,
                     MPI_LONG, MPI_SUM, 0,
                     MPI_COMM_WORLD);

          MPI_Reduce(tmpResult.pValue[row], tmpResult.pValue[row],
                     graphInfo.nEvents - 1 - row,
                     MPI_INT, MPI_SUM, 0,
                     MPI_COMM_WORLD);
        }
      }
      if ( accElapsedTime(&mergeTimer) == FAILURE ) { FORWARD_ERROR; }
    }
    /* *********************************************************************** */

    MPI_Barrier(MPI_COMM_WORLD);

    /* *********************************************************************** */
    /* EVALUATE MERGED RESULTS AND CLEAN CHILD RESULTS */
    // Parent process evaluates results
    if (mpiModule.procId == 0) {
      // PARENT PROCESS
      // EVALUATE RESULTS
      if ( settings.runSamplesHeuristic || settings.hasExternalGt ) {

        MPI_INFO("Evaluating results so far...\n");

        // Re-alloc pairs for every result evaluation
        // This enables the memory exchange between the lastCooc vector and the vector of pairs
        PAIR* pairs = (PAIR*) calloc(graphInfo.nRelevantPairs, sizeof(PAIR));
        if (pairs == NULL) { MEM_ERROR; }

        if ( settings.hasExternalGt ) {
          startTimer(&extGtTimer);
          double externalPPV = calcPPV(&externalGt, pairs, &tmpResult,
                                       rankCurrentSample * mpiModule.numProcs);
          if ( externalPPV < 0 ) { FORWARD_ERROR; }
          MPI_INFO("Current external PPV is %lf.\n", externalPPV);
          if ( accElapsedTime(&extGtTimer) == FAILURE ) { FORWARD_ERROR; }
        }

        if ( settings.runSamplesHeuristic ) {
          startTimer(&sampleHeuTimer);
          double internalPPV = calcPPV(&internalGt, pairs, &tmpResult,
                                       rankCurrentSample * mpiModule.numProcs);
          if ( internalPPV < 0 ) { FORWARD_ERROR; }
          if ( internalPPV >= 0 && internalPPV <= 1 ) { // Possible range
            // Was not the first set of samples
            MPI_INFO("Current internal PPV is %lf.\n", internalPPV);
            if ( internalPPV >= settings.internalPpvThreshold ) {
              MPI_INFO("Internal PPV is above the threshold (%lf).\n",
                       settings.internalPpvThreshold);
              continueSamplingByHeuristic = FALSE;
            }
          }
          if ( accElapsedTime(&sampleHeuTimer) == FAILURE ) { FORWARD_ERROR; }
        }

        // Free pairs for after every result evaluation
        // This enables the memory exchange between the lastCooc vector and the vector of pairs
        free(pairs);
        pairs = NULL;

      }
    }
    // while child processes clear their results (already merged)
    else {
      // CHILD PROCESS
      for (uint row = 0; row < (graphInfo.nEvents - 1); row++) {
        for (uint col = 0; col < (graphInfo.nEvents - 1 - row); col++) {
          tmpResult.pValue[row][col] = 0;
          tmpResult.coocSum[row][col] = 0;
          tmpResult.coocSquareSum[row][col] = 0;
        }
      }
    }

    // Parent process broadcasts the decision whether or not to continue sampling
    MPI_Bcast(&continueSamplingByHeuristic, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    /* *********************************************************************** */
  }
  /*  END OF SAMPLING */
  /* *********************************************************************** */

  /* *********************************************************************** */
  /* PRINT OUT MEAN TIMINGS */
  if ( getTotalElapsedTime(&swapTimer) < 0 ) { FORWARD_ERROR; }
  MPI_INFO("Mean time per sample used swapping: %lf s\n",
           swapTimer.totalElapsedTime / rankCurrentSample);
  if ( getTotalElapsedTime(&coocTimer) < 0 ) { FORWARD_ERROR; }
  MPI_INFO("Mean time per sample used calculating co-occurrence: %lf s\n",
           coocTimer.totalElapsedTime / rankCurrentSample);

  if ( mpiModule.numProcs > 1 ) {
    if ( getTotalElapsedTime(&mergeTimer) < 0 ) { FORWARD_ERROR; }
    MPI_INFO("Mean time per sample used merging pre results: %lf s\n",
             mergeTimer.totalElapsedTime / rankCurrentSample);
  }

  if (mpiModule.procId == 0 && settings.hasExternalGt ) {
    // PARENT PROCESS
    if ( getTotalElapsedTime(&extGtTimer) < 0 ) { FORWARD_ERROR; }
    MPI_INFO("Mean time per sample used calculating external PPV: %lf s\n",
             extGtTimer.totalElapsedTime / rankCurrentSample);
  }
  if (mpiModule.procId == 0 && settings.runSamplesHeuristic ) {
    // PARENT PROCESS
    if ( getTotalElapsedTime(&sampleHeuTimer) < 0 ) { FORWARD_ERROR; }
    MPI_INFO("Mean time per sample used calculating internal PPV: %lf s\n",
             sampleHeuTimer.totalElapsedTime / rankCurrentSample);
  }
  /* *********************************************************************** */

  /* *********************************************************************** */
  /* CREATE OUTPUT FILE */
  if (mpiModule.procId == 0) {
    PAIR* pairs = (PAIR*) calloc(graphInfo.nRelevantPairs, sizeof(PAIR));
    if (pairs == NULL) { MEM_ERROR; }
    // Update total number of samples done
    settings.nSamples = rankCurrentSample * mpiModule.numProcs;
    resultList(pairs, &tmpResult, graphInfo.nEvents, settings.nSamples);
    MPI_INFO("Creating output file...\n");
    if ( createOutput(&tmpResult, pairs) == FAILURE ) { FORWARD_ERROR; }
    MPI_INFO("Output file created: %s\n", settings.outputFileName);
    free(pairs);
    pairs = NULL;
  }
  /* *********************************************************************** */

  /* *********************************************************************** */
  /* CLEAN UP */
  if (mpiModule.procId == 0) {
    MPI_INFO("Cleaning up...\n");
  }

  threadDeleteGraph(graph);

  if ( settings.runSamplesHeuristic ) {
    freeGT(&internalGt);
  }

  if ( settings.hasExternalGt ) {
    freeGT(&externalGt);
  }

  for (uint row = 0; row < graphInfo.nEvents; row++) {
    free(graphInfo.eventList[row]);
    graphInfo.eventList[row] = NULL;
  }

  free(graphInfo.eventList);
  graphInfo.eventList = NULL;

  threadRandFree(randGenerator);

  for (uint row = 0; row < (graphInfo.nEvents-1); row++) {
    free(graphInfo.originalCooc[row]);
    graphInfo.originalCooc[row] = NULL;

    free(tmpResult.pValue[row]);
    tmpResult.pValue[row] = NULL;

    free(tmpResult.coocSum[row]);
    tmpResult.coocSum[row] = NULL;

    free(tmpResult.coocSquareSum[row]);
    tmpResult.coocSquareSum[row] = NULL;
  }

  free(graphInfo.originalCooc);
  graphInfo.originalCooc = NULL;

  free(tmpResult.pValue);
  tmpResult.pValue = NULL;

  free(tmpResult.coocSum);
  tmpResult.coocSum = NULL;

  free(tmpResult.coocSquareSum);
  tmpResult.coocSquareSum = NULL;

  free(tmpResult.lastCooc);
  tmpResult.lastCooc = NULL;

  deleteGraphInfo(&graphInfo);

  // Destroy global lock if is forced sequential run
  #if FORCE_THREADWISE_SEQUENTIAL_RUN
  omp_destroy_lock(&forceSequentialLock);
  #endif
  /* *********************************************************************** */

  if (mpiModule.procId == 0) {
    MPI_INFO("Cleaned up!\n\n");
  }

  MPI_Barrier(MPI_COMM_WORLD);

  return SUCCESS;
}

void mpiFinalize (void)
{
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
}
