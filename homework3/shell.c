#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

/*
  Function Declarations for builtin shell commands:
 */


char** getargv(char* input, int* n);
void handlefunc(char** argv, int n);
void sigchldhandler(int sig);
void ctrlchandler(int signum);
void ctrlzhandler();
char* readinput();
void initjobs();
void addjob(char** argv, pid_t child_pid, char* last);
void removejob(pid_t pid, int status);

struct job {
  int defined;
  int pid;
  char* name;
  char* status;
  int bg;
  struct job* next;
};

int waiting = 0;
int sentbg = 0;

struct job* jobs;

int lsh_cd(char **args);
int lsh_exit(char **args);
int shellbg(char **args);
int shellfg(char **args);
int shell_jobs();
int shell_kill();
int shell_bg(char **args);
int shell_fg(char **args);
int shellkill(char **args);

char *builtin_str[] = {
  "cd",
  "exit",
  "jobs",
  "kill",
  "bg",
  "fg",
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_exit,
  &shell_jobs,
  &shell_kill,
  &shell_bg,
  &shell_fg,
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(char **args)
{   

  if (args[1] == NULL) {
    chdir(getenv("HOME"));

  } 

  else 

  {

   //printf("%s",args[1]);

    if (chdir(args[1]) != 0) 

    {

     printf("%s: directory not found\n",args[1]);

    }

  }

  return 1;
}

int shell_jobs(){
  struct job* curr = jobs;
  int i = 0;
  while(curr){
    if(curr->defined == 1 ){
      if(curr->bg == 0){
        printf("[%i] %d %s %s\n",i,  curr->pid, curr->status, curr->name);
      }else{
        printf("[%i] %d %s %s %s\n",i,  curr->pid, curr->status, curr->name, "&");
      }
      
    }
    curr = curr->next;
    i++;
  }
  return 1;
}

int shell_kill(char **args){

  int length = 0;
  while(args[length] != NULL){
    length++;
  }

  char* number = args[length - 1];
  int jobnum = atoi(&number[1]);
  if(number[0] != '%'){
    printf("Invalid flag %c.\n", number[0]);
    return 1;
  }
  pid_t pid;
  int i = 0;
  struct job* current = jobs->next;
  while(current){
    if(i+1 == jobnum){
      kill(current->pid, 9);
      break;
    }
    current = current->next;
    i++;
  }
  return 1;
}

int shell_bg(char **args){
  int length = 0;
  while(args[length] != NULL){
    length++;
  }

  
  char* number = args[length - 1];
  if(number[0] != '%'){
    printf("Invalid flag %c.\n", number[0]);
    return 1;
  }
  int jobnum = atoi(&number[1]);
  pid_t pid;
  int i = 0;
  struct job* current = jobs->next;
  while(current){
    if(i+1 == jobnum){
      kill(current->pid, SIGCONT);
      current->bg = 1;
      strcpy(current->status, "Running");
      break;
    }
    current = current->next;
    i++;
  }
  return 1;
}

int shell_fg(char **args){
  int length = 0;
  while(args[length] != NULL){
    length++;
  }

  
  char* number = args[length - 1];
  if(number[0] != '%'){
    printf("Invalid flag %c.\n", number[0]);
    return 1;
  }
  int jobnum = atoi(&number[1]);
  pid_t pid;
  int i = 0;
  struct job* current = jobs->next;
  while(current){
    if(i+1 == jobnum){
      kill(current->pid, SIGCONT);
      current->bg = 0;
      strcpy(current->status, "Running");
      break;
    }
    current = current->next;
    i++;
  }
  int status;
  waiting = current->pid;
  int copy = current->pid;
  while(waiting == copy){
    sleep(0.1);
  }

  return 1;
}


int lsh_exit(char **args)
{

  /*

  pid_t pid;
  pid = fork();

  raise(SIGHUP);
  raise(SIGCONT);
  
  kill(pid, SIGHUPS);

  */

  pid_t pid;
  if(pid = fork() == 0){
    struct job* killing = jobs;
    while(killing){
      kill(killing->pid, 9);
      killing = killing->next;
    }
    exit(0);
  }else{
    waitpid(pid, NULL, 0);
    struct job* current = jobs;
  }
  return 0;
    //this has to be fixed
}

int lsh_launch(char **args)
{
  pid_t pid;
  int status;

  int length = 0;
  while(args[length] != NULL){
    length++;
  }

  char* last = args[length - 1];

  pid = fork();
  if (pid == 0) {
    // burrito problem
    if (execvp(args[0], args) == -1) {
      printf("%s: command not found\n",*args);
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    //wrong
    perror("lsh");
   
  } else if(strcmp("&", last) != 0){
    addjob(args, pid, last);
    waiting = pid;
    while(waiting == pid){
      sleep(0.1);
    }
  }

  return 1;
}

int lsh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

char *lsh_read_line(void)
{
#ifdef LSH_USE_STD_GETLINE
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We received an EOF
    } else  {
      perror("lsh: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
#define LSH_RL_BUFSIZE 1024
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void lsh_loop(void)
{

  char *line;
  char **args;
  int status;

  initjobs();

  do {
    printf("> ");
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  // Load config files, if any.

  signal(SIGINT, ctrlchandler);
  signal(SIGCHLD, sigchldhandler);
  signal(SIGTSTP, ctrlzhandler);

  // Run command loop.
  lsh_loop();



  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}

void initjobs(){
  jobs = malloc(sizeof(struct job)); // the head (we should keep it null)
  jobs->name = malloc(5*sizeof(char));
  jobs-> status = NULL;
  jobs->defined = 5;
  strcpy(jobs->name, "HEAD");
  jobs->next = NULL;
}

void addjob(char** argv, pid_t child_pid, char* last){

  struct job* ending = jobs;
  while(ending->next != NULL){
    ending = ending->next;
  }
  ending->next = malloc(sizeof(struct job));
  ending = ending->next;
  int size = strlen(argv[0])+1;
  ending-> defined = 1;
  ending->name = malloc(size*sizeof(char));
  strcpy(ending->name, argv[0]);
  ending->status = malloc(8*sizeof(char));
  strcpy(ending->status, "Running");
  ending->pid = child_pid;
  ending->next = NULL;
  int background = strcmp(last, "&") == 0;
  ending->bg = background;
}

void removejob(pid_t pid, int status){

  // we need termsig here

  struct job* last = jobs;
  struct job* current = jobs->next;
  while(current){
    if(current->pid == pid){
      if(current->next){
        last->next = current->next;
      }else{
        last->next = NULL;
      }
      free(current->name);
      free(current->status);
      free(current->next);
      free(current);
      return;
    }
    last = current;
    current = current->next;
  }
}

void sigchldhandler(int sig){
  pid_t pid;
  int   status;
  while ((pid = waitpid(-1, &status, WNOHANG)) != -1)
    {
      removejob(pid, status);
      if(waiting == pid){
        waiting = 0;
      }
      if(sentbg || waiting == 0){
        sentbg = 0;
        break;
      }
    }
}

void ctrlzhandler(){
  int copy = waiting;
  if(waiting != 0){
    kill((pid_t) waiting, SIGTSTP);
  }

  struct job* curr = jobs;
  while(curr){
    // sometimes this fails to upate the list
    if(curr->pid == copy){
      //set status to terminated]
      char* newstat = "Stopped";
      strcpy(curr->status, newstat);
    }
    curr = curr->next;
  }
  waiting = 0;
  sentbg = 1;
}

void ctrlchandler(int signum){
  printf("\n");
}

int cdPlus(char **args)
{

  if (args[1] == NULL) {
    chdir(getenv("HOME"));

  }

  else

  {

   //printf("%s",args[1]);

    if (chdir(args[1]) != 0)

    {

     printf("%s: directory not found\n",args[1]);

    }

  }

  return 1;
}


