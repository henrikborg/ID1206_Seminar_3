#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include "green.h"
#include <stdio.h>

#define FALSE 0
#define TRUE 1
#define STACK_SIZE 4096
#define PERIOD 100

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, FALSE};
static green_t *running = &main_green;
//green_t *ready_queue_end; // for task 2.4 according to 
                            // git tag named v1.0 <git tag -l -n2>
green_thread_queue_t ready_queue;

#ifdef TIMER
  static sigset_t block;

  void timer_handler(int);
#endif

static void init() __attribute__((constructor));

void init() {
  getcontext(&main_cntx);

#ifdef TIMER
  sigemptyset(&block);
  sigaddset(&block, SIGVTALRM);

  struct sigaction act = {0};
  act.sa_handler = timer_handler;
  assert(sigaction(SIGVTALRM, &act, NULL) == 0);

  struct timerval interval;
  sigprocmask(SIG_UNBLOCK, &block, NULL);
  interval.tv_sec = 0;
  interval.tv_usec = PERIOD;

  struct itimerval period;
  period.it_interval = interval;
  period.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &period, NULL),
#endif
}

#ifdef TIMER
  int green_disable_timer() {sigprocmask(SIG_BLOCK, &block, NULL);}
  int green_enable_timer () {sigprocmask(SIG_BLOCK, &block, NULL);}
#else
  int green_disable_timer() {}
  int green_enable_timer() {}
#endif


void add_to_end_of_ready_queue(green_t *new) {
//  printf("add to end of queue\n");
/*  // This works for task 2.4 of we change code below a bit
    // according to the git tag named v1.0 <git tag -l -n2>
    if(NULL == ready_queue_end) {
    ready_queue_end = new;
    ready_queue_end->next = NULL;//new;
    running->next = new;
  } else {
    ready_queue_end->next = new;
    ready_queue_end = new;
  }*/

  // Add new to the readyqueue queue
  if(NULL != new)
  if(ready_queue.next == NULL) {
    ready_queue.next = new;
    //running->next = new;
    ready_queue.end = new;
  } else {
    ready_queue.end->next = new;
    ready_queue.end = new;
  }
}

green_t* schedule() {
  // select next thread for execution
  green_t *next = ready_queue.next;

  if(next == ready_queue.end) {
    // reached end of queue
    ready_queue.next = NULL;
    ready_queue.end = NULL;
//    printf("READY SCHEDULE EMPTY\n");
  } else {
    if(NULL != next)
      ready_queue.next = ready_queue.next->next;
  }

  return next;
}

void green_thread() {
//  printf("green_thread\n");
  green_disable_timer();

  green_t *this = running;

  void* result = (*this->fun)(this->arg);
  this->retval = result;

  // place waiting (joining) thread in ready queue
  if(NULL != this->join) {
    add_to_end_of_ready_queue(this->join);
    this->join = NULL;
  }

  // save result of execution
  this->retval = result;

  // we're  a zombie
  this->zombie = 1;

  //green_yield();

/*  // find the next thread to run
  green_t *next = schedule();

  running = next;
  setcontext(next->context);
*/
  green_enable_timer();
}

int green_create(green_t *new, void *(*fun)(void*), void *arg) {
//  printf("green_create\n");
  green_disable_timer();

  ucontext_t *cntx = (ucontext_t *)malloc(sizeof(ucontext_t));
  getcontext(cntx);

  void *stack = malloc(STACK_SIZE);

  cntx->uc_stack.ss_sp = stack;
  cntx->uc_stack.ss_size = STACK_SIZE;
  makecontext(cntx, green_thread, 0);

  new->context  = cntx;
  new->fun      = fun;
  new->arg      = arg;
  new->next     = NULL;
  new->join     = NULL;
  new->retval   = NULL;
  new->zombie   = FALSE;

  // add new to the ready queue
  add_to_end_of_ready_queue(new);

  green_enable_timer();

  return 0;
}

int green_yield() {
//  printf("green_yield\n");
  green_disable_timer();

  green_t *susp = running;

  // add susp to ready queue
  add_to_end_of_ready_queue(susp);

  // select next thread for execution
  green_t *next = schedule();

  running = next;

  if(NULL != susp && NULL != next)
    swapcontext(susp->context, next->context);

  green_enable_timer();

  return 0;
}

int green_join(green_t *thread, void **retval) {
//  printf("green_join\n");

  green_disable_timer();

  if(!thread->zombie) {
    green_t *susp = running;

    // add as joining thread
    //susp->join = thread;  // ??? or is it the other way around ???
    thread->join = susp;  // johanmon


    // select the next thread for execution
    green_t *next = schedule();
    //if(NULL == next)
    //  next = susp->next;

    running = next;
    if(NULL != next && NULL != susp)
      swapcontext(susp->context, next->context);
  }

  if(NULL != retval) {
    *retval = thread->retval;
    thread->retval = NULL;
  }

  free((void*)thread->context->uc_stack.ss_sp);
  free(thread->context);
  thread->context = NULL;

  green_enable_timer();

  return 0;
}

void green_cond_init(green_cond_t* cond_queue) {
  green_disable_timer();

  cond_queue = (green_cond_t*)malloc(sizeof(green_cond_t));
  cond_queue->next = NULL;
  cond_queue->end = NULL;

  green_enable_timer();
}

green_t* cond_schedule(green_cond_t *cond) {
  // select next thread for execution
  green_t *next = cond->next;

  if(next == cond->end) {
    // reached end of queue
    cond->next = NULL;
    cond->end = NULL;
//    printf("COND SCHEDULE EMPTY\n");
  } else {
    if(NULL != next) {
      cond->next = cond->next->next;
      if(NULL == cond->next)
        cond->end = NULL;
    } else {
      cond->next = NULL;
      cond->end = NULL;
    }
  }

  return next;
}

void add_to_end_of_cond_queue(green_cond_t *cond, green_t *new) {
//  printf("add to end of cond queue\n");

  // Add new to the readyqueue queue
  if(NULL != new) {
    if(cond->next == NULL) {
      cond->next = new;
      //new->next = NULL;
      cond->end = new;
    } else {
      cond->end->next = new;
      cond->end = new;
      //new->next = NULL;
    }
  }
}


void green_cond_wait(green_cond_t* cond) {
//  printf("green_cond_wait\n");

  green_disable_timer();

  add_to_end_of_cond_queue(cond, running);

  green_t *susp = running;

  // select next thread for execution from the ready queue
  green_t *next = schedule(); 

  //if(NULL == next)
  //  next = cond_schedule(cond);

  running = next;

  //if(susp != next)
  if(NULL != susp && NULL != next)
    swapcontext(susp->context, next->context);

  green_enable_timer();
}

void green_cond_signal(green_cond_t* cond) {
//  printf("green_cond_signal\n");
  green_disable_timer();

  if(NULL != cond->next) {
    // select next thread for execution from the cond queue
    green_t *susp = cond_schedule(cond);

    // fake an ordinary yield
    running = susp;

    green_yield();

    // and add it to the ready queue
    //add_to_end_of_ready_queue(susp);

    // get next to run from ready queue
    //green_t *next = schedule();

    // fake as if we do a yield
    //running = next;
  }

  green_enable_timer();
}

#ifdef TIMER
void timer_handler(int sig) {
  gree_t *susp = running;
  add_to_end_of_ready_queue(susp);

  green_t *next = schedule();

  running = next;
  swapcontext(susp->context, next->context);
}
#endif
