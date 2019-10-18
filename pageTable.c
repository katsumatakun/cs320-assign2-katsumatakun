#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define N 256


typedef enum {false, true} bool;
typedef enum {R, FIFO, LRU, SC, OPT} stg;

struct PTE{   //PTE = page Table Entry
  int frameNum;
  bool valid;
};

struct pageTable{
  int numPages;
  int numFrames;
  struct PTE* pages;
  int* freeFrames;
  int numFreeFrames;
};


void displayPTE(struct PTE p){
  printf("frame: %d, valid: %d\n", p.frameNum, p.valid);
}


void createPageTable(struct pageTable* pt, int numPages, int numFrames){
  pt->numPages = numPages;
  pt->numFrames = numFrames;

  pt->pages = (struct PTE*) malloc(sizeof(struct PTE)*numPages);
  for (int i=0; i< pt->numPages; i++){
    pt->pages[i].frameNum = -1;
    pt->pages[i].valid = false;
  }
  pt->freeFrames = (int*) malloc(sizeof(int)*numFrames);
  pt->numFreeFrames = numFrames;
  for (int i=0; i < pt->numFreeFrames; i++){
    pt->freeFrames[i] = i;
  }
}

void displayPageTable(struct pageTable pt){

  printf("===================\nPage Table:\n");
  for (int i = 0; i < pt.numPages; i++){
    printf("Page: %d  ", i);
    displayPTE(pt.pages[i]);
  }

  printf("Free Frames: ");
  for (int i = 0; i <pt.numFreeFrames; i++){
    printf("%d ",pt.freeFrames[i]);
  }
  printf("\n===================\n\n");
}

int getSwapPage(struct pageTable* pt, int verbose, stg strategy){
  int p;
  // if(strategy==FIFO)
  p = rand()%(pt->numPages);
  while (!pt->pages[p].valid){
    p = rand()%(pt->numPages);
  }
  if(verbose)
    printf("Swapping page %d from frame %d\n\n", p, pt->pages[p].frameNum);
  return p;
}


void storePage(struct pageTable* pt, int pageNum, int verbose, stg strategy){
  int p;
  int fr;

  if (pt->numFreeFrames > 0){
    pt-> numFreeFrames--;
    pt->pages[pageNum].frameNum = pt->freeFrames[pt->numFreeFrames];
    if(verbose)
      printf("\tCompulsory Page Fault: Inserting page %d at frame %d\n\n", pageNum,  pt->pages[pageNum].frameNum);
  }
  else{
    if(verbose)
      printf("\tCapacity page Fault: ");
    p = getSwapPage(pt, verbose, strategy);
    pt->pages[p].valid = false;
    fr = pt->pages[p].frameNum;
    pt->pages[pageNum].frameNum = fr;

   }
  pt->pages[pageNum].valid = true;
}

int accessPage(struct pageTable* pt, int pageNum, int verbose, stg strategy){

  if (pt->pages[pageNum].valid){
    if(verbose)
      printf("Page: %d found at frame %d\n\n", pageNum, pt->pages[pageNum].frameNum);
    return 0;
  }
  else{
    if(verbose)
      printf("Page %d  not in Memory\n", pageNum);
    storePage(pt, pageNum, verbose, strategy);
    return 1;
  }
}


int main(int argc, char *argv[]) {

  struct pageTable pt;
  int p;
  int pflag=0;
  int file_flag=0;
  char c;
  char *file_name = NULL;
  int num_pages = 16;
  int num_frames = 8;
  int verbose = 0;
  float num_requests = 25.0;
  int num_increment=0;
  int num_loop=0;
  char* strategy_name=NULL;
  stg strategy = R;
  while ((c = getopt (argc, argv, "p:f:c:r:g:vi:s:")) != -1){
    printf("optind is %d\n", optind);
    switch (c)
      {
      case 'p':
        num_pages = strtol(optarg, NULL, 10);
        printf("%d\n",num_pages);
        break;
      case 'f':
        num_frames = strtol(optarg, NULL, 10);
        printf("%s\n", optarg);
        break;
      case 'r':
        file_flag = 1;
        file_name = optarg;
        break;
      case 'g':
        num_requests = (float)strtol(optarg, NULL, 10);
        break;
      case 'v':
        verbose = 1;
        break;
      case 'i':
        num_increment = strtol(optarg, NULL, 10);
        break;
      case 's':
        strategy_name = optarg;
        break;
      }
    }

  if(!strcmp(strategy_name,"FIFO")){
    strategy = FIFO;
  }
  else if(!strcmp(strategy_name,"LRU")){
    strategy = LRU;
  }
  else if(!(strcmp(strategy_namem, "SC"))){
    strategy = SC;
  }
  else if(!(strcmp(strategy_namem, "OPT"))){
    strategy = OPT;
  }

  srand(time(0)); // seed the random number generator
  // if(verbose)
  //   displayPageTable(pt);
  int num_fault=0;
  if(!file_flag){
    do{
      createPageTable(&pt,num_pages,num_frames);
      for (int i= 0; i < num_requests; i++){
         p = rand()%(pt.numPages);
         num_fault = num_fault + accessPage(&pt,p,verbose,strategy);
      }
      printf("Total number of page fault: %d\n", num_fault);
      printf("Overall hit rate: %f\n", 1-((float)num_fault/25.0));
      if(verbose)
        displayPageTable(pt);
      num_frames = num_frames+num_increment;
      /** printf("num_increment %d\n", num_increment);
      printf("num_frames %d\n",num_frames);
      printf("num_pages %d\n", num_pages);**/
    }while(num_increment && num_frames < num_pages);
  }
  else{
    FILE *fp;
    char sequence[N] = {'\0'};
    if((fp = fopen(file_name, "r")) == NULL ) {
      printf("Input file not open\n");
      return -1;
    }
    while(fgets(sequence, N, fp) != NULL){
      char copy_seq[N];
      int copy_f=num_frames;
      do{
        strcpy(copy_seq, sequence);
        num_requests = 0.0;
        num_fault = 0;
        createPageTable(&pt,num_pages,copy_f);
        // char* pattern = strtok(sequence, " ");
        char* pattern = strtok(copy_seq, " ");
        while(pattern){
          num_fault = num_fault + accessPage(&pt,strtol(pattern, NULL, 10), verbose, strategy);
          num_requests++;
          pattern = strtok(NULL, " ");
      }
      printf("Total number of page fault: %d\n", num_fault);
      printf("Overall hit rate: %f\n", 1-((float)num_fault/num_requests));
      if (verbose)
        displayPageTable(pt);
      copy_f = copy_f+num_increment;
    }while(num_increment && copy_f < num_pages);
  }
  return 0;
}
}
