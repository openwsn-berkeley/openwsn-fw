#ifndef __ICMPv6RPL_H
#define __ICMPv6RPL_H

/**
\addtogroup IPv6
\{
\addtogroup ICMPv6RPL
\{
*/

#include "opentimers.h"

//=========================== define ==========================================

#define DIO_PERIOD             10000   // in miliseconds
#define DAO_PERIOD             60000   // in miliseconds

// Non-Storing Mode of Operation (1)
#define MOP_DIO_A                 0<<5
#define MOP_DIO_B                 0<<4
#define MOP_DIO_C                 1<<3
// least preferred (0)
#define PRF_DIO_A                 0<<2
#define PRF_DIO_B                 0<<1
#define PRF_DIO_C                 0<<0
// Grounded (1)
#define G_DIO                     1<<7

#define FLAG_DAO_A                0<<0
#define FLAG_DAO_B                0<<1
#define FLAG_DAO_C                0<<2
#define FLAG_DAO_D                0<<3
#define FLAG_DAO_E                0<<4
#define FLAG_DAO_F                0<<5
#define D_DAO                     1<<6
#define K_DAO                     0<<7

#define E_DAO_Transit_Info        0<<7

#define PC1_A_DAO_Transit_Info    0<<7
#define PC1_B_DAO_Transit_Info    1<<6

#define PC2_A_DAO_Transit_Info    0<<5
#define PC2_B_DAO_Transit_Info    0<<4

#define PC3_A_DAO_Transit_Info    0<<3
#define PC3_B_DAO_Transit_Info    0<<2

#define PC4_A_DAO_Transit_Info    0<<1
#define PC4_B_DAO_Transit_Info    0<<0

#define Prf_A_dio_options         0<<4
#define Prf_B_dio_options         0<<3

#define DEFAULT_PATH_CONTROL_SIZE 0

#define RPL_OPTION_PIO 0x8
#define RPL_OPTION_CONFIG 0x4

// max number of parents and children to send in DAO
//section 8.2.1 pag 67 RFC6550 -- using a subset
#define MAX_TARGET_PARENTS        0x01

enum{
  OPTION_ROUTE_INFORMATION_TYPE   = 0x03,
  OPTION_DODAG_CONFIGURATION_TYPE = 0x04,
  OPTION_TARGET_INFORMATION_TYPE  = 0x05,
  OPTION_TRANSIT_INFORMATION_TYPE = 0x06,
};

//=========================== static ==========================================

/**
\brief Well-known IPv6 multicast address for "all routers".
*/
static const uint8_t all_routers_multicast[] = {
   0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a
};

//=========================== typedef =========================================

//===== DIO

/**
\brief Header format of a RPL DIO packet.
*/
BEGIN_PACK
typedef struct {
   uint8_t         rplinstanceId;      ///< set by the DODAG root.
   uint8_t         verNumb;
   dagrank_t       rank;
   uint8_t         rplOptions;
   uint8_t         DTSN;
   uint8_t         flags;
   uint8_t         reserved;
   uint8_t         DODAGID[16];
} icmpv6rpl_dio_ht;
END_PACK


BEGIN_PACK
typedef struct {
   uint8_t type; // 0x08
   uint8_t optLen; // 30d
   uint8_t prefLen;//64
   uint8_t flags; //96 L=0,A=1,R=1,00000
   uint32_t vlifetime; //0xFFFFFFFF infinity
   uint32_t plifetime; //0xFFFFFFFF infinity
   uint32_t reserved;
   uint8_t  prefix[16]; // myaddress
}icmpv6rpl_pio_t;
END_PACK

BEGIN_PACK
typedef struct {
   uint8_t type; // 0x04
   uint8_t optLen; // 14d
   uint8_t flagsAPCS;
   uint8_t DIOIntDoubl; //8 -> trickle period - max times it will double ~20min
   uint8_t DIOIntMin; // 12 ->  min trickle period -> 16s
   uint8_t DIORedun; // 0
   uint16_t maxRankIncrease; //  2048
   uint16_t minHopRankIncrease; //256
   uint16_t OCP; // 0 OF0
   uint8_t reserved;
   uint8_t defLifetime; // limit for DAO period  -> 0xff
   uint16_t lifetimeUnit; // 0xffff
}icmpv6rpl_config_ht;
END_PACK

//===== DAO

