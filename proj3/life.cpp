// Thi Quynh Ha Nguyen proj3 part 2 life.cpp
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <vector>
#include <fstream>


#define RANGE 1 
#define ALLDONE 2
#define GO 3 
#define GENDONE 4 
#define EMPTY 5 
#define REPEAT 6 
#define STOP 7 
#define MAXTHREAD 10
#define MAXGRID 40

struct msg {
  int iSender; /* sender of the message (0 .. number-of-threads) */
  int type; /* its type */
  int value1; /* first value */
  int value2; /* second value */
};

sem_t mutexSems[MAXTHREAD + 1]; //Mutual Exclusion semaphores
sem_t sendSems[MAXTHREAD + 1]; //Producer semaphores
sem_t recvSems[MAXTHREAD + 1]; //Receiver semaphores
pthread_t threads[MAXTHREAD]; //Stores thread ids
struct msg mailboxes[MAXTHREAD + 1]; //Mailboxes

std::vector<std::vector<int>> oggrid; 
std::vector<std::vector<int>> nextgrid; 
int nGens; 

// Post operation increment the semaphore by 1.
// Wait operation check the semaphore value
// + if value > 0, decrement the semaphore by 1
// + if value = 0, block until semaphhore > 0
// (semaphore is an integer whose value is never fall below 0)

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
void setGrid(std::vector<std::vector<int>> * grid, int x, int y){
  int i, j;
  for (i = 0; i < x; i++){
    std::cout<<'\t';
    for (j = 0; j < y; j ++){
      std::cout<<' '<<(*grid)[i][j];
    }
    std::cout<<std::endl;
  }
  std::cout<<"\n"<<std::endl;
}

void parseInput(std::string file){
  std::string line; 
  int i,j, row = 0, col = 0, maxCol = 0; 
  char c; 
  std::ifstream myfile (file); 
  std::vector<int> rows; 
  if (myfile.is_open())
  {
    while ( getline (myfile,line) ){
      for(std::string::iterator it = line.begin(); it != line.end(); ++it) {
        c = *it;
        if (c == '0'){
          rows.push_back(0);
          col++;
        } else if( c== '1'){
          rows.push_back(1);
          col++;
        }
        if (col > MAXGRID){
          std::cout<<" Exceed the maximum number of rows or columns in the grid."<<std::endl;
          exit(-1);
        }
      }
      if (col > maxCol){
        maxCol = col;
      }
      oggrid.push_back(rows);
      nextgrid.push_back(rows);
      col = 0;
      if (row++ > MAXGRID){
        std::cout<<" Exceed the maximum number of rows or columns in the grid."<<std::endl;
        exit(-1);
      }
      rows = std::vector<int>();
    }
    myfile.close();
  }
  for (i = 0; i < row; i++){
    j = oggrid[i].size();
    while (j++ != maxCol){
      oggrid[i].push_back(0);
    }
  }
}

