#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {false, true} bool;

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

int getSwapPage(struct pageTable* pt){
  int p;

  p = rand()%(pt->numPages);
  while (!pt->pages[p].valid){
    p = rand()%(pt->numPages);
  }
  printf("Swapping page %d from frame %d\n\n", p, pt->pages[p].frameNum);
  return p;
}


void storePage(struct pageTable* pt, int pageNum){
  int p;
  int fr;
  
  if (pt->numFreeFrames > 0){
    pt-> numFreeFrames--;
    pt->pages[pageNum].frameNum = pt->freeFrames[pt->numFreeFrames];
    printf("\tCompulsory Page Fault: Inserting page %d at frame %d\n\n", pageNum,  pt->pages[pageNum].frameNum);
  }
  else{
    printf("\tCapacity page Fault: ");
    p = getSwapPage(pt);
    pt->pages[p].valid = false;
    fr = pt->pages[p].frameNum;
    pt->pages[pageNum].frameNum = fr;
    
   }
  pt->pages[pageNum].valid = true;
}

void accessPage(struct pageTable* pt, int pageNum){
  
  if (pt->pages[pageNum].valid){
    printf("Page: %d found at frame %d\n\n", pageNum, pt->pages[pageNum].frameNum);
  }
  else{
    printf("Page %d  not in Memory\n", pageNum);
    storePage(pt, pageNum); 
  }
}


int main(int argc, char *argv[]) {

  struct pageTable pt;
  int p;


  
  srand(time(0)); // seed the random number generator    
  createPageTable(&pt,16,8);
  displayPageTable(pt);

  for (int i= 0; i < 25; i++){
     p = rand()%(pt.numPages);
     
     accessPage(&pt,p);
  }
  
  displayPageTable(pt);

  return 0;
}

