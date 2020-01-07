#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include "green.h"
#include <stdio.h>

#define FALSE 0
#define TRUE 1

#define STACK_SIZE 4096

static ucontext_t main_cntx = {0};
static green_t main_green = {&main_cntx, NULL, NULL, NULL, NULL, FALSE};
static green_t *running = &main_green;
//static queue readyqueue;
//green_t *ready_queue_end;
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
#else
  //int green_disable_timer() {}
  //int green_enable_timer() {}
#endif
}

void add_to_end_of_ready_queue(green_t *new) {
  printf("add to end of queue\n");
/*  if(NULL == ready_queue_end) {
    ready_queue_end = new;
    ready_queue_end->next = NULL;//new;
    running->next = new;
  } else {
    ready_queue_end->next = new;
    ready_queue_end = new;
  }*/

  // Add new to the readyqueue queue
  if(ready_queue.next == NULL) {
    ready_queue.next = new;
    //running->next = new;
    ready_queue.end = new;
  } else {
    ready_queue.end->next = new;
    ready_queue.end = new;
  }
}

void green_thread() {
  printf("green_thread\n");
  green_t *this = running;

  void* result = (*this->fun)(this->arg);
  this->retval = result
  ;
  // place waiting (joining) thread in ready queue
  //TODO
  if(NULL != this->join) {
    add_to_end_of_ready_queue(this->join);
    this->join = NULL;
  }

  // save result of execution
  //TODO
  //this->retval = result;
  this->zombie = 1;

  // free allocated memory structure
  if(NULL != this->join)
    if(1 == this->join->zombie)
      this->join = NULL;

  // we're  a zombie
  //TODO
  if(1 == this->zombie)
    green_yield();

  // find the next thread to run
  green_t *next = this->next;

  running = next;
  setcontext(next->context);
}

int green_create(green_t *new, void *(*fun)(void*), void *arg) {
  printf("green_create\n");
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

  return 0;
}

green_t* schedule() {
  // select next thread for execution
  green_t *next = ready_queue.next;

  if(next == ready_queue.end) {
    // reached end of queue
    ready_queue.next = NULL;
    ready_queue.end = NULL;
    printf("READY SCHEDULE EMPTY\n");
  } else {
  if(NULL != next)
    ready_queue.next = ready_queue.next->next;
  }

  return next;
}

int green_yield() {
  printf("green_yield\n");
  green_t *susp = running;

  // add susp to ready queue
  add_to_end_of_ready_queue(susp);

  // select next thread for execution
  green_t *next = schedule();//susp->next;

  running = next;
  swapcontext(susp->context, next->context);
  return 0;
}

int green_join(green_t *thread, void **retval) {
  printf("green_join\n");
/*  if(thread->zombie) {
    // collect the result
    // TODO
    res = thread->retval;

    return 0;
  }
*/
  if(!thread->zombie) {
    green_t *susp = running;
    // add as joining thread
    // TODO
    //susp->join = thread;  // ??? or is it the other way around ???
    thread->join = susp;  // johanmon

    //while(1 != susp->join->zombie)
    //  wgreen_yield();
    // select the next thread for execution
    // TODO
    green_t *next = schedule();
    //if(NULL != susp) {
    //  green_t *next = susp->next;

      running = next;
      if(NULL != next)
        swapcontext(susp->context, next->context);
    //}
  }

  if(NULL != retval) {
    *retval = thread->retval;
    thread->retval = NULL;
  }

  free((void*)thread->context->uc_stack.ss_sp);
  //free((void*)thread->context->uc_stack.ss_size);
  free(thread->context);
  thread->context = NULL;

  return 0;
}

void green_cond_init(green_cond_t* cond_queue) {
  cond_queue = (green_cond_t*)malloc(sizeof(green_cond_t));
  cond_queue->next = NULL;
  cond_queue->end = NULL;
}

green_t* cond_schedule(green_cond_t *cond) {
  // select next thread for execution
  green_t *next = cond->next;

  if(next == cond->end) {
    // reached end of queue
    cond->next = NULL;
    cond->end = NULL;
    printf("COND SCHEDULE EMPTY\n");
  } else {
    cond->next = cond->next->next;
  }

  return next;
}

void add_to_end_of_cond_queue(green_cond_t *cond, green_t *new) {
  printf("add to end of queue\n");

  // Add new to the readyqueue queue
  if(cond->next == NULL) {
    cond->next = running;
    //running->next = new;
    cond->end = new;
  } else {
    cond->end->next = new;
    cond->end = new;
  }
}


void green_cond_wait(green_cond_t* cond) {
  printf("green_cond_wait\n");
  /*if(NULL == cond_queue->next) {
    cond_queue->next = running;
  } else {
    cond_queue->last->next = running; // add to end of list
  }
  cond_queue->last = running;       // update end of list
  */
  add_to_end_of_cond_queue(cond, running);

  green_t *susp = running;

  // select next thread for execution
  green_t *next = ready_queue.next;//susp->next;

  running = next;

  if(susp != next)
  if(NULL != susp && NULL != next)
    swapcontext(susp->context, next->context);
}

void green_cond_signal(green_cond_t* cond) {
  printf("green_cond_signal\n");

  /*if(NULL != cond_queue->next) {
    //add_to_end_of_ready_queue(cond_queue->next);

    // select next thread for execution
    green_t *next = cond_queue->next;

    //if(cond_queue->next != cond_queue->last) {
    if(NULL != cond_queue->next) {
      //cond_queue->next = cond_queue->next->next;  // update last pointer
      cond_queue->next = cond_queue->next->next;  // update present pointer
    } else {
      // reached end of queue
      cond_queue->next = NULL;
      cond_queue->last = NULL;
    }

    green_t *susp = running;

    // add susp to ready queue
    add_to_end_of_ready_queue(susp);

    // select next thread for execution
    //green_t *next = cond_queue->next;

    running = next;
    swapcontext(susp->context, next->context);
  }*/

  green_t *susp = running;

  // select next thread for execution
  // move first suspended thread to end of ready queue
  green_t *next = cond_schedule(cond);
  if(NULL != next)
    add_to_end_of_ready_queue(next);

  next = schedule();
  if(NULL == next)
    next = cond_schedule(cond);

  add_to_end_of_ready_queue(susp);

  running = next;
  swapcontext(susp->context, next->context);
}

#ifdef TIMER
void timer_handler(int sig) {
  gree_t *susp = running;
  add_to_end_of_ready_queue(susp);

  if(NULL != cond_queue->next) {
    //add_to_end_of_ready_queue(cond_queue->next);

    // select next thread for execution
    green_t *next = cond_queue->next;

    //if(cond_queue->next != cond_queue->last) {
    if(NULL != cond_queue->next) {
      //cond_queue->next = cond_queue->next->next;  // update last pointer
      cond_queue->next = cond_queue->next->next;  // update present pointer
    } else {
      // reached end of queue
      cond_queue->next = NULL;
      cond_queue->last = NULL;
    }
  }

  running = next;
  swapcontext(susp->context, next->context);
}
#endif

