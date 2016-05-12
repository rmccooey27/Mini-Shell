/*************************************************************
 * msh.cc
 * Author: Regan McCooey
 * Date: 2/28/16
 * Class: CSCI 346
 * Purpose: to implement a miniture version of a bash shell 
 * note: does not handle spaces within arguments properly
 * because I call parse on the input twice 
*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <unistd.h>
#include <boost/lexical_cast.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include "stringlist.h"
#include "lexer.h"

using namespace std;

void intro();
struct tm* getTimeStruct();
void printCommands();
string botAction(string);
void botResponse(string, string, int);
void getAnswer(string);
void generateSleep(float);
void openFile(string);
void closeFile(int);
void listDirectory(string);
void getProcessAge();
string readLine(int);
void time();
int isFileExecutable(const char*);
void execPath(string, int);
bool checkFilePath(string);
bool tryToExec(string program, string &input);

struct filesOpen {
   FILE *fp;
   string filename;
};

FILE *fp;
struct timespec boot;

int main(int argc, char **argv) {
   clock_gettime(CLOCK_REALTIME, &boot);
   setbuf(stdout, NULL);
   intro();
   printCommands();
   bool debug = false;
   if (argc > 1 && (strcmp(argv[1],"-x") == 0)) {
      debug = true;
   }
   while (true) {
      printf("What next? ");
      string s = readLine(0);

      if (debug) {
	 printf("%s\n", s.c_str());
      }

      int pipe = s.find("|", 0);
      string execCheck = s.substr(0,2);
      if (s == "quit") {
	 printf("Cya later! :)\n");
	 return 0;
      } else if (s == "help") {
	 printCommands();
      } else if ((s == "how are you?") || (s == "how are you")) {
	 printf("Great! Thanks for asking :)\n");
      } else if ((execCheck == "./" || execCheck.substr(0,1) == "/") && (pipe < 0)) {	
	 //exec without piping and direct paths
	 execPath(s, pipe); 
      } else if (( execCheck == "./" || execCheck.substr(0,1) == "/") && (pipe > 0)) { 
	 //exec with piping and a direct path as the first arg
	 string pipe2 = s.substr(pipe+2);
	 string execCheck2 = pipe2.substr(0, 2);
	 if ((execCheck2 == "./") || (execCheck2.substr(0,1) == "/")) {
	    //direct path for the second arg
	    execPath(s, pipe);
	 } else {
	    //find path for the second arg
	    string prog = botAction(pipe2);
	    if (!tryToExec(prog, pipe2)) {
	       printf("error: second program does not exist\n");
	    } else {
	       string pipe1 = s.substr(0, pipe);
	       string newStr = pipe1+" | "+pipe2;
	       execPath(newStr, pipe);
	    }
	 }
      } else {

	 string action = botAction(s);
      
	 botResponse(action,s, pipe);
      }
   }

   return 0;
}


/************************************
 * void time()
 * pre: none
 * post: the time will be printed
 ***********************************/
void time() {

   struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    int seconds = now.tv_sec;
    int nanoseconds = now.tv_nsec;
    printf("%d seconds and %d nanoseconds\n", seconds, nanoseconds);
}


/***********************************
 * void intro()
 * pre: none
 * post: intro text will be printed
 **********************************/
void intro() {

   printf("Welcome chat assistant!\n");
   printf("The current time is ");
   time();
   printf("since midnight, Jan 1, 1970!\n");
   printf("Type \"help\" if you are lost\n");
}

/***********************************
 * struct tm *getTimeStruct()
 * pre: none
 * post: time struct will be returned
 **********************************/

struct tm* getTimeStruct() {

   time_t rawtime;
   struct tm * timeinfo;
   time (&rawtime);
   timeinfo = localtime(&rawtime);
   return timeinfo;
}
 

/***********************************
 * string botResponse()
 * pre: action and input are valid 
 * strings
 * post: a response will be generated
 * based on the action and input
 **********************************/

