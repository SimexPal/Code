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


#ifndef COMPILETIMEOPTIONS_H
#define COMPILETIMEOPTIONS_H

#include <limits.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0

#define SUCCESS 1
#define FAILURE 0

// Default settings
#define DEFAULT_APPENDRUNINFO TRUE
#define DEFAULT_INCLUDEGTMISSINGNODES FALSE
#define DEFAULT_IGNOREGTMISSINGNODES FALSE
#define DEFAULT_ISBIPARTITEGRAPH TRUE
#define DEFAULT_BIPARTITESIDEOFINTEREST 'l'
#define DEFAULT_WRITEBINARYGRAPH FALSE
#define DEFAULT_DIRECTEDGECOOCVALUE 1
#define DEFAULT_NSWAPS 0
#define DEFAULT_ELNESWAPS FALSE
#define DEFAULT_RUNCURVEBALL TRUE
#define DEFAULT_RUNSWAPHEURISTIC TRUE
#define DEFAULT_NDEGREESSWAPHEURISTIC 9
#define DEFAULT_NEVENTSPERDEGREESWAPHEURISTIC 6
#define DEFAULT_THRESHOLDTHETA 5e-3
#define DEFAULT_NSAMPLES 0
#define DEFAULT_RUNSAMPLESHEURISTIC TRUE
#define DEFAULT_RATIOGTPAIRSPERRESULTPAIR 2e-3
#define DEFAULT_INTERNALPPVTHRESHOLD 0.95
#define DEFAULT_NMAXSAMPLES 10000
#define DEFAULT_DATESTR ""
#define DEFAULT_RUNINDEX ""
#define DEFAULT_MINRELEVANTCOOC 1

// Maximum length of file path/name string
#define MAX_FILEPATH_SIZE 2048
#define MAX_FILENAME_SIZE 2048

// Maximum length of date string
#define MAX_DATE_STR_SIZE 1024

// Maximum #chars for a long int
#define MAX_INT_STR_SIZE 21 

// Maximum string length of a node of the graph
#define MAX_NODE_STRING_LENGTH 15

// Maximum string length of an edge (node_left|node_right) of the graph
#define MAX_EDGE_STRING_LENGTH (MAX_NODE_STRING_LENGTH*2)

// The program, in particular co-occurrance computation,
// is currently implemented expecting 16 threads
// DO NOT CHANGE unless you are sure about it!!
// TODO: Change to (omp_get_max_threads()) once cooc calc is generalized
#define NUMBER_OF_THREADS 16 

// Number of subblocks used from co-occurence calculation
// DO NOT CHANGE - this must be in accordance to the number of threads impletented
// #Threads >= (#Subblocks)(#Subblocks + 1)/2 
// because every combination of two subblocks must be handled by a unique thread
#define NUMBER_OF_SUBBLOCKS 5

// Definition of a block of bits and its length
// DO NOT exceed uintmax_t (uint64_t in the current architecture)
typedef uint64_t BLOCK;
// Number of bits in one block
//  sizeof(x) return the number of CHAR_BIT's
//  CHAR_BIT is the number of bits in one byte (char) for the given architecture
#define BITS_PER_BLOCK ((size_t) sizeof(BLOCK) * CHAR_BIT)

// Avoid NaN and Inf zScores
#define AVOID_NAN_AND_INF_ZSCORE TRUE

/******************** DEGUB/PRINT OUT FLAGS ********************/
#define PRINT_GRAPH_AS_ADJACENCY_MATRIX_ORIGINAL FALSE
#define ORIGINAL_ADJACENCY_MATRIX_FILE_NAME "originalAdjMatrix.dbg"
#define PRINT_GRAPH_AS_ADJACENCY_MATRIX_EACH_SWAP FALSE
#define SAMPLES_ADJACENCY_MATRICES_FILE_NAME "swapsAdjMatrix_thread_"

#define PRINT_GRAPH_AS_ACTOR_ADJACENCY_LIST_ORIGINAL FALSE
#define ORIGINAL_ADJACENCY_LIST_FILE_NAME "originalAdjList.dbg"
#define PRINT_GRAPH_AS_ACTOR_ADJACENCY_LIST_EACH_SWAP FALSE
#define SWAPS_ADJACENCY_LISTS_FILE_NAME "swapsAdjList_thread_"
#define PRINT_CURVEBALL_STEPS FALSE

#define PRINT_LIST_OF_NODES_OF_INTEREST FALSE

#define PRINT_SHUFFLE_STEPS FALSE

#define PRINT_THREADS_COMPUTING_COOCCURRENCE FALSE

#define PRINT_EVENT_SELECTION_SWAP_HEURISTIC FALSE
#define PRINT_INFO_ABOUT_SWAP_HEURISTIC_STEPS FALSE
#define PRINT_THETA_FOR_EACH_SAMPLE_SET FALSE

#define PRINT_SUB_BLOCKS_INDEXING_INFO FALSE

#define PRINT_PAIR_LISTS_FOR_SAMPLE_HEURISTIC FALSE

#define PRINT_PAIR_LISTS_FOR_EXTERNAL_GROUNTTRUTH FALSE

#define TEST_GRAPH TRUE
#define TEST_GRAPH_EACH_SWAP FALSE

#define FORCE_THREADWISE_SEQUENTIAL_RUN FALSE
#define PRINT_THREADWISE_SEQUENTIAL_RUN_VERBOSE FALSE

#endif // COMPILETIMEOPTIONS_H
