#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
// #include <sys/wait.h>

#define BUFF_SIZE 1024;
#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"

int main(int argc, char **argv)
{

  shell_loop();

  return EXIT_SUCCESS;
}


void shell_loop(){

    char *line;
    char ** args;
    int status;
    
    do{
        printf("> ");
        line= read_line();
        args= split_line(line);
        status= sh_execute(args);

        free(line);
        free(args);
    }while (status);
   
}


char* read_line(){

    int c;
    int pos=0;
    int buffsize= BUFF_SIZE;
    char * buffer= malloc(sizeof(char)* buffsize);

    if(!buffer){
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE); 
    }

    while(1){

        c= getchar();

        if(c==EOF || c=='\n'){
            buffer[pos]='\0';
            return buffer;
        }
        else{
            buffer[pos]=c;
        }
        pos++;

        if(pos>=buffsize){
            buffsize+= BUFF_SIZE;
            buffer=realloc(buffer,buffsize);
            if(!buffer){
                fprintf(stderr, "sh: allocation error\n");
                exit(EXIT_FAILURE); 
            }
        }
    }
}


char ** split_line(char *line){

    int buffsize= SH_TOK_BUFSIZE;
    int pos=0;
    char ** tokens= malloc(sizeof(char *) * buffsize);

    char* token;

    if (!tokens) {
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SH_TOK_DELIM);

    while(token){

        tokens[pos++]=token;

        if(pos>=buffsize){
            buffsize+= SH_TOK_BUFSIZE;
            tokens=realloc(tokens, buffsize* sizeof(char *));
            if(!tokens){
                fprintf(stderr, "sh: allocation error\n");
                exit(EXIT_FAILURE); 
            }
        }

        token= strtok(NULL, SH_TOK_DELIM);
    }

    tokens[pos]=NULL;

    return tokens;
}


int sh_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("sh");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}


int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);


char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &sh_cd,
  &sh_help,
  &sh_exit
};

int sh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}



int sh_cd(char **args){

  if(args[1]=NULL){
    fprintf(stderr, "sh: expected argument to \"cd\"\n");
  }
  else{
    if(chdir(args[1])!=0){
      perror("sh");
    }
    
  }
  return 1;
}

int sh_help(char ** args){
  int i;
  printf("Ayush Shukla's SH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < sh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int sh_exit(char ** args){
  return 0;
}


int sh_execute(char **args){
  int i;

  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < sh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return sh_launch(args);
}