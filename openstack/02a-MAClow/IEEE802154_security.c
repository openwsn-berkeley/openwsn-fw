/**
\brief Security operations defined by IEEE802.15.4e standard

\author Savio Sciancalepore <savio.sciancalepore@poliba.it>, June 2015.
\author Giuseppe Piro <giuseppe.piro@poliba.it> June 2015.
\author Gennaro Boggia <gennaro.boggia@poliba.it> June 2015.
\author Luigi Alfredo Grieco <alfredo.grieco@poliba.it>, June 2015.
\author Malisa Vucinic <malishav@gmail.com>, June 2015.
*/

#include "packetfunctions.h"
#include "cryptoengine.h"
#include "IEEE802154.h"
#include "IEEE802154E.h"
#include "idmanager.h"
#include "openserial.h"
#include "neighbors.h"
#include "IEEE802154_security.h"

//=============================define==========================================
#ifdef L2_SECURITY_ACTIVE
//=========================== variables =======================================

ieee802154_security_vars_t ieee802154_security_vars;

//=========================== prototypes ======================================

//=========================== admin ===========================================

/**
\brief Initialization of security tables and parameters.
*/
void IEEE802154_security_init(void) {

    // TODO joinPermitted flag should be set dynamically upon a button press
    // and propagated through the network via EBs
    ieee802154_security_vars.joinPermitted = TRUE;

   // invalidate beacon key (key 1)
   ieee802154_security_vars.k1.index = IEEE802154_SECURITY_KEYINDEX_INVALID;
   memset(&ieee802154_security_vars.k1.value[0], 0x00, 16);

   // invalidate data key (key 2)
   ieee802154_security_vars.k2.index = IEEE802154_SECURITY_KEYINDEX_INVALID;
   memset(&ieee802154_security_vars.k2.value[0], 0x00, 16);
      }

//=========================== public ==========================================
/**
\brief Adding of Auxiliary Security Header to the IEEE802.15.4 MAC header
*/
void IEEE802154_security_prependAuxiliarySecurityHeader(OpenQueueEntry_t* msg){

   uint8_t frameCounterSuppression;
   uint8_t temp8b;
   open_addr_t* temp_keySource;
   uint8_t auxiliaryLength;

   frameCounterSuppression = IEEE154_ASH_FRAMECOUNTER_SUPPRESSED; //the frame counter is carried in the frame

   //max length of MAC frames
   // length of authentication Tag
   msg->l2_authenticationLength = IEEE802154_security_authLengthChecking(msg->l2_securityLevel);

   //length of auxiliary security header
   auxiliaryLength = IEEE802154_security_auxLengthChecking(msg->l2_keyIdMode,
                                                           frameCounterSuppression, // frame counter suppressed
                                                           0); //length of Key ID field


   if ((msg->length+auxiliaryLength+msg->l2_authenticationLength+2) >= 128 ){ //2 bytes of CRC, 127 MaxPHYPacketSize
      return;
   }

   //start setting the Auxiliary Security Header
   if (msg->l2_keyIdMode != IEEE154_ASH_KEYIDMODE_IMPLICIT){//if the KeyIdMode is zero, keyIndex and KeySource are omitted
      temp8b = msg->l2_keyIndex; //key index field
      packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));
      *((uint8_t*)(msg->payload)) = temp8b;
   }

   //insert the keyIdMode field
   switch (msg->l2_keyIdMode){
      case IEEE154_ASH_KEYIDMODE_IMPLICIT: //no KeyIDMode field - implicit
      case IEEE154_ASH_KEYIDMODE_DEFAULTKEYSOURCE:// macDefaultKeySource
         break;
      case IEEE154_ASH_KEYIDMODE_EXPLICIT_16: //keySource with 16b address
         temp_keySource = &msg->l2_keySource;
         packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));
         *((uint8_t*)(msg->payload)) = temp_keySource->addr_64b[6];
         packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));
         *((uint8_t*)(msg->payload)) = temp_keySource->addr_64b[7];
         break;
      case IEEE154_ASH_KEYIDMODE_EXPLICIT_64: //keySource with 64b address
         temp_keySource = &msg->l2_keySource;
         packetfunctions_writeAddress(msg,temp_keySource,OW_LITTLE_ENDIAN);
         break;
      default://error
         openserial_printError(COMPONENT_SECURITY,ERR_SECURITY,
                              (errorparameter_t)msg->l2_frameType,
                              (errorparameter_t)0);
         return;
   }

   //security control field
   packetfunctions_reserveHeaderSize(msg, sizeof(uint8_t));
 
   temp8b = 0;
   temp8b |= msg->l2_securityLevel << IEEE154_ASH_SCF_SECURITY_LEVEL;//3b
   temp8b |= msg->l2_keyIdMode << IEEE154_ASH_SCF_KEY_IDENTIFIER_MODE;//2b
   temp8b |= frameCounterSuppression << IEEE154_ASH_SCF_FRAME_CNT_MODE; //1b

      temp8b |= 1 << IEEE154_ASH_SCF_FRAME_CNT_SIZE; //1b

   temp8b |= 0 << 1;//1b reserved
   *((uint8_t*)(msg->payload)) = temp8b;
}