void botResponse(string action, string input, int pipe) {

   if (action == "say") { 
      printf("%s\n", input.substr(4).c_str());
   } else if (action == "tell") {
      getAnswer(input.substr(5));
   } else if (action == "sleep") {
      double num = atof(input.substr(6).c_str());
      generateSleep(num);
   } else if (action == "list") {
      listDirectory(input.substr(5));
   } else if (action == "open") {
      openFile(input.substr(5));
   } else if (action == "close") {
      closeFile(atoi(input.substr(6).c_str()));
   } else if (action == "read") {
      int desc = atoi(input.substr(5).c_str());
      printf("Reading line from file %d:\n", desc);
      printf("%s\n", readLine(desc).c_str());
   } else {

      if (pipe < 0) {
	 if(!tryToExec(action, input)) {
	    //check for a path and see if it's executable
	    printf("Sorry I don't know how to do that!\n"); 
	 } else {
	    execPath(input, pipe);
	 }
      } else {
	 //try to exec both sides
	 string prog1 = input.substr(0, pipe);
	 string prog2 = input.substr(pipe+2);
	 string action1 = botAction(prog1);
	 string action2 = botAction(prog2);
	 bool left = tryToExec(action1, prog1);
	 bool right = true;
	 if (!(action2.substr(0, 2) == "./" || (action2.substr(0, 1) == "/"))) {
	      right = tryToExec(action2, prog2);
	 }
	 if (!(left && right)) {
	    printf("error: cannot exec both of these programs\n");
	 } else {
	    execPath(prog1+" | "+prog2, pipe);
	 }
	 
      }
   }

}
	  
/***********************************
 * string botAction()
 * pre: input is a valid string
 * post: the first word in input
 * will be returned
 **********************************/
string botAction(string input) {
   string a = "";
   int ct = 0;
   while (input[ct] != ' ') {
      a += input[ct];
      ct++;
   }
   return a;
}

/***********************************
 * string getAnswer()
 * pre: input is a valid string
 * post: an answer will be generated
 * based on input
 **********************************/

void getAnswer(string input) {
   if (input.substr(0,2) == "me") {
      input = input.substr(3);
   }
   string answer = "";
   if (input == "the time") {
      printf("The current time is ");
      time();
   } else if (input == "your name") {
      printf("My name is MiniShell\n");
   } else if (input == "your id") {
      printf("My process id is %d\n", getpid());
   } else if (input == "your parent's id") {
      printf("My parent's process id is %d\n", getppid());
   } else if (input == "your age") {
      getProcessAge();
   } else {
      printf("I don't know the answer to that :(\nYou should google it!\n");
   }
       
}

/***********************************
 * string generateSleep()
 * pre: num is a valid float
 * post: sleeps the thread for num 
 * seconds
 **********************************/
void generateSleep(float num) {
   printf("Going to sleep for %f seconds\n", num);
   usleep(num*1000000);
   printf("OK that was a nice nap!");
  
}

/***********************************
 * string processAge()
 * pre: now has been initialized
 * in the beginning of the process
 * post: the seconds the process 
 * has been running will return
 **********************************/
void getProcessAge() {
   struct timespec now;
   clock_gettime(CLOCK_REALTIME, &now);
   int seconds = now.tv_sec - boot.tv_sec;
   printf("I am %d seconds old!\n", seconds);

}

/***********************************
 * string listDirectory(string)
 * pre: dirName is a valid string
 * post: the directories in dirName
 * will be printed 
 **********************************/
void listDirectory(string dirName) {
   if (dirName == "") {
      printf("Please enter a valid directory\n");
   } else {
      DIR *dp = NULL;
      struct dirent *dptr = NULL;
      if ((dp = opendir(dirName.c_str())) == NULL) {
	 printf("Error: Can't open directory\n");
      }
      printf("The contents of the directory are: \n");
      while((dptr = readdir(dp)) != NULL) {
	 printf("%s\n", dptr->d_name);
      }
      closedir(dp);
   }
}

/***********************************
 * string openFile(string)
 * pre: filename is a valid string
 * post: the file will be opened
 * and its file number will be printed
 **********************************/
void openFile(string filename) {
   if (filename == "") {
      printf("Error - there is no filename\n");
   }
   int desc = open(filename.c_str(), 0);
   if (desc == -1) {
      printf("Error - could not open file\n");
   } else {
      printf("OK, I Opened file %d\n", desc);
   }
}



/***********************************
 * string closeFile(int desc)
 * pre: desc is a valid int
 * post: the file will be closed
 **********************************/
