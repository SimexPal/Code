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


#include "../headers/argParser.h"

int argParser(int argc, char **argv)
{
  /* Default config */
  strcpy(settings.inputFilePath,"./");
  strcpy(settings.inputFileName,"");
  settings.isBinaryInput                = FALSE;
  settings.writeBinaryGraph             = DEFAULT_WRITEBINARYGRAPH;
  strcpy(settings.outputFilePath,"./");
  strcpy(settings.outputFileName,"");
  settings.appendRunInfo                = DEFAULT_APPENDRUNINFO;

  strcpy(settings.externalGtFileName,"");
  settings.hasExternalGt                = FALSE;
  settings.includeGTMissingNodes        = DEFAULT_INCLUDEGTMISSINGNODES;
  settings.ignoreGTMissingNodes         = DEFAULT_IGNOREGTMISSINGNODES;

  settings.gotExternalSeed              = FALSE;
  settings.seed                         = 0;

  settings.isBipartiteGraph             = DEFAULT_ISBIPARTITEGRAPH;
  settings.bipartiteSideOfInterest      = DEFAULT_BIPARTITESIDEOFINTEREST;
  settings.directEdgeCoocValue          = DEFAULT_DIRECTEDGECOOCVALUE;

  settings.nSwaps                       = DEFAULT_NSWAPS;
  settings.elneSwaps                    = DEFAULT_ELNESWAPS;
  settings.runCurveball                 = DEFAULT_RUNCURVEBALL;
  settings.runSwapHeuristic             = DEFAULT_RUNSWAPHEURISTIC;
  settings.nDegreesSwapHeuristic        = DEFAULT_NDEGREESSWAPHEURISTIC;
  settings.nEventsPerDegreeSwapHeuristic= DEFAULT_NEVENTSPERDEGREESWAPHEURISTIC;
  settings.thresholdTheta               = DEFAULT_THRESHOLDTHETA;

  settings.minRelevantCooc              = DEFAULT_MINRELEVANTCOOC;

  settings.nSamples                     = DEFAULT_NSAMPLES;
  settings.nMaxSamples                  = DEFAULT_NMAXSAMPLES;
  settings.runSamplesHeuristic          = DEFAULT_RUNSAMPLESHEURISTIC;
  settings.ratioGtPairsPerResultPair    = DEFAULT_RATIOGTPAIRSPERRESULTPAIR;
  settings.internalPpvThreshold         = DEFAULT_INTERNALPPVTHRESHOLD;

  strcpy(settings.dateStr,DEFAULT_DATESTR);
  strcpy(settings.runIndex,DEFAULT_RUNINDEX);

  settings.isHelpRun = FALSE;

  /* Iterate through input arguments to configure the program */
  int argvIdx=1;
  while (argvIdx < argc) {
    bool isValidArg = FALSE;
    if ( !strcmp(argv[argvIdx],"-in") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          strcpy(settings.inputFileName, argv[argvIdx]);

          if ( strstr(settings.inputFileName, "/") ) {
            STDERR_INFO("Input file name must not contain a path.");
            RETURN_ERROR;
          }

          char* ext = getFilenameExt(settings.inputFileName);
          if ( !strcmp(ext, ".gbin") ) {
            settings.isBinaryInput = TRUE;
            settings.writeBinaryGraph = FALSE;
          }
          else {
            settings.isBinaryInput = FALSE;
            settings.writeBinaryGraph = DEFAULT_WRITEBINARYGRAPH;
          }

          isValidArg = TRUE;
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No input file specified after \"-in\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-inpath") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          strcpy(settings.inputFilePath, argv[argvIdx]);
          isValidArg = TRUE;
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No input path specified after \"-inpath\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-writebinarygraph") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          if ( !strcmp(argv[argvIdx],"false")) {
            settings.writeBinaryGraph = FALSE;
            isValidArg = TRUE;
          } else if ( !strcmp(argv[argvIdx],"true") ) {
            if ( settings.isBinaryInput ) { // If input is binary
              settings.writeBinaryGraph = FALSE; // No reason to write it back
            } else {
              settings.writeBinaryGraph = TRUE;
            }
            isValidArg = TRUE;
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("Either \"true\" or \"false\" must be used after "
                    "\"-writebinarygraph\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-out") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          strcpy(settings.outputFileName, argv[argvIdx]);
          if ( strstr(settings.outputFileName, "/") ) {
            STDERR_INFO("Output file name must not contain a path.");
            RETURN_ERROR;
          }
          isValidArg = TRUE;
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No output file specified after \"-out\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-outpath") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          strcpy(settings.outputFilePath, argv[argvIdx]);
          isValidArg = TRUE;
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No output path specified after \"-outpath\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-appendruninfo") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          if ( !strcmp(argv[argvIdx],"false")) {
            settings.appendRunInfo = FALSE;
            isValidArg = TRUE;
          } else if ( !strcmp(argv[argvIdx],"true") ) {
            settings.appendRunInfo = TRUE;
            isValidArg = TRUE;
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("Either \"true\" or \"false\" must be used after "
                    "\"-appendruninfo\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-gt") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          strcpy(settings.externalGtFileName, argv[argvIdx]);
          settings.hasExternalGt = TRUE;
          isValidArg = TRUE;
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No ground truth file specified after \"-gt\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-disallowgtmissingnodes") ) {
      settings.includeGTMissingNodes = FALSE;
      settings.ignoreGTMissingNodes = FALSE;
      isValidArg = TRUE;
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-includegtmissingnodes") ) {
      settings.includeGTMissingNodes = TRUE;
      settings.ignoreGTMissingNodes = FALSE;
      isValidArg = TRUE;
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-ignoregtmissingnodes") ) {
      settings.includeGTMissingNodes = FALSE;
      settings.ignoreGTMissingNodes = TRUE;
      isValidArg = TRUE;
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-seed") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          settings.seed = strtoul(argv[argvIdx], NULL, 10);
          settings.gotExternalSeed = TRUE;
          isValidArg = TRUE;
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No positive number specified after \"-swaps\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-swaps") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          if ( !strcmp(argv[argvIdx], "elne") ) {
            settings.elneSwaps = TRUE;
            settings.runSwapHeuristic = FALSE;
            isValidArg = TRUE;
          } else {
            settings.nSwaps = strtoul(argv[argvIdx], NULL, 10);
            settings.runSwapHeuristic = FALSE;
            if ( settings.nSwaps > 0 ) {
              isValidArg = TRUE;
            }
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No positive number specified after \"-swaps\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-degrees") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          settings.nDegreesSwapHeuristic = strtoul(argv[argvIdx], NULL, 10);
          if ( settings.nDegreesSwapHeuristic > 0 ) {
            isValidArg = TRUE;
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No positive number specified after "
                    "\"-degrees\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-eventsperdegree") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          settings.nEventsPerDegreeSwapHeuristic =
              strtoul(argv[argvIdx], NULL, 10);
          if ( settings.nEventsPerDegreeSwapHeuristic > 0 ) {
            isValidArg = TRUE;
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No positive number specified after "
                    "\"-eventsperdegree\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-theta") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          settings.thresholdTheta = strtod(argv[argvIdx], NULL);
          if ( settings.thresholdTheta > 0 ) {
            isValidArg = TRUE;
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No positive number specified after \"-theta\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-mincooc") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          settings.minRelevantCooc = strtoul(argv[argvIdx], NULL, 10);
          if ( settings.minRelevantCooc > 0 ) {
            isValidArg = TRUE;
          } else if ( argv[argvIdx][0] == '0' ) {
            isValidArg = TRUE;
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No positive number, or zero, specified after "
                    "\"-mincooc\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-samples") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          if ( strstr(argv[argvIdx], "heu") ) {
            settings.runSamplesHeuristic = TRUE;
          } else {
            settings.nSamples = strtoul(argv[argvIdx], NULL, 10);
            settings.runSamplesHeuristic = FALSE;
            if ( settings.nSamples > 0 ) {
              isValidArg = TRUE;
            }
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No positive number specified after "
                    "\"-samples\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-maxsamples") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          settings.nMaxSamples = strtoul(argv[argvIdx], NULL, 10);
          if ( settings.nMaxSamples > 0 ) {
            isValidArg = TRUE;
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No positive number specified after "
                    "\"-maxsamples\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-ratiogtpairs") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          settings.ratioGtPairsPerResultPair = strtod(argv[argvIdx], NULL);
          if ( settings.ratioGtPairsPerResultPair > 0 &&
               settings.ratioGtPairsPerResultPair < 1 ) {
            isValidArg = TRUE;
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("Ratio specified after \"-ratiogtpairs\" option"
                    " is not in the interval (0,1).");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-internalppv") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          settings.internalPpvThreshold = strtod(argv[argvIdx], NULL);
          if ( settings.internalPpvThreshold > 0 &&
               settings.internalPpvThreshold < 1 ) {
            isValidArg = TRUE;
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("Ratio specified after \"-internalppv\" option"
                    " is not in the interval (0,1).");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-date") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          strcpy(settings.dateStr,argv[argvIdx]);
          isValidArg = TRUE;
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No string specified after \"-date\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-index") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          strcpy(settings.runIndex,argv[argvIdx]);
          if ( strtoul(argv[argvIdx], NULL, 10) > 0 ) {
            isValidArg = TRUE;
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No positive number specified after \"-index\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-curveball") ) {
      settings.runCurveball = TRUE;
      isValidArg = TRUE;
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-singleswitch") ) {
      settings.runCurveball = FALSE;
      isValidArg = TRUE;
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-bipartite") ) {
      settings.isBipartiteGraph = TRUE;
      isValidArg = TRUE;
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-nonbipartite") ) {
      settings.isBipartiteGraph = FALSE;
      isValidArg = TRUE;
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-sideofinterest") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if ( argv[argvIdx][0] == 'r' ) {
          settings.bipartiteSideOfInterest = 'r';
          isValidArg = TRUE;
        }
        else if ( argv[argvIdx][0] == 'l' ) {
          settings.bipartiteSideOfInterest = 'l';
          isValidArg = TRUE;
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No side of interest specified after "
                    "\"-sideofinterest\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-directedgevalue") ) {
      argvIdx++;
      if (argvIdx < argc) {
        if (argv[argvIdx][0] != '-' ) {
          settings.directEdgeCoocValue = strtoul(argv[argvIdx], NULL, 10);
          if ( settings.directEdgeCoocValue > 0 ) {
            isValidArg = TRUE;
          }
        }
      }
      if ( !isValidArg ) {
        STDERR_INFO("No positive number specified after "
                    "\"-directedgevalue\" option.");
        RETURN_ERROR;
      }
      argvIdx++;
    }

    else if ( !strcmp(argv[argvIdx],"-help") || !strcmp(argv[argvIdx],"-h") ) {
      fprintf(stdout, "Program version - compiled at: %s %s\n",
              __DATE__, __TIME__);
      fprintf(stdout,"Parameters:");
      fprintf(stdout,"\n  -in                     "
                     "<inputFileName.txt> or <inputFileName.gbin>       "
                     "(Only MANDATORY argument)");
      fprintf(stdout,"\n  -inpath                 "
                     "<path/to/>                                        "
                     "(Path to input file)");
      fprintf(stdout,"\n  -writebinarygraph       "
                     "\"true\" or \"false\"                                 "
#if DEFAULT_WRITEBINARYGRAPH
                     "(Default is writing the binary representation of the "
                     "input dataset if the given one was in text form.");
#else
                     "(Default is not writing the binary representation of the "
                     "input dataset even if the given one was in text form)");
