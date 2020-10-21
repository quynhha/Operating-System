///Thi Quynh Ha Nguyen proj4 <doit>
// fix from proj1 to use in cpp
#include <iostream>
using namespace std;
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <cstring>

typedef struct Process{
    int pid;
    char *processName;
}Process;

char** parseShell(char inputString[]){
  
  int i = 0; //index of input string
  char* token; //token from strtok
  char** arguments = new char*[32];
  
  token = strtok(inputString," ");
  arguments[i++] = token;
  while(token!=NULL){
      token = strtok(NULL," ");
      arguments[i++] = token;
  }
  arguments[i] = (char*)'\0';
  return arguments;
  
}
long int MillisecondCovert(struct timeval timestrc){
    long int sec1 = timestrc.tv_sec * 1000;
    long int sec2 = timestrc.tv_usec / 1000;

    return (sec1 + sec2);
}
int isBackground(char* argm[]){
  int i = 0;
  char* arg = argm[0];
  while (arg != (char*)'\0'){
   if (strcmp(arg, "&") == 0){ //Checks if argument is the & character
      argm[i] = (char*)'\0';
      return i; //returns the position of the & character
    }
    arg = argm[++i];
  }
  return -1;
}
bool isProcessRunning(int pid){
  if (waitpid(pid,NULL,WNOHANG) == 0){
    return true;
  }
  else{
    return false;
  }
}

vector<Process> listJobs(vector<Process>& backgroundStack){
  vector<Process> tempStack; 
  Process tempProcess; //Process being looked at
  int i = 1; //Iterating variable to indicate task number
  //Empty main stack and for all tasks still in progress, and add to temporary stack
  while(!backgroundStack.empty()){
    tempProcess = backgroundStack.front();
    backgroundStack.back();
    
    if (isProcessRunning(tempProcess.pid)){
      tempStack.push_back(tempProcess);
    }
  }
  //Add processes not completed back to main stack and print them to the console
  while(!tempStack.empty()){
    cout << '[' << i++ << "] " << tempProcess.pid << " " << tempProcess.processName << endl;
    backgroundStack.push_back(tempProcess);
    tempStack.back();
  }
  return tempStack;
}
void printData(char *argvString[100], long currentTime, long pastTime) {

    struct rusage aRusage;

    printf("\nThe command %s", argvString[0]);
    printf(" used the following system resources:\n\n");
    printf("____________________________\n");

    getrusage(RUSAGE_CHILDREN, &aRusage);
    long int usr_time = MillisecondCovert(aRusage.ru_utime);
    long int sys_time = MillisecondCovert(aRusage.ru_stime);
    long int elapsedTime = currentTime - pastTime;
    long int getInvol = aRusage.ru_nivcsw;
    long int getVol = aRusage.ru_nvcsw;
    long int getPageFltMAJ = aRusage.ru_majflt;
    long int getPageFltMIN = aRusage.ru_minflt;
    
    printf("The amount of CPU time used by user(in millisecond):%ldms\n", usr_time);
    printf("The amount of system time(in millisecond):%ldms\n", sys_time);
    printf("The amount of CPU time used (both user and system time) (in milliseconds): %ldms\n", usr_time + sys_time);
    printf("The elapsed (wall-clock) time for the command to execute (in milliseconds): %ldms\n", elapsedTime);
    printf("The number of times process was preempted involuntarily: %ld\n", getInvol);
    printf("The number of times process gave up CPU voluntarily: %ld\n", getVol);
    printf("The number of page faults(Require disk I/O,): %ld\n", getPageFltMAJ);
    printf("The number of page faults (Could be satisfied without disk I/O): %ld\n\n", getPageFltMIN);

}
/**
 * Forks a new process in which to execute a task, used for all command executions under the doit program
 * @arg argvNew, character array of commands and arguments to pass to execvp
 * @return 0 if forked process succeeded and was awaited, exits otherwise
*/
int exec(char* argvNew[]){
  int pid; //process id
  
  //Initialize resource usage measurements
  long startClock, endClock;//Measures start and end "wall-clock time"
  struct timeval timeCheck;
  struct rusage usageS;//rusage struct from sys/resource.h
  struct rusage usageP;
  
  
  gettimeofday(&timeCheck, NULL);
  
  startClock = MillisecondCovert(timeCheck);
  
  getrusage(RUSAGE_SELF,&usageS);
  
  
  if ((pid = fork()) < 0) {
    cerr << "Fork error\n";
    exit(1);
  }
  else if (pid == 0){
    //Child process
    //Execute command that was inputed
    if (execvp(argvNew[0], argvNew) < 0) {
      cerr << "Invalid command" << endl;
      exit(1);
    }
  }
  else {
    //Parent process
    wait(0); //Wait for child to finish
    //Collect metrics
    gettimeofday(&timeCheck, NULL);
    endClock = MillisecondCovert(timeCheck);
    if ((getrusage(RUSAGE_SELF,&usageP)) == 0){
      printData(argvNew,endClock, startClock);
      return 0;
    } else{
      cout << "Error getting usage metrics" << endl;
      exit(1);
    }
  }
  return 0;
}