/**
\brief Key searching and encryption/authentication operations.
*/
owerror_t IEEE802154_security_outgoingFrameSecurity(OpenQueueEntry_t*   msg){
   uint8_t nonce[13];
   uint8_t *key;
   owerror_t outStatus;
   uint8_t* a;
   uint8_t len_a;
   uint8_t* m;
   uint8_t len_m;

   key = msg->l2_frameType == IEEE154_TYPE_BEACON ? ieee802154_security_vars.k1.value : ieee802154_security_vars.k2.value;

   // First 8 bytes of the nonce are always the source address of the frame
   memcpy(&nonce[0],idmanager_getMyID(ADDR_64B)->addr_64b,8);

   // Fill last 5 bytes with the ASN part of the nonce
   ieee154e_getAsn(&nonce[8]);
   packetfunctions_reverseArrayByteOrder(&nonce[8], 5);  // reverse ASN bytes to big endian 

   //identify data to be authenticated and data to be encrypted
   switch (msg->l2_securityLevel) {
      case IEEE154_ASH_SLF_TYPE_MIC_32:  // authentication only cases
      case IEEE154_ASH_SLF_TYPE_MIC_64:
      case IEEE154_ASH_SLF_TYPE_MIC_128: 
         a = msg->payload;             // first byte of the frame
         len_a = msg->length;          // whole frame
         m = &msg->payload[len_a];     // concatenate MIC at the end of the frame
         len_m = 0;                    // length of the encrypted part
         break;
      case IEEE154_ASH_SLF_TYPE_ENC_MIC_32:  // authentication + encryption cases
      case IEEE154_ASH_SLF_TYPE_ENC_MIC_64:
      case IEEE154_ASH_SLF_TYPE_ENC_MIC_128:
         a = msg->payload;             // first byte of the frame
         m = msg->l2_payload;          // first byte where we should start encrypting (see 15.4 std)
         len_a = m - a;                // part that is only authenticated is the difference of two pointers
         len_m = msg->length - len_a;  // part that is encrypted+authenticated is the rest of the frame
         break;
    case IEEE154_ASH_SLF_TYPE_ENC:    // encryption only
         // unsecure, should not support it
         return E_FAIL;
    default:
         // reject anything else
         return E_FAIL;
   }

   // assert
   if (len_a + len_m > 125) {
      openserial_printError(COMPONENT_SECURITY,ERR_SECURITY,
                           (errorparameter_t)msg->l2_frameType,
                           (errorparameter_t)2);
      return E_FAIL;
   }

   if (msg->l2_authenticationLength != 0) {
      //update the length of the packet
      packetfunctions_reserveFooterSize(msg,msg->l2_authenticationLength);
   }

   //Encryption and/or authentication
   // cryptoengine overwrites m[] with ciphertext and appends the MIC
   outStatus = cryptoengine_aes_ccms_enc(a,
                                          len_a,
                                          m,
                                          &len_m,
                                          nonce,
                                          2, // L=2 in 15.4 std
                                          key,
                                          msg->l2_authenticationLength);

   //verify that no errors occurred
   if (outStatus != E_SUCCESS) {
      openserial_printError(COMPONENT_SECURITY,ERR_SECURITY,
      (errorparameter_t)msg->l2_frameType,
      (errorparameter_t)3);
   }

   return outStatus;
}