#endif
      fprintf(stdout,"\n  -out                    "
                     "<outFileName.laps>                                "
#if DEFAULT_APPENDRUNINFO
                     "(Default is inputFileName_<runinfo>.laps)");
#else
                     "(Default is inputFileName.laps)");
#endif
      fprintf(stdout,"\n  -outpath                "
                     "<path/to/>                                        "
                     "(Path to output file)");
      fprintf(stdout,"\n  -appendruninfo          "
                     "\"true\" or \"false\"                                 "
                     "(Default is %s)", DEFAULT_APPENDRUNINFO ? "true":"false");

      fprintf(stdout,"\n  -gt                     "
                     "<path/to/groundTruthFile>                         "
                     "(Default is no external ground truth)");

#if DEFAULT_INCLUDEGTMISSINGNODES && DEFAULT_IGNOREGTMISSINGNODES
  #error Cannot both include and ignore external GT missing nodes!
#elif DEFAULT_INCLUDEGTMISSINGNODES
  #define DEFAULT_GTMISSINGNODEOPTION "including"
#elif DEFAULT_IGNOREGTMISSINGNODES
  #define DEFAULT_GTMISSINGNODEOPTION "ignoring"
#else
  #define DEFAULT_GTMISSINGNODEOPTION "not allowing"
