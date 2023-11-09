#pragma once

#include <assert.h>
#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */
#include <string.h>  /* strlen */

#include "StaticArray.h" /* GetSize */

/**
 *  \brief Dynamic c-string container on the heap
 */
struct atb_String {
  char *data;
  size_t capacity;
  size_t size;
};

/**
 *  \brief Non-owning c-string view
 */
struct atb_StringView {
  char *data;
  size_t size;
};

/**
 *  \brief Non-owning constant c-string view
 */
struct atb_ConstStringView {
  const char *data;
  size_t size;
};

/**
 *  \return atb_StringView From other c-string
 *
 *  \param[in] other Ptr to the begin of a raw c-string (null-terminated)
 */
static inline struct atb_StringView atb_StringView_FromCStr(char *const other);

/**
 *  \return atb_StringView From other string
 *
 *  \param[in] other Ptr to the begin of a raw c-string (null-terminated)
 */
static inline struct atb_StringView
atb_StringView_FromStr(struct atb_String *const other);

/**
 *  \return atb_ConstStringView TODO
 *
 *  \param[in] str TODO
 */
#define atb_ConstStringView_FromStaticString(str)                              \
  (struct atb_ConstStringView) {                                               \
    .data = str, .size = (atb_StaticArray_GetSize(str) - 1)                    \
  }

/**
 *  \return atb_ConstStringView From other
 *
 *  \param[in] other Ptr to the begin of a raw const c-string (null-terminated)
 */
static inline struct atb_ConstStringView
atb_ConstStringView_FromCStr(const char *const other);

/**
 *  \return atb_ConstStringView From other string
 *
 *  \param[in] other Ptr to the begin of a raw const c-string (null-terminated)
 */
static inline struct atb_ConstStringView
atb_ConstStringView_FromStr(struct atb_String const *const other);

/**
 *  \brief Statically initialize a atb_String data structure
 */
#define atb_String_INITIALIZER()                                               \
  { NULL, 0, 0 }

/**
 *  \brief Initialize a atb_String data structure
 */
static inline void atb_String_Init(struct atb_String *const str);

/**
 *  \return atb_String An empty string
 */
static inline struct atb_String atb_String_MakeEmpty();

/**
 *  \return atb_String A Copy of N char from other
 *
 *  \param[in] view Sub c-string view
 */
static inline struct atb_String
atb_String_MakeCopyFromSubStr(struct atb_ConstStringView view);

/**
 *  \return atb_String A Copy of other
 */
static inline struct atb_String
atb_String_MakeCopyFrom(const struct atb_String *const other);

/**
 *  \return atb_String A new string using the buffer pointed by other
 *
 *  \note The string will TAKES OWNERSHIP of the underlying buffer. Hence, the
 *        value pointed by other will be set to NULL afterward.
 *
 *  \note Since atb_String use free to deallocate, it is expected that the
 *        pointer pointed by other has been allocated using malloc.
 *
 *  \param[inout] other Ptr of a buffer that the string will use
 *  \param[in] capacity Expected capacity of the buffer
 *  \param[in] size Expected size (if relevant) of the buffer
 */
struct atb_String atb_String_MakeByMovingSubStr(char **const other,
                                                size_t capacity, size_t size);

/**
 *  \return atb_String A new string using the null terminated c-str other
 *
 *  \note The string will TAKES OWNERSHIP of the underlying c-str. Hence, the
 *        value pointed by other will be set to NULL afterward.
 *
 *  \note Since atb_String use free to deallocate, it is expected that the c-str
 *        pointed by other has been allocated using malloc.
 *
 *  \param[inout] other Ptr of a null-terminated c-str that the string will use
 */
static inline struct atb_String atb_String_MakeByMovingCStr(char **const other);

/**
 *  \return atb_String A new string moving all data from other
 *  \param[inout] other atb_String that will be moved from.
 */
static inline struct atb_String
atb_String_MakeByMoving(struct atb_String *const other);

/**
 *  \brief Grow/Shrink the internal string allocated memory buffer
 *  \param[in] capacity The new capacity, in bytes (include the null char)
 */
void atb_String_Reserve(struct atb_String *const str, size_t capacity);

/**
 *  \brief TODO
 */
struct atb_String_Generator {
  void *self;
  void (*FillRange)(void *const self, char *first, char *last);
};

/**
 *  \brief Resize the string and fill it (if grown) using the generator provided
 *  \param[in] new_size The new size, in number of char (null char not included)
 *  \param[in] gen The generator used to fill the string, if the string grow
 */
static inline void
atb_String_ResizeAndFill(struct atb_String *const str, size_t new_size,
                         const struct atb_String_Generator gen);

/**
 *  \brief Resize the string and fill it (if grown) with the given value
 *  \param[in] new_size The new size, in number of char (null char not included)
 *  \param[in] fill If >= 0, the char used to fill the string.
 */
void atb_String_Resize(struct atb_String *const str, size_t new_size, int fill);

/**
 *  \brief Free all memory allocated by the string
 */
void atb_String_Delete(struct atb_String *const str);

/**
 *  \brief Shrink the underlying buffer of the string in order to match the size
 */
static inline void atb_String_ShrinkToFit(struct atb_String *const str);

