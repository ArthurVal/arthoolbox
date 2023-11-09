#pragma once

#include <assert.h>

#define atb_StaticArray_GetSize(array) (sizeof(array) / sizeof(*array))

#define atb_StaticArray_Begin(array) array
#define atb_StaticArray_End(array)                                             \
  atb_StaticArray_Begin(array) + atb_StaticArray_GetSize(array)

#define atb_StaticArray_ForEach(elem, array)                                   \
  for (elem = atb_StaticArray_Begin(array);                                    \
       elem != atb_StaticArray_End(array); ++elem)

#define atb_StaticArray_RBegin(array)                                          \
  atb_StaticArray_End(array) - 1
#define atb_StaticArray_REnd(array)                                            \
  atb_StaticArray_RBegin(array) - atb_StaticArray_GetSize(array)

#define atb_StaticArray_RForEach(elem, array)                                  \
  for (elem = atb_StaticArray_RBegin(array);                                   \
       elem != atb_StaticArray_REnd(array); --elem)
