#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
  int pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  }
  
  if(pid == 0){
    // Child: Consumer
    int i;
    for(i=0; i<5; i++){
      int val = consume();
      printf(1, "[USER] Child consumed %d\n", val);
    }
    exit();
  } else {
    // Parent: Producer
    int i;
    for(i=0; i<5; i++){
      produce(i * 10);
      printf(1, "[USER] Parent produced %d\n", i * 10);
    }
    wait();
    printf(1, "[USER] Producer-Consumer test done\n");
  }
  exit();
}
