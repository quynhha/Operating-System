// Thi Quynh Ha Nguyen Project 1 Code

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <unistd.h>

#define MAX_CHAR 128
#define MAX_ARGS 32


typedef struct Process{
    int pid;
    char *command;
    long startingTime;
    int num;
    struct Process* next;
}Process;

typedef struct ProcessLL{
	int size;
	Process* process; 
};

/**
 * Function to convert the time into millisecond
 * @param Structs
 * @return
 */
long int MillisecondCovert(struct timeval timestrc){
    long int sec1 = timestrc.tv_sec * 1000;
    long int sec2 = timestrc.tv_usec / 1000;

    return (sec1 + sec2);
}

/**
 * Function to fork the shell
 * @param ID
 * @return
 */

int forkDoIt(int ID){
    ID = fork();

    if (ID < 0){
        fprintf(stderr,"Fork error.\n\n");
        exit(1);
    }

    return ID;
}

void parseString(char *argv[MAX_ARGS], char string[MAX_CHAR]){
    char *stok;
    int i = 0;
    stok = strtok(string, " ");

    while ( (stok != NULL) && (i <= MAX_ARGS)){
        argv[i] = stok;
        argv[i+1] = NULL;
        stok = strtok(NULL, " ");
        i++;
    }
}
char *removeNewLine(char *string) {
    int length = strlen(string);

    // check to see if the end of string is a newline character,
    // and if yes then replace will null character
    if (string[length - 1] == '\n' && length > 0) {
        string[length - 1] = '\0';
    }

    return string;
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

    printf("The amount of CPU time used (both user and system time) (in milliseconds): %ldms\n", usr_time + sys_time);
    printf("The elapsed (wall-clock) time for the command to execute (in milliseconds): %ldms\n", elapsedTime);
    printf("The number of times process was preempted involuntarily: %ld\n", getInvol);
    printf("The number of times process gave up CPU voluntarily: %ld\n", getVol);
    printf("The number of page faults(Require disk I/O,): %ld\n", getPageFltMAJ);
    printf("The number of page faults (Could be satisfied without disk I/O): %ld\n\n", getPageFltMIN);

}
void execute(char* arg[], int num) {
    
}

int main(int argc, char *argv[]) {
    char *prompt = "==>";
    char *argvA[MAX_ARGS];
    char userInput[MAX_CHAR];
    int randomPID;
    long int pastTime;
    long int presentTime;
    //Process *arrayP[MAX_ARGS];

    struct timeval tv1;
    struct timeval tv2;

    if(argc == 1){
        while (1){
            printf("Please enter your command with needed parameters\n");
            printf("%s", prompt);
            fgets(userInput,MAX_CHAR,stdin);
            removeNewLine(userInput);
            parseString(argvA,userInput);

            if (strcmp(userInput,"exit") == 0 || strcmp(userInput,"Exit") == 0 ){
                printf("Exiting!\n");
                exit(0);
            }

            if (strcmp(argvA[0],"cd") == 0){
                if(argvA[1]) {
                    chdir(argvA[1]);
                    printf("The current directory is changed to: %s", argvA[1]);
                    printf("\n");
                }
                continue;
            }
            if((strcmp(argvA[0], "set") == 0) && (strcmp(argvA[1], "prompt") == 0 )&&
               (strcmp(argvA[2], "=") == 0 ) && argvA[3]){
                if(argvA[3]){
                    prompt = argvA[3] ;
                    printf("Prompt set up to: %s\n", strcat(prompt, ":"));
                }
                continue;
            }
            if(strcmp(argvA[0],"jobs") == 0 ){
            	
                       /// printf("[%d] %d %s\n", i, crt -> pid, crt -> command);
                       
                    }	
            
             
            
           
            gettimeofday(&tv1, NULL);
            pastTime = MillisecondCovert(tv1);

            int pid = forkDoIt(randomPID);

            if (pid == 0){
                if (execvp(argvA[0],argvA) < 0){
                    fprintf(stderr,"System call error! Error from execvp.\n");
                    exit(1);
                }
            } else{
                wait(0);
                gettimeofday(&tv2,NULL);
                presentTime = MillisecondCovert(tv2);
                printData(argvA,presentTime,pastTime);
            }
        }
    } else{
    
        int i = 0;
        while (argv[i] != NULL){
            argvA[i] = argv[i+1];
            argvA[i+1] = NULL;
            i++;
        }
        if(strcmp(argvA[i-1],"&") == 0 ){
        /// printf("[%d] %d,i,pid)
        }
      
        gettimeofday(&tv1,NULL);
        pastTime = MillisecondCovert(tv1);
        int pid = forkDoIt(randomPID);

        if (pid == 0){
            if (execvp(argvA[0],argvA) < 0){
                fprintf(stderr,"System call error! Error from execvp.\n");
                exit(1);
            }
        }else{
            wait(0);
            gettimeofday(&tv2,NULL);
            presentTime = MillisecondCovert(tv2);
            printData(argvA,presentTime,pastTime);
        }
    }
    exit(0);
}

