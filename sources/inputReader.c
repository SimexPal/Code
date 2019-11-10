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


#include "../headers/inputReader.h"

/*************** HASH TABLE FUNCTIONS ***************/

/** \fn addNodeToHashTable
* Add a new node to @nodeGroup hash table, only if @nodeName is not yet contained.
*  In case @nodeName is already part of @nodeGroup, does nothing.
*  In case @nodeName is not yet part of @nodeGroup, assign a new key (@*id) to it.
* @param nodeGroup Hash table structure handled by [uthash.h](http://troydhanson.github.io/uthash)
* @param *id       Pointer to current key to be assigned to a new value
* @param nodeName[]    String which contains the value to be hashed if it is not yet done
**/
NODE_HASH_TABLE* addNodeToHashTable(NODE_HASH_TABLE* nodeGroup, uint* id, char nodeName[]) {
  NODE_HASH_TABLE* found = NULL; // Create a dummy item to check uniqueness
  HASH_FIND_STR(nodeGroup,nodeName,found); // Check whether @nodeName is a value of @nodeGroup,
                                           // if not @found gets NULL.
  if (!found) { // If @found is NULL, @nodeName is not yet a value of @nodeGroup
    NODE_HASH_TABLE* newItem; // Create and allocate a new item for the @nodeGroup table
    newItem = (NODE_HASH_TABLE*)calloc(1, sizeof(NODE_HASH_TABLE));
    if ( newItem == NULL ) { MEM_ERROR_V(NULL); }
    newItem->nodeName = (char*)calloc(strlen(nodeName)+1, sizeof(char));
    if ( newItem->nodeName == NULL ) { MEM_ERROR_V(NULL); }
    strcpy(newItem->nodeName, nodeName); // Copy @nodeName to new item's value
    newItem->id = *id; // Copy @*id to new item's key
    HASH_ADD_STR(nodeGroup,nodeName,newItem); // Add new item, with value == @nodeName to @nodeGroup
    (*id)++; // Increment @*id ensuring key are always different from each other.
    /* NOTE: After the increment @*id contains the number of items on @nodeGroup */
  }
  return nodeGroup; // Return new pointer to the hash structure
}

/** \fn addEdgeToHashTable
* Add a new edges to @nodeGroup hash table, only if @nodeName is not yet contained.
*  In case @nodeName is already part of @nodeGroup, throws an error.
*  In case @nodeName is not yet part of @nodeGroup, assign a new key (@*id) to it.
* @param nodeGroup Hash table structure handled by [uthash.h](http://troydhanson.github.io/uthash)
* @param *id       Pointer to current key to be assigned to a new value
* @param nodeName[]    String which contains the value to be hashed if it is not yet done
**/
EDGE_HASH_TABLE* addEdgeToHashTable(EDGE_HASH_TABLE* nodeGroup, uint* id, char nodeName[]) {
  EDGE_HASH_TABLE* found = NULL; // Create a dummy item to check uniqueness
  HASH_FIND_STR(nodeGroup,nodeName,found); // Check whether @nodeName is a value of @nodeGroup,
                                           // if not @found gets NULL.

  if (!found) { // If @found is NULL, @nodeName is not yet a value of @nodeGroup
    EDGE_HASH_TABLE* newItem; // Create and allocate a new item for the @nodeGroup table
    newItem = (EDGE_HASH_TABLE*)calloc(1, sizeof(EDGE_HASH_TABLE));
    if ( newItem == NULL ) { MEM_ERROR_V(NULL); }
    newItem->nodeName = (char*)calloc(strlen(nodeName)+1, sizeof(char));
    if ( newItem->nodeName == NULL ) { MEM_ERROR_V(NULL); }
    newItem->nodeName = (char*)calloc(strlen(nodeName)+1, sizeof(char));
    strcpy(newItem->nodeName, nodeName); // Copy @nodeName to new item's value
    newItem->id = *id; // Copy @*id to new item's key
    HASH_ADD_STR(nodeGroup,nodeName,newItem); // Add new item, with value == @nodeName to @nodeGroup
    (*id)++; // Increment @*id ensuring key are always different from each other.
    /* NOTE: After the increment @*id contains the number of items on @nodeGroup */
  }
  else { // If @found is not NULL, meaning a try to add an already existing edge, throws an error.
    uint firstAppearanceLine, secondAppearanceLine;
    if ( settings.isBipartiteGraph == TRUE ) { //If graph is bipartite, each line uses one id
        firstAppearanceLine = found->id + 1;
        secondAppearanceLine = *id + 1;
    } else { //If graph is nonbipartite, each line uses two ids (a->b and b->a)
        firstAppearanceLine = (found->id + 1 + 1) / 2;
        secondAppearanceLine = (*id + 1 + 1) / 2;
    }
    STDERR_INFO("Found repeating edge: (%s)\n"
                "First appearence at line %u.\n"
                "Now found again at line %u.",
                nodeName,
                firstAppearanceLine,
                secondAppearanceLine);
    RETURN_ERROR_V(NULL);
  }
  return nodeGroup; // Return new pointer to the hash structure
}

