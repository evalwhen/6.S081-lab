#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAX_CMD_LEN  50

char* uargv[10];
char buffer[MAX_CMD_LEN];

typedef enum cmd_type {NORMAL, REDIRECT} ctype;

int parsecmd();
char gettoken(char **pss, char *es, char **pts, char **pte);

struct {
  char* uargv[10];
  ctype ctype;
  char rechar; // redirect char;
  char* filename; // redirect filename;
  int fd;
} cmd;

int main(int argc, char* argv[]) {

  while(1) {
    write(1, "@ ", 2);
    if (!parsecmd()) {
      continue;
    }

    //debug
    /* fprintf(2, "command: %d\n", strlen(cmd.uargv[0])); */
    /* fprintf(2, "command: %s\n", cmd.uargv[1]); */
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
      /* fprintf(2, "entering redirect\n"); */
      int pid = fork();
      if (pid == 0) {
        close(cmd.fd);
        open(cmd.filename, O_CREATE | O_RDWR);
        exec(cmd.uargv[0], cmd.uargv);
      } else {
        wait(0);
      }
    } else {
      continue;
    }
  }
}

int parsecmd() {
  memset(cmd.uargv, 0, 10);
  memset(buffer, 0, MAX_CMD_LEN);
  cmd.ctype = NORMAL;

  gets(buffer, MAX_CMD_LEN);

  if (buffer[0] == 0) {
    return 0;
  }

  char* ss = buffer;
  char* es = ss + strlen(buffer);

  char *ts, *te;

  int i = 0;

  while (ss < es) {
    char tok = gettoken(&ss, es, &ts, &te);

    if (tok == 'a') {
      cmd.uargv[i++] = ts;

    } else if (tok == '>' || tok == '<') {

      cmd.rechar = tok;
      cmd.ctype = REDIRECT;
      cmd.fd = (tok == '>' ? 1 : 0);

      tok = gettoken(&ss, es, &ts, &te);

      if (tok != 'a') {
        fprintf(2, "missing redirect filename!\n");
        return 0;

      } else {
        cmd.filename = ts;
      }
    } else if (tok == '|') {
      //TODO: pipeline
      break;
    }
  }
  cmd.uargv[i] = 0;

  return 1;
}

char gettoken(char **pss, char *es, char **pts, char **pte) {

  char *s;
  char tok;

  s = *pss;

  if (s == es) {
    return '*';
  }

  while ( s < es && *s == ' ') {
    s++;
  }

  *pts = s;

  switch (*s) {
  case '<':
  case '>':
    tok = *s;
    s++;
    break;
  default:
    tok = 'a';

    /* while (*s != ' ' && *s != '<' && *s != '>') */
    while (s < es && *s != ' ' && *s != '\n') {
      s++;
    }
    *s = '\0';
    *pte = s;
    s++;
    break;
  }

  while (s < es && *s == ' ') {
    s++;
  }
  *pss = s;

  return tok;
}
