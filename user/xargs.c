#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

#define MAX_LINE 50

int
read_line(int fd, char* line) {
  char c;
  int counter = 0;
  while(read(fd, &c, sizeof(c)) == sizeof(c) && c != '\n') {
    *line++ = c;
    counter++;
  }
  *line = '\0';

  return counter;
}

int
main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("usage: xargs [command]\n");
    exit();
  }

  char line[MAX_LINE];
  memset(line, 0, sizeof(line));
  while (read_line(0, line) != 0) {
    /* printf("%s\n", line); */
    char* arg[MAXARG];
    char** p;
    memset(arg, 0, MAXARG);
    p = arg;
    for(int i = 1; i < argc; i++) {
      *p++ = argv[i];
    }
    *p++ = line;
    int pid;
    if ((pid = fork()) == 0) {
      exec(argv[1], arg);
    }
    wait();
  }
  exit();
}
