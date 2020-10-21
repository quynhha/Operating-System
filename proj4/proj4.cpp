//Thi Quynh Ha Nguye proj4 
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#define RANGE 1
#define ALLDONE 2
#define BLANK 3
#define CARRY 4
#define MAXTHREAD 16
#define DEFAULTBYTESTOREAD 1024
#define MAXCHUNK 8192
struct msg{ 
	int iSender; /* sender of the message (0 .. number-of-threads) */
	int type; /* its type */
	int value1; /* first value */
	int value2; /* second value */
};
struct Info{
	int bytes;
	int prtchar;
};

struct msg mailboxes[MAXTHREAD + 1];
sem_t mutexSems[MAXTHREAD + 1];//Mutual Exclusion semaphores
sem_t sendSems[MAXTHREAD + 1];//producer for mailboxes
sem_t recvSems[MAXTHREAD + 1];//consumer for mailboxes
pthread_t threads[MAXTHREAD];// thread id storage
char* filemap;
int threadNum;
long fileSize;

// Post operation increment the semaphore by 1.
// Wait operation check the semaphore value
// + if value > 0, decrement the semaphore by 1
// + if value = 0, block until semaphhore > 0
// (semaphore is an integer whose value is never fall below 0)


void SendMsg(int iTo, struct msg &Msg){
  sem_wait(&sendSems[iTo]); 
  sem_wait(&mutexSems[iTo]); 
  mailboxes[iTo] = Msg; //Inserts message in mailbox
  sem_post(&recvSems[iTo]); 
  sem_post(&mutexSems[iTo]); 
}

void RecvMsg(int iTo, struct msg &Msg){
  sem_wait(&recvSems[iTo]); 
  sem_wait(&mutexSems[iTo]); 
  Msg = mailboxes[iTo];//Get the message in the mailbox
  sem_post(&sendSems[iTo]); 
  sem_post(&mutexSems[iTo]); 
}

void freeThread(int t){
  int j = 0;
  while (j <= t){
    if (j != 0){
      (void)pthread_join(threads[j - 1], NULL); //Joins all threads
    }
    (void)sem_destroy(&mutexSems[j]);
    (void)sem_destroy(&sendSems[j]);
    (void)sem_destroy(&recvSems[j]);
    j++;
  }
}
struct Info readDefault(int chunk, int fd){
   struct Info out;
   int prtchar = 0;
  
   char buffer[chunk];
   int cnt; 
   
   while( (cnt = read(fd, buffer, chunk)) > 0 ){
      for (int i = 0; i < cnt; i++){
        char cur = buffer[i];
        if(isprint(cur) || isspace(cur)){
        prtchar++;
        }
      }
   } 
   out.bytes = chunk;
   out.prtchar = prtchar;
   return out;
} 

struct Info readMap(int fd, int size, int threads){
   struct Info out;
   int prtchar = 0;
   char *pchFile;
   if ((pchFile = (char *) mmap (NULL, size, PROT_READ, MAP_SHARED, fd, 0)) 
	== (char *) -1){
	perror("Could not mmap file");
	exit (1);
    }
    for(int i = 0; i < size; i++){
        char c = pchFile[i];
    	if(isprint(c) || isspace(c)){
    	 prtchar++;
    	}
    }
    out.prtchar = prtchar;
    out.bytes = size;
    if(munmap(pchFile, size) < 0){
	perror("Could not unmap memory");
	exit(1);
    }
  return out;
   
}

int main(int argc, char *argv[]){
  int fd;
  char *pchFile;
  struct stat inputstat;
  struct Info out;
  int tn = 1; //Number of threads to create
  int chunk = DEFAULTBYTESTOREAD;
  int i = 0, j; //Loop iterators
  void *Manage(void *); //Declaration of thread function
  struct msg Msg = msg(); //Initializes message struct
  //read input
  if (argc < 2){
    std::cout<<"Input doesn't fit the required arguement!\nPlease enter ./proj4 filename or ./projec4 srcfile [size|mmap]\nExitting!\n"<<std::endl;
    exit(1);
  }
  if(argc > 2){
    if(strcmp (argv[2],"mmap") == 0){
      chunk = -1;
    }
    else chunk = atoi(argv[2]);
  }
  
  if (tn > MAXTHREAD){
    std::cout<<"The limit threads that Addem can program is:"<< MAXTHREAD<<" threads." << std::endl;
    std::cout<<"Set the thread range to: 10 threads."<<std::endl;
    tn = MAXTHREAD;
  }
  if (tn < 1){
    tn = 1;
    std::cout<<"The input range for threads must be a integer that is larger or equal to 1."<<std::endl;
    std::cout<<"Set the thread range to: 1 thread."<<std::endl;
  }
  if (tn <=0){
    std::cout<<"Both input must be positive integers"<<std::endl;
    exit(1);
  }
  
  //Intialize semaphores for mailboxes and threads
  for (i; i <= tn; i++){
    if (sem_init(&mutexSems[i],1,1) < 0){
      std::cout<< "Can not initialize semaphore" << std::endl;
      exit(1);
    }
    if (sem_init(&sendSems[i],1,1) < 0){
      std::cout<< "Can not initialize semaphore" << std::endl;
      exit(1);
    }
    if (sem_init(&recvSems[i],1,0) < 0){
      std::cout<< "Can not initialize semaphore" << std::endl;
      exit(1);
    }
    if (i > 0){
      if (pthread_create(&threads[i], NULL, Manage, (void *)(intptr_t)i) != 0) {
        std::cout<< "pthread_create() fails, no new thread is created" << std::endl;
        exit(1);
      }
    }
  }
  if((fd = open(argv[1], O_RDONLY)) < 0){
    std::cerr<< "Error Opening file." << std::endl;
    exit(1);
  }
  if (fstat(fd, &inputstat) < 0){
    std::cerr<< "Could not stat file to obtain its size" << std::endl;
    exit(1);
  }
  if (chunk > MAXCHUNK){
    std::cerr<<"Chunk size is too big. Auto set to: "<< MAXCHUNK <<"."<<std::endl;
    chunk = MAXCHUNK; 
  }
  if (chunk == 0){
    std::cerr<<"Chunk size is too small. Invalid input. Auto set to: "<< DEFAULTBYTESTOREAD <<"."<<std::endl;
    exit(1);
  } 
  if (chunk == -1){
    out = readMap(fd,inputstat.st_size,tn);
  } else {
    out = readDefault(chunk,fd);
  }
  int perc = ((out.prtchar*100)/ inputstat.st_size) ;
  std::cout << out.prtchar <<" printable characters out of " << inputstat.st_size << " bytes, " << perc <<"%."<<std::endl;
  freeThread(tn);
  close(fd);
  return 0;
}
void *Manage(void * arg){
  int thisId = *((int*)(&arg)); //Stores id of this thread
  
  struct msg message = msg(); //Initializes receive message
  struct msg send = msg(); 
  
  int i; 
  RecvMsg(thisId, message); 
  if (message.iSender != 0 || message.type != RANGE || !message.value1 || !message.value2){ 
    std::cout<<"Error in the message received!!"<<std::endl;
    exit(1);
  }int start = message.value1;
  int end = message.value2;
  int pos = start;
  int numstr = 0;
  for (int i = message.value1; i < message.value2; i++){
    char cur = filemap[i];
    if (isprint(cur) || isspace(cur)){
      numstr++;
    }
  }
  send = {thisId, ALLDONE, numstr, 0}; //Creates and sends ALLDONE message
  SendMsg(0,send);
  return 0;
}
