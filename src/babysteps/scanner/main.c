#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void findbody(FILE *fp) {
    int c,lastc;
    /* fusion cam outputs a blank line as header/body delim */
    /* scan for blank line */
    lastc = 0;
    while ((c = fgetc(fp)) != EOF) {
        if (lastc == c) {
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

    findbody(fp);

    printf("-- labels       -------------------\n");
    lp = line;
    while ((c = fgetc(fp)) != EOF) {
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
    findbody(fp);

    printf("-- bounding box -------------------\n");
    wp = word;
    while ((c = fgetc(fp)) != EOF) {
        printf("%c",(char)c);
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

int main(int argc, char **argv) {

    boundingbox("solder.nc");
    labels("solder.nc");

    //printf("\nlines %d\n",lines);


    return 0;
}