/**
 *  \brief Append size byte of other at the end of str
 *  \param[in] view A sub c-string range
 */
void atb_String_AppendSubStr(struct atb_String *const str,
                             struct atb_ConstStringView view);

/**
 *  \brief Append other at the end of str
 *
 *  \param[in] other An other atb_String appended into str
 */
static inline void atb_String_Append(struct atb_String *const str,
                                     struct atb_String const *const other);

/**
 *  \brief Pop N char from the string
 *
 *  \param[in] n Number of character to pop from the string
 */
static inline void atb_String_PopN(struct atb_String *const str, size_t n);

/**
 *  \return True if lhs is equal to rhs (up to size char)
 *
 *  \param[in] rhs The string to compare to
 *  \param[in] size The number of char used to compare rhs with lhs
 */
bool atb_String_IsEqualToSubStr(struct atb_String const *const lhs,
                                struct atb_ConstStringView rhs);

/**
 *  \return True if lhs is equal to rhs
 *
 *  \param[in] rhs A atb_string to compare to
 */
static inline bool atb_String_IsEqualTo(struct atb_String const *const lhs,
                                        struct atb_String const *const rhs);

/*****************************************************************************/
/*                             STATIC INLINE Definitions */
/*****************************************************************************/

/* StringView ****************************************************************/
static inline struct atb_StringView atb_StringView_FromCStr(char *const other) {
  assert(other != NULL);

  struct atb_StringView output;
  output.data = other;
  output.size = strlen(other);
  return output;
}

static inline struct atb_StringView
atb_StringView_FromStr(struct atb_String *const other) {
  assert(other != NULL);

  struct atb_StringView output;
  output.data = other->data;
  output.size = other->size;
  return output;
}

/* ConstStringView ***********************************************************/
static inline struct atb_ConstStringView
atb_ConstStringView_FromCStr(const char *const other) {
  assert(other != NULL);

  struct atb_ConstStringView output;
  output.data = other;
  output.size = strlen(other);
  return output;
}

static inline struct atb_ConstStringView
atb_ConstStringView_FromStr(struct atb_String const *const other) {
  assert(other != NULL);

  struct atb_ConstStringView output;
  output.data = other->data;
  output.size = other->size;
  return output;
}

/* String ********************************************************************/
static inline void atb_String_Init(struct atb_String *const str) {
  assert(str != NULL);

  str->data = NULL;
  str->capacity = 0;
  str->size = 0;
}

static inline struct atb_String atb_String_MakeEmpty() {
  struct atb_String str;
  atb_String_Init(&str);
  return str;
}

static inline struct atb_String
atb_String_MakeCopyFromSubStr(struct atb_ConstStringView view) {
  assert(view.data != NULL);

  struct atb_String output = atb_String_MakeEmpty();
  atb_String_AppendSubStr(&(output), view);

  return output;
}

static inline struct atb_String
atb_String_MakeCopyFrom(const struct atb_String *const other) {
  assert(other != NULL);

  return atb_String_MakeCopyFromSubStr(atb_ConstStringView_FromStr(other));
}

static inline struct atb_String
atb_String_MakeByMovingCStr(char **const other) {
  assert(other != NULL);
  assert(*other != NULL);

  const size_t str_size = strlen(*other);
  return atb_String_MakeByMovingSubStr(other, str_size + 1, str_size);
}

static inline struct atb_String
atb_String_MakeByMoving(struct atb_String *const other) {
  assert(other != NULL);

  struct atb_String output = *other;
  atb_String_Init(other);
  return output;
}

static inline void
atb_String_ResizeAndFill(struct atb_String *const str, size_t new_size,
                         const struct atb_String_Generator gen) {
  assert(str != NULL);

  if (new_size >= str->capacity) {
    atb_String_Reserve(str, new_size + 1);

    if (gen.FillRange != NULL) {
      /* NOTE, size is superior to str->size (which is the old size) since here
       * size is superior or equals to the previous capacity. Hence begin <= end
       */
      char *begin = str->data + str->size;
      char *end = str->data + new_size;
      gen.FillRange(gen.self, begin, end);
    }
  }

  str->size = new_size;
  str->data[str->size] = '\0';
}

static inline void atb_String_ShrinkToFit(struct atb_String *const str) {
  assert(str != NULL);

  if (str->capacity > (str->size + 1)) {
    atb_String_Reserve(str, (str->size + 1));
  }
}

static inline void atb_String_Append(struct atb_String *const str,
                                     struct atb_String const *const other) {
  assert(str != NULL);
  assert(other != NULL);
  assert(str != other);
  assert(str->data != other->data);

  atb_String_AppendSubStr(str, atb_ConstStringView_FromStr(other));
}

static inline void atb_String_PopN(struct atb_String *const str, size_t n) {
  assert(str != NULL);

  if (str->capacity != 0) {
    if (n > str->size) {
      n = str->size;
    }

    str->size -= n;
    str->data[str->size] = '\0';
  }
}

static inline bool atb_String_IsEqualTo(struct atb_String const *const lhs,
                                        struct atb_String const *const rhs) {
  assert(lhs != NULL);
  assert(rhs != NULL);

  return atb_String_IsEqualToSubStr(lhs, atb_ConstStringView_FromStr(rhs));
}
