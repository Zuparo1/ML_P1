
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

static size_t hash( string s ) {
  size_t hashVal;
  for( hashVal = 0; *s != '\0'; s++ )
    hashVal = *s + 31 * hashVal;
  return hashVal;
  }

struct item { string s; unsigned long long v; };
typedef struct item item;

struct hashTable { item* items; size_t size; };
typedef struct hashTable hashTable;

static void hashInsert( hashTable t, item i ) {
// We expect the load factor to be very low, which means that linear probing suffices.
  assert( t.size >= 1 );
  size_t h = hash( i.s ) % t.size;
  while( t.items[h].s != 0 ) 
    h = ( h + 1 ) % t.size;
  t.items[h] = i;
  return;
  }

static unsigned long long hashFind( hashTable t, string s ) {
  size_t h = hash( s ) % t.size;
  while( true ) {
    if( t.items[h].s == 0 ) {
      fprintf( stderr, "\nError: Unknown value %s in the dataset.\n", s );
      exit( 1 );
      }
    if( strcmp( s, t.items[h].s ) == 0 )
      return t.items[h].v;
    h++;
    }
  }

static hashTable makeHashTable( item xs[], size_t n ) {
  hashTable t;
  t.size = n * 10;
  t.items = (item*) malloc( t.size * sizeof( item ) );
  assert( t.items );
  item nullItem;
  nullItem.s = NULL;
  for( size_t i = 0; i < t.size; i++ )
    t.items[i] = nullItem;
  for( size_t i = 0; i < n; i++ )
    hashInsert( t, xs[i] );
  return t;
  }

struct indexizedAttr {
  attributeType attributeType;
  attributeValue internal;
  bool isMissingValue;
  };
typedef struct indexizedAttr indexizedAttr;

static void
  copyColumn(
    indexizedAttr* columnTmp,
    csvFile csvF,
    size_t attributeIndex,
    attributeType attributeType,
    hashTable t
    ) {
  for(
    size_t exampleIndex = 0;
    exampleIndex < csvF.numRows;
    exampleIndex++
    ) {
    char* s = csvF.rows[ exampleIndex ].fields[ attributeIndex ];
    columnTmp[ exampleIndex ].isMissingValue = strcmp( s, "?" ) == 0;
    if( columnTmp[ exampleIndex ].isMissingValue ) {
      columnTmp[ exampleIndex ].internal.ord = MISSING_VALUE;
      continue;
      }
    if( attributeType == ORDINAL || attributeType == WEIGHT ) {
      errno = 0;
      double x = strtod( s, NULL );
      if( errno != 0 ) {
        fprintf( stderr, "\nError: %s is an illegal floating point number.\n", s );
        fflush( stderr ); exit( 1 );
        }
      columnTmp[ exampleIndex ].internal.ord = x;
      continue;
      }
    if( attributeType == NOMINAL || attributeType == OUTPUT ) 
      columnTmp[ exampleIndex ].internal.nom = hashFind( t, s );
    }
  }


static size_t
  count(
    attributeType t,
    attributeType* columnTypes,
    size_t numColumns
    ) {
  size_t n = 0;
  for( size_t i = 0; i < numColumns; i++ )
    if( columnTypes[i] == t )
      n++;
  return n;
  }


