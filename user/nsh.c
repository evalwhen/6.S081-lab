#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAX_CMD_LEN  50

char* uargv[10];
char buffer[MAX_CMD_LEN];

typedef enum cmd_type {NORMAL, REDIRECT} ctype;

struct {
  char* uargv[10];
  ctype ctype;
  char rechar; // redirect char;
  char* filename; // redirect filename;
} cmd;

int parse_cmd();

int main(int argc, char* argv[]) {

  while(1) {
    write(1, "@ ", 2);
    if (!parse_cmd()) {
      continue;
    }

    //debug
    /* fprintf(2, "command: %d\n", cmd.ctype); */
    /* printf("uargv 1: %s\n", cmd.uargv[1]); */
    /* printf("rechar 1: %d\n", cmd.rechar); */
    /* printf("filename: %s\n", cmd.filename); */
    if (cmd.ctype == NORMAL) {
      int pid = fork();
      if (pid == 0) {
        exec(cmd.uargv[0], cmd.uargv);
        fprintf(2, "exec faild: %s\n", cmd.uargv[0]);
      } else {
        wait(0);
      }
    } else if (cmd.ctype == REDIRECT) {
      if (cmd.rechar == '>') {
        int pid = fork();
        if (pid == 0) {
          close(1);
          open(cmd.filename, O_CREATE | O_WRONLY);
          exec(cmd.uargv[0], cmd.uargv);
        } else {
          wait(0);
        }
      }
    } else {
      continue;
    }
  }
}

int parse_cmd() {
  memset(cmd.uargv, 0, 10);
  memset(buffer, 0, MAX_CMD_LEN);
  cmd.ctype = NORMAL;

  gets(buffer, MAX_CMD_LEN);

  // 1 = '\n'
  if (strlen(buffer) - 1 == 0) {
    return 0;
  }

  int i = 0;
  uint wlen = 0; // word length
  uint blankcount = 0;
  char* c;

  for (c = buffer; *c != '\n'; c++) {
    if (*c == '<' || *c == '>') {
      cmd.rechar = *c;
      break;
    }

    if (*c != ' ') {
      wlen++;
      blankcount = 0;
    } else {
      blankcount++;
      if (blankcount == 1) {
        cmd.uargv[i++] = c - wlen;
        *c = '\0';
        wlen = 0;
      }
    }
  }

  // this is a redirect cmd.
  if (*c == '<' || *c == '>') {
    wlen = 0;
    c++;
    while(*c == ' '){
      c++;
    }
    while(*c != '\n' && *c != ' ') {
      c++;
      wlen++;
    }
    cmd.filename = c - wlen;
    *c = '\0';
    cmd.ctype = REDIRECT;
  } else {

    if (blankcount == 0) {
      *c = '\0'; // \n => \0
      cmd.uargv[i] = c - wlen;
    }
  }

  return i + 1;
}
