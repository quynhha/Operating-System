//Thi Quynh Ha Nguye proj3 part 1 addem.cpp
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define RANGE 1
#define ALLDONE 2
#define MAXTHREAD 10

struct msg{ 
	int iSender; /* sender of the message (0 .. number-of-threads) */
	int type; /* its type */
	int value1; /* first value */
	int value2; /* second value */
};
struct msg mailboxes[MAXTHREAD + 1];
sem_t mutexSems[MAXTHREAD + 1];//Mutual Exclusion semaphores
sem_t sendSems[MAXTHREAD + 1];//producer for mailboxes
sem_t recvSems[MAXTHREAD + 1];//consumer for mailboxes
pthread_t threads[MAXTHREAD];// thread id storage

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

int main(int argc, char *argv[]){
  int r; //range to add
  int tn; //Number of threads to create
  
  int Sum = 0; //Overall final sum
  int i = 0, j; //Loop iterators
  void *ThreadSum(void *); //Declaration of thread function
  struct msg Msg = msg(); //Initializes message struct
  //read input
  if (argc != 3){
    std::cout<<"Input doesn't fit the required arguement!\nPlease enter your number of threads follow with the range!!!\nExitting!\n"<<std::endl;
    exit(1);
  }
  // convert the string to integer
  //(auto round down the number if it is a non-integer component)
  tn = atoi(argv[1]);
  r = atoi(argv[2]);
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
  if (tn <=0 || r <= 0){
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
      if (pthread_create(&threads[i], NULL, ThreadSum, (void *)(intptr_t)i) != 0) {
        std::cout<< "pthread_create() fails, no new thread is created" << std::endl;
        exit(1);
      }
    }
  }
  int rS; //Size of range
  //Initialize operation by sending all messages
  rS = r / tn ; //Range size
  i = 1;
  j = 1;
  Msg.iSender = 0;
  Msg.type = RANGE;
  while (i < r + 1){ 
    Msg.value1 = i;
    i += rS;
    if (i + rS > r + 1){
      i = r + 1;
    }
    Msg.value2 = i;
    SendMsg(j++, Msg);
  }
  
  j = 0;
  while (j < tn){
    Msg = msg();
    RecvMsg(0,Msg);
    if (Msg.type == ALLDONE){ 
      Sum += Msg.value1; 
    }else{
      std::cout<<"There are errors in received messages."<<std::endl;
      exit(1);
    }
    j++;
  }
  std::cout << "The total for 1 to " << r << " using " << tn << " threads is " << Sum << "." << std::endl;
  freeThread(tn);
  return 0;
}

void *ThreadSum(void * arg){
  int thisId = *((int*)(&arg)); //Stores id of this thread
  struct msg message = msg(); //Initializes receive message
  struct msg send = msg(); 
  int thisSum = 0; //Thread's sum
  int i; 
  RecvMsg(thisId, message); 
  if (message.iSender != 0 || message.type != RANGE || !message.value1 || !message.value2){ 
    std::cout<<"Error in the message received!!"<<std::endl;
    exit(1);
  }
  for (i = message.value1; i < message.value2; i++){ //Sums all values in range
    thisSum += i;
  }
  send = {thisId, ALLDONE, thisSum, 0}; //Creates and sends ALLDONE message
  SendMsg(0,send);
  return 0;
}
