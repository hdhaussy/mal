#include <ctype.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct object object_t;

struct object {
  enum {
    OBJ_ERROR,
    OBJ_INT,
    OBJ_SYMBOL,
    OBJ_LIST,
  } type;
  object_t* next;
};

typedef struct readctxt {
  char* start;
  char* next;
} readctxt_t;

int isseparator(int c) {
  return c==0 || isspace(c) || c==',' || c=='(' || c==')';
}

object_t* make_object(size_t datasz,int type) {
  object_t* obj = malloc(sizeof(object_t)+datasz);
  obj->type = type;
  obj->next = 0;
  return obj;
}

object_t* make_error(const char* msg) {
  size_t len = strlen(msg);
  object_t* obj = make_object(len+1,OBJ_ERROR);
  strncpy((char*)(obj+1),msg,len);
  return obj;
}

object_t *make_integer(long value) {
  object_t* obj = make_object(sizeof(long),OBJ_INT);
  long* pval = (long*)(obj+1);
  *pval = value;
  return obj;
}

object_t* make_symbol(const char* data,size_t len) {
  object_t* obj = make_object(len+1,OBJ_SYMBOL);
  char* pval = (char*)(obj+1);
  memcpy(pval,data,len+1);
  *(pval+len) = 0;
  return obj;
}

object_t* read_atom(readctxt_t* ctxt) {
  object_t* obj;
  if(isdigit(*ctxt->next) || ((*ctxt->next=='-' || *ctxt->next=='+') && isdigit(*(ctxt->next+1)))) {
    // numerical value
    char* end;
    long int ival = strtol(ctxt->next,&end,0);
    if(end==ctxt->next) return make_error("invalid integer value");
    obj = make_integer(ival);
    ctxt->next = end;
  } else {
    // symbol value
    char* end = ctxt->next;
    while(!isseparator(*end)) end++;
    if(end==ctxt->next) return make_error("invalid symbol");
    size_t len = end-ctxt->next;
    obj = make_symbol(ctxt->next,len);
    ctxt->next = end;
  }
  return obj;
}

object_t* read_form(readctxt_t* ctxt);

object_t* read_list(readctxt_t* ctxt) {
  object_t* obj = malloc(sizeof(object_t)+sizeof(object_t*));
  obj->type = OBJ_LIST;
  obj->next = 0;
  object_t* prev = NULL;
  object_t* next;
  // eat '(' char
  ctxt->next++;
  while(1) {
    // eat spaces
    while(isspace(*ctxt->next) || *ctxt->next==',') ctxt->next++;
    if(*ctxt->next==0) return make_error("incomplete list");
    if(*ctxt->next==')') {
      ctxt->next++;
      break;
    }
    next = read_form(ctxt);
    if(next->type==OBJ_ERROR) return next;
    if(prev==NULL) {
      object_t** pval = (object_t**)(obj+1);
      *pval = next;
    }
    else {
      prev->next = next;
    }
    prev = next;
  }
  return obj;
}

object_t* read_form(readctxt_t* ctxt) {
  object_t* obj = NULL;
  // eat spaces
  while(isspace(*ctxt->next) || *ctxt->next==',') ctxt->next++;
  if(*ctxt->next) {
    // get next char
    if(*ctxt->next=='(') obj = read_list(ctxt);
    else obj = read_atom(ctxt);
  }
  return obj;
}

object_t* READ() {
  object_t* obj = NULL;
  readctxt_t ctxt;
  ctxt.start = readline("user> ");
  if(ctxt.start) {
    add_history(ctxt.start);
    ctxt.next = ctxt.start;
    obj = read_form(&ctxt);
  }
  return obj;
}

object_t* EVAL(object_t* str) {
  return str;
}

void print_object(object_t* obj) {
  switch(obj->type) {
    case OBJ_ERROR:
      printf("ERROR: %s",(char*)(obj+1));
      break;
    case OBJ_INT:
      printf("%d",*(int*)(obj+1));
      break;
    case OBJ_SYMBOL:
      printf("%s",(char*)(obj+1));
      break;
    case OBJ_LIST:
      printf("(");
      obj = *(object_t**)(obj+1);
      while(obj) {
        print_object(obj);
        obj = obj->next;
        if(obj) printf(" ");
      }
      printf(")");
      break;
  }
}

void PRINT(object_t* obj) {
  print_object(obj);
  printf("\n");
}

int REPL() {
  object_t* obj = READ();
  if(obj) {
    PRINT(EVAL(obj));
  }
  return obj != NULL;
}

int main(int argc,char** argv) {
  while(REPL());
  return 0;
}
