/*
Copyright (c) 2018, Classifium AB
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "datasets.h"
#include "read_forest.h"
#include "print_forest.h"
#include "predict.h"
#include "print_confusion_matrix.h"


int main( int argc, char** argv ) {
  if( argc != 2 ) {
    printf( "\nUsage: %s filestem\n\n", argv[0] );
    return 1;
    }
  char* filestem = argv[1];
  assert( filestem );
  unsigned char* forestFile = addSuffix( filestem, ".forest" );
  FILE* forestStream = fopen( forestFile, "r" );
  if( forestStream == NULL ) {
    fprintf( stderr, "\nError: Could not open %s\n", forestFile );
    fflush( stderr );
    exit( 1 );
    }
  dataset d;
  readInternalToExternal( forestStream, &d );
  size_t numTrees;
  tree* trees = readForest( forestStream, &numTrees );
  fclose( forestStream );
  free( forestFile );

  unsigned char* testFile = addSuffix( filestem, ".test" );
  FILE* testStream = fopen( testFile, "r" );
  if( testStream != NULL ) {
    readTestFile( filestem, &d );
    double errorRate;
    size_t** cm = confusionMatrix( trees, numTrees, d, &errorRate );
    printf( "\nError rate = %lf\n", errorRate );
    printf( "\nConfusion matrix\n" );
      printf( "================\n\n" );
    printConfusionMatrix( cm, d );
    fclose( testStream );
    }
  free( testFile );

  unsigned char* casesFile = addSuffix( filestem, ".cases" );
  FILE* casesStream = fopen( casesFile, "r" );
  if( casesStream != NULL ) {
    processCasesFile( filestem, &d, trees, numTrees );
    fclose( casesStream );
    }
  free( casesFile );

  return 0;
  }






