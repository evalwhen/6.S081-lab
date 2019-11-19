#include "kernel/types.h"
#include "user/user.h"

#define MIN  2
#define MAX  35

void filter(int fd);

// https://swtch.com/~rsc/thread/
int
main(int argc , char* argv[]) {
  int fd[2];

  pipe(fd);
  int pid = fork();
  if (pid == 0) {
    close(fd[1]);

    filter(fd[0]);
  } else {

    close(fd[0]);

    for (int i = MIN; i <= MAX; i++) {
      write(fd[1], &i, sizeof(int));
    }
    close(fd[1]);
  }

  exit();
}

void filter(int fd) {

  int p;
  int cfd[2];

 top:

  // first element is primes;
  if(read(fd, &p, sizeof(int)) == sizeof(int)) {
    printf("prime %d\n", p);

  } else {
    return;
  }

  pipe(cfd);

  int pid = fork();

  if (pid == 0) {
    fd = cfd[0];
    close(cfd[1]);
    goto top;

  } else {
    close(cfd[0]);

    while(1) {
      int n;
      if (read(fd, &n, sizeof(int)) == sizeof(int)) {
        if (n % p) {
          write(cfd[1], &n, sizeof(int));
        }
      } else {
        return;
      }
    }
  }
}
