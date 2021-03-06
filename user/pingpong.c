#include "kernel/types.h"
#include "user/user.h"

int
main(int argc , char* argv[]) {
  int parent_fd[2];
  int child_fd[2];
  pipe(parent_fd);
  pipe(child_fd);
  char buffer[1];


  // child
  if (fork() == 0) {
    close(parent_fd[1]);
    close(child_fd[0]);
    if (read(parent_fd[0], buffer, 1) > 0) {
      write(child_fd[1], "0", 1);
      printf("%d: received ping\n", getpid());
    }

    // parent
  } else {
    close(parent_fd[0]);
    close(child_fd[1]);
    write(parent_fd[1], "0", 1);
    if (read(child_fd[0], buffer, 1) > 0) {
      printf("%d: received pong\n", getpid());
    }
  }

  exit(0);
}
