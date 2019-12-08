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
  int fifo_frame_idx; //Next frame in which the page in it will be swapped
  int* lru_array; //array to keep truck on recent bits
  int* sc_array; //array used to keep truck on used bits
  int sc_point; //Next frame to be checked
  int accessCount;//How many requests are there so far
};

/*struct node{
  int item;
  struct node* next;
};

struct node* insert_item(int item, struct node* sequence_list){
    if(sequence_list==NULL){
      sequence_list = (struct node*)malloc(sizeof(struct node));
      sequence_list->item = item;
      sequence_list->next=NULL;
    }
    else{
      struct node* q = sequence_list;
      struct node* new_node_p = (struct node*)malloc(sizeof(struct node));
      while(q->next != NULL){
        q = q->next;
      }
      new_node_p->item = item;
      new_node_p->next = NULL;
      q->next = new_node_p;
  }
  return sequence_list;
}*/

void displayPTE(struct PTE p){
  printf("frame: %d, valid: %d\n", p.frameNum, p.valid);
}


void createPageTable(struct pageTable* pt, int numPages, int numFrames){
  pt->numPages = numPages;
  pt->numFrames = numFrames;
  pt->fifo_frame_idx = numFrames-1;
  pt->accessCount=0;
  pt->sc_point=numFrames-1;

  pt->pages = (struct PTE*) malloc(sizeof(struct PTE)*numPages);
  for (int i=0; i< pt->numPages; i++){
    pt->pages[i].frameNum = -1;
    pt->pages[i].valid = false;
  }
  pt->freeFrames = (int*) malloc(sizeof(int)*numFrames);
  pt->lru_array = (int*) malloc(sizeof(int)*numFrames);
  pt->sc_array = (int*) malloc(sizeof(int)*numFrames);
  pt->numFreeFrames = numFrames;
  for (int i=0; i < pt->numFreeFrames; i++){
    pt->freeFrames[i] = i;
    pt-> lru_array[i] = 0;
    pt-> sc_array[i] = 0;
  }

}