/** \fn deleteAllHashTableItems
* Iterate through the hash table, deleting and freeing each hash item and its structure
* @param nodeGroup Hash table structure handled by [uthash.h](http://troydhanson.github.io/uthash)
**/
void deleteAllHashTableItems_nodes(NODE_HASH_TABLE* nodeGroup) {
  NODE_HASH_TABLE *current_item, *tmp; // Create a pointer to the current item to be deleted and freed and a dummy one
  HASH_ITER(hh, nodeGroup, current_item, tmp) { // Iterate through @nodeGroup hash table on @current_item
    HASH_DEL(nodeGroup,current_item);   // delete @current_item from @nodeGroup (and advance to next item)
    free(current_item->nodeName);
    free(current_item);               // free @current_item
  }
}
void deleteAllHashTableItems_edges(EDGE_HASH_TABLE* nodeGroup) {
  EDGE_HASH_TABLE *current_item, *tmp; // Create a pointer to the current item to be deleted and freed and a dummy one
  HASH_ITER(hh, nodeGroup, current_item, tmp) { // Iterate through @nodeGroup hash table on @current_item
    HASH_DEL(nodeGroup,current_item);   // delete @current_item from @nodeGroup (and advance to next item)
    free(current_item->nodeName);
    free(current_item);               // free @current_item
  }
}

/*************** LINKED LIST FUNCTIONS ***************/
/** \fn ListInit
* Initialize the linked list
**/
LIST* ListInit(){
  return NULL;
}

/** \fn insertElementToList
* Insert new element to the begining of the linked list
* @param begin Pointer to the first element of the linked list
* @param data Data to be inserted as an element of the linked list
**/
LIST* insertElementToList(LIST* begin, uint data) {
  LIST *newElem; // Create new element pointer
  if ( (newElem = calloc(1, sizeof(LIST))) ) { // Try to allocate a new element
    newElem->data = data; // Tranfer the data to the element
    if(begin==NULL) {// If the linked list is empty
      newElem->next = NULL; // The new (and first) element points to the end (first is also the last)
    }
    else {
      newElem->next = begin; // Otherwise, new element is inserted in front of the first one
    }
  }
  else { // If allocation fails (calloc return NULL), prints error message and exit the program
    MEM_ERROR_V(NULL);
  }
  return newElem; // Return the new pointer to the first element of the linked list
}

/** \fn removeFirstElement
* Remove the first element of the linked list and assign its data to @value
* @param begin Pointer to the first element of the linked list
* @param value Pointer to variable which will have the first element's data assign to
**/
LIST* removeFirstElement(LIST* begin, uint* value) {
  LIST* tmp = begin; // Create a temporary pointer pointing to the begining of the linked list
  *value = tmp->data; // Assign the data of the first element to @value
  begin = begin->next; // Redefine the first element of the linked list
  free(tmp); // Free the former first element of the linked list
  return begin; // Return a pointer to the new first element of the linked list
}

/** \fn listCountElements
* Count the number of elements contained in the linked list
* @param begin Pointer to the first element of the linked list
**/
uint listCountElements(LIST* begin){
  LIST* tmp = begin; // Create a temporary pointer to iterate through the linked list
  uint numberOfElements = 0; // Initialize the number of elements counter
  while(tmp != NULL){ // Iterate through the linked list until reaches its end
    tmp = tmp->next; // Move to next element
    numberOfElements++; // Increment the number of elements counter each step
  }
  return numberOfElements; // Return the number of elements
}

/** \fn printList
* Print out each element's data in the linked list
* @param begin Pointer to the first element of the linked list
**/
void printList(LIST* begin){
  LIST* tmp = begin; // Create a temporary pointer to iterate through the linked list
  while(tmp != NULL){ // Iterate through the linked list until reaches its end
    fprintf(stdout, "%u \n", tmp->data); // Print out the current element's data
    tmp = tmp->next;
  }
}

/** \fn insertNodeIDToList
* Add a new element to the linked list.
*  In case @nodeName is NOT part of @nodeGroup, return an error.
* @param begin Pointer to the first element of the linked list
* @param nodeGroup  Pointer to the hashable structure which should contain @nodeName
* @param nodeName[] String which contains the value which @id will be included to linked list
**/
LIST* insertNodeIDToList(LIST* begin, NODE_HASH_TABLE* nodeGroup, char nodeName[]){
  NODE_HASH_TABLE* found;
  HASH_FIND_STR(nodeGroup, nodeName, found);
  if(found != NULL) {
    begin = insertElementToList(begin,found->id);
    if ( begin == NULL ) { FORWARD_ERROR_V(NULL); }
  }
  else {
    STDERR_INFO( "Unexpected behaviour: "
                 "node \"%s\" not found in hash table!", nodeName);
    RETURN_ERROR_V(NULL);
  }
  return begin;
}

