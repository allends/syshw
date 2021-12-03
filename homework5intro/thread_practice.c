#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

void *thread_stuff(void *vargp)
{
  while (1)
  {
    printf("hello from the thread\n");
    sleep(1);
  }
  return NULL;
}

int main()
{
  pthread_t thread;
  pthread_create(&thread, NULL, thread_stuff, NULL);
  pthread_join(thread, NULL);
}