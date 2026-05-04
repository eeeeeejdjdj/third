#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
  char *msg = "hello trace\n";
  printf(1, "[USER] calling write\n");
  write(1, msg, 12);
  exit();
}
