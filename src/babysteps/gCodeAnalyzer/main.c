#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define LINELEN 128
FILE *fp;

char line[LINELEN];

void removeSpaces(char* source) {
  char* i = source;
  char* j = source;
  while(*j != 0) {
    *i = *j++;
    if(*i != ' ')
      i++;
  }
  *i = 0;
}

void removeComments(char* source) { 
  unsigned char inComment = 0;
  char* i = source;
  char* j = source;
  while(*j != 0) {
    *i = *j++;
    if(*i == '(') inComment = 1;
    if(*i == '}') inComment = 0;
    if (!inComment) {
        i++;
    }
  }
  *i = 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
       fp = fopen("test.nc","r");
    } else { 
        printf("%s\n",argv[1]);
    }

    while (fgets(line,LINELEN,fp)) {
        line[strlen(line)-1] = 0; // delete \n
        removeSpaces(line);
        removeComments(line);
        if (strlen(line)) printf("%s\n",line);
    }

    fclose(fp);

    return 0;
}