/** \fn freeList
* Free the whole list from memory
* @param begin Pointer to the first element of the linked list
**/
void freeList(LIST* begin){
  LIST* tmp; // Create a temporary pointer to iterate through the linked list
  while(begin != NULL){ // Iterate through the linked list until reaches its end
    tmp = begin; // Save the location of former first element
    begin = begin->next; // Move to next element, the new first element
    free(tmp); // Free the former first element
  }
}



/** \fn readInputBipartite
* Read the input file containing the data set and fill the graph structure
* The input file such be an ASCII file where each line represents an edge (link between two nodes)
* The node names should be separated with a whitespace
* The left side nodes are the nodes of interest, with which the weighted output graph will be generated
* The right side nodes should be of no interest to the user
* Each different string represents a new node to its partition
* The same string in different sides will be considered two different nodes
**/
bool readInputBipartite(GRAPH* g, char* inputFile){

  if ( settings.isBinaryInput ) {
    if ( readBinaryGraph(g, inputFile) == FAILURE ) { FORWARD_ERROR; }
    return SUCCESS;
  } else {
    // TODO: new function for reading text form graph
  }

  // Initialize/clear graphInfo
  initGraphInfo(&graphInfo);

  FILE* inputFileStream = fopen(inputFile,"r");
  if( inputFileStream == NULL ) {
    STDERR_INFO( "%s: could not read.\n", inputFile);
    FORWARD_ERROR;
  }


  graphInfo.nEdges = 0;
  graphInfo.maxNodeStrLenght = 0;
  NODE_HASH_TABLE *actors = NULL; // Actors are the non interensting nodes
  NODE_HASH_TABLE *events = NULL; // Events are the nodes of interest
  EDGE_HASH_TABLE *edges  = NULL; // Temporary table to check whether there is a repeating edge in the input file
  char* rightSideStr = NULL;
  size_t rhBufferSize = 0;
  size_t rhStrSize = 0;
  char* leftSideStr = NULL;
  size_t lhBufferSize = 0;
  size_t lhStrSize = 0;
  char* actorSideStr = NULL;
  char* eventSideStr = NULL;
  char* edgeStr = NULL;
  uint eventId = 0;
  uint actorId = 0;
  uint edgeId = 0;

  /*read the file once to get the set of actor and event node*/
  size_t lineCounter = 0;
  while( (lhStrSize = getdelim(&leftSideStr, &lhBufferSize, ' ', inputFileStream)) != -1 ) {
    ++lineCounter;

    rhStrSize = getdelim(&rightSideStr, &rhBufferSize, '\n', inputFileStream);
    if ( rhStrSize == -1 ) {
      STDERR_INFO("Unable to read right-hand side of line %zu.", lineCounter);
      RETURN_ERROR;
    }
    leftSideStr[lhStrSize-1] = '\0'; // Substitute delimiter with NULL char
    rightSideStr[rhStrSize-1] = '\0'; // Substitute delimiter with NULL char

    if ( settings.bipartiteSideOfInterest == 'l' ) {
        eventSideStr = leftSideStr;
        actorSideStr = rightSideStr;
    } else if ( settings.bipartiteSideOfInterest == 'r' ) {
        eventSideStr = rightSideStr;
        actorSideStr = leftSideStr;
    } else {
        STDERR_INFO( "Unexpected behaviour: "
                     "could not decide on the side of interest!\n"
                     "It was given %c.", settings.bipartiteSideOfInterest);
        RETURN_ERROR;
    }


    actors = addNodeToHashTable(actors, &actorId, actorSideStr);
    if ( actors == NULL ) { FORWARD_ERROR; }
    events = addNodeToHashTable(events, &eventId, eventSideStr);
    if ( events == NULL ) { FORWARD_ERROR; }

    edgeStr = (char*)realloc(edgeStr,
          (strlen(leftSideStr) + strlen(rightSideStr) + 2) * sizeof(char));
    strcpy(edgeStr, leftSideStr); // Create an string representing the edge
    strcat(edgeStr, "\t");        // by concatenating the two node strings
    strcat(edgeStr, rightSideStr);// separeted by an whitespace (tab character)
    edges = addEdgeToHashTable(edges, &edgeId, edgeStr);
    if ( edges == NULL ) { FORWARD_ERROR; }
    // Find the length of the largest side-of-interest string
    if (strlen(eventSideStr) > graphInfo.maxNodeStrLenght) {
      graphInfo.maxNodeStrLenght = strlen(leftSideStr);
    }
  }
  graphInfo.maxNodeStrLenght += 1; //One extra for the end-of-string (\0) character
  free(edgeStr);
  deleteAllHashTableItems_edges(edges); // Delete the edges hash table, since it is only used to check the uniqueness of each edge

  graphInfo.nEdges  = edgeId; // store the number of edges
  graphInfo.nActors = actorId; // store number of actors
  graphInfo.nEvents = eventId; // store number of events
  graphInfo.nPairs  = graphInfo.nEvents*(graphInfo.nEvents-1)/2; // store the number of pairs of events

  graphInfo.nBlocksPerEvent = graphInfo.nActors / BITS_PER_BLOCK;
  graphInfo.nBlocksPerEvent += graphInfo.nActors % BITS_PER_BLOCK ? 1 : 0;
  graphInfo.nBlocksAdjMatrix =
      (ulint) graphInfo.nBlocksPerEvent * graphInfo.nEvents;

  // Allocate memory for graph and graph information
  if ( allocGraph(g) == FAILURE ) { FORWARD_ERROR; }
  if ( allocGraphInfo(&graphInfo) == FAILURE ) { FORWARD_ERROR; }


  // Create one linked list per actor
  LIST* actorTmpAdjLists[graphInfo.nActors];
  for(uint actorIt = 0; actorIt < graphInfo.nActors; actorIt++) { //initialize the array of Lists
    actorTmpAdjLists[actorIt] = ListInit();
  }

  rewind(inputFileStream);  //rewind the file pointer in order to read the file again

  /* read the file again and extract edges, store them in a temporary adjacency list*/
  NODE_HASH_TABLE* found; // Intermediary hashable struct
  while( (lhStrSize = getdelim(&leftSideStr, &lhBufferSize, ' ', inputFileStream)) != -1 ) {
    ++lineCounter;

    rhStrSize = getdelim(&rightSideStr, &rhBufferSize, '\n', inputFileStream);
    if ( rhStrSize == -1 ) {
      STDERR_INFO("Unable to read right-hand side of line %zu.", lineCounter);
      RETURN_ERROR;
    }
    leftSideStr[lhStrSize-1] = '\0'; // Substitute delimiter with NULL char
    rightSideStr[rhStrSize-1] = '\0'; // Substitute delimiter with NULL char

    // Not needed, but keeping for clarity and robustness
    if ( settings.bipartiteSideOfInterest == 'l' ) {
        eventSideStr = leftSideStr;
        actorSideStr = rightSideStr;
    } else if ( settings.bipartiteSideOfInterest == 'r' ) {
        eventSideStr = rightSideStr;
        actorSideStr = leftSideStr;
    } else {
        STDERR_INFO( "Unexpected behaviour: "
                     "could not decide on the side of interest!\n"
                     "It was given %c.", settings.bipartiteSideOfInterest);
        RETURN_ERROR;
    }

    HASH_FIND_STR(actors, actorSideStr, found); // @found points to item with value == @eventSideStr
    if(found != NULL) {
      // add the event id to the actor list
      actorTmpAdjLists[found->id] =
          insertNodeIDToList(actorTmpAdjLists[found->id], events, eventSideStr);
      if ( actorTmpAdjLists[found->id] == FAILURE ) { FORWARD_ERROR; }
    }
    else {
      STDERR_INFO("Unexpected behaviour: no node with value '%s' "
                  "in hash table.", actorSideStr);
      RETURN_ERROR;
    }
    found=NULL;

    HASH_FIND_STR(events, eventSideStr, found);
    if(found != NULL) { // @found points to item with value == @eventSideStr
      // store the the name of the side-of-interest nodes
      strcpy(graphInfo.eventList[found->id], eventSideStr);
    }
    else {
      STDERR_INFO("Unexpected behaviour: no node with value '%s' "
                  "in hash table.", eventSideStr);
      RETURN_ERROR;
    }
    found=NULL;
  }

  // Move edges from linked lists to array based actor lists
  // First position of the accumulated degrees array should be zero
  g->actorAccumulatedDegrees[0] = 0;
  // Calculate total graph co-occurrence
  graphInfo.coocSum = 0;
  for(uint actorIt = 0; actorIt < graphInfo.nActors; actorIt++) {
    uint listLength = (uint) listCountElements(actorTmpAdjLists[actorIt]);
    // Position x+1 of @actorAccumulatedDegrees contains
    //  the sum of the degrees of all nodes before node x, including its own.
    g->actorAccumulatedDegrees[actorIt+1] = g->actorAccumulatedDegrees[actorIt]
              + listLength;

    for(uint edgeIt = g->actorAccumulatedDegrees[actorIt];
        edgeIt < g->actorAccumulatedDegrees[actorIt + 1]; edgeIt++) {
      actorTmpAdjLists[actorIt] =
          removeFirstElement(actorTmpAdjLists[actorIt], &eventId);
      g->actorAdjLists[edgeIt] = eventId;
      g->actorEdgeMaps[edgeIt] = actorIt;
    }

    graphInfo.coocSum += (listLength * (listLength-1)) / 2;
  }

  // Build adj. matrix
  adjMatrixFromAdjLists( g );

  // Force canonical form of graph
  if ( canonizeGraph(g) == FAILURE ) { FORWARD_ERROR; }

  fclose(inputFileStream);
  deleteAllHashTableItems_nodes(actors);
  deleteAllHashTableItems_nodes(events);

  #if PRINT_GRAPH_AS_ADJACENCY_MATRIX_ORIGINAL
    printAdjMatrix(g, ORIGINAL_ADJACENCY_MATRIX_FILE_NAME);
  #endif
  #if PRINT_GRAPH_AS_ACTOR_ADJACENCY_LIST_ORIGINAL
    printActorAdjLists(g, ORIGINAL_ADJACENCY_LIST_FILE_NAME);
    printActorAccumulatedDegrees(g, ORIGINAL_ADJACENCY_LIST_FILE_NAME);
    printActorEdgeMap(g, ORIGINAL_ADJACENCY_LIST_FILE_NAME);
  #endif

  #if PRINT_LIST_OF_NODES_OF_INTEREST
    printEventList();
  #endif

  #if TEST_GRAPH
    if ( initOriginalNodeDegrees(g) == FAILURE ) { FORWARD_ERROR; }
    if ( graphTest(g) == FAILURE ) { FORWARD_ERROR; }
  #endif

  return SUCCESS;
}

