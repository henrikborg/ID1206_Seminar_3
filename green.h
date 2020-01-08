#include  <ucontext.h>

typedef struct green_t {
  ucontext_t *context;
  void *(*fun)(void*);
  void *arg;
  struct green_t *next;
  struct green_t *join;
  void *retval;
  int zombie;
} green_t;

typedef struct green_thread_queue {
  struct green_t *next;
  struct green_t *end;
} green_thread_queue_t;

typedef struct cond {
  struct green_t *next;
  struct green_t *end;
} green_cond_t;

int green_disable_timer();
int green_enable_timer();
int green_create(green_t *thread, void *(*fun)(void*), void *arg);
int green_yield();
int green_join(green_t *thread, void **val);
void green_cond_init(green_cond_t*);
void green_cond_wait(green_cond_t*);
void green_cond_signal(green_cond_t*);
