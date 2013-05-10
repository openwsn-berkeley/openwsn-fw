/**
\brief Handler of commmands received from the OpenSim server.

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, May 2012.
*/

#ifndef __OPENSIM_CMDHANDLER_H
#define __OPENSIM_CMDHANDLER_H

//=========================== define ==========================================

//=========================== typedef =========================================

//=========================== prototypes ======================================

void opensim_cmdHandler_handle(
   int   cmdType,
   int   paramLen,
   char* paramBuf
 );

#endif