bool readInputNonBipartite(GRAPH* g, char* inputFile){

  if ( settings.isBinaryInput ) {
    if ( readBinaryGraph(g, inputFile) == FAILURE ) { FORWARD_ERROR; }
    return SUCCESS;
  } else {
    // TODO: new function for reading text form graph
  }

  // Initialize/clear graphInfo
  initGraphInfo(&graphInfo);

  FILE* inputFileStream = fopen(inputFile,"r");
  if(inputFileStream==NULL) {
    STDERR_INFO( "%s: doesn't exist.\n", inputFile);
    RETURN_ERROR;
  }

  graphInfo.nEdges = 0;
  graphInfo.maxNodeStrLenght = 0;
  NODE_HASH_TABLE *events = NULL; // Events are the nodes of interest
  EDGE_HASH_TABLE *edges  = NULL; // Temporary table to check whether there is a repeating edge in the input file
  char* rightSideStr = NULL;
  size_t rhBufferSize = 0;
  size_t rhStrSize = 0;
  char* leftSideStr = NULL;
  size_t lhBufferSize = 0;
  size_t lhStrSize = 0;
  char* edgeStr = NULL;
  uint eventId = 0;
  uint edgeId = 0;

  /*read the file once to get the set of nodes (events)*/
  size_t lineCounter = 0;
  while( (lhStrSize = getdelim(&leftSideStr, &lhBufferSize, ' ', inputFileStream)) != -1 ) {
    ++lineCounter;

    rhStrSize = getdelim(&rightSideStr, &rhBufferSize, '\n', inputFileStream);
    if ( rhStrSize == -1 ) {
      STDERR_INFO("Unable to read right-hand side of line %zu.", lineCounter);
      RETURN_ERROR;
    }
    leftSideStr[lhStrSize-1] = '\0'; // Substitute delimiter with NULL char
    rightSideStr[rhStrSize-1] = '\0'; // Substitute delimiter with NULL char

    events = addNodeToHashTable(events, &eventId, leftSideStr);
    if ( events == NULL ) { FORWARD_ERROR; }

    events = addNodeToHashTable(events, &eventId, rightSideStr);
    if ( events == NULL ) { FORWARD_ERROR; }

    edgeStr = (char*)realloc(edgeStr,
          (strlen(leftSideStr) + strlen(rightSideStr) + 2) * sizeof(char));
    strcpy(edgeStr, leftSideStr); // Create an string representing the edge
    strcat(edgeStr, "\t");        // by concatenating the two node strings
    strcat(edgeStr, rightSideStr);// separeted by an whitespace (tab character)
    edges = addEdgeToHashTable(edges, &edgeId, edgeStr); // Add the new edge to the edge list
    if ( edges == NULL ) { FORWARD_ERROR; }

    strcpy(edgeStr, rightSideStr); // Do the same procedure, but invert
    strcat(edgeStr, "\t");        //  both strings. In non-bipartite graph,
    strcat(edgeStr, leftSideStr);//   it is assumed that the graph is also undirected.
    edges = addEdgeToHashTable(edges, &edgeId, edgeStr);
    if ( edges == NULL ) { FORWARD_ERROR; }

    // Find the length of the largest string
    if ( (strlen(rightSideStr) > graphInfo.maxNodeStrLenght) ||
         (strlen(leftSideStr) > graphInfo.maxNodeStrLenght)   ) {
      graphInfo.maxNodeStrLenght = strlen(leftSideStr);
    }
  }
  graphInfo.maxNodeStrLenght += 1; //One extra for the end-of-string (\0) character
  free(edgeStr);
  deleteAllHashTableItems_edges(edges); // Delete the edges hash table, since it is only used to check the uniqueness of each edge

  graphInfo.nEdges  = edgeId; // store the number of edges
  graphInfo.nEvents = eventId; // store number of events
  graphInfo.nActors = eventId; // store number of actors, which is equal as the number of events for non-bipartite graphs.
  graphInfo.nPairs  = graphInfo.nEvents*(graphInfo.nEvents-1)/2; // store the number of pairs of events

  graphInfo.nBlocksPerEvent = graphInfo.nActors / BITS_PER_BLOCK;
  graphInfo.nBlocksPerEvent += graphInfo.nActors % BITS_PER_BLOCK ? 1 : 0;
  graphInfo.nBlocksAdjMatrix =
      (ulint) graphInfo.nBlocksPerEvent * graphInfo.nEvents;

  // Allocate memory for graph and graph information
  if ( allocGraph(g) == FAILURE ) { FORWARD_ERROR; }
  if ( allocGraphInfo(&graphInfo) == FAILURE ) { FORWARD_ERROR; }


  // Create one linked list per node
  LIST* actorTmpAdjLists[graphInfo.nEvents];
  for(uint actorIt = 0; actorIt < graphInfo.nActors; actorIt++) { //initialize the array of Lists
    actorTmpAdjLists[actorIt] = ListInit();
  }


  rewind(inputFileStream);  //rewind the file pointer in order to read the file again

  /* read the file again and extract edges, store them in a temporary adjacency list*/
  // Edges are stored in a redundant way, being stored both a -> b and a <- b
  // This allows the reuse of the co-occurence calculation method used for bipartite graph,
  // which have, so far, proved to be the fastest one, specially in very sparse graphs.
  // However, it obviously requires double the memory. In this context, improvements may be done.
  NODE_HASH_TABLE* found; // Intermediary hashable struct
  while( (lhStrSize = getdelim(&leftSideStr, &lhBufferSize, ' ', inputFileStream)) != -1 ) {
    ++lineCounter;

    rhStrSize = getdelim(&rightSideStr, &rhBufferSize, '\n', inputFileStream);
    if ( rhStrSize == -1 ) {
      STDERR_INFO("Unable to read right-hand side of line %zu.", lineCounter);
      FORWARD_ERROR;
    }
    leftSideStr[lhStrSize-1] = '\0'; // Substitute delimiter with NULL char
    rightSideStr[rhStrSize-1] = '\0'; // Substitute delimiter with NULL char

    HASH_FIND_STR(events, leftSideStr, found);
    if(found != NULL) {
      // add the right side node id to the left side node list
      actorTmpAdjLists[found->id] =
          insertNodeIDToList(actorTmpAdjLists[found->id], events, rightSideStr);
      if ( actorTmpAdjLists[found->id] == FAILURE ) { FORWARD_ERROR; }
      // add the left side node name to the list of nodes names
      strcpy(graphInfo.eventList[found->id], leftSideStr);
    }
    else {
      STDERR_INFO("Unexpected behaviour: no node with value '%s' "
                  "in hash table.", leftSideStr);
      RETURN_ERROR;
    }
    found=NULL;

    HASH_FIND_STR(events, rightSideStr, found);
    if(found != NULL) {
      // add the left side node id to the right side node list
      actorTmpAdjLists[found->id] =
          insertNodeIDToList(actorTmpAdjLists[found->id], events, leftSideStr);
      if ( actorTmpAdjLists[found->id] == FAILURE ) { FORWARD_ERROR; }
      // add the right side node name to the list of  nodes names
      strcpy(graphInfo.eventList[found->id], rightSideStr);
    }
    else {
      STDERR_INFO("Unexpected behaviour: no node with value '%s' "
                  "in hash table.", rightSideStr);
      RETURN_ERROR;
    }
    found=NULL;
  }

  // Move edges from linked lists to array based actor lists
  // First position of the accumulated degrees array should be zero
  g->actorAccumulatedDegrees[0] = 0;
  // Calculate total graph co-occurrence
  graphInfo.coocSum = 0;
  for(uint actorIt = 0; actorIt < graphInfo.nActors; actorIt++) {
    uint listLength = (uint) listCountElements(actorTmpAdjLists[actorIt]);
    // Position x+1 of @actorAccumulatedDegrees contains
    //  the sum of the degrees of all nodes before node x, including its own.
    g->actorAccumulatedDegrees[actorIt+1] = g->actorAccumulatedDegrees[actorIt]
              + listLength;

    for(uint edgeIt = g->actorAccumulatedDegrees[actorIt];
        edgeIt < g->actorAccumulatedDegrees[actorIt + 1]; edgeIt++) {
      actorTmpAdjLists[actorIt] =
          removeFirstElement(actorTmpAdjLists[actorIt], &eventId);
      g->actorAdjLists[edgeIt] = eventId;
      g->actorEdgeMaps[edgeIt] = actorIt;
    }

    graphInfo.coocSum += (listLength * (listLength-1)) / 2;
  }
  // Additional co-occurence from direct edge (a->b) -> cooc(a,b) += directEdgeV
  graphInfo.coocSum += (graphInfo.nEdges * settings.directEdgeCoocValue) / 2;

  // Build adj. matrix (with redundance)
  adjMatrixFromAdjLists( g );

  // Force canonical form of graph
  if ( canonizeGraph(g) == FAILURE ) { FORWARD_ERROR; }

  fclose(inputFileStream);
  deleteAllHashTableItems_nodes(events);

  #if PRINT_GRAPH_AS_ADJACENCY_MATRIX_ORIGINAL
    printAdjMatrix(g, ORIGINAL_ADJACENCY_MATRIX_FILE_NAME);
  #endif
  #if PRINT_GRAPH_AS_ACTOR_ADJACENCY_LIST_ORIGINAL
    printActorAdjLists(g, ORIGINAL_ADJACENCY_LIST_FILE_NAME);
    printActorAccumulatedDegrees(g, ORIGINAL_ADJACENCY_LIST_FILE_NAME);
    printActorEdgeMap(g, ORIGINAL_ADJACENCY_LIST_FILE_NAME);
    printEdgeLinks(g, ORIGINAL_ADJACENCY_LIST_FILE_NAME);
  #endif

  #if PRINT_LIST_OF_NODES_OF_INTEREST
    printEventList();
  #endif

  #if TEST_GRAPH
    if ( initOriginalNodeDegrees(g) == FAILURE ) { FORWARD_ERROR; }
    if ( graphTest(g) == FAILURE ) { FORWARD_ERROR; }
  #endif


  return SUCCESS;
}

