//important imports
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define TOK_BUFSIZE 10 //maximum size of command tokens
//tokens is defined as a sequences of contiguous characters separated by any of the characters that are part of delimiters
#define TOK_DELIM " \t\r\n\a" //delimiters we used to seprate tokens  
#define clear() printf("\033[H\033[J") //clear screen

//an array of stings indicating colors code important to obtain a colored output
const char * colors[7] = {
  "\x1b[0m", //white
  "\x1b[31m", //red
  "\x1b[32m", //green
  "\x1b[33m", //yellow
  "\x1b[34m", //blue
  "\x1b[35m", //magenta
  "\x1b[36m"
}; //cyan
int currentColor = 1; //initate current printing color as red

// function used to print welcome screen 
void shellInit() {
  clear();
  printf("%s%s", colors[1], "******************"
    "*****************************************************");
  printf("\n\t**Simple linux shell built using c language");
  printf("\n\t**Copyright (c) @Taha @Khayyal @Younes @Azab @Shenawy");
  printf("%s%s", "\n*******************"
    "****************************************************", colors[0]);
  printf("\n");
}
//function used to print current directory
void pwd() {
  char cwd[1024]; //declare the variable cwd in which we assign current directory

  /*
  calling getcwd c function 
  SYNOPSIS : char *getcwd(char *buf, size_t size);
  The getcwd() function place an absolute pathname of the current working directory in cwd and return it.
  The size argument is the size in bytes of the array cwd. 
  */
  getcwd(cwd, sizeof(cwd));
  printf("\nDir: %s", cwd);
  printf("\n");
}

//function used to handle interrupt signal when pressing control + c
void sigintHandler(int sig_num) {
  printf("\nINTERRUPTED!!\n");
}
/*function allow user to type commands smoothly , store all commands he wrote 
and print information about directory and environment each time user seek command input*/
char * getInput() {
  /* generating prompt */
  char prompt[1024]; //string hold the sentance we want to print when accepting input
  char hostn[1024] = ""; // a string in which we store current machine host name
  char cwd[1024];
  //the prompt is printed in form Logname@hostname+currentdirectory+">"
  //example khayyal@DESKTOP-1NPJ77U/home/khayyal>   
  //calling gethostname c function which return host name of size sizeof(hostn) and assign it in the variable hostn
  gethostname(hostn, sizeof(hostn));
  //assign inconstant color as the start of the prompt
  //the inconestant color allow us to attain different prompt color each time we enter an input 
  //we use strcpy (string copy) to assign the code of color to our string prompt 
  strcpy(prompt, colors[++currentColor]);
  strcat(prompt, getenv("LOGNAME")); //concatinate Logname with our string prompt
  strcat(prompt, "@");
  strcat(prompt, hostn); //concatinate hostname with our string prompt
  strcat(prompt, getcwd(cwd, sizeof(cwd))); //concatinate current directory with our string prompt
  strcat(prompt, colors[0]);
  strcat(prompt, "> ");
  if (currentColor == 6) currentColor = 0; //if we reached color cyan start again from white

  /* accept commands and store it*/
  char * line; //string used to store user input
  line = readline(prompt); //read the user input 
  if (strlen(line) != 0) {
    add_history(line); //store non empty commands
  }

  return line;
}
/*
function used to recoginze pipes and return each pipe element as a command
it takes 2 argument as input , the line user entered and an integer buffersize 
used to determine the pipe level(how many pipes/commands we have in our code) 
*/
char ** pipeSpliter(char * line, size_t * buffersize) {
  /*if we accepted an input successfully so we have at least one command entered,thats why we intiate bufsize with 1.
  next, we take the entered line, looping over its character ,char by char, comparing them with the pipe operator "|"
  once the cuurent character is "|" we increment bufsize indicating that we have 2 command now and so on 
  */
  size_t bufsize = 1, pos = 0;
  while (line[pos] != '\0') {
    if (line[pos] == '|') ++bufsize;
    ++pos;
  }
  /*after we dermined the number of commands we need to handle, we split the input to aquire that commands
  we use the "|" operated as an indicator that the upcoming characters belongs to another command (split pipe elements)
  ex: pipSpliter("ls| wc") ---> returns two commands ls and wc
  we use strtok, a function which breaks string into a series of tokens using a delimiter we specfiy "|" in our case*/
  char * p = strtok(line, "|");
  //allocating dynamic memory space for the variable pipe_args in which we will store the commands as an array
  //so pipe_args[0] will held the frist command, pipe_args[1] will held the second command,etc
  char ** pipe_args = malloc(bufsize * sizeof(char * ));
  /*
  actually we cant get pipe elements with just one strtok call, as it doesnt return the elements but 
  only a pointer to the first token found in the string. that is , we use Consecutive calls
  On a first call, the function expects as argument a C string whose first character is used as the starting location to scan for tokens.
  after the frist,the pinter P should point to the frist token (pipe element) and a null pointer is assigned for the rest of the line  
  In subsequent calls, the function expects a null pointer and uses the position right after the end of the last token as the new starting location for scanning.
  to summerize we can say that we use Consecutive strtok calls, in ther frist call we give the string we wish to break as an argument,
  the following calls we just give the function a null pointer and loop until we reach the last element; 

*/
  for (int no = 0; no < bufsize; ++no) {
    pipe_args[no] = p; //the frist time we loop we assign the frist token to pipe_args[0]
    p = strtok(NULL, "|"); //other times we call strtok, assign the returned token to p which will be assigned to pipe_args in the following call
  }

  * buffersize = bufsize;
  return pipe_args;
}
/*a function used to interpret a single command 
it accept a command and return some tokens, these tokens represent the command and the args it take
example:parser("cd shell") should return 2 tokens, "cd" and "shell"*/

