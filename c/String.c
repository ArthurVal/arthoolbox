#include "String.h"

#include <stdio.h>  /* perror */
#include <stdlib.h> /* malloc/free/... */

struct atb_String atb_String_MakeByMovingSubStr(char **const other,
                                                size_t capacity, size_t size) {
  assert(other != NULL);
  assert(*other != NULL);
  assert(capacity > size);

  struct atb_String str;
  str.data = *other;
  str.capacity = capacity;
  str.size = size;

  str.data[str.size] = '\0';

  *other = NULL;
  return str;
}

void atb_String_Reserve(struct atb_String *const str, size_t new_capacity) {
  assert(str != NULL);

  if (new_capacity == 0)
    return;

  if (str->data == NULL) {
    if ((str->data = malloc(new_capacity * sizeof(*str->data))) == NULL) {
      perror("atb_String_Reserve - malloc");
      exit(EXIT_FAILURE);
    }

    str->capacity = new_capacity;
    str->size = 0;

  } else {
    if ((str->data = realloc(str->data, new_capacity * sizeof(*str->data))) ==
        NULL) {
      perror("atb_String_Reserve - realloc");
      exit(EXIT_FAILURE);
    }

    str->capacity = new_capacity;

    if (str->capacity <= str->size) {
      str->size = str->capacity - 1;
      str->data[str->size] = '\0';
    }
  }
}

static void FillRangeWithValue(void *const value, char *first, char *last) {
  memset(first, *((int *)value), (size_t)(last - first));
}

void atb_String_Resize(struct atb_String *const str, size_t new_size,
                       int fill) {
  assert(str != NULL);

  struct atb_String_Generator gen = {
      .self = &fill,
      .FillRange = (fill >= 0 ? &FillRangeWithValue : NULL),
  };

  atb_String_ResizeAndFill(str, new_size, gen);
}

void atb_String_Delete(struct atb_String *const str) {
  assert(str != NULL);

  if (str->data != NULL) {
    free(str->data);
  }

  atb_String_Init(str);
}

void atb_String_AppendSubStr(struct atb_String *const str,
                             struct atb_ConstStringView view) {
  assert(str != NULL);
  assert(view.data != NULL);
  assert(str->data != view.data);

  if (view.size == 0)
    return;

  if (str->capacity <= (str->size + view.size)) {
    /* The capacity is too small -> needs to be expanded */
    atb_String_Reserve(str, str->size + view.size + 1);
  }

  memcpy(str->data + str->size, view.data, view.size);
  str->size += view.size;
  str->data[str->size] = '\0';
}

bool atb_String_IsEqualToSubStr(struct atb_String const *const lhs,
                                struct atb_ConstStringView rhs) {
  assert(lhs != NULL);

  return ((lhs->data == NULL) && (rhs.data == NULL)) ||
         ((lhs->size == rhs.size) &&
          (memcmp(lhs->data, rhs.data, rhs.size) == 0));
}
