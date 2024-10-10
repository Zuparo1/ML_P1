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
#include <math.h>
#include "types.h"
#include "print_confusion_matrix.h"

static void pad( size_t n ) {
  for( size_t i = 0; i < n; i++ )
    putchar( ' ' );
  return;
  }

/*
              classLabel1 | classLabel2 |
-----------------------------------------
classLabel1 |          50 |          10 |
*/

static void printRow( size_t width, char* labels[], size_t n ) {
  for( size_t i = 0; i < n; i++ ) {
    pad( width - strlen( labels[ i] ) - 1 );
    printf( "%s", labels[ i ] );
    pad( 1 );
    putchar( '|' );
    }
  printf( "\n" );
  return;
  }

static void printLine( size_t width, size_t n ) {
  for( size_t i = 0; i < n; i++ ) 
    for( size_t k = 0; k < width + 1; k++ ) 
      putchar( '-' );
  printf( "\n" );
  return;
  }

static size_t max2( size_t x, size_t y ) { return x < y ? y : x; }

static size_t numberLength( size_t x ) {
  char s[ 100 ];
  sprintf( s, "%zu", x );
  return strlen( s );
  }

static char* numberToString( size_t x ) {
  size_t n = numberLength( x );
  char* s = (char*) malloc( ( n + 1 ) * sizeof( char ) );
  assert( s );
  sprintf( s, "%zu", x );
  return s;
  }

// width = max length + 2

static size_t maxLength( size_t** cm, dataset d ) {
  size_t n = d.numOutputClasses;
  size_t m = 0;
  m = max2( m, strlen( "Predicted:" ) );
  m = max2( m, strlen( "Actual:" ) );
  for( size_t c = 0; c < n; c++ )
    m = max2( m, strlen( d.outputInternalToExternal[c] ) );
  for( size_t c = 0; c < n; c++ )
    for( size_t r = 0; r < n; r++ )
      m = max2( m, numberLength( cm[r][c] ) );
  return m;
  }

static void printcm( size_t** cm, dataset d, size_t lower, size_t upper ) {
  size_t n = upper - lower + 1;
  size_t width = maxLength( cm, d ) + 2;
  char* labels[ n + 1 ];
  
  // First row:
  labels[0] = "";
  for( size_t c = lower; c < upper+1; c++ )
    labels[ c - lower + 1 ] = "Actual:";
  printRow( width, labels, n+1 );

  // Second row:
  labels[0] = "";
  for( size_t c = lower; c < upper+1; c++ )
    labels[ c - lower + 1 ] = d.outputInternalToExternal[c];
  printRow( width, labels, n+1 );

  printLine( width, n+1 );

  for( size_t r = 0; r < d.numOutputClasses; r++ ) {

    labels[0] = "Predicted:";
    for( size_t c = lower; c < upper+1; c++ )
      labels[ c - lower + 1 ] = "";
    printRow( width, labels, n+1 );

    labels[0] = d.outputInternalToExternal[r];
    for( size_t c = lower; c < upper+1; c++ )
      labels[ c - lower + 1 ] = numberToString( cm[ r ][ c ] );
    printRow( width, labels, n+1 );
    for( size_t c = lower; c < upper+1; c++ )
      free( labels[ c - lower + 1 ] );

    printLine( width, n+1 );
    }

  return;
  }

void printConfusionMatrix( size_t** cm, dataset d ) {
  size_t numCols = d.numOutputClasses;
  size_t width = maxLength( cm, d ) + 2;
  size_t numColsPerLine = 80 / width;
  if( numColsPerLine < 1 ) numColsPerLine = 1;
  size_t lower = 0;
  while( lower < numCols ) {
    size_t upper = lower + numColsPerLine - 1;
    if( upper >= numCols ) upper = numCols - 1;
    printcm( cm, d, lower, upper );
    lower += numColsPerLine;
    }
  return;
  }