/**
\brief Header format of a RPL DAO packet.
*/
BEGIN_PACK
typedef struct {
   uint8_t         rplinstanceId;      ///< set by the DODAG root.
   uint8_t         K_D_flags;
   uint8_t         reserved;
   uint8_t         DAOSequence;
   uint8_t         DODAGID[16];
} icmpv6rpl_dao_ht;
END_PACK

/**
\brief Header format of a RPL DAO "Transit Information" option.
*/
BEGIN_PACK
typedef struct {
   uint8_t         type;               ///< set by the DODAG root.
   uint8_t         optionLength;
   uint8_t         E_flags;
   uint8_t         PathControl;
   uint8_t         PathSequence;
   uint8_t         PathLifetime;
} icmpv6rpl_dao_transit_ht;
END_PACK

/**
\brief Header format of a RPL DAO "Target" option.
*/
BEGIN_PACK
typedef struct {
   uint8_t         type;               ///< set by the DODAG root.
   uint8_t         optionLength;
   uint8_t         flags;
   uint8_t         prefixLength;
} icmpv6rpl_dao_target_ht;
END_PACK

//=========================== module variables ================================



typedef struct {
   // admin
   bool                      busySendingDIO;          ///< currently sending DIO.
   bool                      busySendingDAO;          ///< currently sending DAO.
   uint8_t                   fDodagidWritten;         ///< is DODAGID already written to DIO/DAO?
   // DIO-related
   icmpv6rpl_dio_ht          dio;                     ///< pre-populated DIO packet.
   icmpv6rpl_pio_t           pio;                     ///< pre-populated PIO com
   icmpv6rpl_config_ht       conf;
   open_addr_t               dioDestination;          ///< IPv6 destination address for DIOs.
   uint16_t                  dioTimerCounter;         ///< counter to determine when to send DIO.
   opentimers_id_t           timerIdDIO;              ///< ID of the timer used to send DIOs.
   uint16_t                  dioPeriod;               ///< dio period in seconds.
   // DAO-related
   icmpv6rpl_dao_ht          dao;                     ///< pre-populated DAO packet.
   icmpv6rpl_dao_transit_ht  dao_transit;             ///< pre-populated DAO "Transit Info" option header.
   icmpv6rpl_dao_target_ht   dao_target;              ///< pre-populated DAO "Transit Info" option header.
   opentimers_id_t           timerIdDAO;              ///< ID of the timer used to send DAOs.
   uint16_t                  daoTimerCounter;         ///< counter to determine when to send DAO.
   uint16_t                  daoPeriod;               ///< dao period in seconds.
   // routing table
   dagrank_t                 myDAGrank;               ///< rank of this router within DAG.
   uint16_t                  rankIncrease;            ///< the cost of the link to the parent, in units of rank
   bool                      haveParent;              ///< this router has a route to DAG root
   uint8_t                   ParentIndex;             ///< index of Parent in neighbor table (iff haveParent==TRUE)
   // actually only here for debug
   icmpv6rpl_dio_ht*         incomingDio;             //keep it global to be able to debug correctly.
   icmpv6rpl_pio_t*          incomingPio;             //pio structure incoming
   icmpv6rpl_config_ht*      incomingConf;            //configuration incoming
   bool                      daoSent;
} icmpv6rpl_vars_t;



//=========================== prototypes ======================================

void     icmpv6rpl_init(void);
void     icmpv6rpl_sendDone(OpenQueueEntry_t* msg, owerror_t error);
void     icmpv6rpl_receive(OpenQueueEntry_t* msg);
void     icmpv6rpl_writeDODAGid(uint8_t* dodagid);
uint8_t  icmpv6rpl_getRPLIntanceID(void);
owerror_t icmpv6rpl_getRPLDODAGid(uint8_t* address_128b);
void     icmpv6rpl_setDIOPeriod(uint16_t dioPeriod);
void     icmpv6rpl_setDAOPeriod(uint16_t daoPeriod);
bool     icmpv6rpl_getPreferredParentIndex(uint8_t* indexptr);
bool     icmpv6rpl_getPreferredParentEui64(open_addr_t* addressToWrite);
bool     icmpv6rpl_isPreferredParent(open_addr_t* address);
dagrank_t icmpv6rpl_getMyDAGrank(void);
void     icmpv6rpl_setMyDAGrank(dagrank_t rank);
void     icmpv6rpl_killPreferredParent(void);
void     icmpv6rpl_updateMyDAGrankAndParentSelection(void);
void     icmpv6rpl_indicateRxDIO(OpenQueueEntry_t* msg);
bool     icmpv6rpl_daoSent(void);


/**
\}
\}
*/

#endif


