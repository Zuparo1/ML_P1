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
#include "read_csv_file.h"


static size_t countFields( char* s ) {
  size_t n = 0;
  while( strsep( &s, "," ) != NULL )
    n++;
  return n;
  }

static char* removeSpace( char* s ) {
  while( isspace( *s ) )
    s++;
  char* t = s;
  while( *t != 0 )
    t++;
  t--;
  while( isspace( *t ) )
    t--;
  t++;
  *t = 0;
  return s;
  }

static void fillInFields( char** fields, char* s ) {
  size_t n = 0;
  char* p = strsep( &s, "," );
  while( p != NULL ) {
    p = removeSpace( p );
    char* q = (char*) malloc( ( strlen( p ) + 2 ) * sizeof( char ) );
    assert( q );
    strcpy( q, p );
    fields[ n++ ] = q;
    p = strsep( &s, "," );
    }
  }


static void freeRow( row row ) {
  for( size_t i = 0; i < row.numFields; i++ )
    free( row.fields[i] );
  if( row.fields != NULL ) free( row.fields );
  }

void freeCsvFile( csvFile csvF ) {
  for( size_t i = 0; i < csvF.numRows; i++ )
    freeRow( csvF.rows[i] );
  if( csvF.rows != NULL ) free( csvF.rows );
  }


#define BUFSIZE 10000000
static char buf1[ BUFSIZE ];
static char buf2[ BUFSIZE ];

static const row emptyRow = { NULL, 0 };
static const csvFile emptyCsvFile = { NULL, 0 };

static row readRowAux( FILE* stream ) {
  char* line = fgets( buf1, BUFSIZE - 10, stream );
  if( line == NULL ) return emptyRow;
  if( strlen( buf1 ) > BUFSIZE - 20 ) {
    fprintf( stderr, "\nError: Too long line found.\n" );
    fflush( stderr );
    exit( 1 );
    }
  assert( strcpy( buf2, buf1 ) );
  row row;
  row.numFields = countFields( buf1 ); // buf1 is overwritten.
  if( row.numFields == 0 ) return emptyRow;
  row.fields = (char**) malloc( row.numFields * sizeof( char* ) );
  assert( row.fields );
  fillInFields( row.fields, buf2 );
  return row;
  }

static row readRow( FILE* stream ) {
  row row = readRowAux( stream );
  if( row.numFields == 1 ) {
    freeRow( row );
    return readRow( stream );
    }
  return row;
  }
  
static size_t countRows( char* fileName ) {
  FILE* stream = fopen( fileName, "r" );
  if( stream == NULL ) {
    fprintf( stderr, "\nError: Cannnot open the file %s\n", fileName );
    fflush( stderr );
    exit( 1 );
    }
  size_t numRows = 0;
  row row;
  row = readRow( stream );
  while( row.numFields >= 1 ) { // 0 means end of file.
    freeRow( row );
    numRows++;
    row = readRow( stream );
    }
  freeRow( row );
  int errCode = fclose( stream );
  if( errCode != 0 ) {
    fprintf( stderr, "\nError: Cannnot close the file %s\n", fileName );
    fflush( stderr );
    exit( 1 );
    }
  return numRows;
  }
  
csvFile readCsvFile( char* fileName, size_t maxNumRows ) {
  csvFile csvF;
  csvF.numRows = countRows( fileName );
  if( csvF.numRows > maxNumRows ) csvF.numRows = maxNumRows;
  if( csvF.numRows == 0 ) return emptyCsvFile;
  FILE* stream = fopen( fileName, "r" );
  if( stream == NULL ) {
    fprintf( stderr, "\nError: cannnot open the file %s\n", fileName );
    fflush( stderr );
    exit( 1 );
    }
  csvF.rows = (row*) malloc( csvF.numRows * sizeof( row ) );
  assert( csvF.rows );
  for( size_t i = 0; i < csvF.numRows; i++ ) {
    csvF.rows[i] = readRow( stream );
    assert( csvF.rows[i].numFields >= 1 );
    }
  int errcode = fclose( stream );
  if( errcode != 0 ) {
    fprintf( stderr, "\nError: cannnot close the file %s\n", fileName );
    fflush( stderr );
    exit( 1 );
    }
  return csvF;
  }
  

static void printrow( row row ) {
  for( size_t i = 0; i < row.numFields; i++ )
    if( i == row.numFields - 1 )
      printf( "%s\n", row.fields[i] );
    else
      printf( "%s, ", row.fields[i] );
  }
  
void printcsvfile( csvFile csvF ) {
  for( size_t i = 0; i < csvF.numRows; i++ )
    printrow( csvF.rows[i] );
  }
  
