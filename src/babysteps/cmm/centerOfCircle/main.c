#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* references
    http://www.ambrsoft.com/TrigoCalc/Circle3D.htm
    
   example
        5,0 0,5 7,7 
        eq 9x2 + 9y2 - 73x - 73y + 140 = 0
        center 4.06,4.06 
        radius 4.16

    result from running this
    centerX: 4.06
    centerY: 4.06
    radius:  4.16
*/

void centerradius(float x1, float y1, float x2, float y2, float x3, float y3, float *centerX, float *centerY, float *radius) {
    float a,b,c,d;

    a = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
    b = (x1 * x1 + y1 * y1) * (y3 - y2) + (x2 * x2 + y2 * y2) * (y1 - y3) + (x3 * x3 + y3 * y3) * (y2 - y1);
    c = (x1 * x1 + y1 * y1) * (x2 - x3) + (x2 * x2 + y2 * y2) * (x3 - x1) + (x3 * x3 + y3 * y3) * (x1 - x2);
    d = (x1 * x1 + y1 * y1) * (x3 * y2 - x2 * y3) + (x2 * x2 + y2 * y2) * (x1 * y3 - x3 * y1) + (x3 * x3 + y3 * y3) * (x2 * y1 - x1 * y2);
    
    *radius = sqrt((b * b + c * c - 4 * a * d) / (4 * a * a));
    *centerX = -b / (2 * a);
    *centerY = -c / (2 * a);
}

int main(int argc, char **argv) {
    float cx,cy,r;
    centerradius(5,0,0,5,7,7,&cx,&cy,&r);
    
    printf("centerX: %0.2f\n",cx);
    printf("centerY: %0.2f\n",cy);
    printf("radius:  %0.2f\n",r);

    return 0;
}