bool writeBinaryGraph(GRAPH* g, char* inputFile)
{
  // The input data set representation written in the binary file must have:
  // First sizeof(uint) bytes: sizeof(GRAPHINFO) -- compatibility check
  // Followed by sizeof(GRAPHINFO) bytes: graphInfo struct
  // Followed by a single character 'b' or 'n': type of graph - [non]bipartite
  // Followed by nEvents strings of size maxNodeStrLenght: the events' names
  // Finally followed by nBlocksAdjMatrix BLOCKs: the binary adjacency matrix

  char graphBinFileName[MAX_FILEPATH_SIZE + MAX_FILENAME_SIZE];
  strcpy(graphBinFileName, inputFile);
  char* ext = getFilenameExt(graphBinFileName);
  strcpy(ext, ".gbin");

  FILE* graphBinFile = fopen(graphBinFileName, "w");
  if ( graphBinFile == NULL ) {
    STDERR_INFO("Unable to create graph binary file: %s\n"
                "Please check if you have write permission there.",
                graphBinFileName);
    RETURN_ERROR;
  }


  // Write a uint with the sizeof(GRAPHINFO) of current version
  uint sizeGInfo = sizeof(GRAPHINFO);
  fwrite(&sizeGInfo, sizeof sizeGInfo, 1, graphBinFile);

  // Write graphInfo as "header" of binary graph file
  // But first throw away memory addresses of allocated blocks -- NEEDED
  GRAPHINFO tmpGInfo;
  initGraphInfo(&tmpGInfo);
  copyGraphInfoValues(&tmpGInfo, &graphInfo);
  fwrite(&tmpGInfo, sizeof tmpGInfo, 1, graphBinFile);

  // Write 'b' for bipartite or 'n' for nonbipartite
  char graphType = settings.isBipartiteGraph ? 'b' : 'n';
  fwrite(&graphType, sizeof graphType, 1, graphBinFile);

  // Write each event name
  for (uint eventIt=0; eventIt < graphInfo.nEvents; ++eventIt) {
    fwrite(graphInfo.eventList[eventIt],
           sizeof **graphInfo.eventList,
           graphInfo.maxNodeStrLenght, graphBinFile);
  }

  // Write adjacency matrix
  fwrite(g->adjMatrix, sizeof *g->adjMatrix,
         graphInfo.nBlocksAdjMatrix, graphBinFile);

  fclose(graphBinFile);

  // Force canonical form of graph -- enable fair read back check
  if ( canonizeGraph(g) == FAILURE ) { FORWARD_ERROR; }

  // Read back to check
  GRAPH testG;
  initGraph(&testG);
  if ( copyGraph(&testG, g) == FAILURE ) { FORWARD_ERROR; }
  deleteGraph(g);

  GRAPHINFO testGInfo;
  initGraphInfo(&testGInfo);
  if ( copyGraphInfo(&testGInfo, &graphInfo) == FAILURE ) { FORWARD_ERROR; }
  deleteGraphInfo(&graphInfo);

  if ( readBinaryGraph(g, graphBinFileName) == FAILURE ) {
    STDERR_INFO("Unexpected behaviour: "
                "Read back test of binary graph file failed. "
                "Could not finish reading.");
    RETURN_ERROR;
  }

  if ( !areEqualGraphInfos(&graphInfo, &testGInfo) ) {
    STDERR_INFO("Unexpected behaviour: "
                "Read back test of binary graph file failed. "
                "Written and read graph infomation structures are not equal.");
    RETURN_ERROR;
  }

  if ( !areEqualGraphs(g, &testG) ) {
    STDERR_INFO("Unexpected behaviour: "
                "Read back test of binary graph file failed. "
                "Written and read graphs are not equal.");
    RETURN_ERROR;
  }

  deleteGraph(&testG);
  deleteGraphInfo(&testGInfo);

  return SUCCESS;
}

