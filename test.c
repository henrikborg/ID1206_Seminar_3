#include <stdio.h>
#include "green.h"

green_cond_t cond;
green_cond_t cond2;

/* TASK 2.4 */
#ifdef TASK2
void *test(void *arg) {
  int i = *(int*)arg;
  int loop = 4;

  while(loop > 0) {
    printf("thread %d: %d\n", i, loop);
    loop--;
    green_yield();
  }
}
#endif

/* TASK 3 */
/* TASK 4 */
#ifdef TASK3
int flag = 0;

void *test(void *arg) {
  int id = *(int*)arg;
  int loop = 4;
  //static int flag = 0;

  while(loop > 0) {
    //  while(1) {
    if(flag == id) {
      printf("thread %d %d: %d\n", id, flag, loop);
      loop--;
      flag = (flag + 1) % THREADS;
      green_cond_signal(&cond);
      //green_cond_wait(&cond);
      //break;
    } else {
      //printf("thread %d %d: %d\n", id, flag, loop);
      //flag = (flag + 1) % THREADS;
      green_cond_wait(&cond);
      //green_cond_signal(&cond);
    }
    //}
    //loop--;
    //flag = (flag + 1) % THREADS;
    //green_cond_signal(&cond);
  }
}
#endif

// With another benchmark
#ifdef TASK4_1
int flag = 0;

void *test(void *arg) {
  int i = *(int*)arg;
  int loop = 4;
  //int flag = 0;
  int id = *(int*)arg;
  int counter = 0;

  while(loop > 0) {
    while(1) {
      if(flag == id) {
        //printf("thread %d: %d\n", id, loop);
        break;
      } else {
        printf("Suspend on cond %d, flag is %d, loop is %d\n", id, flag, loop);
        green_cond_wait(&cond);
      }
    }
    counter++;
    loop--;
    flag = (flag + 1) % THREADS;
    printf("Signal  on cond %d, flag is %d, loop is %d\n", id, flag, loop);
    green_cond_signal(&cond);
  }
}
#endif

#ifdef TASK5
int flag = 0;

void *test(void *arg) {
  int i = *(int*)arg;
  int loop = 4;
  //int flag = 0;
  int id = *(int*)arg;
  int counter = 0;

  while(loop > 0) {
    while(flag != id) {
      printf("Suspend on cond %d, flag is %d, loop is %d\n", id, flag, loop);
      green_cond_wait(&cond);
    }
    counter++;
    loop--;
    flag = (id + 1) % THREADS;
    printf("Signal  on cond %d, flag is %d, loop is %d\n", id, flag, loop);
    green_cond_signal(&cond);
  }
}
#endif

int main() {
  green_t g0, g1, g2, g3, g4, g5, g6, g7;
  //green_t* g_vector[] = {&g0, &g1, &g2, &g3};
  int a0 = 0;
  int a1 = 1;
  int a2 = 2;
  int a3 = 3;
  int a4 = 4;
  int a5 = 5;
  int a6 = 6;
  int a7 = 7;
  //int* a_vector[] = {&a0, &a1, &a2, &a3};

  green_cond_init(&cond);
  green_enable_timer();

  /*for(int i = 0; i < THREADS; i++) {
    green_create(g_vector[i], test, a_vector);
  }*/
  int i = 0;
  if(i++ < THREADS) green_create(&g0, test, &a0);
  if(i++ < THREADS) green_create(&g1, test, &a1);
  if(i++ < THREADS) green_create(&g2, test, &a2);
  if(i++ < THREADS) green_create(&g3, test, &a3);
  if(i++ < THREADS) green_create(&g4, test, &a4);
  if(i++ < THREADS) green_create(&g5, test, &a5);
  if(i++ < THREADS) green_create(&g6, test, &a6);
  if(i++ < THREADS) green_create(&g7, test, &a7);

  /*for(int i = 0; i < THREADS; i++) {
    green_join(g_vector[i], NULL);
  }*/
  i = 0;
  if(i++ < THREADS) green_join(&g0, NULL);
  if(i++ < THREADS) green_join(&g1, NULL);
  if(i++ < THREADS) green_join(&g2, NULL);
  if(i++ < THREADS) green_join(&g3, NULL);
  if(i++ < THREADS) green_join(&g4, NULL);
  if(i++ < THREADS) green_join(&g5, NULL);
  if(i++ < THREADS) green_join(&g6, NULL);
  if(i++ < THREADS) green_join(&g7, NULL);

#ifdef TASK2
  printf("TASK 2 ");
#endif
#ifdef TASK3
  printf("TASK 3/4 ");
#endif
//#ifdef TASK4
//  printf("TASK 4 ");
//#endif
  printf("done\n");

  return 0;
}