int main(int argc, char *argv[])
 /* argc -- number of arguments */
 /* argv -- an array of strings */
{
  string inputString = ""; //Shell input
  int i; //Iterator
  char* inputCString; //Input for shell converted to c string
  char** inputArgs; //Arguments for shell after parsing
  char** inputArgsBackground; //Arguments for background task after removing &
  int inputSize; //Number of characters in input, excluding null terminator
  vector<Process> backgroundStack; //Stack with ongoing background processes
  int pid; //Process id of background process
  int pos; //Position of & character
  
  if (argc > 1){
    //Command execution case
    if (exec(argv + 1) == 0){
      exit(0);
    } else{
      exit(1);
    }
    
  }
  else{
    //Shell case
    char prompt[] = "==>";
    while (true){
      cout << prompt;
      //Getting input
      try{
        getline(cin, inputString);
      }catch(int e){
        cout << "Error reading prompt" << endl;
        exit(1);
      }
      
      //Converting input to c-style string before parsing
      inputSize = inputString.length();
      inputCString = new char[inputSize + 1];
      strcpy(inputCString, inputString.c_str());
      //parsing input
      inputArgs = parseShell(inputCString);
      if(!inputArgs || !inputArgs[0]){
        cout << endl;
        continue;
      }
      //Handle background task separately by forking this process
      if((pos = isBackground(inputArgs)) >= 1){
        //Making array without & character
        inputArgsBackground = new char*[pos];
        for (i = 0; i < pos; i++){
          inputArgsBackground[i] = inputArgs[i];
        }
        inputArgsBackground[pos] = (char*)'\0';
        if ((pid = fork()) < 0) {
          cerr << "Fork error to execute background task\n";
          continue;
        }
        else if (pid == 0){
          //Child process for background process
          if (exec(inputArgsBackground) == 0){
            cout << '[' << backgroundStack.size() + 1 << "] " << getpid() << " Completed" << endl;
            cout << prompt;
          }else{
            cout << "Background process failed to complete" << endl;
          }
          exit(0);
        }else{
          //Parent process, which will continue to run shell
          backgroundStack.push_back((Process){pid, inputArgs[0]});
          cout << '[' << backgroundStack.size() << "] " <<  pid << endl;
          continue; //Go back to beginning of shell loop
        }
      }
      //Checking for special cases
      if(strcmp(inputArgs[0], "exit") == 0){
        wait(0);
        exit(0);
      }
      if(strcmp(inputArgs[0], "jobs") == 0){
      	int i = 1;
        for(auto & elem : backgroundStack){
           
          cout << '[' << i++ << "] " << elem.pid << " " << elem.processName << endl;
        }
      }
      else if(strcmp(inputArgs[0], "cd") == 0 && inputArgs[1]){
        chdir(inputArgs[1]);
        cout << "Changing directory to: " << inputArgs[1] << endl;
      }else if(inputArgs[1] && inputArgs[2] && inputArgs[3] && strcmp(inputArgs[0], "set") == 0 && strcmp(inputArgs[1], "prompt") == 0 && strcmp(inputArgs[2], "=") == 0){
        strcpy(prompt, inputArgs[3]);
      }else{
        exec(inputArgs);
      }
    }
  }
}
