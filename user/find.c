#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


void
find(char* path, char* pattern) {
  int fd;
  char buffer[512];
  char* p;

  if((fd = open(path, 0)) < 0) {
    printf("find: can not open %s\n", path);
    return;
  }

  struct stat st;
  if(fstat(fd, &st) < 0) {
    printf("find: can not stat %s\n", path);
    close(fd);
    return;
  }

  if (st.type != T_DIR) {

    printf("find: %s is not a directory.\n", path);
  } else {
    struct dirent de;
    struct stat st;
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buffer)) {
      printf("find: path is too long.\n");
      exit(0);
    }
    strcpy(buffer, path);
    p = buffer + strlen(path);
    *p++ = '/';

    /* printf("%s\n", buffer); */
    while(read(fd, &de, sizeof(de)) == sizeof(de)) {
      // what is it.
      if (de.inum == 0) {
        continue;
      }
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if (stat(buffer, &st) < 0) {
        printf("find: can not stat %s\n", buffer);
        continue;
      }

      if (st.type == T_FILE) {
        /* printf("pattern: %s\n", pattern); */
        if (strcmp(p, pattern) == 0) {
          printf("%s\n", buffer);
        }
      } else if (st.type == T_DIR && strcmp(p, ".") != 0 && strcmp(p, "..") != 0) {
        find(buffer, pattern);
      }
    }
    close(fd);
  }
}

int
main(int argc, char* argv[]) {
  if (argc < 3) {
    printf("usage: find dir pattern\n");
  } else {
    find(argv[1], argv[2]);
  }
  exit(0);
}
