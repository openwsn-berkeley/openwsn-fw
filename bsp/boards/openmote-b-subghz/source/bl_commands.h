/******************************************************************************
*  Filename:       bl_commands.h
*  Revised:        $Date: 2012-10-03 22:23:04 +0200 (on, 03 okt 2012) $
*  Revision:       $Revision: 8460 $
*
*  Description:    Commands and return messages supported by the
*                  ROM-based boot loader.
*                  
*
*  Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#ifndef __BL_COMMANDS_H__
#define __BL_COMMANDS_H__

//*****************************************************************************
//
// This command is used to receive an acknowledge from the the boot loader
// proving that communication has been established.  This command is a single
// byte.
//
// The format of the command is as follows:
//
//     uint8_t ui8Command[1];
//
//     ui8Command[0] = COMMAND_PING;
//
//*****************************************************************************
#define COMMAND_PING            0x20

//*****************************************************************************
//
// This command is sent to the boot loader to indicate where
// to store data and how many bytes will be sent by the
// COMMAND_SEND_DATA commands that follow. The command
// consists of two 32-bit values that are both transferred MSB first.
// The first 32-bit value is the address to start programming data
// into, while the second is the 32-bit size of the data that will be
// sent. The Program Size parameter must be a multiple of 4. This
// command should be followed by a COMMAND_GET_STATUS to
// ensure that the program address and program size were valid
// for the boot loader.
//
// The format of the command is as follows:
//
//     uint8_t ui8Command[9];
//
//     ui8Command[0] = COMMAND_DOWNLOAD;
//     ui8Command[1] = Program Address [31:24];
//     ui8Command[2] = Program Address [23:16];
//     ui8Command[3] = Program Address [15:8];
//     ui8Command[4] = Program Address [7:0];
//     ui8Command[5] = Program Size [31:24];
//     ui8Command[6] = Program Size [23:16];
//     ui8Command[7] = Program Size [15:8];
//     ui8Command[8] = Program Size [7:0];
//
//*****************************************************************************
#define COMMAND_DOWNLOAD        0x21

//*****************************************************************************
//
// This command is sent to the boot loader to transfer execution control to the
// specified address.  The command is followed by a 32-bit value, transferred
// MSB first, that is the address to which execution control is transferred.
//
// The format of the command is as follows:
//
//     uint8_t ui8Command[5];
//
//     ui8Command[0] = COMMAND_RUN;
//     ui8Command[1] = Run Address [31:24];
//     ui8Command[2] = Run Address [23:16];
//     ui8Command[3] = Run Address [15:8];
//     ui8Command[4] = Run Address [7:0];
//
//*****************************************************************************
#define COMMAND_RUN             0x22

//*****************************************************************************
//
// This command returns the status of the last command that was issued.
// Typically this command should be received after every command is sent to
// ensure that the previous command was successful or, if unsuccessful, to
// properly respond to a failure.  The command requires one byte in the data of
// the packet and the boot loader should respond by sending a packet with one
// byte of data that contains the current status code.
//
// The format of the command is as follows:
//
//     uint8_t ui8Command[1];
//
//     ui8Command[0] = COMMAND_GET_STATUS;
//
// The following are the definitions for the possible status values that can be
// returned from the boot loader when <tt>COMMAND_GET_STATUS</tt> is sent to
// the CC2538 device.
//
//     COMMAND_RET_SUCCESS
//     COMMAND_RET_UNKNOWN_CMD
//     COMMAND_RET_INVALID_CMD
//     COMMAND_RET_INVALID_ADD
//     COMMAND_RET_FLASH_FAIL
//
//*****************************************************************************
#define COMMAND_GET_STATUS      0x23

//*****************************************************************************
//
// This command should only follow a COMMAND_DOWNLOAD command
// or another COMMAND_SEND_DATA command, if more data
// is needed. Consecutive send data commands automatically increment
// the address and continue programming from the previous
// location. The transfer size is limited by the maximum size of
// a packet, which allows up to 252 data bytes to be transferred at a
// time. The command terminates programming once the number
// of bytes indicated by the COMMAND_DOWNLOAD command has
// been received. Each time this function is called, it should be
// followed by a COMMAND_GET_STATUS command to ensure that
// the data was successfully programmed into the flash. If the boot
// loader sends a NAK to this command, the boot loader will not increment
// the current address which allows for retransmission of
// the previous data.
//
// The format of the command is as follows:
//
//     uint8_t ui8Command[9];
//
//     ui8Command[0] = COMMAND_SEND_DATA
//     ui8Command[1] = Data[0];
//     ui8Command[2] = Data[1];
//     ui8Command[3] = Data[2];
//     ui8Command[4] = Data[3];
//     ui8Command[5] = Data[4];
//     ui8Command[6] = Data[5];
//     ui8Command[7] = Data[6];
//     ui8Command[8] = Data[7];
//
//*****************************************************************************
#define COMMAND_SEND_DATA       0x24

//*****************************************************************************
//
// This command is used to tell the boot loader to reset.  This is used after
// downloading a new image to the cc2538 device to cause the new application
// or the new boot loader to start from a reset.  The normal boot sequence
// occurs and the image runs as if from a hardware reset.  It can also be used
// to reset the boot loader if a critical error occurs and the host device
// wants to restart communication with the boot loader.
//
// The format of the command is as follows:
//
//     uint8_t ui8Command[1];
//
//     ui8Command[0] = COMMAND_RESET;
//
// The boot loader responds with an ACK signal to the host device before
// actually executing the software reset on the cc2538 device running the
// boot loader.  This informs the updater application that the command was
// received successfully and the part will be reset.
//
//*****************************************************************************
#define COMMAND_RESET           0x25

//*****************************************************************************
//
// This command will erase the required flash pages. A single flash
// page has the size of 2KB which is the minimum size that can be
// erased. The command consists of two 32-bit values that are both
// transferred MSB first. The first 32-bit value is the address in a
// flash page from where the erase starts, and the second 32-bits
// value is the number of bytes comprised by the erase. The Boot
// Loader responds with an ACK signal to the host device after the
// actual erase operation is performed. The page erase operation
// will always start erasing at the start address of the page that
// incorporates the input Data Address. The erase will end at the
// end of the page which is determined from the input parameters.
// On CC2538™ the flash starts at address 0x00200000.
//
// The format of the command is as follows:
//
//     uint8_t ui8Command[9];
//
//     ui8Command[0] = COMMAND_ERASE;
//     ui8Command[1] = Data Address [31:24];
//     ui8Command[2] = Data Address [23:16];
//     ui8Command[3] = Data Address [15:8];
//     ui8Command[4] = Data Address [7:0];
//     ui8Command[5] = Data Size [31:24];
//     ui8Command[6] = Data Size [23:16];
//     ui8Command[7] = Data Size [15:8];
//     ui8Command[8] = Data Size [7:0];
//
//*****************************************************************************
#define COMMAND_ERASE           0x26

//*****************************************************************************
//
// This command is used to check a flash area using CRC32. The command consists
// of two 32-bit values that are both transferred MSB first. The first 32-bit
// value is the address in flash from where the CRC32 calculation starts and
// the second 32-bits value is the number of bytes comprised by the CRC32
// calculation. The command will send the ACK in response to the command after
// the actual CRC32 calculation.
// The result is finally returned as 4 byte in a packet. The Boot Loader will
// then wait for an ACK from the host as a confirmation that the packet was 
// received.
//
// The format of the command is as follows:
//
//     uint8_t ui8Command[9];
//
//     ui8Command[0]= COMMAND_CRC32;
//     ui8Command[1]= Data Address [31:24];
//     ui8Command[2]= Data Address [23:16];
//     ui8Command[3]= Data Address [15: 8];
//     ui8Command[4]= Data Address [ 7: 0];
//     ui8Command[5]= Data Size [31:24];
//     ui8Command[6]= Data Size [23:16];
//     ui8Command[7]= Data Size [15: 8];
//     ui8Command[8]= Data Size [7: 0];w\end{verbatim}
//
//*****************************************************************************
#define COMMAND_CRC32           0x27

//*****************************************************************************
//
// This command will make the Boot Loader to return the value of the Chip ID.
// The Boot Loader should first respond by sending the ACK in response to the
// command and then send a packet with four bytes of data that contains the
// Chip ID value. The two LSB bytes hold the chip part number while the
// remaining bytes are reserved for future use and will have the value of 0x00.
// The Boot Loader will then wait for an ACK from the host as a confirmation
// that the packet was received.
// 
// The format of the command is as follows:
// 
//     uint8_t ui8Command[1];
//     ui8Command[0]= COMMAND_GET_CHIP_ID;
//
//*****************************************************************************
#define COMMAND_GET_CHIP_ID     0x28

//*****************************************************************************
//
// This command is used in to switch system clock source from internal 
// 16 MHz RC oscillator to external 32 MHz crystal oscillator.
// This can be used to increase transfer
// speed on the boot loader serial interfaces. The device will respond with an
// ACK on the command packet before the actual switch to the external 32 MHz
// crystal oscillator is done. 
// If the command is transferred on the UART interface the host must execute a
// UART baud rate auto-detection [3.2.3.1.1] in order to establish the baud
// rate to be used while the device is running from the external 32 MHz crystal
// oscillator. The device must be reset in order to switch back to running from
// the internal 16 MHz RC oscillator.
//
// The format of the command is as follows:
//
//     uint8_t ui8Command[1];
//
//     ui8Command[0]= COMMAND_SET_XOSC;
//
//*****************************************************************************
#define COMMAND_SET_XOSC        0x29

//*****************************************************************************
//
// his command used for reading either an 8-bit or 32-bit word at a specified 
// address within the device memory map.
// The command consists of a 32-bit value that is transferred MSB first 
// followed by an 8-bit value specifying the read access width. The 32-bit 
// value is the address within the device memory map to be read and the 8-bit 
// value specifies the access width of the read operation. Allowed values for 
// the access width are 1 and 4 (1 = 1 byte / 4 = 4 bytes).
// The command will send an ACK in response to the command after the specified
// data is read from memory. The read data is finally returned as 4 bytes 
// (MSB first) in a packet. The Boot Loader will then wait for an ACK from
// the host as a confirmation that the packet was received.
// For an access width value of 1 the read data will be located in the LSB of 
// the four bytes returned while the MSB bytes will have the value of 0x00.
// If an invalid access width is specified within the command the Boot Loader
// will ACK the command and respond with 4 bytes with value 0x00 in a packet. 
// An eventual following COMMAND_GET_STATUS command will in this case report 
// COMMAND_RET_INVALID_CMD.
//
// The format of the command is as follows:
//
//    uint8_t ui8Command[6];
// 
//     ui8Command[0]= COMMAND_MEMORY_READ;
//     ui8Command[1]= Data Address [31:24];
//     ui8Command[2]= Data Address [23:16];
//     ui8Command[3]= Data Address [15: 8];
//     ui8Command[4]= Data Address [ 7: 0];
//     ui8Command[5]= Read access width [7:0];
//
//*****************************************************************************
#define COMMAND_MEMORY_READ    0x2A

//*****************************************************************************
//
// This command is used for writing either an 8-bit or 32-bit word to a 
// specified address within the device memory map.
// The command consists of two 32-bit values that are transferred MSB first
// followed by an 8-bit value specifying the write access width. The first 
// 32-bit value is the address within the device memory map to be written.
// The second 32-bit value is the data to be written. The 8-bit value specifies
// the access width of the write operation. Allowed values for the access width
// are 1 and 4 (1 = 1 byte / 4 = 4 bytes).
// The command will send an ACK in response to the command after the specified
// data is written to memory. 
// For an access width value of 1 the data to be written must be located in the
// LSB of the four data bytes in the command.
// If an invalid access width is specified within the command the Boot Loader 
// will ACK the command but it will not perform any memory write operation. 
// An eventual following COMMAND_GET_STATUS command will in this case
// report COMMAND_RET_INVALID_CMD.
// 
// The format of the command is as follows:
// 
//     uint8_t ui8Command[10];
// 
//     ui8Command[0]= COMMAND_MEMORY_WRITE;
//     ui8Command[1]= Data Address [31:24];
//     ui8Command[2]= Data Address [23:16];
//     ui8Command[3]= Data Address [15: 8];
//     ui8Command[4]= Data Address [ 7: 0];
//     ui8Command[5]= Data [31:24]
//     ui8Command[6]= Data [23:16]
//     ui8Command[7]= Data [15: 8]
//     ui8Command[8]= Data [7: 0]
//     ui8Command[9]= Read access width [7:0];
//
//*****************************************************************************
#define COMMAND_MEMORY_WRITE   0x2B

//*****************************************************************************
//
// This is returned in response to a COMMAND_GET_STATUS command and indicates
// that the previous command completed successful.
//
//*****************************************************************************
#define COMMAND_RET_SUCCESS     0x40

//*****************************************************************************
//
// This is returned in response to a COMMAND_GET_STATUS command and indicates
// that the command sent was an unknown command.
//
//*****************************************************************************
#define COMMAND_RET_UNKNOWN_CMD 0x41

//*****************************************************************************
//
// This is returned in response to a COMMAND_GET_STATUS command and indicates
// that the previous command was formatted incorrectly.
//
//*****************************************************************************
#define COMMAND_RET_INVALID_CMD 0x42

//*****************************************************************************
//
// This is returned in response to a COMMAND_GET_STATUS command and indicates
// that the previous download command contained an invalid address value.
//
//*****************************************************************************
#define COMMAND_RET_INVALID_ADR 0x43

//*****************************************************************************
//
// This is returned in response to a COMMAND_GET_STATUS command and indicates
// that an attempt to program or erase the flash has failed.
//
//*****************************************************************************
#define COMMAND_RET_FLASH_FAIL  0x44

//*****************************************************************************
//
// This is the value that is sent to acknowledge a packet.
//
//*****************************************************************************
#define COMMAND_ACK             0xcc

//*****************************************************************************
//
// This is the value that is sent to not-acknowledge a packet.
//
//*****************************************************************************
#define COMMAND_NAK             0x33

#endif // __BL_COMMANDS_H__