bool readBinaryGraph(GRAPH* g, char* binFileName)
{
  // The input data set representation written in the binary file must have:
  // First sizeof(uint) bytes: sizeof(GRAPHINFO) -- compatibility check
  // Followed by sizeof(GRAPHINFO) bytes: graphInfo struct
  // Followed by a single character 'b' or 'n': type of graph - [non]bipartite
  // Followed by nEvents strings of size maxNodeStrLenght: the events' names
  // Finally followed by nBlocksAdjMatrix BLOCKs: the binary adjacency matrix

  FILE* graphBinFile = fopen(binFileName, "r");
  if ( graphBinFile == NULL ) {
    STDERR_INFO("Unable to open graph binary file: %s\n", binFileName);
    RETURN_ERROR;
  }

  // Read a uint which must have the same sizeof(GRAPHINFO)
  uint sizeGInfo = 0;
  fread(&sizeGInfo, sizeof sizeGInfo, 1, graphBinFile);
  if ( sizeGInfo != sizeof(GRAPHINFO) ) {
    STDERR_INFO("Graph binary file seems not to be compatible with current "
                "program version. Try reading the original text file.");
    RETURN_ERROR;
  }

  // Initialize/clear graphInfo
  initGraphInfo(&graphInfo);

  // Read graphInfo as "header" of binary graph file
  fread(&graphInfo, sizeof graphInfo, 1, graphBinFile);

  // Read 'b' for bipartite or 'n' for nonbipartite
  char graphType = ' ';
  fread(&graphType, sizeof graphType, 1, graphBinFile);
  if( graphType == 'b' && !settings.isBipartiteGraph ) {      // Read b but is n
    STDERR_INFO("Binary graph file contains a bipartite graph. "
                "Please use \"-bipartite\" option "
                "or choose another input file");
    RETURN_ERROR;
  } else if( graphType == 'n' && settings.isBipartiteGraph ) {// Read n but is b
    STDERR_INFO("Binary graph file contains a bipartite graph. "
                "Please use \"-bipartite\" option "
                "or choose another input file");
    RETURN_ERROR;
  } else if( graphType != 'b' && graphType != 'n' ) {    // Read netheir b nor n
    STDERR_INFO("Unexpected graph type identifier: %c\n"
                "This means the binary graph file is not compatible "
                "with current program version, or was corrupted.", graphType);
    RETURN_ERROR;
  }

  // Allocate memory for graph and graph information
  if ( allocGraphInfo(&graphInfo) == FAILURE ) { FORWARD_ERROR; }
  if ( allocGraph(g) == FAILURE ) { FORWARD_ERROR; }

  // Read event names list
  for ( uint eventIt = 0; eventIt < graphInfo.nEvents; ++eventIt ) {
    fread(graphInfo.eventList[eventIt],
          sizeof **graphInfo.eventList,
          graphInfo.maxNodeStrLenght, graphBinFile);
  }

  // Read adjacency matrix
  fread(g->adjMatrix, sizeof *g->adjMatrix,
        graphInfo.nBlocksAdjMatrix, graphBinFile);

  fclose(graphBinFile);

  // Build adjacency lists from read adjacency matrix
  if ( adjListsFromAdjMatrix(g) == FAILURE ) { FORWARD_ERROR; }

  // Find edge links
  if ( g->edgeLinks != NULL ) {
    for (uint edgeIt = 0; edgeIt < graphInfo.nEdges ; edgeIt++) {
      g->edgeLinks[edgeIt] = findLinkedEgde(g, edgeIt);
      if ( g->edgeLinks[edgeIt] == graphInfo.nEdges ) { FORWARD_ERROR; }
    }
  }

  // Get subblock indexes
  for (uint actorIt = 0; actorIt < graphInfo.nActors; ++actorIt ) {
    setIndexesOfSubBlocks(g, actorIt);
  }

  #if PRINT_GRAPH_AS_ADJACENCY_MATRIX_ORIGINAL
    printAdjMatrix(g, ORIGINAL_ADJACENCY_MATRIX_FILE_NAME);
  #endif
  #if PRINT_GRAPH_AS_ACTOR_ADJACENCY_LIST_ORIGINAL
    printActorAdjLists(g, ORIGINAL_ADJACENCY_LIST_FILE_NAME);
    printActorAccumulatedDegrees(g, ORIGINAL_ADJACENCY_LIST_FILE_NAME);
    printActorEdgeMap(g, ORIGINAL_ADJACENCY_LIST_FILE_NAME);
  #endif

  #if PRINT_LIST_OF_NODES_OF_INTEREST
    printEventList();
  #endif

  #if TEST_GRAPH
    if ( initOriginalNodeDegrees(g) == FAILURE ) { FORWARD_ERROR; }
    if ( graphTest(g) == FAILURE ) { FORWARD_ERROR; }
  #endif

  return SUCCESS;
}












