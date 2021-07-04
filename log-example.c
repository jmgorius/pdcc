#include "log.h"

#include <threads.h>

#define THREAD_COUNT 10
#define THREAD_ITERATIONS 20

int thread_function(void *data) {
  long thread_id = (long)data;
  for (int i = 0; i < THREAD_ITERATIONS; ++i) {
    log_info("Thread %ld: %d", thread_id, i);
  }
  log_warning("Thread %ld terminated", thread_id);

  return thrd_success;
}

void lock_mutex(bool lock, void *data) {
  if (lock)
    mtx_lock((mtx_t *)data);
  else
    mtx_unlock((mtx_t *)data);
}

int main(void) {
  mtx_t mutex;
  mtx_init(&mutex, mtx_plain);

  log_set_lock_func(lock_mutex, &mutex);

  thrd_t threads[THREAD_COUNT];
  // Launch all threads
  for (long i = 0; i < THREAD_COUNT; ++i)
    thrd_create(&threads[i], thread_function, (long *)i);

  // Wait for the threads to finish
  for (int i = 0; i < THREAD_COUNT; ++i)
    thrd_join(threads[i], 0);

  mtx_destroy(&mutex);
  return 0;
}