void closeFile(int desc) {
   if (close(desc) == -1) {
      printf("Error could not close file %d\n", desc);
   } else {
      printf("File %d closed\n", desc);
   }
}

/***********************************
 * void printCommands()
 * pre: none
 * post: all of the available 
 * commands will be printed
 **********************************/
void printCommands() {
   printf("Hi I can respond to all of the following commands! :)\n");
   printf(" how are you?\n tell me the time\n tell me your name\n");
   printf(" tell me your age\n tell me your id\n tell me your parent's id\n");
   printf(" say [any phrase]\n sleep [amount of time]\n open [filename]\n");
   printf(" read [file number]\n I can also execute any program!\n close [file number]\n quit\n");
}

/********************************
 * string readLine(int fd)
 * pre: fd is a file descriptor 
 * that is currently open
 * post: a line will be read 
 * from the given file 
 * descriptor and returned 
 *******************************/

string readLine(int fd) {
    string s = "";
    while (true) {
        char c;
        int n = read(fd, &c, 1);
        if (n == 0) {
            printf("There is no more data available in file %d\n", fd);
            return s;
        } else if (n < 0) {
            printf("There was a problem reading from file %d: error number %d\n", fd, n);
            return s;
        } else if (c == '\n') {
            return s;
        } else { 
            s = s + c;
        }
    }
}

/********************************************
 * int isFileExecutable(const char *filename
 * pre: filename is a valid string
 * post: a zero will be returned if the file is 
 * not executable, a non-zero if it is 
 *********************************************/

int isFileExecutable(const char *filename) {

   struct stat statinfo;
   int result = stat(filename, &statinfo);
   if (result < 0) return 0;
   if (!S_ISREG(statinfo.st_mode)) return 0;
   
   if (statinfo.st_uid == geteuid()) return statinfo.st_mode & S_IXUSR;
   if (statinfo.st_gid == getegid()) return statinfo.st_mode & S_IXGRP;
   return statinfo.st_mode & S_IXOTH;
}


/********************************************
 * void execPath(string, p)
 * pre: path is a valid string
 * p is the correct index ofwhere the pipe is
 * if there is one in the path
 * input redirection is first 
 * output redirection is second
 * post: the program(s) in the path will be 
 * executed 
 *******************************************/
