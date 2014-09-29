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
#include "openwsn.h"

#define RSVP_MAX_SCHED 10 //max cycles combinations that can be transported.

enum{
   RSVP_PATH_MESSAGE     =1,
   RSVP_RESV_MESSAGE     =2,
   RSVP_PATHERR_MESSAGE  =3,
   RSVP_RESVERR_MESSAGE  =4,
   RSVP_PATHTEAR_MESSAGE  =5,
   RSVP_RESVTEAR_MESSAGE  =6,
   RSVP_RESVCONF_MESSAGE  =7,
};

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

/* identifies a rsvp session- each session describes a path between two nodes.*/
typedef struct{
   rsvp_object_header_t header;
   open_addr_t ip_dest_address; //dest address
   uint8_t dest_port; //dest port
   uint8_t protocol_id; //protocol id
   uint8_t flags; //
   bool isLSP_tunnel_session;
   uint8_t tunnel_id;
}rsvp_session_t;

/*This object contains the value for the refresh period R used by the creator of the message. This information
required in every Path and Resv message.*/

typedef struct{
   rsvp_object_header_t header;
   uint16_t rsvp_time_values;
}rsvp_time_values_t;

/* This object defines the reservation style plus style-specific information that is in FLOWSPEC or FILTER_SPEC
object and it is required in every Resv message. The RSVP defines four reservation styles, but only three styles, WF, FF,
and SE are defined for signaling purpose.
 * 
 */
typedef struct{
   rsvp_object_header_t header;
   uint8_t rsvp_style_t;
}rsvp_style_t;
/*
 * Since two objects, Filter Specification and Sender Template, contain similar parameters we models these object as
one Sender Object.. This object presents a subset of session data packets that should receive the desired QoS
(specified by a FLOWSPEC object), in a Resv message and contains a sender IP address.
 * 
 */
typedef struct {
   rsvp_object_header_t header;
   open_addr_t source_ip_address;//address of the sender.
   uint8_t source_port; //dest port
   uint8_t lsp_id; //id of the LSP
   uint8_t sender_object_id;//id of this object;
}rsvp_sender_template_t;
/* Next or previous hop object -- depending if the msg is upstream or downstream*/
typedef struct{
   rsvp_object_header_t header;
   open_addr_t address;
   uint8_t local_interface_handle; //handle to the use slot LIH_value in the std.
}rsvp_hop_t;


typedef struct{
   rsvp_object_header_t header;
   uint8_t lsp_enc_type;//TDM
   uint8_t lsp_sw_type; //switching type
}rsvp_label_request_t;


typedef struct{
   uint8_t num_cycles;
   uint8_t num_participants;
}cycle_table_tuple_t;

typedef struct{
     cycle_table_tuple_t cycles[RSVP_MAX_SCHED];
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
   rsvp_session_t session;
   rsvp_hop_t hop;
   rsvp_time_values_t time_values;
   rsvp_label_request_t label_request;
   rsvp_sender_template_t sender_template;
    rsvp_sender_llnspec_t sender_llnspec;
}rsvp_path_msg_t;

//downstream message including the label information. Some required objects are omitted by now.
typedef struct{
   rsvp_msg_header_t header;
   rsvp_session_t session;
   rsvp_hop_t hop;
   rsvp_time_values_t time_values;
   rsvp_style_t  style;
    rsvp_sender_template_t sender_template;
   rsvp_label_object_t label;
}rsvp_resv_msg_t;



void    rsvp_init(void);
void    rsvp_qos_request(uint8_t bandwith, uint16_t refresh_period,open_addr_t dest);

#endif

