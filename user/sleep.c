#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  char err_msg[] = "too few args!\n";
  if (argc < 2) {
    write(2, err_msg, strlen(err_msg));
  } else {
    int seconds = atoi(argv[1]);
    sleep(seconds);
  }
  exit(0);
}
