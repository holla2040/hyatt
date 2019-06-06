#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void findPattern(FILE *fp, char c1, char c2) {
    int c,lastc;
    /* fusion cam outputs a blank line as header/body delim */
    /* scan for blank line */
    lastc = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\r') continue;
        // printf("%c %d\n",c,c);
        if (lastc == c1 && c == c2)  {
            break;
        }
        lastc = c;
    }
}

void labels(char *fn) {
    FILE *fp;
    int c;

    char line[80];
    char *lp;

    fp = fopen(fn,"r");
//    findPattern(fp,'\n','\n');

    printf("\n\n-- labels       -------------------\n");
    lp = line;
    while ((c = fgetc(fp)) != EOF) {
        // if (c == '\r') continue;
        if (c == '\n') {
            *lp = 0;
            if (line[0] == '(' && line[strlen(line)-2] == ')') {
                printf("%s\n",line);
            }
            lp = line;
        } else {
            *lp++ = c;
        } 
    }
    fclose(fp);
}

void boundingbox(char *fn) {
    FILE *fp;
    int c;
    float v;
    float xmin,xmax,ymin,ymax;
    xmin = 100000;
    ymin = 100000;
    xmax = -100000;
    ymax = -100000;

    char word[80];
    char *wp;

    fp = fopen(fn,"r");
    findPattern(fp,'\n','\n');

    printf("\n\n-- bounding box -------------------\n");
    wp = word;
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\r') continue;
        // printf("%c",(char)c);
        if (c > 96 && c < 123) {
            c |= 0x20; // uppercase
        }
        if (c == ' ' || c == '\n') {
            *wp = 0;
            if (strlen(word) == 0) {
                continue;
            }

            v = atof(&word[1]);
   //         printf("%-15s %c %f\n",word,word[0],v);
            switch (word[0]) {
                case 'X':
                    if (v > xmax) xmax = v;
                    if (v < xmin) xmin = v;
                    break;
                case 'Y':
                    if (v > ymax) ymax = v;
                    if (v < ymin) ymin = v;
                    break;
            }

            wp = word;
        } else {
            *wp++ = c;
        }
    }
    printf("X%.4f Y%.4f\n",xmin,ymin);
    printf("X%.4f Y%.4f\n",xmax,ymin);
    printf("X%.4f Y%.4f\n",xmax,ymax);
    printf("X%.4f Y%.4f\n",xmin,ymax);
    printf("X%.4f Y%.4f\n",xmin,ymin);
    
    fclose(fp);
    
}

void printOp(FILE *fp) {
    int c,lastc;
    lastc = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (c == '\r') continue;
        if (lastc == '\n' && c == '\n')  {
            return;
        }
        printf("%c",c);
        lastc = c;
    }
}

void runOp(char *fn,char *name) {
    FILE *fp;
    int c;

    char line[80];
    char *lp;

    fp = fopen(fn,"r");

    printf("\n\n-- runOp %s  -------------------\n",name);
    lp = line;
    while ((c = fgetc(fp)) != EOF) {
        // if (c == '\r') continue;
        if (c == '\n') {
            *lp = 0;
            if (line[0] == '(' && line[strlen(line)-2] == ')') {
                if (strstr(line,name)) {
                    printf("%s\n",line);
                    printOp(fp);
                }
            }
            lp = line;
        } else {
            *lp++ = c;
        } 
    }
    fclose(fp);
}


int main(int argc, char **argv) {
    char fn[30];
    if (argc == 2) {
        strcpy(fn,argv[1]);
    } else {
        strcpy(fn,"solder.nc");
    }
    boundingbox(fn);
    labels(fn);

    runOp(fn,"solder");
    runOp(fn,"border");
    runOp(fn,"pockets");

    return 0;
}
