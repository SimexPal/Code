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


#include "../headers/output.h"

bool buildOutputFileName()
{

  if ( !strcmp(settings.outputFileName, "") ) {
    // if not given, use input file name
    strcpy(settings.outputFileName, settings.inputFileName);
  }

  char* ext = getFilenameExt(settings.outputFileName);
  *ext = '\0'; // Remove extension
  if ( settings.appendRunInfo ) {
    strcat(settings.outputFileName,"_");

    char numsamples_str[MAX_INT_STR_SIZE];
    sprintf(numsamples_str, "%u", settings.nSamples);
    strcat(settings.outputFileName, numsamples_str);
    strcat(settings.outputFileName, "_samples_");

    char numswaps_str[MAX_INT_STR_SIZE];
    sprintf(numswaps_str, "%lu", settings.nSwaps);
    strcat(settings.outputFileName, numswaps_str);
    strcat(settings.outputFileName, "_swaps");

    if ( strcmp(settings.dateStr, "") ) {
      strcat(settings.outputFileName, "_date_");
      strcat(settings.outputFileName, settings.dateStr);
    }

    if ( strcmp(settings.runIndex, "") ) {
      strcat(settings.outputFileName, "_index_");
      strcat(settings.outputFileName, settings.runIndex);
    }

  }

  strcat(settings.outputFileName, ".laps"); // Append .laps extension

  char outFileNameTmp[MAX_FILEPATH_SIZE+MAX_FILENAME_SIZE];
  strcpy(outFileNameTmp, settings.outputFilePath);
  strcat(outFileNameTmp, "/");
  strcat(outFileNameTmp, settings.outputFileName);

  strcpy(settings.outputFileName, outFileNameTmp);

  return SUCCESS;
}

bool buildFormatStrings(char* headerFormatStr, char* dataFormatStr)
{
  strcpy(headerFormatStr, "%"); // "%"
  char maxNodeStrLenghtStr[MAX_INT_STR_SIZE];
  sprintf(maxNodeStrLenghtStr, "%u", graphInfo.maxNodeStrLenght);
  strcat(headerFormatStr, maxNodeStrLenghtStr); // "%42"
  strcat(headerFormatStr, "s "); // "%42s "
  strncat(headerFormatStr, headerFormatStr,
          strlen(headerFormatStr)); // "%42s %42s "
  strcpy(dataFormatStr, headerFormatStr); // "%42s %42s "

  strcat(headerFormatStr, "%15s %15s %15s %15s\n");
  strcat(dataFormatStr, "%15g %15g %15g %15u\n");

  return SUCCESS;
}

bool createOutput (TMPRESULT* results, PAIR* pairs)
{ 

  if ( buildOutputFileName() == FAILURE ) { FORWARD_ERROR; }

  FILE *outputFileStream = fopen(settings.outputFileName, "w");
  if ( outputFileStream == NULL ) {
    STDERR_INFO("Output file %s could not be created at %s. This can be because"
                " the path does not exist or you do not have write"
                " permition there.\n",
                settings.outputFileName,
                settings.outputFilePath);
    RETURN_ERROR;
  }

  fprintf(outputFileStream,
          "Program version - compiled at: %s %s\n", __DATE__, __TIME__);

  time_t currentTime;
  time( &currentTime );
  fprintf(outputFileStream,
          "Execution date and time: %s", ctime( &currentTime ));

  char headerFormatStr[200] = "\0";
  char dataFormatStr[200] = "\0";
  if ( buildFormatStrings(headerFormatStr, dataFormatStr) == FAILURE ) {
    FORWARD_ERROR;
  }
  fprintf(outputFileStream, headerFormatStr,
          "Node1", "Node2", "pValue", "zScore", "Cooc(FDSM)", "oriCooc");
  for (uint pairIt=0; pairIt < graphInfo.nRelevantPairs; pairIt++) {
    uint row = pairs[pairIt].eventId1;
    uint col = pairs[pairIt].eventId2 - row - 1;
    fprintf(outputFileStream, dataFormatStr,
            graphInfo.eventList[pairs[pairIt].eventId1],
            graphInfo.eventList[pairs[pairIt].eventId2],
            (double)pairs[pairIt].pValue/(double)settings.nSamples,
            pairs[pairIt].zScore,
            (double)results->coocSum[row][col]/(double)settings.nSamples,
            graphInfo.originalCooc[row][col]);
  }

  fclose(outputFileStream);

  return SUCCESS;
}
