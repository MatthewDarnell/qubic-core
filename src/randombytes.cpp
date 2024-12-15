//
// Created by Matthew Darnell on 12/14/24.
//
#include <stdlib.h>
#include "randombytes.h"


void randombytes(unsigned int *data)
{
  *data = arc4random();
}