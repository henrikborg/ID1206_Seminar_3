#include <stdio.h>
#include "green.h"

static green_cond_t cond_queue;
green_cond_t cond;

void *test(void *arg) {
  int i = *(int*)arg;
  int loop = 4;
  int flag = 0;
  int id = *(int*)arg;
  int counter = 0;

  while(loop > 0) {
    if(flag == id) {
      printf("thread %d: %d\n", id, loop);
      //green_yield();
    } else {
      //flag = (flag + 1) % 2;
      printf("Suspend on cond %d, flag is %d, loop is %d\n", id, flag, loop);
      green_cond_wait(&cond);
    }
    counter++;
    loop--;
    flag = (id + 1) % 2;
    printf("Signal on cond %d, flag is %d, loop is %d\n", id, flag, loop);
    green_cond_signal(&cond);
  }
}

int main() {
  green_t g0, g1;
  int a0 = 0;
  int a1 = 0;

  green_cond_init(&cond_queue);

  green_create(&g0, test, &a0);
  green_create(&g1, test, &a1);

  green_join(&g0, NULL);
  green_join(&g1, NULL);

  printf("done\n");
  return 0;
}
