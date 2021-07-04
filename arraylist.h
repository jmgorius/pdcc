//===----------------------------------------------------------------------===//
// arraylist - Resizable array implementation
//
//===----------------------------------------------------------------------===//

#ifndef INCLUDED_ARRAYLIST_H
#define INCLUDED_ARRAYLIST_H

#include <stddef.h>
#include <stdlib.h>

#ifndef ARRAYLIST_INIT_CAPACITY
#define ARRAYLIST_INIT_CAPACITY 8
#endif

#ifndef ARRAYLIST_ABORT
#define ARRAYLIST_ABORT() abort()
#endif

typedef struct ArrayList {
  size_t capacity;
  size_t size;
  char buffer[];
} ArrayList;

#define arraylist_ptr(a)                                                       \
  ((ArrayList *)((char *)(a)-offsetof(ArrayList, buffer)))

#define arraylist_free(a)                                                      \
  do {                                                                         \
    if ((a)) {                                                                 \
      free(arraylist_ptr((a)));                                                \
      (a) = 0;                                                                 \
    }                                                                          \
  } while (0)

#define arraylist_size(a) ((a) ? arraylist_ptr((a))->size : 0)

#define arraylist_capacity(a) ((a) ? arraylist_ptr((a))->capacity : 0)

#define arraylist_push(a, e)                                                   \
  do {                                                                         \
    if (arraylist_capacity((a)) == arraylist_size((a))) {                      \
      (a) = arraylist_grow_impl((a), sizeof(*(a)),                             \
                                !arraylist_capacity((a))                       \
                                    ? ARRAYLIST_INIT_CAPACITY                  \
                                    : arraylist_capacity((a)));                \
    }                                                                          \
    (a)[arraylist_ptr((a))->size++] = (e);                                     \
  } while (0)

#define arraylist_pop(a) (a)[--arraylist_ptr((a))->size]

#define arraylist_grow(a, n) ((a) = arraylist_grow_impl((a), sizeof(*(a)), n))

#define arraylist_trunc(a, n)                                                  \
  ((a) = arraylist_grow_impl((a), sizeof(*(a)), n - arraylist_capacity((a))))

#define arraylist_clear(a) ((a) ? (arraylist_ptr((a))->size = 0) : 0)

void *arraylist_grow_impl(void *a, size_t elem_size, ptrdiff_t n);

#endif // INCLUDED_ARRAYLIST_H