/**
\brief Parsing of IEEE802.15.4 Auxiliary Security Header.
*/
void IEEE802154_security_retrieveAuxiliarySecurityHeader(OpenQueueEntry_t*      msg, 
                                                        ieee802154_header_iht* tempheader){

   uint8_t frameCnt_Suppression;
   uint8_t frameCnt_Size;
   uint8_t temp8b;
   uint8_t i;
   uint8_t receivedASN[5];
   macFrameCounter_t l2_frameCounter;

   //Retrieve the Security Control field
   //1byte, Security Control Field
   temp8b = *((uint8_t*)(msg->payload)+tempheader->headerLength);
   msg->l2_securityLevel = (temp8b >> IEEE154_ASH_SCF_SECURITY_LEVEL)& 0x07;//3b

   //identify the length of the MIC looking the security level
   msg->l2_authenticationLength = IEEE802154_security_authLengthChecking(msg->l2_securityLevel);

   //retrieve the KeyIdMode field
   msg->l2_keyIdMode = (temp8b >> IEEE154_ASH_SCF_KEY_IDENTIFIER_MODE)& 0x03;//2b

   //retrieve information on the Frame Counter Mode
   frameCnt_Suppression = (temp8b >> IEEE154_ASH_SCF_FRAME_CNT_MODE)& 0x01;//1b
   frameCnt_Size = (temp8b >> IEEE154_ASH_SCF_FRAME_CNT_SIZE)& 0x01;//1b

   //if the frame counter is zero, it is 4-bytes long, 5 otherwise
   if (frameCnt_Size == IEEE154_ASH_FRAMECOUNTER_COUNTER) {
      frameCnt_Size = 4;
   } else {
      frameCnt_Size = 5;
   }

   tempheader->headerLength++;

   //Frame Counter field
   if (frameCnt_Suppression == IEEE154_ASH_FRAMECOUNTER_PRESENT){//the frame counter is here
      //the frame counter size can be 4 or 5 bytes
      for (i=0;i<frameCnt_Size;i++){
          receivedASN[i] = *((uint8_t*)(msg->payload)+tempheader->headerLength);
          tempheader->headerLength++;
      }

      l2_frameCounter.bytes0and1 = receivedASN[0]+256*receivedASN[1];
      l2_frameCounter.bytes2and3 = receivedASN[2]+256*receivedASN[3];
      if (frameCnt_Size == 5){ //we have the ASN as the frame counter
         l2_frameCounter.byte4 = receivedASN[4];
      }

      if (l2_frameCounter.byte4 == 0xff){ //frame counter overflow
         openserial_printError(COMPONENT_SECURITY,ERR_SECURITY,
                              (errorparameter_t)msg->l2_frameType,
                              (errorparameter_t)4);
         return;
      }

   }

   //retrieve the Key Identifier field
   switch (msg->l2_keyIdMode){
      case IEEE154_ASH_KEYIDMODE_IMPLICIT:
      case IEEE154_ASH_KEYIDMODE_DEFAULTKEYSOURCE:
         //key is derived implicitly
         memcpy(&(msg->l2_keySource), &tempheader->src, sizeof(open_addr_t));
         break;
      case IEEE154_ASH_KEYIDMODE_EXPLICIT_16:
         packetfunctions_readAddress(((uint8_t*)(msg->payload)+tempheader->headerLength),
                                     ADDR_16B,
                                     &msg->l2_keySource,
                                     OW_LITTLE_ENDIAN);
         tempheader->headerLength+=2;
         break;
      case IEEE154_ASH_KEYIDMODE_EXPLICIT_64:
         packetfunctions_readAddress(((uint8_t*)(msg->payload)+tempheader->headerLength),
                                     ADDR_64B,
                                     &msg->l2_keySource,
                                     OW_LITTLE_ENDIAN);
         tempheader->headerLength+=8;
         break;
      default: //error
         openserial_printError(COMPONENT_SECURITY,ERR_SECURITY,
                              (errorparameter_t)msg->l2_frameType,
                              (errorparameter_t)5);
         return;
      }

   //retrieve the KeyIndex
   if (msg->l2_keyIdMode != IEEE154_ASH_KEYIDMODE_IMPLICIT){
      temp8b = *((uint8_t*)(msg->payload)+tempheader->headerLength);
      msg->l2_keyIndex = (temp8b);
      tempheader->headerLength++;
   } else {
      //key is derived implicitly
      msg->l2_keyIndex = 1;
   }
}

