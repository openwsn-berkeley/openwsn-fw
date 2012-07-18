#ifndef __BBK_H
#define __BBK_H

/**
\addtogroup App
\{
\addtogroup rT
\{
*/

//=========================== define ==========================================

//=========================== typedef =========================================
typedef struct {
    uint8_t	  asn[5];        //either side
    uint8_t       channel; //either side
    uint8_t       retry;
    uint8_t       seq[4];   //Tx
} demo_t;
//=========================== variables =======================================

//=========================== prototypes ======================================

void bbk_init();
void    construct_demo(demo_t*);
/**
\}
\}
*/

#endif
