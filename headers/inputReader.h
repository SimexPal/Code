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


#ifndef INPUTREADER_H
#define INPUTREADER_H

#include <stdio.h>   /* gets */
#include <stdlib.h>  /* atoi, malloc */
#include <string.h>  /* strcpy */
#include "uthash/src/uthash.h"
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "compileTimeOptions.h"
#include "utils.h"
#include "timer.h"
#include "argParser.h"
#include "algorithm.h"

// Hash table structure - used to identify nodes without repetition
typedef struct node_hash_table {
    uint32_t id;                    /* key */
    char* nodeName;
    UT_hash_handle hh;         /* makes this structure hashable */
}NODE_HASH_TABLE;

// Hash table structure - used to identify edges without repetition
typedef struct edge_hash_table {
    uint32_t id;                    /* key */
    char* nodeName;
    UT_hash_handle hh;         /* makes this structure hashable */
}EDGE_HASH_TABLE;

// Hash table handlers
NODE_HASH_TABLE* addNodeToHashTable(NODE_HASH_TABLE* NodeGroup, uint* nodeID, char *nodeName);
EDGE_HASH_TABLE* addEdgeToHashTable(EDGE_HASH_TABLE* NodeGroup, uint* nodeID, char *nodeName);
void deleteAllHashTableItems(NODE_HASH_TABLE* NodeGroup);

// Linked list structure - used to read out infomation from hash table
// Intermediate step in creating the static vectors that represents the graph
typedef struct list LIST;
struct list {
  uint data;
  LIST *next;
};
// Linked list handlers
LIST* ListInit();
LIST* insertElementToList(LIST* begin, uint data);
LIST* insertNodeIDToList(LIST* begin, NODE_HASH_TABLE* nodeGroup, char* nodeName);
LIST* removeFirstElement(LIST* begin, uint* value);
uint listCountElements(LIST* begin);
void freeList(LIST* begin);

//Debugs
void printList(LIST* begin);

// Main input reader functions
bool readInputBipartite(GRAPH* g, char* inputFile);
bool readInputNonBipartite(GRAPH* g, char* inputFile);

bool writeBinaryGraph(GRAPH* g, char* inputFile);
bool readBinaryGraph(GRAPH* g, char* binFileName);

#endif