/**
\brief Identification of the key used to protect the frame and unsecuring operations.
*/
owerror_t IEEE802154_security_incomingFrame(OpenQueueEntry_t* msg){
   uint8_t nonce[13];
   owerror_t outStatus;
   uint8_t* a;
   uint8_t len_a;
   uint8_t* c;
   uint8_t len_c;
   uint8_t *key;

   key = msg->l2_frameType == IEEE154_TYPE_BEACON ? ieee802154_security_vars.k1.value : ieee802154_security_vars.k2.value;

   // First 8 bytes of the nonce are always the source address of the frame
   memcpy(&nonce[0],msg->l2_nextORpreviousHop.addr_64b,8);
   // Fill last 5 bytes with ASN part of the nonce
   ieee154e_getAsn(&nonce[8]);
   packetfunctions_reverseArrayByteOrder(&nonce[8], 5);  // reverse ASN bytes to big endian 

   //identify data to be authenticated and data to be decrypted
   switch (msg->l2_securityLevel) {
      case IEEE154_ASH_SLF_TYPE_MIC_32:  // authentication only cases
      case IEEE154_ASH_SLF_TYPE_MIC_64:
      case IEEE154_ASH_SLF_TYPE_MIC_128: 
         a = msg->payload;                                           // first byte of the frame
         len_a = msg->length-msg->l2_authenticationLength;           // whole frame
         c = &msg->payload[len_a];                                   // MIC is at the end of the frame
         len_c = msg->l2_authenticationLength;                       // length of the encrypted part
         break;
      case IEEE154_ASH_SLF_TYPE_ENC_MIC_32:  // authentication + encryption cases
      case IEEE154_ASH_SLF_TYPE_ENC_MIC_64:
      case IEEE154_ASH_SLF_TYPE_ENC_MIC_128:
         a = msg->payload;             // first byte of the frame
         c = msg->l2_payload;          // first byte where we should start decrypting 
         len_a = c - a;                // part that is only authenticated is the difference of two pointers
         len_c = msg->length - len_a;  // part that is decrypted+authenticated is the rest of the frame
         break;
      case IEEE154_ASH_SLF_TYPE_ENC:    // encryption only
         // unsecure, should not support it
         return E_FAIL;
      default:
         // reject anything else
         return E_FAIL;
    }

   // assert
   if (len_a + len_c > 125) {
      openserial_printError(COMPONENT_SECURITY,ERR_SECURITY,
                           (errorparameter_t)msg->l2_frameType,
                           (errorparameter_t)11);
      return E_FAIL;
   }

   //decrypt and/or verify authenticity of the frame
   outStatus = cryptoengine_aes_ccms_dec(a,
                                          len_a,
                                          c,
                                          &len_c,
                                          nonce,
                                          2,
                                    key,
                                          msg->l2_authenticationLength);

   //verify if any error occurs
   if (outStatus != E_SUCCESS){
      openserial_printError(COMPONENT_SECURITY,ERR_SECURITY,
                           (errorparameter_t)msg->l2_frameType,
                           (errorparameter_t)12);
   }

   packetfunctions_tossFooter(msg,msg->l2_authenticationLength);
   
   return outStatus;
}

