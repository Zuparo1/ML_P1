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
#include "predict.h"

static outputClass indexOfMax( outputDistribution counts, size_t numOutputClasses ) {
  assert( numOutputClasses >= 1 );
  double maxCount = counts[0];
  double s = counts[0];
  for( size_t c = 1; c < numOutputClasses; c++ )
    if( counts[c] > maxCount )
      maxCount = counts[c];
  for( size_t c = 1; c < numOutputClasses; c++ )
    s += counts[c] * (double)(c + 1 );

  size_t numMax = 0;
  for( size_t c = 0; c < numOutputClasses; c++ )
    if( counts[c] == maxCount ) numMax++;
  if( numMax < 1 ) numMax = 1;

  size_t chosen = (size_t) fabs( fmod( s + 1.0e9, (double)numMax ) );
  size_t current = 0;
  size_t maxI = 0;
  for( size_t c = 0; c < numOutputClasses; c++ )
    if( counts[c] == maxCount )
      if( current == chosen ) {
        maxI = c;
        break;
        }
      else
        current++;
  assert( maxI < numOutputClasses );

  return (outputClass) maxI;
  }

#define getBit( XS, BITINDEX ) ( ( XS[ ( BITINDEX ) >> 4 ] >> ( ( BITINDEX  ) & 15 ) ) & 1 )

static outputDistribution
  treePredict(
    size_t exampleIndex,
    tree tree,
    dataset dataset
    ) {
  assert( tree != NULL );
  while( true ) {
    if( tree->isLeaf ) 
      return tree->info.outputDistribution;
    split split = tree->info.split;
    attributeValue v = dataset.inputs[ split.attributeIndex ][ exampleIndex ];
    assert( split.missingGoesLeft == false ||  split.missingGoesLeft == true );
    // Should v go left or right?
    bool left =
      v.ord == MISSING_VALUE ?
        split.missingGoesLeft
      :
      split.attributeType == NOMINAL ?
        getBit( split.bitmaskOrLimit.bitmask, v.nom ) == 0 
      :
        v.ord < split.bitmaskOrLimit.limit;
  
    tree = left ? tree->left : tree->right;
    }

  }

static void
  addDist(
    outputDistribution counts,
    outputDistribution delta,
    size_t numOutputClasses
    ) {
  for( size_t i = 0; i < numOutputClasses; i++ )
    counts[i] += delta[i];
  return;
  }

outputClass
  forestPredict(
    size_t exampleIndex,
    tree* trees,
    size_t numTrees,
    dataset dataset
    ) {
  size_t n = dataset.numOutputClasses;
  double totalDist[n];
  for( size_t i = 0; i < n; i++ )
    totalDist[i] = 0.0;
  for( size_t t = 0; t < numTrees; t++ ) {
    outputDistribution dist =
      treePredict( exampleIndex, trees[t], dataset );
    addDist( totalDist, dist, dataset.numOutputClasses );
    }
  return indexOfMax( totalDist, n );
  }

size_t**
  confusionMatrix(
    tree* trees,
    size_t numTrees,
    dataset d,
    double* errorRate // Output
    ) {
  size_t n = d.numOutputClasses;
  size_t** cm = (size_t**) malloc( n * sizeof( size_t* ) );
  assert( cm );
  for( size_t c = 0; c < n; c++ ) {
    cm[c] = (size_t*) calloc( n, sizeof( size_t ) );
    assert( cm[c] );
    }
  size_t numErrors = 0;
  for( size_t e = 0; e < d.numExamples; e++ ) {
    outputClass actual = d.outputs[ e ];
    outputClass predicted = forestPredict( e, trees, numTrees, d );
    cm[ predicted ][ actual ]++;
    if( predicted != actual )
      numErrors++;
    }
  *errorRate = (double) numErrors / (double) d.numExamples;
  return cm;
  }