void deletePageTable(struct pageTable* pt){
  free(pt->pages);
  free(pt->freeFrames);
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

void printSequence(int* sequence, int size){
  for(int i=0; i<size; i++){
    printf("%d ",sequence[i]);
  }
  printf("\n");
}

int optimal(struct pageTable* pt, int* sequence, int size){
  int count = 0;
  int* currentPages = malloc(sizeof(int)*pt->numPages);
  for(int i=0; i<pt->numPages; i++){
    if(pt->pages[i].valid){
      currentPages[i]=1;
      count++;
    }
    else
      currentPages[i]=-1;
  }
  int num;
  int num_sequence = pt->accessCount;
  while(count>1 && num_sequence<size){
    num = sequence[num_sequence];
    if(currentPages[num]==1){
      currentPages[num]=-1;
      count--;
    }
    num_sequence++;
  }

  for(int i=0; i<pt->numPages; i++){
    if(currentPages[i]==1){
      return i;
  }
}
}

int getSwapPage(struct pageTable* pt, int verbose, stg strategy, int* sequence, int size){
  int p;
  int minFrame = 0;
  // printf("fifo frame %d\n",pt->fifo_frame_idx );
  if(strategy==FIFO){
    for (int i=0; i<pt->numPages; i++){
      if(pt->pages[i].valid && pt->pages[i].frameNum == pt->fifo_frame_idx){
        p = i;
        pt->fifo_frame_idx--;
        if (pt->fifo_frame_idx==-1){
          pt->fifo_frame_idx=pt->numFrames-1;
        }
        break;
      }
    }
  }
  else if(strategy==LRU){
    int min = pt->lru_array[0];
    for (int j=1; j<pt->numFrames; j++){
      if(pt->lru_array[j]<min){
        min = pt->lru_array[j];
        minFrame = j;
      }
    }
    for (int k=0; k<pt->numPages; k++){
      if(pt->pages[k].valid && pt->pages[k].frameNum == minFrame){
        p = k;
        pt->lru_array[minFrame]=pt->accessCount;
        break;
      }
    }
  }

  else if(strategy==SC){
    int found=0;
    while(!found){
      if(pt->sc_array[(pt->sc_point)%pt->numFrames]==1){
        pt->sc_array[(pt->sc_point)%pt->numFrames]=0;

      }
      else{
        found=1;
        pt->sc_array[(pt->sc_point)]=1;
        minFrame = (pt->sc_point);
        // pt->sc_array[(pt->sc_point)%pt->numFrames]=1;
        // minFrame = (pt->sc_point)%pt->numFrames;
      }
      pt->sc_point--;
      if(pt->sc_point==-1){
        pt->sc_point=pt->numFrames-1;
      }
    }
    for (int i=0; i<pt->numPages; i++){
      if(pt->pages[i].valid && pt->pages[i].frameNum == minFrame){
        p = i;
        pt->lru_array[minFrame]=1;
        break;
      }
    }

  }
  else if (strategy==OPT){
    p = optimal(pt, sequence, size);
  }
  else{
    p = rand()%(pt->numPages);
    while (!pt->pages[p].valid){
      p = rand()%(pt->numPages);
    }
 }
  if(verbose)
    printf("Swapping page %d from frame %d\n\n", p, pt->pages[p].frameNum);
  return p;
}


void storePage(struct pageTable* pt, int pageNum, int verbose, stg strategy, int* sequence, int size){
  int p;
  int fr;

  if (pt->numFreeFrames > 0){
    pt-> numFreeFrames--;
    pt->pages[pageNum].frameNum = pt->freeFrames[pt->numFreeFrames];
    pt->lru_array[pt->pages[pageNum].frameNum] = pt->accessCount;
    pt->sc_array[pt->pages[pageNum].frameNum] = 0;
    if(verbose)
      printf("\tCompulsory Page Fault: Inserting page %d at frame %d\n\n", pageNum,  pt->pages[pageNum].frameNum);
  }
  else{
    if(verbose){
      printf("\tCapacity page Fault: ");
    }
    p = getSwapPage(pt, verbose, strategy, sequence, size);
    pt->pages[p].valid = false;
    fr = pt->pages[p].frameNum;
    pt->pages[pageNum].frameNum = fr;
   }
  pt->pages[pageNum].valid = true;
  // displayPageTable(*pt);
}

int accessPage(struct pageTable* pt, int pageNum, int verbose, stg strategy, int* sequence, int size){

  pt->accessCount++;

  if (pt->pages[pageNum].valid){
    if(verbose)
      printf("Page: %d found at frame %d\n\n", pageNum, pt->pages[pageNum].frameNum);
    pt->lru_array[pt->pages[pageNum].frameNum] = pt->accessCount;
    pt->sc_array[pt->pages[pageNum].frameNum] = 1;
    return 0;
  }
  else{
    if(verbose)
      printf("Page %d  not in Memory\n", pageNum);
    storePage(pt, pageNum, verbose, strategy, sequence, size);
    return 1;
  }
}
int has_newline(char* str, int len){
  for(int i=0; i<len; i++){
    if(str[i]=='\n')
      return 1;
  }
  return 0;
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
  int num_requests = 25;
  int num_increment=0;
  int num_loop=0;
  char* strategy_name="R";
  stg strategy = R;
  int locality=0;
  int page=0;
  int request=0;
  char* request_str="";
  char* page_str="";
  while ((c = getopt (argc, argv, "p:f:c:r:g:vi:s:l")) != -1){
    // printf("optind is %d\n", optind);
    switch (c)
      {
      case 'p':
        num_pages = strtol(optarg, NULL, 10);
        // printf("num pages input %d\n",num_pages);
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
      case 'l':
        printf("opting for L %d\n", optind);
        locality = 1;
        request_str = argv[optind];
        page_str = argv[optind+1];
        request = strtol(argv[optind], NULL, 10);
        page= strtol(argv[optind+1], NULL, 10);
        break;
      }
    }

  if(!strcmp(strategy_name,"FIFO")){
    strategy = FIFO;
  }
  else if(!strcmp(strategy_name,"LRU")){
    strategy = LRU;
  }
  else if(!(strcmp(strategy_name, "SC"))){
    strategy = SC;
  }
  else if(!(strcmp(strategy_name, "OPT"))){
    strategy = OPT;
  }

  srand(time(0)); // seed the random number generator
  // if(verbose)
  //   displayPageTable(pt);
  int num_fault=0;
  int num_for_locality = 0;
  int* int_sequence = NULL;
  FILE *fp_out, *anom_out;
  int is_anom = 0;
  int num_fault_before = 10000;
  int copy_frame = num_frames;

  //////////////////////////////////////
  if (num_requests==10000){
    char outfile_name[20];
    strcpy(outfile_name, strategy_name);
    strcat(outfile_name, request_str);
    strcat(outfile_name, "-");
    strcat(outfile_name, page_str);
    strcat(outfile_name, ".txt");
    if((fp_out = fopen(outfile_name, "w")) == NULL ) {
      printf("Input file not open\n");
      return -1;
    }
  }
  ///////////////////////////////////////
  //////////////////////////////////////////////////////
  if (num_requests==25 && page==0 && request==0){
    if((anom_out = fopen("anom_pattern.txt", "w")) == NULL ) {
      printf("Input file not open\n");
      return -1;
    }
  }
  /////////////////////////////////////////////////////////////
// for(int x=0; x<10000; x++){
  if(!file_flag){
    int_sequence = (int*)malloc(sizeof(int)*num_requests);
    if(page && request){
      for (int i= 0; i < num_requests; i++){
          num_for_locality = rand()%100;
          // printf("num for loc %d\n", num_for_locality);
          if(num_for_locality<=request){
            p = rand()%((num_pages*page/100));
            // printf("page %d\n", p);
          }
          else{
            p = rand()%((num_pages*(100-page)/100));
          }
          int_sequence[i] = p;
        }
      }
    else{
      for (int i= 0; i < num_requests; i++){
        p = rand()%(num_pages);
        int_sequence[i] = p;
        if(num_requests==25){
          fprintf(anom_out, "%d ", p);
        }
      }
      if(num_requests==25){
        fprintf(anom_out, "\n");
      }
    }
    // printSequence(int_sequence, num_requests);
    do{
      ////////////////////////////////////////////////////
      createPageTable(&pt,num_pages,copy_frame);
      for(int i=0; i<num_requests; i++){
        num_fault = num_fault + accessPage(&pt,int_sequence[i],verbose,strategy,int_sequence, num_requests);
      }
      printf("Total number of page fault: %d\n", num_fault);
      printf("Overall hit rate: %f\n", 1-((float)num_fault/num_requests));
      ////////////////////////////////////////////////////////////////////////
      if(num_requests==10000)
        fprintf(fp_out,"%f ", 1-((float)num_fault/num_requests));
      ///////////////////////////////////////////////////////////////////////////
      if(verbose)
        displayPageTable(pt);
      deletePageTable(&pt);
      copy_frame = copy_frame+num_increment;
      if(num_fault > num_fault_before){
        is_anom++;
      }
      num_fault_before = num_fault;
      num_fault=0;
    }while(num_increment && copy_frame <= num_pages);
    free(int_sequence);
    copy_frame = num_frames;
    num_fault_before = 10000;
  }
  else{
    FILE *fp;
    int factor=0;
    char* sequence = NULL;
    char* sequence2 = NULL;
    char finished[2];
    int beginnig_of_line=0;
    int num_requests=0.0;
    int copy_f;
    if((fp = fopen(file_name, "r")) == NULL ) {
      printf("Input file not open\n");
      return -1;
    }
    do{
      do{
        factor++;
        if(sequence){
          sequence2 = sequence;
          // printf("sequence2 %s\n", sequence2);
          sequence = (char*)(malloc(20*factor));
          // printf("sequence after malloc %s\n", sequence);
          free(sequence2);
          // printf("sequence after free sequence2 %s\n", sequence);
        }
        else{
          sequence = (char*)(malloc(20*factor));
        }
        fseek(fp, beginnig_of_line, SEEK_SET);
        fgets(sequence, 20*factor, fp);
        // printf("sequence after fgets %s\n", sequence);
      }while(!has_newline(sequence,20*factor));
      int_sequence = malloc(sizeof(int)*20*factor);
      char* pattern = strtok(sequence, " ");
      while(pattern){
        int_sequence[num_requests] = strtol(pattern, NULL, 10);
        num_requests++;
        pattern = strtok(NULL, " ");
      }
      copy_f = num_frames;
      printSequence(int_sequence, num_requests);
      do{
        createPageTable(&pt,num_pages,copy_f);
        for(int i=0; i<num_requests; i++){
          // printf("int %d ",int_sequence[i]);
          num_fault = num_fault + accessPage(&pt, int_sequence[i], verbose, strategy, int_sequence, num_requests);
        }
        printf("Total number of page fault: %d\n", num_fault);
        printf("Overall hit rate: %f\n", 1-((float)num_fault/num_requests));
        if (verbose)
        displayPageTable(pt);
        deletePageTable(&pt);
        num_fault = 0;
        copy_f = copy_f+num_increment;
      }while(num_increment && copy_f <= num_pages);

      free(int_sequence);
      beginnig_of_line = ftell(fp);
      factor = 0;
      num_requests = 0;
      free(sequence);
      sequence = NULL;
      printf("======================\n");
    }while(fgets(finished, 2, fp) != NULL);

    fclose(fp);
    return 0;
  }
  if(num_requests==10000)
  fclose(fp_out);
  if (num_requests==30) {
    fclose(anom_out);
  }
// }
// printf("num anom: %d, anom rate: %f\n",is_anom, (float)(is_anom/10000.0));
}
