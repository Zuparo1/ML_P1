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
#ifndef __TYPES_H_INCLUDED__
#define __TYPES_H_INCLUDED__

#include <stdlib.h>
#include <stdbool.h>

typedef enum { NOMINAL, ORDINAL, WEIGHT, LABEL, IGNORE, OUTPUT } attributeType;
#define NONE 255

union attributeValue {
  unsigned long long nom;
  double ord;
  };
typedef union attributeValue attributeValue;

#define MISSING_VALUE 1.7976931348623157E308

typedef attributeValue* inputColumn;
typedef inputColumn* inputs;
typedef unsigned short outputClass;
typedef outputClass* outputs;
typedef char* string;

struct dataset { 
  size_t numAttributes;
  size_t numExamples; 
    // An example is an input and an output, that is one row in the csv dataset file.
  attributeType* attributeTypes;
  size_t* numInputClasses;
  size_t numOutputClasses;
  string** internalToExternal; 
    // To translate from the internal representation to the external one. 
    // Used as internalToExternal[ attributeIndex ][ attributeValue.nom ].

  string* outputInternalToExternal; 
  // Internal representation:
  inputs inputs;
  outputs outputs;
  double* weights;
  };
typedef struct dataset dataset;

typedef double* outputDistribution; 
  // For class c, dist[c] is the sum of the derivative terms for c.
  // Length is numOutputClasses


// Types for tree nodes:

typedef unsigned short* bitmask;

union bitmaskOrLimit {
  bitmask bitmask;
  double limit;
  };

struct split {
  size_t attributeIndex;
  attributeType attributeType;
  union bitmaskOrLimit bitmaskOrLimit;
  bool missingGoesLeft;
  };
typedef struct split split;

union info {
  split split; // For an internal node.
  outputDistribution outputDistribution; // For a leaf.
  };
typedef union info info;

struct node {
  bool  isLeaf;
  info info;
  struct node* left;
  struct node* right;
  };
typedef struct node node;
typedef struct node* tree;

#endif
