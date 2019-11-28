#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAX_CMD_LEN  50

char* uargv[10];
char buffer[MAX_CMD_LEN];

typedef enum cmd_type {EXEC, REDIRECT, PIPE} ctype;

struct cmd {
  ctype ctype;
};

struct execcmd {
  ctype ctype;
  char* uargv[10];
} execcmd[10];

int iexeccmd = 0;

struct redirectcmd {
  ctype ctype;
  struct cmd *cmd;
  char rechar; // redirect char;
  char* filename; // redirect filename;
  int fd;
} redirectcmd[10];

int iredirectcmd = 0;

struct pipecmd {
  ctype ctype;
  struct cmd* left;
  struct cmd* right;
} pipecmd[10];

int ipipecmd = 0;

//----------------------------------------
/* struct _cmd { */
/*   ctype ctype; */
/*   char* uargv[10]; */
/*   char rechar; // redirect char; */
/*   char* filename; // redirect filename; */
/*   int fd; */
/* } cmd; */

int parsecmd(char** ss, char* es, struct cmd **cmd);
char gettoken(char **pss, char *es, char **pts, char **pte);
void runcmd(struct cmd* cmd);


int main(int argc, char* argv[]) {
  /* struct pipecmd* pcmd; */

  while(1) {

    struct cmd *cmd;
    write(1, "@ ", 2);
    gets(buffer, MAX_CMD_LEN);
    char* ss = buffer;
    char* es = ss + strlen(buffer);

    if (!parsecmd(&ss, es, &cmd)) {
      continue;
    }

    //debug
    /* fprintf(2, "cmd type: %d\n", cmd->ctype); */
    /* fprintf(2, "command: %s\n", cmd.uargv[1]); */
    /* printf("rechar 1: %d\n", cmd.rechar); */
    /* printf("filename: %s\n", cmd.filename); */
    if (fork() == 0) {
      runcmd(cmd);
    }
    wait(0);
  }
}

int parsecmd(char** ss, char* es, struct cmd **cmd) {
  /* memset(cmd.uargv, 0, 10); */
  /* memset(buffer, 0, MAX_CMD_LEN); */
  /* gets(buffer, MAX_CMD_LEN); */

  if ((*ss) == es) {
    return 0;
  }

  /* char* ss = buffer; */
  /* char* es = ss + strlen(buffer); */

  char *ts, *te;

  int i = 0;

  struct execcmd *ecmd;
  ecmd = &execcmd[iexeccmd++];
  ecmd->ctype = EXEC;
  *cmd = (struct cmd* ) ecmd;

  while (*ss < es) {
    char tok = gettoken(ss, es, &ts, &te);

    if (tok == 'a') {
      ecmd->uargv[i++] = ts;

    } else if (tok == '>' || tok == '<') {
      struct redirectcmd *rcmd;
      rcmd = &redirectcmd[iredirectcmd++];

      if ((*cmd)->ctype == EXEC) {
        struct execcmd *ecmd;
        ecmd = (struct execcmd* )(*cmd);
        ecmd->uargv[i] = 0;
        /* i = 0; */
      }
      rcmd->cmd = *cmd;

      rcmd->rechar = tok;
      rcmd->ctype = REDIRECT;
      rcmd->fd = (tok == '>' ? 1 : 0);

      tok = gettoken(ss, es, &ts, &te);

      if (tok != 'a') {
        fprintf(2, "missing redirect filename!\n");
        return 0;

      } else {
        rcmd->filename = ts;
      }

      *cmd = (struct cmd* ) rcmd;
    } else if (tok == '|') {

      struct pipecmd *pcmd;
      pcmd = &pipecmd[ipipecmd++];
      pcmd->ctype = PIPE;
      pcmd->left = *cmd;

      struct cmd *rcmd;
      if (!parsecmd(ss, es, &rcmd)) {

        fprintf(2, "whrong right pipe cmd!\n");
        return 0;
      } else {
        pcmd->right = rcmd;
        *cmd = (struct cmd*) pcmd;
      }

      break;
    } else {

      fprintf(2, "command syntax error!\n");
      return 0;
    }
  }

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

void runcmd(struct cmd* cmd) {
  struct execcmd* ecmd;
  struct redirectcmd* rcmd;

  switch(cmd->ctype) {
  case EXEC:
    ecmd = (struct execcmd *) cmd;
    /* fprintf(2, "uargv 0: %s\n", ecmd->uargv[0]); */
    /* fprintf(2, "uargv 1: %s\n", ecmd->uargv[1]); */
    exec(ecmd->uargv[0], ecmd->uargv);
    fprintf(2, "exec faild: %s\n", ecmd->uargv[0]);
    break;

  case REDIRECT:
    rcmd = (struct redirectcmd *) cmd;

    close(rcmd->fd);
    open(rcmd->filename, O_CREATE | O_RDWR);
    runcmd(rcmd->cmd);
    break;
  default:
    fprintf(2, "unknown cmd type!\n");
    break;
  }
}
