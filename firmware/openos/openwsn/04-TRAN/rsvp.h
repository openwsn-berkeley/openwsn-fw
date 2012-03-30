/**
 \brief RSVP headers and message structures.

RSVP_Msg
   rsvp_msg_header
   rsvp_objects*
       rsvp_object_header
       rsvp_specific_object content.

 \author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, March 2012.
 */

#ifndef __RSVP_H
#define __RSVP_H

#include "stdint.h"

//general header of any rsvp message
typedef struct{
	uint8_t version_flags;//4 high bit version, 4 low flags
	uint8_t msg_type;
	uint16_t checksum;
	uint8_t sendTTL;
	uint8_t reserved;
	uint16_t length;
}rsvp_msg_header_t;

//header of any of the rsvp objects
typedef struct{
	uint16_t length;
	uint8_t class_num;
	uint8_t c_type;
}rsvp_object_header_t;

typedef struct{
	uint8_t num_cycles;
	uint8_t num_participants;
}cycle_table_tuple_t;

typedef struct{
     cycle_table_tuple_t cycles[10];
}rsvp_trans_cycle_table_t;

typedef struct{
	rsvp_object_header_t header;
	uint8_t level;
	uint8_t ordering;
	uint16_t freq_blacklist;//bitmap
	uint8_t freq_offset;
	rsvp_trans_cycle_table_t cycle_table;
}rsvp_label_object_t;


typedef struct{
	uint8_t offset;
	uint8_t mode;
}rsvp_enhanced_mode_tuple_t;

typedef struct{
	rsvp_object_header_t header;
    uint8_t total_demand;
    uint8_t demand;
    rsvp_enhanced_mode_tuple_t ehmode;
}rsvp_sender_llnspec_t;




//reservation request message. Includes QoS requirements. Some required objects are omitted by now.
typedef struct{
	rsvp_msg_header_t header;
rsvp_sender_llnspec_t sender_llnspec;
}rsvp_path_msg_t;

//downstream message including the label information. Some required objects are omitted by now.
typedef struct{
	rsvp_msg_header_t header;
	rsvp_label_object_t label;
}rsvp_resv_msg_t;


#endif

