#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int has_newline(char* str, int len){
  for(int i=0; i<len; i++){
    if(str[i]=='\n')
      return 1;
  }
  return 0;
}

int main(int argc, char* argv[]){
  FILE *fp;
  int x=0;

  if((fp = fopen(argv[1], "r")) == NULL ) {
    printf("Input file not open\n");
    return -1;
  }
  char* sequence = NULL;
  char* sequence2 = NULL;
  char finished[2];
  int beginnig_of_line=0;
  int* int_sequence = NULL;
  int num_requests=0;
  int num_fault;
  do{
      do{
        x++;
        if(sequence){
          sequence2 = sequence;
          printf("sequence2 %s\n", sequence2);
          sequence = (char*)(malloc(20*x));
          printf("sequence after malloc %s\n", sequence);
          free(sequence2);
          printf("sequence after free sequence2 %s\n", sequence);
        }
        else{
          sequence = (char*)(malloc(20*x));
        }
        fseek(fp, beginnig_of_line, SEEK_SET);
        fgets(sequence, 20*x, fp);
        printf("sequence after fgets %s\n", sequence);
      }while(!has_newline(sequence,20*x));
      int_sequence = malloc(sizeof(int)*20*x);
      char* pattern = strtok(sequence, " ");
      while(pattern){
        int_sequence[num_requests] = strtol(pattern, NULL, 10);
        num_requests++;
        pattern = strtok(NULL, " ");
      }
      for(int i=0; i<num_requests; i++){
        printf("int %d ",int_sequence[i]);
      }
      beginnig_of_line = ftell(fp);
      x = 0;
      num_requests = 0;
      free(sequence);
      sequence = NULL;
      printf("======================\n");
  }while(fgets(finished, 2, fp) != NULL);

  fclose(fp);
}
