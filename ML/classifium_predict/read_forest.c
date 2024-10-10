
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
#include "types.h"

static bitmask readBitmask( FILE* stream ) {
  size_t n;
  assert( fscanf( stream, "%zu ", &n ) == 1 );
  assert( sizeof( unsigned short ) == 2 );
  bitmask b = (unsigned short*) malloc( n * sizeof( unsigned short ) );
  assert( b );
  for( size_t i = 0; i < n; i++ )
    assert( fscanf( stream, "%hu ", & b[i] ) == 1 );
  return b;
  }


static split readSplit( FILE* stream ) {
  split split;
  assert( fscanf( stream, "%zu ", & split.attributeIndex ) == 1 );
  assert( fscanf( stream, "%d ", (int*)( & split.attributeType ) ) == 1 );
  assert( split.attributeType == NOMINAL || split.attributeType == ORDINAL );
  assert( fscanf( stream, "%d ", (int*)( & split.missingGoesLeft ) ) == 1 );
  if( split.attributeType == NOMINAL )
    split.bitmaskOrLimit.bitmask = readBitmask( stream );
  else
    assert( fscanf( stream, "%lf ", & split.bitmaskOrLimit.limit ) == 1 );
  return split;
  }


static outputDistribution readOutputDistribution( FILE* stream ) {
  size_t numOutputClasses;
  assert( fscanf( stream, "%zu ", & numOutputClasses ) == 1 );
  outputDistribution dist =
    (double*) malloc( numOutputClasses * sizeof( double ) );
  assert( dist );
  for( outputClass c = 0; c < numOutputClasses; c++ )
    assert( fscanf( stream, "%lf ", & dist[ c ] ) == 1 );
  return dist;
  }


static node readNode( FILE* stream ) {
  node node;
  assert( fscanf( stream, "%d ", (int*)( & node.isLeaf ) ) == 1 );
  assert( node.isLeaf == false || node.isLeaf == true );
  if( node.isLeaf )
    node.info.outputDistribution = readOutputDistribution( stream );
  else
    node.info.split = readSplit( stream );
  assert( fscanf( stream, "\n" ) == 1 );
  return node;
  }


tree readTree( FILE* stream ) {
  tree t = (node*) malloc( sizeof( node ) );
  assert( t );
  *t = readNode( stream );
  if( ! t->isLeaf  ) {
    t->left = readTree( stream );
    t->right = readTree( stream );
  }
  return t;
  }

tree* readForest( FILE* stream, size_t* numTrees ) {
  assert( fscanf( stream, "%zu\n", numTrees ) == 1 );
  assert( *numTrees >= 1 && *numTrees <= 1000000 );
  tree* trees = (tree*) malloc( *numTrees * sizeof( tree ) );
  assert( trees );
  for( size_t i = 0; i < *numTrees; i++ )
    trees[i] = readTree( stream );
  return trees;
  }

static string readString( FILE* stream ) {
  char xs[ 100000 ];
  assert( fscanf( stream, "%s\n", xs ) == 1 );
  size_t n = strlen( xs );
  string s = (string) malloc( ( n + 2 ) * sizeof( char ) );
  assert( s );
  strcpy( s, xs );
  return s;
  }

void readInternalToExternal( FILE* stream, dataset* d ) {
  assert( fscanf( stream, "%zu\n", & d->numAttributes ) == 1 );
  d->numInputClasses =
    (size_t*) malloc( d->numAttributes * sizeof( size_t ) );
  assert( d->numInputClasses );
  d->internalToExternal =
    (string**) malloc( d->numAttributes * sizeof( string* ) );
  assert( d->internalToExternal );

  for( size_t a = 0; a < d->numAttributes; a++ )
    assert( fscanf( stream, "%zu\n", & d->numInputClasses[a] ) == 1 );
  for( size_t a = 0; a < d->numAttributes; a++ ) {
    d->internalToExternal[ a ] =
      (string*) malloc( d->numInputClasses[ a ] * sizeof( string ) );
    assert( d->internalToExternal[ a ] );
    }

  for( size_t a = 0; a < d->numAttributes; a++ )
    for( size_t v = 0; v < d->numInputClasses[a]; v++ )
      d->internalToExternal[ a ][ v ] = readString( stream );

  assert( fscanf( stream, "%zu\n", & d->numOutputClasses ) == 1 );
  d->outputInternalToExternal =
    (string*) malloc( d->numOutputClasses * sizeof( string ) );

  for( size_t v = 0; v < d->numOutputClasses; v++ )
    d->outputInternalToExternal[ v ] = readString( stream );

  return;
  }

