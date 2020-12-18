#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>


char* READ() {
  char* str = readline("user> ");
  if(str) add_history(str);
  return str;
}

char* EVAL(char* str) {
  return str;
}

void PRINT(char* str) {
  printf("%s\n",str);
}

int REPL() {
  char* str = READ();
  if(str) {
    PRINT(EVAL(str));
    free(str);
  }
  return str != NULL;
}

int main(int argc,char** argv) {
  while(REPL());
  return 0;
}
