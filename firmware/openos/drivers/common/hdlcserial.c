/**
\brief Definition of the "openserial" driver.

\author Min Ting, October 2012.
\author Fabien Chraim <chraim@eecs.berkeley.edu>, October 2012.
*/

#include "hdlcserial.h"

//=========================== variables =======================================

//=========================== prototypes ======================================

//=========================== public ==========================================

uint16_t crcIteration(uint16_t crc, uint8_t byte) {
   return (crc >> 8) ^ fcstab[(crc ^ byte) & 0xff];
}

//=========================== private =========================================

//#############################################################################
//#### OLD ### OLD ### OLD ### OLD ### OLD ### OLD ### OLD ### OLD ### OLD ####
//#############################################################################

/*
uint8_t hdlcify(uint8_t* buf, uint8_t inputLen) {
   uint16_t fcs;
   uint8_t  stuff_count;
   uint8_t  crc1;
   uint8_t  crc2;
   
   uint8_t  bufIdx;
   
   uint8_t  tempBuf[HDLC_MAX_LEN];
   uint8_t  tempBufLen;
   uint8_t  tempBufIdx;
   
   //===== step 1. compute crc
   fcs = HDLC_CRCINIT;                           // initial FCS
   for (bufIdx=0;bufIdx<inputLen;bufIdx++) {
      fcs = crcIteration(fcs, buf[bufIdx]);      // iterate through bytes
   }
   fcs = ~fcs;                                   // take one's complement
   crc1 = fcs;
   crc2 = fcs>>8;
   
   //===== step 2. count the number of bytes to stuff
   stuff_count = 0;
   for (bufIdx=0;bufIdx<inputLen;bufIdx++) {
      if ( (buf[bufIdx]==HDLC_FLAG) || (buf[bufIdx]==HDLC_ESCAPE)) {
         stuff_count++;
      }
   }
   if ((crc1==HDLC_FLAG) || (crc1==HDLC_ESCAPE)) {
      stuff_count++;
   }
   if ((crc2==HDLC_FLAG) || (crc2==HDLC_ESCAPE)) {
      stuff_count++;
   }
   
   //===== step 3. prepare temporary buffer
   // flag, (pkt+stuffed), crc, flag
   tempBufLen                     = 1+(inputLen+stuff_count)+2+1;
   // opening flags
   tempBuf[0]                     = HDLC_FLAG;
   // CRC
   tempBuf[tempBufLen-3]          = crc1;
   tempBuf[tempBufLen-2]          = crc2;
   // closing flags
   tempBuf[tempBufLen-1]          = HDLC_FLAG;
   
   //===== step 4. fill temporary buffer
   tempBufIdx=1;
   for (bufIdx=0;bufIdx<inputLen;bufIdx++){
      if (buf[bufIdx]==HDLC_FLAG) {
         tempBuf[tempBufIdx]      = HDLC_ESCAPE;
         tempBuf[tempBufIdx+1]    = HDLC_FLAG_ESCAPED;
         tempBufIdx += 2;
      } else if(buf[bufIdx]==HDLC_ESCAPE) {
         tempBuf[tempBufIdx]      = HDLC_ESCAPE;
         tempBuf[tempBufIdx+1]    = HDLC_ESCAPE_ESCAPED;
         tempBufIdx += 2;
      } else {
         tempBuf[tempBufIdx]      = buf[bufIdx];
         tempBufIdx += 1;
      }
   }
   
   //===== step 5. copy temporary buffer back into buf
   memcpy(buf,tempBuf,tempBufLen);
   
   return tempBufLen;
}
*/

/*
uint8_t dehdlcify(uint8_t* buf, uint8_t len) {
   uint16_t   crcExp;
   uint8_t    stuff_count;
   
   uint8_t    bufIdx;
   
   uint8_t    tempBuf[20];
   uint8_t    tempBufLen;
   uint8_t    tempBufIdx;
   
   //===== step 0. Make sure this is an HDLC frame
   if(buf[0] != HDLC_FLAG || buf[len-1] != HDLC_FLAG) {
      return 0;
   }
   
   //===== step 1. count the number of stuffed bytes
   stuff_count = 0;
   for (bufIdx=0;bufIdx<len;bufIdx++) {
      if (buf[bufIdx]==HDLC_ESCAPE) {
         stuff_count++;
      }
   }
   
   //===== step 2. unstuff into temporary buffer
   tempBufLen = len-stuff_count-1-2-1; // stuffed, flag, crc, flag
   bufIdx = 1;
   for (tempBufIdx=0;tempBufIdx<tempBufLen;tempBufIdx++){
      if (buf[bufIdx]==HDLC_ESCAPE){
         if        (buf[bufIdx+1]==HDLC_FLAG_ESCAPED) {
            tempBuf[tempBufIdx] = HDLC_FLAG;
         } else if (buf[bufIdx+1]==HDLC_ESCAPE_ESCAPED) {
            tempBuf[tempBufIdx] = HDLC_ESCAPE;
         } else {
            // this should never happen
         }
         bufIdx += 2;
      } else {
         tempBuf[tempBufIdx] = buf[bufIdx];
         bufIdx += 1;
      }
      tempBuf[tempBufIdx] = buf[bufIdx];
      bufIdx += 1;
   }
   
   //===== step 3. check CRC
   // read CRC from buf
   crcExp = buf[bufIdx] + (buf[bufIdx+1]<<8);
   // recalculate and check CRC
   if (doesCrcCheck(tempBuf,tempBufLen,crcExp)) {
      // CRC checks
      memcpy(buf,tempBuf,tempBufLen);
      return tempBufLen;
   } else {
      // CRC does not check
      return 0;
   }
}
*/


