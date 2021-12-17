
#include "delay.h"

void tempo(unsigned char i) 
{
    for (i; i > 0; i--) 
    {
        volatile unsigned char j, k;
        for (j = 0; j < 41; j++) 
        {
            for (k = 0; k < 3; k++);
        }
    }
}

void tempoS(unsigned char i)
{   
    for (i=0;i<4;i++)
    {
        tempo(250);
    }
}