void execPath(string path, int p) {
   
   bool piped = false;
   if (p > 0) {
      piped = true;
   }

   char **words = split_words(path.c_str());
   bool inBack = false;
   bool outFile = false;
   bool inFile = false;
            
   if (checkFilePath(words[0])) {
  
      if (strcmp(words[stringlist_len(words)-1], "&") == 0) {
	 inBack = true;
	 words = stringlist_copy(words, 0, stringlist_len(words)-1);
      }
    
      int arrow1;
      int arrow2;
      int inArrow;
      string inFilename = "";
      string outFilename = "";
      if (stringlist_len(words) >= 3)  {
	 arrow1 = strcmp(words[stringlist_len(words)-2], ">");
	 arrow2 = strcmp(words[stringlist_len(words)-2], ">>");
	 inArrow = strcmp(words[stringlist_len(words)-2], "<");
	 
	 //output redirection 
	 if (arrow1 == 0 || arrow2 == 0) {
	    outFile = true;
	    outFilename = words[stringlist_len(words)-1];
	    words = stringlist_copy(words, 0, stringlist_len(words)-2);
	 } else if ((inArrow == 0) && !outFile) {
	    //input redirection and no output redirection
	    inFile = true;
	    inFilename = words[stringlist_len(words)-1];
	    words = stringlist_copy(words, 0, stringlist_len(words)-2);
	   
	 } 
	 if ((stringlist_len(words) >= 3) && !(outFile && inFile)) {
	    //check for input redirection on top of an output redirection
	    inArrow = strcmp(words[stringlist_len(words)-2], "<");	       
	    if (inArrow == 0) {
	       inFile = true;
	       inFilename = words[stringlist_len(words)-1];
	       words = stringlist_copy(words, 0, stringlist_len(words)-2);
	    }
	 }
      }
      
      int fd[2];
      char **word2 = stringlist_empty();
      if (piped) {
	 char **words1 = stringlist_empty();
	 bool done = false;
	 int i;
	 for (i = 0; (i < stringlist_len(words) && !done); i++) {
	    stringlist_append(&words1, words[i]);
	    if (strcmp(words[i+1], "|") == 0) {
	       done = true;	       
	    }
	 }
	 //second exec 
	 word2 = stringlist_copy(words, i+1, stringlist_len(words));
	 //first exec 
	 words = stringlist_copy(words1, 0, stringlist_len(words1));
	 
	 //create the pipe
	 pipe(fd);
      }
	
	    
      //fork one child
      int child = fork();
      if (child == 0) {

	 //redirection for pipe
	 if (piped) {
	    close(1); //closing std out 
	    close(fd[0]);
	    dup2(fd[1], 1);
	    close(fd[1]);
	 }

	 //can only have output redirection if not piped
	 if (outFile  && !piped) {
	    int desc;
	    if (arrow1 == 0) {
	       desc = open(outFilename.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0660);
	    } else {
	       desc = open(outFilename.c_str(), O_CREAT | O_RDWR | O_APPEND, 0660);
	    }
	    close(1);
	    dup2(desc, 1);
	    close(desc);
	 } 

	 //input redirection
	 if (inFile) {
	    int desc = open(inFilename.c_str(), 0); 
	    if (desc < 0) {
	       printf("error: can't open file\n");
	       return;
	    }
	    close(0);
	    dup2(desc, 0);
	    close(desc);
	 }
	 execv(words[0], words);
      }
      
      //create second child for pipe
      int child2;
      if (piped) {
	 child2 = fork();

	 //this pipe can only have output redirection
	 if (child2 == 0) { //changing std in for pipe
	    close(0);
	    close(fd[1]);
	    dup2(fd[0], 0);
	    close(fd[0]);

	    if (outFile) {
	       int desc;
	       if (arrow1 == 0) {
		  desc = open(outFilename.c_str(), O_CREAT | O_RDWR | O_TRUNC | S_IWUSR | S_IRUSR, 0660);
	       } else {
		  desc = open(outFilename.c_str(), O_CREAT | O_APPEND | O_RDWR | S_IWUSR | S_IRUSR, 0660);
	       }

	       close(1);
	       dup2(desc, 1);
	       close(desc);
	    } 
	    execv(word2[0], word2);
	 }
      }

      if (piped) { //parent closes access to pipe
	 close(fd[0]);
	 close(fd[1]);
      }

      //running in background
      if (!inBack) {
	 int status;
	 waitpid(child, &status, 0);
	 printf("Process %d finished with status %d\n", child, status);
	 int status2;
	 if (piped) {
	    waitpid(child2, &status2, 0);
	    printf("Process %d finished with status %d\n", child2, status2);
	 }	 
	 
      } else {
	 printf("Process %d run in background\n", child);
	 if (piped) {
	    printf("Process %d run in background\n", child2);
	 }
      }
   } else {
      printf("invalid filename!\n");
   }
}


/************************************
 * bool checkFilePath(string path)
 * pre: path is a valid string
 * post: returns true or false based
 * on whether or not we can exec
 * the file 
 ***********************************/
bool checkFilePath(string path) {
   char **words = split_words(path.c_str());
   int fileCheck = isFileExecutable(words[0]);
   if (fileCheck == 0) {
      return false;
   } 
   return true;
}

/************************************
 * bool tryToExec(string, string &)
 * pre: program and input are valid
 * post: a true/false will be returned
 * depending on if there is a path 
 * found for the program given
 * the full path will be passed in 
 * the reference parameter input
 ***********************************/
bool tryToExec(string program, string &input) {

   bool exists = false;
   int i = 0;
   string path = "";
   while (!exists && i < 3) {
      switch (i) {
      case 0:
	 path = "/usr/local/bin/"+program;
	 exists = checkFilePath(path);
	 break;
      case 1:
	 path = "/bin/"+program;
	 exists = checkFilePath(path);
	 break;
      case 2:
	 path = "/usr/bin/"+program;
	 exists = checkFilePath(path);
      }
      i++;
   }
   if (exists) {
      char **words = split_words(input.c_str()); 
      char **newList = stringlist_empty();
      stringlist_append(&newList, path.c_str());
      
      int x = 1;
      while (words[x] != NULL) {
	 stringlist_append(&newList, words[x]);
	 x++;
      }
      string newExec = stringlist_join(newList, " ");
      input = newExec;
      return true;
   }
   return false;
}
