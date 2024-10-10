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
#include <assert.h>
#include <math.h>
#include "types.h"
#include "read_forest.h"

static void printAttributeType( attributeType t ) {
  if( t == NOMINAL ) { printf( "NOMINAL" ); return; }
  if( t == ORDINAL ) { printf( "ORDINAL" ); return; }
  if( t == IGNORE ) { printf( "IGNORE" ); return; }
  if( t == LABEL ) { printf( "LABEL" ); return; }
  if( t == OUTPUT ) { printf( "OUTPUT" ); return; }
  if( t == WEIGHT ) { printf( "WEIGHT" ); return; }
  if( t == NONE ) { printf( "NONE" ); return; }
  assert( false );
  }

static void
  printOutputDistribution(
    outputDistribution dist,
    size_t numOutputClasses
     ) {
  for( size_t i = 0; i < numOutputClasses; i++ )
    printf( " %lf ", dist[i] );
  return;
  }


#define getBit( XS, BITINDEX ) ( ( XS[ ( BITINDEX ) >> 4 ] >> ( ( BITINDEX  ) & 15 ) ) & 1 )

static void printBitmask( bitmask xs, size_t n ) {
  for( int bitNo = n-1; bitNo >= 0; bitNo-- )
    printf( "%u", getBit( xs, (size_t) bitNo ) );
  }



static void printSplit( split split, size_t numInputClasses ) {
  printf( "attributeIndex = %zu ", split.attributeIndex );
  printAttributeType( split.attributeType );
  printf( " missingGoesLeft = %d", split.missingGoesLeft );
  if( split.attributeType == NONE )
    return;
  if( split.attributeType == NOMINAL ) {
    printf(" bitmask = " );
    printBitmask( split.bitmaskOrLimit.bitmask, numInputClasses );
    return;
    }
  assert( split.attributeType == ORDINAL );
  printf(" limit = %lf", split.bitmaskOrLimit.limit );
  return;
  }


static void indent( int indentation ) {
  for( int i = 0; i < indentation; i++ )
    printf( " " );
  }

static void
  printTreeAux(
    int indentation,
    tree xs,
    size_t* numInputClasses,
    size_t numOutputClasses
    ) {
  indent( indentation );
  if( xs->isLeaf ) {
    printf( "outputDistribution = " );
    printOutputDistribution( xs->info.outputDistribution, numOutputClasses );
    return;
    }
  printSplit(
    xs->info.split,
    numInputClasses[ xs->info.split.attributeIndex ]
    );
  printf( "\n" );
  printTreeAux( indentation + 2, xs->left, numInputClasses, numOutputClasses );  printf( "\n" );
  printTreeAux( indentation + 2, xs->right, numInputClasses, numOutputClasses );  printf( "\n" );
  }

static void printTree( tree xs, size_t* numInputClasses, size_t numOutputClasses ) {
  printTreeAux( 0, xs, numInputClasses, numOutputClasses );
  }


void printForest( tree* trees, size_t numTrees, dataset d ) {
  for( size_t i = 0; i < numTrees; i++ ) {
    printf( "\n--------------------------------\n" );
    printTree( trees[i], d.numInputClasses, d.numOutputClasses );
    }
  return;
  }




