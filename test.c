#include <stdio.h>
#include "green.h"

green_cond_t cond;

/* 2.4 */
void *test(void *arg) {
  int i = *(int*)arg;
  int loop = 4;

  while(loop > 0) {
    printf("thread %d: %d\n", i, loop);
    loop--;
    green_yield();
  }
}

/* 3 */
int flag = 0;

void *test_3(void *arg) {
  int id = *(int*)arg;
  int loop = 4;

  while(loop > 0) {
    if(flag == id) {
      printf("thread %d: %d\n", id, loop);
      loop--;
      flag = (flag + 1) % 3;
      green_cond_signal(&cond);
    } else {
      //flag = (flag + 1) % 3;
      green_cond_wait(&cond);
    }
  }
}

/* 4 */
void *test_4(void *arg) {
  int i = *(int*)arg;
  int loop = 4;
  int flag = 0;
  int id = *(int*)arg;
  int counter = 0;

  while(loop > 0) {
    //while(1) {
      if(flag == id) {
        printf("thread %d: %d\n", id, loop);
        break;
        green_yield();
      } else {
        printf("Suspend on cond %d, flag is %d, loop is %d\n", id, flag, loop);
        //green_cond_wait(&cond);
      }
    //}
    counter++;
    loop--;
    flag = (flag + 1) % 4;
    printf("Signal on cond %d, flag is %d, loop is %d\n", id, flag, loop);
    //green_cond_signal(&cond);
  }
}

int main() {
  green_t g0, g1, g2, g3;
  int a0 = 0;
  int a1 = 1;
  int a2 = 2;
  int a3 = 3;

  green_cond_init(&cond);

  green_create(&g0, test, &a0);
  green_create(&g1, test, &a1);
  green_create(&g2, test, &a2);
  //green_create(&g3, test, &a3);

  green_join(&g0, NULL);
  green_join(&g1, NULL);
  green_join(&g2, NULL);
  //green_join(&g3, NULL);

  printf("done\n");
  return 0;
}
