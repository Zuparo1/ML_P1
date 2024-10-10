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
#include <ctype.h>
#include <assert.h>
#include "read_names_file.h"

static int nonSpaceIndex( char* s, int curr ) {
  if( ! isspace( s[ curr ] ) )
    return curr;
  return nonSpaceIndex( s, curr + 1 );
  }

static int spaceIndex( char* s, int curr ) {
  if( s[ curr] == 0 || isspace( s[ curr ] ) )
    return curr;
  return spaceIndex( s, curr + 1 );
  }


static int countTokens( char* s, int curr ) {
  curr = nonSpaceIndex( s, curr );
  if( s[ curr ] == 0 )
    return 0;
  curr = spaceIndex( s, curr );
  return 1 + countTokens( s, curr );
  }

static char* readToken( char* s, int* curr ) {
  int first = nonSpaceIndex( s, *curr );
  int lastplus1 = spaceIndex( s, first );
//  printf( "\nfirst = %d lastplus1 = %d\n", first, lastplus1 ); fflush( stdout );
  assert( first < lastplus1 );
  char* t = (char*) malloc( ( lastplus1 - first + 1 ) * sizeof( char ) );
  for( int i = first; i < lastplus1; i++ )
    t[ i - first ] = s[ i ];
  t[ lastplus1 -  first ] = 0;
  *curr = lastplus1;
  return t;
  }


static char** makeTokens( char* s, int n ) {
  char** tokens = (char**) malloc( n * sizeof( char* ) );
  assert( tokens );
  int curr = 0;
  for( int i = 0; i < n; i++ ) 
    tokens[ i ] = readToken( s, &curr );
  return tokens;
  }

static char* readEntireFile( char* fileName ) {
  FILE* f = fopen( fileName, "r" );
  if( f == NULL ) {
    fprintf( stderr, "\nError: Could not open %s\n", fileName );
    fflush( stderr );
    exit( 1 );
    }
  fseek( f, 0, SEEK_END );
  long fsize = ftell( f );
  rewind( f );
  char* s = (char*) malloc( ( fsize + 10 ) * sizeof( char ) );
  assert( s );
  assert( fread( s, fsize, 1, f ) );
  assert( fclose( f ) == 0 );
  s[ fsize -1 ] = '\0';
  return s;
  }

static char** fileToTokens( char* fileName, int* n ) {
  char* s = readEntireFile( fileName );
  *n = countTokens( s, 0 );
//  printf( "\nnum tokens = %d\n", *n ); fflush( stdout );
  char** tokens = makeTokens( s, *n );
  free( s );
  return tokens;
  }

static bool isNominal( char* t ) { 
  return 
    strcmp( t, "name" ) == 0 || 
    strcmp( t, "names" ) == 0 || 
    strcmp( t, "nom" ) == 0 || 
    strcmp( t, "nominal" ) == 0;
   }

static bool isOrdinal( char* t ) { 
  return 
    strcmp( t, "ordered" ) == 0 || 
    strcmp( t, "ord" ) == 0 || 
    strcmp( t, "ordinal" ) == 0;
   }

static bool isLabel( char* t ) { 
  return strcmp( t, "lab" ) == 0 || strcmp( t, "label" ) == 0;
   }

static bool isIgnore( char* t ) { 
  return strcmp( t, "ign" ) == 0 || strcmp( t, "ignore" ) == 0;
   }

static bool isWeight( char* t ) { 
  return strcmp( t, "wei" ) == 0 || strcmp( t, "weight" ) == 0;
   }

static bool isOutput( char* t ) { 
  return strcmp( t, "out" ) == 0 || strcmp( t, "output" ) == 0;
   }

static bool isUnsigned( char* t ) {
  while( *t )
    if( ! isdigit( *(t++) ) )
      return false;
  return true;
  }

static int countAttributes( char** tokens, int numTokens, int curr, int mult ) {
  if( curr >= numTokens )
    return 0;
  char* t = tokens[ curr ];
  if( isUnsigned( t ) ) {
    int mult = atoi( t );
    assert( mult >= 1 );
    return countAttributes( tokens, numTokens, curr + 1, mult );
    }
  assert( isNominal( t ) || isOrdinal( t ) || isLabel( t ) || 
    isIgnore( t ) || isWeight( t ) || isOutput( t )
    );
  return mult + countAttributes( tokens, numTokens, curr + 1, 1 );
  }


static void 
  fillInAttributeTypes( 
    char** tokens, 
    int numTokens, 
    int currToken, 
    attributeType* attrTypes,
    int currAttrType,
    int mult
    ) {
  if( currToken >= numTokens )
    return;
  char* t = tokens[ currToken ];
  if( isUnsigned( t ) ) {
    int mult = atoi( t );
    assert( mult >= 1 );
    return 
      fillInAttributeTypes( tokens, numTokens, currToken + 1, 
        attrTypes, currAttrType, mult );
    }
  assert( isNominal( t ) || isOrdinal( t ) || isLabel( t ) || 
    isIgnore( t ) || isWeight(t) || isOutput( t )
    );
  attributeType at =
    isNominal( t ) ? NOMINAL :
    isOrdinal( t ) ? ORDINAL :
    isLabel( t ) ? LABEL :
    isIgnore( t ) ? IGNORE :
    isWeight( t ) ? WEIGHT :
    OUTPUT;
  for( int i = 0; i < mult; i++ )
    attrTypes[ currAttrType + i ] = at;
  return 
    fillInAttributeTypes( tokens, numTokens, currToken + 1, 
      attrTypes, currAttrType + mult, 1 );
  }

attributeType* readNamesFile( char* fileName, int* numAttributes ) {
  int numTokens = 0;
  char** tokens = fileToTokens( fileName, &numTokens );
  *numAttributes = countAttributes( tokens, numTokens, 0, 1 );
  attributeType* attrTypes = 
    (attributeType*) malloc( *numAttributes * sizeof( attributeType ) );
  assert( attrTypes );
  fillInAttributeTypes( tokens, numTokens, 0, attrTypes, 0, 1 );
  for( int i = 0; i < numTokens; i++ )
    free( tokens[ i ] );
  free( tokens );
  return attrTypes;
  }