static void 
  csvFileToDataset(
    csvFile csvF,
    attributeType* columnTypes,
    size_t numColumns,
    dataset* d 
// Some fields of d are filled in by readInternalToExternal and the remaining below.
    ) {
  for( size_t i = 0; i < csvF.numRows; i++ ) 
    if( csvF.rows[i].numFields != numColumns ) {
      fprintf( stderr, "\nError: Wrong number of columns on line %zu in the dataset. ", i+1 );
      fflush( stderr );
      }
  for( size_t i = 0; i < csvF.numRows; i++ ) 
    assert( csvF.rows[i].numFields == numColumns ); 
  assert( count( LABEL, columnTypes, numColumns ) <= 1 );
  assert( count( WEIGHT, columnTypes, numColumns ) <= 1 );
  assert( count( OUTPUT, columnTypes, numColumns ) == 1 );

  d->numAttributes = 
    count( NOMINAL, columnTypes, numColumns ) +
    count( ORDINAL, columnTypes, numColumns );
  assert( d->numAttributes >= 1 );
  d->numExamples = csvF.numRows;

  d->attributeTypes = 
    (attributeType*) malloc( d->numAttributes * sizeof( attributeType ) );
  size_t a = 0;
  for( size_t c = 0; c < numColumns; c++ )
    if( columnTypes[c] == ORDINAL || columnTypes[c] == NOMINAL )
      d->attributeTypes[ a++ ] = columnTypes[c];

  d->inputs =
    (inputColumn*) malloc( d->numAttributes * sizeof( inputColumn ) );
  assert( d->inputs );

  for( size_t a = 0; a < d->numAttributes; a++ ) {
    d->inputs[ a ] = // An input column
      (attributeValue*) malloc( d->numExamples * sizeof( attributeValue ) );
    assert( d->inputs[a] );
    }

  d->outputs =
    (outputClass*) malloc( d->numExamples * sizeof( outputClass ) );
  assert( d->outputs );

  d->weights =
    (double*) malloc( d->numExamples * sizeof( double ) );
  assert( d->weights );
  for( size_t e = 0;  e < d->numExamples; e++ )
    d->weights[e] = 1.0;

  hashTable tables[ d->numAttributes ];
  for( size_t a = 0; a < d->numAttributes; a++ ) {
    if( d->attributeTypes[a] != NOMINAL ) 
      continue;
    size_t n = d->numInputClasses[a];
    item xs[n];
    for( size_t i = 0; i < n; i++ ) {
      xs[i].v = i;
      xs[i].s = d->internalToExternal[ a ][ i ];
      }
    tables[a] = makeHashTable( xs, n );
    }

  size_t n = d->numOutputClasses;
  item xs[n];
  for( size_t i = 0; i < n; i++ ) {
    xs[i].v = i;
    xs[i].s = d->outputInternalToExternal[ i ];
    }
  hashTable outputTable = makeHashTable( xs, n );

  indexizedAttr* columnTmp =
    (indexizedAttr*) malloc( d->numExamples * sizeof( indexizedAttr ) );
  assert( columnTmp );

  size_t attributeIndex = 0; 
  for( 
    size_t columnIndex = 0; 
    columnIndex < numColumns; 
    columnIndex++
    ) {
    if( columnTypes[ columnIndex ] == LABEL ||
        columnTypes[ columnIndex ] == IGNORE
        )
      continue;
    copyColumn( 
      columnTmp, 
      csvF, 
      columnIndex, 
      columnTypes[ columnIndex ],
      columnTypes[ columnIndex ] == OUTPUT ? outputTable : tables[ attributeIndex ]
      );
    if( columnTypes[ columnIndex ] == WEIGHT ) {
      for( size_t e = 0;  e < csvF.numRows; e++ )
        d->weights[ e ] = columnTmp[e].internal.ord;
      continue;
      }
    if( 
      columnTypes[ columnIndex ] == ORDINAL || 
      columnTypes[ columnIndex ] == NOMINAL 
      ) {
      for( size_t e = 0;  e < csvF.numRows; e++ )
        d->inputs[ attributeIndex ][ e ] = columnTmp[e].internal;
      attributeIndex++;
      }
    else {
      assert( columnTypes[ columnIndex ] == OUTPUT );
      for( size_t e = 0;  e < csvF.numRows; e++ )
        d->outputs[ e ] = columnTmp[e].internal.nom;
      } // end else

    } // end for

  free( columnTmp );
  return;
  }

  

csvFile 
  readDataset(
    char* filename,
    attributeType* columnTypes,
    size_t numColumns,
    size_t maxNumRows,
    dataset* d
    ) {
  csvFile csvF = readCsvFile( filename, maxNumRows );
  assert( csvF.numRows >= 1 );
  csvFileToDataset( csvF, columnTypes, numColumns, d );
  return csvF;
  }