/**
\brief Identification of the length of the MIC based on the security Level in the frame
*/
uint8_t IEEE802154_security_authLengthChecking(uint8_t securityLevel){

   uint8_t authlen;
   switch (securityLevel) {
      case IEEE154_ASH_SLF_TYPE_NOSEC:
      case IEEE154_ASH_SLF_TYPE_ENC:
         authlen = 0;
         break;
      case IEEE154_ASH_SLF_TYPE_MIC_32:
      case IEEE154_ASH_SLF_TYPE_ENC_MIC_32:
         authlen = 4;
         break;
      case IEEE154_ASH_SLF_TYPE_MIC_64:
      case IEEE154_ASH_SLF_TYPE_ENC_MIC_64:
         authlen = 8;
         break;
      case IEEE154_ASH_SLF_TYPE_MIC_128:
      case IEEE154_ASH_SLF_TYPE_ENC_MIC_128:
         authlen = 16;
         break;
      }

   return authlen;
}

/**
\brief Identification of the length of the IEEE802.15.4 Auxiliary Security Header.
*/
uint8_t IEEE802154_security_auxLengthChecking(uint8_t KeyIdMode, 
                                             uint8_t frameCounterSuppression, 
                                             uint8_t frameCounterSize){
   uint8_t auxilary_len;
   uint8_t frameCntLength;

   auxilary_len = 0;

   if (frameCounterSuppression == IEEE154_ASH_FRAMECOUNTER_PRESENT){
      if (frameCounterSize == 4){
         frameCntLength = 4;
      } else {
         frameCntLength = 5;
      }
   } else {
      frameCntLength = 0;
   }

   switch (KeyIdMode){
      case IEEE154_ASH_KEYIDMODE_IMPLICIT:
         auxilary_len = frameCntLength + 1; //only SecLev and FrameCnt
         break;
      case IEEE154_ASH_KEYIDMODE_DEFAULTKEYSOURCE:
         auxilary_len = frameCntLength + 1 + 1; //SecLev, FrameCnt, KeyIndex
         break;
      case IEEE154_ASH_KEYIDMODE_EXPLICIT_16:
         auxilary_len = frameCntLength + 1 + 3; //SecLev, FrameCnt, KeyIdMode (2 bytes) and KeyIndex
         break;
      case IEEE154_ASH_KEYIDMODE_EXPLICIT_64:
         auxilary_len = frameCntLength + 1 + 9; //SecLev, FrameCnt, KeyIdMode (8 bytes) and KeyIndex
         break;
      default:
         break;
   }

   return auxilary_len;
}

uint8_t IEEE802154_security_getBeaconKeyIndex(void) {
    return ieee802154_security_vars.k1.index;
      }
uint8_t IEEE802154_security_getDataKeyIndex(void) {
    return ieee802154_security_vars.k2.index;
      }

void IEEE802154_security_setBeaconKey(uint8_t index, uint8_t* value) {
    ieee802154_security_vars.k1.index = index;
    memcpy(ieee802154_security_vars.k1.value, value, 16);
}

void IEEE802154_security_setDataKey(uint8_t index, uint8_t* value) {
    ieee802154_security_vars.k2.index = index;
    memcpy(ieee802154_security_vars.k2.value, value, 16);
      }

bool IEEE802154_security_isConfigured() {
    if (ieee802154_security_vars.k1.index != IEEE802154_SECURITY_KEYINDEX_INVALID &&
         ieee802154_security_vars.k2.index != IEEE802154_SECURITY_KEYINDEX_INVALID) {
         return TRUE;
      }
         return FALSE;
      }

