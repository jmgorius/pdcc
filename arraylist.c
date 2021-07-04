#include "arraylist.h"

void *arraylist_grow_impl(void *a, size_t elem_size, ptrdiff_t n) {
  ArrayList *ptr = 0;
  size_t max = (size_t)-1 - sizeof(ArrayList);

  if (a) {
    ptr = arraylist_ptr(a);
    if (n > 0 && ptr->capacity + n > max / elem_size)
      goto fail; // overflow

    ptr = realloc(ptr, sizeof(ArrayList) + elem_size * (ptr->capacity + n));
    if (!ptr)
      goto fail;

    ptr->capacity += n;

    // Clamp the size to the capacity
    if (ptr->size > ptr->capacity)
      ptr->size = ptr->capacity;
  } else {
    if ((size_t)n > max / elem_size)
      goto fail; // overflow

    ptr = malloc(sizeof(ArrayList) + elem_size * n);
    if (!ptr)
      goto fail;

    ptr->capacity = n;
    ptr->size = 0;
  }
  return ptr->buffer;
fail:
  ARRAYLIST_ABORT();
  return 0;
}