#endif
      fprintf(stdout,"\n  -disallowgtmissingnodes "
                     "                                                  "
                     "(Default is %s nodes that "
                     "are in the external ground truth but "
                     "are not in the input file)",
              DEFAULT_GTMISSINGNODEOPTION);

      fprintf(stdout,"\n  -includegtmissingnodes  "
                     "                                                  "
                     "(Default is %s nodes that "
                     "are in the external ground truth but "
                     "are not in the input file)",
              DEFAULT_GTMISSINGNODEOPTION);

      fprintf(stdout,"\n  -ignoregtmissingnodes   "
                     "                                                  "
                     "(Default is %s nodes that "
                     "are in the external ground truth but "
                     "are not in the input file)",
              DEFAULT_GTMISSINGNODEOPTION);

      fprintf(stdout,"\n  -seed                   "
                     "<long int as seed>                                "
                     "(Default is running getting a new seed based on "
                     "current time)");

#if DEFAULT_NSWAPS == 0
      fprintf(stdout,"\n  -swaps                  "
                     "<number of swaps> or \"elne\"                       "
                     "(Default is running the swap heuristic. "
                     "The option elne stands for |Edges|*ln|Edges| swaps)");
#else
      fprintf(stdout,"\n  -swaps                  "
                     "<number of swaps>                                 "
                     "(Default is %lu. Zero means enabling swap heuristic)",
              DEFAULT_NSWAPS);