uint8_t IEEE802154_security_getSecurityLevel(OpenQueueEntry_t *msg) {
    if (IEEE802154_security_isConfigured() == FALSE) {
        return IEEE154_ASH_SLF_TYPE_NOSEC;
}

    if (packetfunctions_isBroadcastMulticast(&msg->l2_nextORpreviousHop)) {
        return IEEE802154_SECURITY_LEVEL;
      }

    if(neighbors_isInsecureNeighbor(&msg->l2_nextORpreviousHop) &&
       ieee802154_security_vars.joinPermitted == TRUE) {
        return IEEE154_ASH_SLF_TYPE_NOSEC;
      }

    return IEEE802154_SECURITY_LEVEL;
}

bool IEEE802154_security_acceptableLevel(OpenQueueEntry_t* msg, ieee802154_header_iht* parsedHeader) {
    if (IEEE802154_security_isConfigured() == FALSE     &&
        msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_NOSEC) {
        return TRUE;
         }

    if (IEEE802154_security_isConfigured() == FALSE             &&
        msg->l2_frameType == IEEE154_TYPE_BEACON                &&
        (msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_MIC_32   ||
         msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_MIC_64   ||
         msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_MIC_128)) {
        return TRUE;
}

    if (IEEE802154_security_isConfigured()               == TRUE &&
        msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_NOSEC      &&
        ieee802154_security_vars.joinPermitted           == TRUE &&
        neighbors_isInsecureNeighbor(&parsedHeader->src) == TRUE) {
        return TRUE;
             }

    if (IEEE802154_security_isConfigured() == TRUE              &&
        msg->l2_frameType == IEEE154_TYPE_BEACON                &&
        (msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_MIC_32   ||
         msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_MIC_64   ||
         msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_MIC_128)) {
        return TRUE;
         }

    if (IEEE802154_security_isConfigured() == TRUE                  &&
        (msg->l2_frameType == IEEE154_TYPE_DATA                     ||
         msg->l2_frameType == IEEE154_TYPE_ACK                      ||
         msg->l2_frameType == IEEE154_TYPE_CMD)                     &&
        (msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_ENC_MIC_32   ||
         msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_ENC_MIC_64   ||
         msg->l2_securityLevel == IEEE154_ASH_SLF_TYPE_ENC_MIC_128)) {
        return TRUE;
            }
    return FALSE;
         }

#else /* L2_SECURITY_ACTIVE */

void IEEE802154_security_init(void) {
    return;
}

void IEEE802154_security_prependAuxiliarySecurityHeader(OpenQueueEntry_t* msg){
    return;
}

void IEEE802154_security_retrieveAuxiliarySecurityHeader(OpenQueueEntry_t* msg, ieee802154_header_iht* tempheader) {
    return;
}

owerror_t IEEE802154_security_outgoingFrameSecurity(OpenQueueEntry_t* msg) {
    return E_SUCCESS;
}

owerror_t IEEE802154_security_incomingFrame(OpenQueueEntry_t* msg) {
    return E_SUCCESS;
}

uint8_t IEEE802154_security_authLengthChecking(uint8_t sec_level) {
    return (uint8_t) 0;
}

uint8_t IEEE802154_security_auxLengthChecking(uint8_t kid, uint8_t sup, uint8_t size) {
    return (uint8_t) 0;
}

uint8_t IEEE802154_security_getBeaconKeyIndex(void) {
    return (uint8_t) 0;
}
uint8_t IEEE802154_security_getDataKeyIndex(void) {
    return (uint8_t) 0;
}

void IEEE802154_security_setBeaconKey(uint8_t index, uint8_t* value) {
    return;
}

void IEEE802154_security_setDataKey(uint8_t index, uint8_t* value) {
    return;
}

bool IEEE802154_security_isConfigured() {
    return TRUE;
}

uint8_t IEEE802154_security_getSecurityLevel(OpenQueueEntry_t *msg) {
    return IEEE154_ASH_SLF_TYPE_NOSEC;
}

bool IEEE802154_security_acceptableLevel(OpenQueueEntry_t* msg, ieee802154_header_iht* parsedheader) {
    return TRUE;
}

#endif /* L2_SECURITY_ACTIVE */

