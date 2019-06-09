#include "project.h"
#include "hyatt.h"

extern parser_block_t gc_block;


#define ZDISPLAYUPDATEINTERVAL 200


void hyattZDisplayInit() {
    uartZDisplay_Start();
}

void hyattZDisplayLoop() {
    char buf[100];

    if (hyattTicks > hyattZDisplayUpdate) {
        hyattZDisplayCommand("ref_stop");
        sprintf(buf,"G%d G%d G%d G%d",54+gc_state.modal.coord_select,gc_state.modal.units,gc_state.modal.motion,gc_state.modal.distance+90);

        hyattZDisplaySet("c",buf);

        if (gc_state.modal.units) {
            hyattZDisplaySet("u","INCH");
        } else {
            hyattZDisplaySet("u","MM");
        }

        if (gc_block.modal.spindle & SPINDLE_ENABLE_CW) {
            hyattZDisplaySet("s","SPINDLE");
        } else {
            hyattZDisplaySet("s","");
        }

        if (gc_state.modal.coolant & COOLANT_MIST_ENABLE) {
            hyattZDisplaySet("a","AIR");
        } else {
            hyattZDisplaySet("a","");
        }


        sprintf(buf,"F%4d/%-3d%%",(uint16_t)gc_state.feed_rate,sys.f_override);
        hyattZDisplaySet("f",buf);

        sprintf(buf,"%9.4f",x);
        hyattZDisplaySet("x",buf);
        sprintf(buf,"%9.4f",y);
        hyattZDisplaySet("y",buf);
        sprintf(buf,"%9.4f",z);
        hyattZDisplaySet("z",buf);

        hyattZDisplaySet("st",lastBlock);

        hyattZDisplayCommand("ref_star");
        hyattZDisplayUpdate = hyattTicks + ZDISPLAYUPDATEINTERVAL;
    }
}

void hyattZDisplayCommand(char *command) {
    char line[100];
    sprintf(line,"%s\xff\xff\xff",command);
    uartZDisplay_PutString(line);
}

void hyattZDisplaySet(char *attr,char *value) {
    char line[100];
    sprintf(line,"%s.txt=\"%s\"\xff\xff\xff",attr,value);
    uartZDisplay_PutString(line);
}