#endif
#if DEFAULT_RUNCURVEBALL
 #define DEFAULT_RANDOMIZE_ALGORITHM "curveball"
#else
 #define DEFAULT_RANDOMIZE_ALGORITHM "single switch"
#endif
      fprintf(stdout,"\n  -curveball              "
                     "                                                  "
                     "(Default is randomizing the graph by the %s algorithm)",
                      DEFAULT_RANDOMIZE_ALGORITHM);
      fprintf(stdout,"\n  -singleswitch           "
                     "                                                  "
                     "(Default is randomizing the graph by the %s algorithm)",
                      DEFAULT_RANDOMIZE_ALGORITHM);
      fprintf(stdout,"\n  -degrees                "
                     "<number of degrees for swap heuristic>            "
                     "(Default is %u)", DEFAULT_NDEGREESSWAPHEURISTIC);
      fprintf(stdout,"\n  -eventsperdegree        "
                     "<number of events per degree for swap heuristic>  "
                     "(Default is %u)", DEFAULT_NEVENTSPERDEGREESWAPHEURISTIC);
      fprintf(stdout,"\n  -theta                  "
                     "<theta threshold for swap heuristic>              "
                     "(Default is %lf)", DEFAULT_THRESHOLDTHETA);

      fprintf(stdout,"\n  -mincooc                "
                     "<minimum co-occurence for pair to be relevant>    "
                     "(Default is %u)", DEFAULT_MINRELEVANTCOOC);


#if DEFAULT_NSAMPLES == 0
      fprintf(stdout,"\n  -samples                "
                     "<number of samples>                               "
                     "(Default is running the sample heuristic)");
#else
      fprintf(stdout,"\n  -samples         "
                     "<number of samples>                               "
                     "(Default is %u. Zero means enabling sample heuristic)",
              DEFAULT_NSAMPLES);
#endif
      fprintf(stdout,"\n  -maxsamples             "
                     "<maximum number of samples>                       "
                     "(Only valid if performing sample heuristic. "
                     "Default is %u)", DEFAULT_NMAXSAMPLES);
      fprintf(stdout,"\n  -ratiogtpairs           "
                     "<length of internal GT/length of result>          "
                     "(Default is %lf)", DEFAULT_RATIOGTPAIRSPERRESULTPAIR);
      fprintf(stdout,"\n  -internalppv            "
                     "<internal PPV threshold>                          "
                     "(Default is %lf)", DEFAULT_INTERNALPPVTHRESHOLD);

#if DEFAULT_ISBIPARTITEGRAPH
 #define DEFAULT_GRAPHTYPE "bipartite"
#else
 #define DEFAULT_GRAPHTYPE "non-bipartite"
#endif
      fprintf(stdout,"\n  -nonbipartite           "
                     "                                                  "
                     "(Default is interpreting the graph as a %s one)",
              DEFAULT_GRAPHTYPE);
      fprintf(stdout,"\n  -bipartite              "
                     "                                                  "
                     "(Default is interpreting the graph as a %s one)",
              DEFAULT_GRAPHTYPE);

      fprintf(stdout,"\n  -sideofinterest         "
                     "<inputfile side of interest: \"left\" or \"right\">   "
                     "(Default is the %c side. Note: this option only concerns "
                     "bipartite graphs)", DEFAULT_BIPARTITESIDEOFINTEREST);

      fprintf(stdout,"\n  -directedgevalue        "
                     "<integer co-occurrence weight of a direct edge>   "
                     "(Default is %u. Note: this option only concerns "
                     "non-bipartite graphs)", DEFAULT_DIRECTEDGECOOCVALUE);

      fprintf(stdout,"\n  -date                   "
                     "<date_string>                                     "
                     "(Default is \"%s\")", DEFAULT_DATESTR);

      fprintf(stdout,"\n  -index                  "
                     "<run_index>                                       "
                     "(Default is \"%s\")\n", DEFAULT_RUNINDEX);
      
      settings.isHelpRun = TRUE;
      return SUCCESS; 
    }

    else {
      STDERR_INFO("Could not identify %s parameter.\n"
                  "Use '-help' to see the list of possible parameters.",
                  argv[argvIdx]);
      RETURN_ERROR;
    }   
  }
  
  if ( !strcmp(settings.inputFileName,"") ) {
    STDERR_INFO("No input file found!\n"
                "Use '-help' to get some help.");
    RETURN_ERROR;
  }

  return SUCCESS;
}
