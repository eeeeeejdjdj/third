#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
  printf(1, "[USER] calling hello syscall\n");
  hello();
  exit();
}