int main(int argc, char *argv[]){
  std::string file; //range to add
  int tn; //Number of threads to create
  int tu = 0; //Number of threads that are used
  int gens = 0;
  int rS;
  int print = 0; //(1 for y and 0 for n)
  int input = 0; //(1 for y and 0 for n)
  int Sum = 0; //Overall final sum
  int i = 0, j; //Loop iterators
  int x, y; //rows and columns
  bool done = false; 
  bool empty = false;
  bool repeat = false;
  struct msg Msg = msg();
  void *ManageThread(void *);
  //read input
  if (argc < 4|| argc > 6){
    std::cout<<"Input doesn't fit the required arguement!\nPlease enter your number of threads follow with the range!!!\nExitting!\n"<<std::endl;
    exit(1);
  }
  tn = atoi(argv[1]);
  //Checks number of threads and adjusts it if < 1 or > MAXTHREAD
  if (tn > MAXTHREAD){
    std::cout<<"Game of life has a limit of threads: "<< MAXTHREAD << std::endl;
    std::cout<<"Auto set up the thread range to: 10 threads." << std::endl;
    tn = MAXTHREAD;
  }
  if (tn < 1){
    tn = 1;
    std::cout<<"The input range for threads must be a integer that is larger or equal to 1."<<std::endl;
    std::cout<<"Set the thread range to: 1 thread."<<std::endl;
  }
  if (tn <=0 ){
    std::cout<<"Both input must be positive integers"<<std::endl;
    exit(1);
  }
  file = argv[2];
  nGens = atoi(argv[3]);
  //Checks if specify printing or input pause
  if (argc >= 5 && *argv[4] == 'y'){
    print = 1;
  }
  if (argc >= 6 && *argv[5] == 'y'){
    input = 1;
  }
  //Parses input file
  parseInput(file);
  //Gets grid size after parsing
  x = oggrid.size();
  y = oggrid[0].size();
  
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
      if (pthread_create(&threads[i], NULL, ManageThread, (void *)(intptr_t)i) != 0) {
        std::cout<< "pthread_create() fails, no new thread is created" << std::endl;
        exit(1);
      }
    }
  }
  //Gets range of rows to split between threads
  rS = x / tn;
  if (x % tn > 0){ //If split is not exact, increases range size by one
    rS++;
  }
  i = 0;
  j = 1;
  Msg.iSender = 0;
  Msg.type = RANGE;
  //Initialize operation by sending all messages
  while (i < x){
    Msg.value1 = i;
    i += rS;
    if (i > x){ //If this message would have range over the limit, adjust it
      i = x;
    }
    Msg.value2 = i;
    SendMsg(j++, Msg);
    tu++; 
  }

  while(!done && !empty && !repeat){ 
    j = 0;
   
    done = true;
    repeat = true;
    empty = true;
    
    oggrid.swap(nextgrid);
    if (print || gens == 0){ 
      std::cout<<"Generation "<<gens<<": "<<std::endl;
      setGrid(&oggrid, x, y);
    }
    //Input checkpoint
    if (input){
      std::cout<<"Please press any button and hit enter for results!!!"<<std::endl;
      getchar();
    }
    //Send go to all working threads
    while (j < tu){
      Msg = msg();
      Msg.type = GO;
      SendMsg(++j,Msg);
    }
    j = 0;
    while (j < tu){ //Waits for message from all working threads
      Msg = msg();
      RecvMsg(0,Msg);
      switch (Msg.type){
        case GENDONE: 
          done = false;
          empty = false;
          repeat = false;
          break;
        case EMPTY: 
          done = false;
          if (Msg.value1 != 1){ 
            repeat = false;
          }
          break;
        case REPEAT: 
          done = false;
          empty = false;
          break;
        case ALLDONE:
          break;
        default:
          freeThread(tn);
          std::cout<<"Invalid message!"<<std::endl;
          exit(1);
      }
      j++;
     
      
    }
    gens++;
  }
  //Send stop to all threads (If threads already exited, it does not matter because their mailboxes are empty)
  //This is for non working threads and if the game ended before the limit of generations
  j = 0;
  while (j < tn){
    Msg = msg();
    Msg.type = STOP;
    SendMsg(++j,Msg);
  }
  std::cout << "The game ends after " << gens << " generations with:" << std::endl;
  setGrid(&nextgrid, x, y);
  freeThread(tn);
  return 0;
}

void *ManageThread(void * arg){
  int thisId = *((int*)(&arg)); //Stores id of this thread
  struct msg message = msg(); //Initializes receive and send messages
  struct msg send = msg();
  int x, y, xi, yi, count = 0, start, end, gen, xbound, ybound;
  bool empty, repeat;
  RecvMsg(thisId, message); 
  if (message.iSender != 0 || message.type != RANGE){
    if (message.type == STOP){ 
      pthread_exit((void *)0);
    }
    std::cout<<"Error in the message received!!"<<std::endl;
    pthread_exit((void *)1);
  }
  start = message.value1; //Stores row range
  end = message.value2;
  xbound = oggrid.size(); //Stores grid boundaries
  ybound = oggrid[0].size();
  for (gen = 1; gen <= nGens; gen++) { //Loops through each generation
    //Receive GO message from thread 0
    empty = true; //Rows assumed to be empty and repeat until proven otherwise
    repeat = true;
    message = msg(); //Receives message from thread 0, expecting it to be GO
    RecvMsg(thisId, message);
    if (message.iSender != 0){
      std::cout<<"Error in the message received!!"<<std::endl;
      pthread_exit((void *)1);
    }
    if (message.type != GO){
      pthread_exit((void *)0);
    }
    //play a generation, looping for each element in this threads range
    for (x = start; x < end; x++){
      for (y = 0; y < ybound; y++){
        count = 0;//Counts all the neighboring elements = 1
        for (xi = x - 1; xi <= x + 1; xi++){
          for (yi = y - 1; yi <= y + 1; yi++){
            if (!(xi < 0 || xi >= xbound || yi < 0 || yi >= ybound) && oggrid[xi][yi] && !(xi == x && yi == y)){
              //Checked if xi and yi are out of bounds or the same as x y and if they = 1 otherwise
              count++;
            }
          }
        }
        //Assigns next generation element in same x y coord based on count around neighbors
        if ((count == 2 && oggrid[x][y] == 1) || count == 3){
          nextgrid[x][y] = 1;
          empty = false; //Sets empty to false if any elements are added on next grid
        }else{
          nextgrid[x][y] = 0;
        }
        if (nextgrid[x][y] != oggrid[x][y]){
          repeat = false; //Sets repeat to false if element does not repeat
        }
      }
    }
    //Check for cases and sends appropriate message
    send = msg();
    send.iSender = thisId;
    if(empty){
      send.type = EMPTY;
      if(repeat){
        send.value1 = 1;
      }
    }
    else if (repeat){
      send.type = REPEAT;
    }
    else{
      send.type = GENDONE;
    }
    //if not last generation, sends message
    if (gen != nGens){
      SendMsg(0,send);
    }
  }
  //Sends ALLDONE if game is done
  send = {thisId, ALLDONE, 0, 0};
  SendMsg(0,send);
  return 0;
}
