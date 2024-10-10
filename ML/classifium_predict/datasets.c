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
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include "read_csv_file.h"
#include "types.h"
#include "read_names_file.h"
#include "read_dataset.h"
#include "predict.h"
#include "datasets.h"


unsigned char* addSuffix( const unsigned char* filestem,  const unsigned char* suffix ) {
  size_t n = strlen( filestem ) + strlen( suffix ) + 2;
  unsigned char* s = (unsigned char*) malloc( n * sizeof( unsigned char ) );
  assert( s );
  strcpy( s, filestem );
  strcat( s, suffix );
  return s;
  }

static const size_t maxNumLines = 1000000000000000;

void readTestFile( unsigned char* filestem, dataset* d ) {
  unsigned char* dataFile = addSuffix( filestem, ".test" );
  unsigned char* namesFile = addSuffix( filestem, ".names" );
  int numColumns = 0;
  attributeType* columnTypes = readNamesFile( namesFile, &numColumns );
  free( namesFile  );
  assert( numColumns >= 2 && numColumns < 1000000 );
  printf( "\nReading %s", dataFile ); fflush( stdout );
  csvFile csvF = 
    readDataset( dataFile, columnTypes, (size_t) numColumns, maxNumLines, d );
  printf( "\nRead %zu lines from %s\n", d->numExamples, dataFile ); fflush( stdout );
  freeCsvFile( csvF );
  free( dataFile );
  return;
  }

static void printRow( FILE* stream, row row ) {
  for( size_t i = 0; i < row.numFields; i++ )
    if( i == row.numFields - 1 )
      fprintf( stream, "%s", row.fields[i] );
    else
      fprintf( stream, "%s,", row.fields[i] );
  }

static void printCsvFile( FILE* stream, csvFile csvFile ) {
  for( size_t i = 0; i < csvFile.numRows; i++ ) {
    printRow( stream, csvFile.rows[i] );
    if( i < csvFile.numRows - 1 )
      fprintf( stream, "\n" );
    }
  return;
  }


void 
  processCasesFile( 
    unsigned char* filestem, 
    dataset* d, 
    tree trees[], 
    size_t numTrees 
    ) {
  unsigned char* dataFile = addSuffix( filestem, ".cases" );
  unsigned char* namesFile = addSuffix( filestem, ".names" );
  int numColumns = 0;
  attributeType* columnTypes = readNamesFile( namesFile, &numColumns );
  free( namesFile  );
  assert( numColumns >= 2 && numColumns < 1000000 );
  printf( "\nReading %s", dataFile ); fflush( stdout );
  csvFile csvF = 
    readDataset( dataFile, columnTypes, (size_t) numColumns, maxNumLines, d );
  printf( "\nRead %zu lines from %s\n", d->numExamples, dataFile ); fflush( stdout );

  size_t n = d->numExamples;
  outputClass* predictions = 
    (outputClass*) malloc( n * sizeof( outputClass ) );
  assert( predictions );
  for( size_t i = 0; i < n; i++ )
    predictions[i] = forestPredict( i, trees, numTrees, *d );
  size_t outputColumnIndex;
  for( int c = 0; c < numColumns; c++ )
    if( columnTypes[c] == OUTPUT ) 
      outputColumnIndex = c;
  assert( n == csvF.numRows );
  for( size_t i = 0; i < csvF.numRows; i++ ) {
    assert( outputColumnIndex < csvF.rows[i].numFields );
    csvF.rows[i].fields[ outputColumnIndex ] =
      d->outputInternalToExternal[ predictions[i] ];
    }
  FILE* stream = fopen( dataFile, "w" );
  if( stream == NULL ) {
    fprintf( stderr, "\nError: Cannot write to %s\n", dataFile );
    fflush( stderr );
    exit( 1 );
    }
  printCsvFile( stream, csvF );
  fclose( stream );
  freeCsvFile( csvF );
  free( dataFile );

  return;
  }