char ** parser(char * line) {

  size_t bufsize = TOK_BUFSIZE; //the variable bufsize indicate the maximum of tokens a command may have
  int position = 0; //
  char ** tokens = malloc(bufsize * sizeof(char * )); //a dynamic array holds the tokens we wish to get 
  char * token;

  if (!tokens) // making sure that the memory allocation successed 
  {
    fprintf(stderr, "allocation error\n");
    exit(EXIT_FAILURE); //terminate the current command execution 
  }
  // we obtain tokens using the same approach we used to obtain pipe elements 
  //the delimiters in this case are \t or \r or \n or \a since we know that the base command and its args are seperated only with spaces
  token = strtok(line, TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;
    // if we reached the end of the memory we allocated(10 tokens each time), we reallocate another memory space
    if (position >= bufsize) {
      bufsize += TOK_BUFSIZE; //increment size by 10
      tokens = realloc(tokens, bufsize * sizeof(char * )); //reallocation
      if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, TOK_DELIM);
  }
  //assign the last position token with null indicating the end of the command, 
  //that tell the command executer to start execute the following command
  tokens[position] = NULL;
  return tokens;
}

/*
    a function to execute the parsed input and handle piping
    parsed : array of pointers aech pointer is an array[process] of strings(char*) [tokens of process]
    Npr : size of parsed array
    input --> parse the input lik this   A -t | B | c -->  { {'A','-T'} , {'B'} , 'c' }  & Npr = 3
*/
void commandHandler(char ** * parsed, int Npr) {
  /*array of Npr-1 pipes between processes */
  int ** Npipes = (int ** ) malloc((Npr - 1) * sizeof(int * ));
  /*array for processes id used in parent to wait them if needed*/
  pid_t * pi = (pid_t * ) malloc(Npr * (sizeof(pid_t)));
  /* the pipe is an array of size 2*/
  for (int i = 0; i < Npr - 1; ++i)
    Npipes[i] = (int * ) malloc(2 * sizeof(int));

  for (int i = 0; i < Npr; ++i) {
    if (i) /*wait for previous process to finish so if it failed cancel execution*/ {
      int status;
      do {
        waitpid(pi[i - 1], & status, WUNTRACED);
      } while (!WIFEXITED(status));
      if (WEXITSTATUS(status)) {
        return;
      }
    }
    /*initializing the pipe and check if it failed*/
    if (i != Npr - 1 && pipe(Npipes[i]) < 0) {
      fprintf(stderr, "\nerror in pipe %d\n", i + 1);
      return;
    }
    /*forking and check if it failed*/
    pi[i] = fork();
    if (pi[i] < 0) {
      fprintf(stderr, "\nerror in fork %d\n", i + 1);
    }
    /*child process*/
    if (pi[i] == 0) {
      /*read data from prevois pipe*/
      if (i > 0) {
        close(Npipes[i - 1][1]); /*close writing side*/
        dup2(Npipes[i - 1][0], STDIN_FILENO); /*replacing stdin reading file by the pipe*/
        close(Npipes[i - 1][0]); /*close reading pipe side*/
      }

      /*write data to pipe*/
      if (i != Npr - 1) {
        close(Npipes[i][0]); /*close reading side*/
        dup2(Npipes[i][1], STDOUT_FILENO); /*replacing stdout writing file by the pipe*/
        close(Npipes[i][1]); /*close writing side*/
      }
      /*execute the process and check if it failed*/
      execvp(parsed[i][0], parsed[i]);
      fprintf(stderr, "\n< Can't execute command %s%s\n", parsed[i][0] ,", make sure you entered a valid command");
      exit(1);
    } else {
      /*close writing side in parent for next child so it won't hang*/
      if (i != Npr - 1) close(Npipes[i][1]);

    }
  }
  /*wait for last child*/
  int status;
  do {
    waitpid(pi[Npr - 1], & status, WUNTRACED);
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));
}

//function to print bultin and supported commands
void help() {
  puts("\n*************WELCOME TO SPAGSHELL VERSION 1.0.0 HELP*************"
    "\n\t********List of Commands supported:********"
    "\n\t\t\t>clear"
    "\n\t\t\t>pwd"
    "\n\t\t\t>cd"
    "\n\t\t\t>exit"
    "\n\t\t\t>help"
    "\n\t\t\t>UNIX commands"
    "\n*******************************************************************");

}

/*
function used to check whether the command to execute is a builtin command or not and if so return its id 
example: isBuiltin("cd shell") this should return 3 indicating that cd is a builtin command and its ID is 3
*/
int isBuiltin(char ** * parsed) {
  int NoOfCmds = 5; // we have 5 builtin commands
  int selectedCmd = 0; //holds the bulitin id or 0 if the command isnt builtin command
  char * ListOfCmds[NoOfCmds]; //creating a list of builtin commands

  ListOfCmds[0] = "clear";
  ListOfCmds[1] = "pwd";
  ListOfCmds[2] = "cd";
  ListOfCmds[3] = "exit";
  ListOfCmds[4] = "help";
  //checking the input command against our builtins
  //we just compare the command frist args (which is the command base) with the whole list of builtins   
  for (int i = 0; i < NoOfCmds; i++) {
    if (strcmp(parsed[0][0], ListOfCmds[i]) == 0) {
      selectedCmd = i + 1;
      break;
    }
  }

  return selectedCmd;
}
//a function used to handle builtin commands
int builtinExecuter(int cmdID, char ** * parsed) {
  switch (cmdID) {
  case 1:
    clear();
    return 1;
  case 2:
    pwd();
    return 1;
  case 3:
    //cd either get a valid directory or NULL
    //if user didnt provide a dir, change dir to the root dir
    parsed[0][1] == NULL ? chdir(getenv("HOME")) : chdir(parsed[0][1]);
    return 1;
  case 4:
    printf("\nTERMINATED!\n");
    exit(EXIT_SUCCESS);
  case 5:
    help();
    return 1;

  default:
    break;
  }

  return 0;
}

int main() {
  int bltnID;
  shellInit(); //initating the shell
  signal(SIGINT, sigintHandler); //declaring signal for interrupt
  //accept input as long as no termination
  while (1) {
    size_t bufsize;
    char ** pipes = pipeSpliter(getInput(), & bufsize); //get pipe elements
    /* our input is parsed 2 times, one to split pipe commands , the other to break commands into tokens
    so, we need 2D array to store the parsed input so that we declared our parsed variable as a multi-dim array
    example: parsed[0] refers to the frist command while parsed[0][0] refers to the frist argument in the frist command   */
    char ** * parsed = (char ** * ) malloc(bufsize * sizeof(char ** ));
    //looping over commands returned by pipe spliting 
    for (int i = 0; i < bufsize; i++) {
      parsed[i] = parser(pipes[i]); //passing every command to parser then assign the result array of tokens to our 2d array parsed
    }

    if (bltnID = isBuiltin(parsed)) //if the input command is a builtin command then pass it to the builtin executer
    {
      builtinExecuter(bltnID, parsed);
    } else //otherwise handle it using commandHandler
    {
      commandHandler(parsed, bufsize);
    }
  }

  return 0;
}
