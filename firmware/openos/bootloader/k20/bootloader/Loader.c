/**
\brief The USB Loader manager
 Based on Freescale k60 loader demo. 

\author Xavi Vilajosana <xvilajosana@eecs.berkeley.edu>, June 2012.
 */


#include <string.h>
#include <stdlib.h>         /* ANSI memory controls */
#include "stdio.h"
#include "types.h"          /* User Defined Data Types */
#include "Bootloader.h"
#include "derivative.h"
#if (defined MCU_MK60N512VMD100) |(defined MCU_MK20D7)
#include "flash_FTFL.h"
#endif
#include "hidef.h"

/*****************************************************************************
 * Function predefinitions.
 *****************************************************************************/
 uint_8           FlashApplication(uint_8* arr,uint_32 length);
 static uint_8    FlashArrayS19   (uint_8* Array,uint_32 size_of_array,uint_8 *Line);
 static uint_8    FlashLineS19    (uint_8 *Line);
 static uint_8    FlashArrayCW    (uint_8 *Array,uint_32 size_of_array,uint_8 *Line);
 static uint_8    FlashLineCW     (uint_8 *Line);
 static uint_8    GetHexValue     (uint_8 text);
 static uint_8    GetSpair        (uint_8 *arr,uint_8 point);
 static uint_8    CheckAddressValid(uint_32 Address);
 static uint_32   get_uint32      (uint_8* arr, uint_32 index);
 /*****************************************************************************
 * Global variables.
 *****************************************************************************/
 
uint_8 S19FileDone;         /* parsing S19 flag */
uint_32 S19Address;         /* address to flash */
extern uint_8 BootloaderStatus;    /* status of loading process */
uint_8 filetype;            /* type of image file */
uint_8 line[260];           /* line buffer */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : FlashApplication
* Returned Value : 0 if successful, other value if error
* Comments       : parse and flash an array to flash memory
*     
*END*--------------------------------------------------------------------*/
uint_8 FlashApplication
    (
        /* [IN] the array to parse */
        uint_8*     arr,
        /* [IN] data length of the array */
        uint_32     length
    )
{
    /* Body */
    uint_8 result;
    uint_32 write_addr;             /* address to write to flash */
    static  uint_32 bytes_written;  /* number of bytes was written to flash */
    uint_32 header;
    header = get_uint32(arr,0);


    /* Get image type */
    if(filetype == UNKNOWN)
    {
        bytes_written = 0;
        /*  first four bytes is SP */
        if( (MIN_RAM1_ADDRESS <=header )&& (header<= MAX_RAM1_ADDRESS))
        {
            filetype = RAW_BINARY;
        }
        /*  first four bytes is user application address */
        else 
        {
			#ifdef LITTLE_ENDIAN
				header = BYTESWAP32(header);
			#endif
            if( (MIN_FLASH1_ADDRESS <=header )&& (header<= MAX_FLASH1_ADDRESS))
            {
                filetype = CODE_WARRIOR_BINARY;
            }
            /*  first four bytes is S-Record header */
            else 
            {
                if(header ==(uint_32)(S19_RECORD_HEADER))
                {
                    filetype = S19_RECORD;
                }
                else
                {
#if (!defined __MK_xxx_H__)
                asm("halt");
#endif
                } /* EndIf */
            } /* EndIf */
        } /* EndIf */
    } /* EndIf */

    /* Flash image */
    switch (filetype)
    {
        case RAW_BINARY: 
            /* Raw binary file found*/
            /* the address to write to flash */
            write_addr =(uint_32) IMAGE_ADDR + bytes_written;
            /* if flash error , break the loop */
            DisableInterrupts;
#if (!defined __MK_xxx_H__)
            result = Flash_ByteProgram((uint_32*)(write_addr),(uint_32*)arr,length); 
#else
            result = Flash_ByteProgram(write_addr,(uint_32*)arr,length); 
#endif
            EnableInterrupts;
            bytes_written += length;
            break;
        case CODE_WARRIOR_BINARY:
            /* CodeWarrior binary file found */
            result = FlashArrayCW(arr,length,line);  /* DES parse and flash array */
            break;
        case S19_RECORD:
            /* S19 file found */
            result = FlashArrayS19(arr,length,line);    /* DES parse and flash array */
            break;
    } /* EndSwitch */
    /* DES Should add programming verification code ...minimal code
    to see if "#Flash Programming Signature" present from linker script */
    return result;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : FlashArrayS19
* Returned Value : 0 if successful, other value if error
* Comments       : Get full lines from an array and flash them
*     
*END*--------------------------------------------------------------------*/
static uint_8 FlashArrayS19
    (   
        /* [IN] the array to parse */
        uint_8 *Array,
        /* [IN] data length of the array */
        uint_32 size_of_array,
        /* [IN] a allocated buffer */
        uint_8 *Line
    )
{
    /* Body */
    uint_16 i,j;
    uint_8 c,result;
    static uint_8 t=0;
    static uint_8 curL, totalL=6; /* current Length, total Length */
    static uint_8 newline = 0;

    for (i=0 ; i<size_of_array;i++)
    {
        c = Array[i];
        if(curL == 4) 
        {
            /* Get total length of current line */
            t = GetSpair(Line,2);
            totalL = (uint_8)((t+2)*2);
        } /* EndIf */
        if (curL>=totalL)                   /* Get full line */
        {  
            result = FlashLineS19(Line);    /* Parse and flash current line */
            for (j=0 ; j<totalL;j++)        /* Reset line and other variables */
            {
                Line[j]=0xFF;
            } /* EndFor */
            curL=0;
            totalL=6;
            newline=0;
        } /* EndIf */
        if (newline==0)                     /* Check for newline */
        {
            if (c=='S') 
            {
                newline=1;
                Line[curL]=c;
                curL++;
            } /* EndIf */
        }
        else
        {
            Line[curL] = c;
            curL++;
        } /* EndIf */
    } /* EndFor */
    return result;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : FlashLineS19
* Returned Value : 0 if successful, other value if error
* Comments       : Parse and flash a full line
*     
*END*--------------------------------------------------------------------*/
static uint_8 FlashLineS19
    (
        /* [IN] the Line to parse */
        uint_8 *Line
    )
{
    /* Body */
    static uint_8 length;
    static uint_8 checksum;
    static uint_8 i, offset,temp,c_temp,j;
    static uint_8 type;
    static uint_8 data;
    static uint_8 cur_point; /* current pointer */
    uint_8 buffer_to_flash[256];

    c_temp=Line[0];
    if (c_temp!='S')
    {
        BootloaderStatus = BootloaderS19Error;
        return FLASH_IMAGE_ERROR;
    } /* EndIf */
    /* Get record length */
    cur_point = 2;
    length = GetSpair(Line,cur_point);
    if(S19FileDone) 
    {
        /* not a valid S19 file */
        BootloaderStatus = BootloaderS19Error;
        return FLASH_IMAGE_ERROR;
    } /* EndIf */
    cur_point--;
    checksum = length;
    type=Line[1];

     /* Take appropriate action */
     switch (type)
        {
            case '1':
            case '2':
            case '3':
                S19Address = (uint_32) NULL;
                type -= '0';
                cur_point=4;
                for (i = 0; i <= type ; i++)
                {
                    /* Formulate address */
                    /* Address needs to be word aligned for successful flash program */
                    data = GetSpair(Line,cur_point);
                    if(S19FileDone) 
                    {       
                        /* not a valid S19 file */
                        BootloaderStatus = BootloaderS19Error;
                        return FLASH_IMAGE_ERROR;
                    } /* EndIf */
                    S19Address = (S19Address << 8) | data;
                    /* Maintain 8-bit checksum */
                    checksum = (unsigned char)((data + checksum) & 0x00FF);
                    cur_point+=2;
                } /* EndFor */

                if (CheckAddressValid(S19Address))
                {
                    /* 32-bit cores program flash in 32-bit words */
                    /* Therefore S19 address needs to be adjusted to be word aligned */
                    /* Pad beginning of buffer if address not word aligned */
                    offset = (uint_8) (S19Address & 0x0003);
                    S19Address = (uint_32) (S19Address & 0xFFFFFFFC);
                    length += offset;
                    for (i = 0; i < offset; i++) 
                    {
                        buffer_to_flash[i] = 0xFF; 
                    } /* EndFor */
                    /* Get data and put into buffer */
                    for (i = offset; i < (length - 5); i++)
                    {
                        data=GetSpair(Line,cur_point);
                        buffer_to_flash[i] =data;
                        cur_point+=2;
                        if(S19FileDone) 
                        {
                            /* not a valid S19 file */
                            BootloaderStatus = BootloaderS19Error;
                            return FLASH_IMAGE_ERROR;
                        } /* EndIf */
                    } /* EndFor */

                    /* Calculate checksum */
                    for (i = offset; i < (length - 5); i++)
                    {
                        checksum = (unsigned char)((buffer_to_flash[i] + checksum) & 0x00FF);
                    } /* EndFor */
                    /* Get checksum byte */
                    data = GetSpair(Line,cur_point);
                    cur_point+=2;
                    if(S19FileDone) 
                    {
                        /* not a valid S19 file */
                        BootloaderStatus = BootloaderS19Error;
                        return FLASH_IMAGE_ERROR;
                    } /* EndIf */

                    if (((data - ~checksum) & 0x00FF) != 0)
                    {
                        BootloaderStatus = BootloaderS19Error;
                        S19FileDone = TRUE;
                        return FLASH_IMAGE_ERROR;
                    } /* EndIf */
                    /* For 32-bit cores Flash_Prog writes 32-bit words, not bytes */
                    /* if last 32-bit word in s-record is not complete, finish word */
                    if((i & 0x0003) != 0x0000) 
                    {
                        /* 32-bit word not complete */
                        buffer_to_flash[i++] = 0xFF;         /* pad end of word */
                        buffer_to_flash[i++] = 0xFF;         /* pad end of word */
                        buffer_to_flash[i++] = 0xFF;         /* pad end of word */
                    } /* EndIf */

                    /* NOTE: 8-bit core does not need to pad the end */
                    /* Write buffered data to Flash */
                    if((S19Address >= (uint_32)FLASH_PROTECTED_ADDRESS) && (S19Address <= MAX_FLASH1_ADDRESS)) 
                    {
                        /* call flash program */
                        DisableInterrupts;
#if (!defined __MK_xxx_H__)
                        temp =  Flash_ByteProgram((uint_32*)S19Address,(uint_32*)buffer_to_flash,i);
#else
                        temp =  Flash_ByteProgram(S19Address,(uint_32*)buffer_to_flash,i);
#endif
                        EnableInterrupts;
                        
                        if(Flash_OK  != temp)
                        {
                            BootloaderStatus = BootloaderFlashError;
                            return FLASH_IMAGE_ERROR;
                        } /* EndIf */
                    } /* EndIf */
                }
                else    /* S-Record points to invalid address */
                {
                    BootloaderStatus = BootloaderS19Error;
                    return FLASH_IMAGE_ERROR;
                } /* EndIf */
                break;
            case '7':
            case '8':
            case '9':
                S19Address = (uint_32) NULL; 
                type = (unsigned char)(type - '0');
                type = (unsigned char)(10 - type);
                cur_point=4;
                checksum = length;
                /* Get Address */
                for (i = 0; i <= type ; i++)
                {
                    for(j=0;j<length-1;j++)
                    {
                        data = GetSpair(Line,cur_point);
                        if(S19FileDone) 
                        {
                            /* not a valid S19 file */
                            BootloaderStatus = BootloaderS19Error;
                            return FLASH_IMAGE_ERROR;
                        } /* EndIf */
                        checksum = (unsigned char)((data + checksum) & 0x00FF);
                        cur_point+=2;
                    } /* EndFor */

                    /* Read checksum value */
                    data=GetSpair(Line,cur_point);
                    if(S19FileDone)
                    {
                        /* not a valid S19 file */
                        BootloaderStatus = BootloaderS19Error;
                        return FLASH_IMAGE_ERROR;
                    } /* EndIf */
                    
                    /* Check checksum */
                    if (((data - ~checksum) & 0x00FF) != 0)
                    {
                        BootloaderStatus = BootloaderS19Error;
                        S19FileDone = TRUE;
                        return FLASH_IMAGE_ERROR;
                    }
                    else 
                    {
                        /* File completely read successfully */
                        BootloaderStatus = BootloaderSuccess;
                        S19FileDone = TRUE;
                        return FLASH_IMAGE_SUCCESS;
                    } /* EndIf */
                } /* EndFor */
                break;
            case '0':
            case '4':
            case '5':
            case '6':
            default:
                break;
        } /* EndSwitch */
    return FLASH_IMAGE_SUCCESS;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : GetHexValue
* Returned Value : unsigned char, hex value of character
* Comments       : Converts ASCII character to hex value 
*     
*END*--------------------------------------------------------------------*/
static uint_8 GetHexValue 
    (
        /* [IN] the text to parse */
        uint_8 text
    )
{
    /* Body */
    switch (text)
    {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return (uint_8)(text - '0');
        case 'A':
        case 'a':
            return 10;
        case 'B':
        case 'b':
            return 11;
        case 'C':
        case 'c':
            return 12;
        case 'D':
        case 'd':
            return 13;
        case 'E':
        case 'e':
            return 14;
        case 'F':
        case 'f':
            return 15;
        default:
            return 0xFF;
    } /* EndSwitch */
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : GetSpair
* Returned Value : unsigned char, converted hex byte
* Comments       : Gets pair of characters, and converts to hex byte
*     
*END*--------------------------------------------------------------------*/
static uint_8 GetSpair
    (
        /* [IN] the array to parse */
        uint_8 *arr,
        /* [IN] point to array data */
        uint_8 point
    )
{
    /* Body */
    uint_8 ch;
    uint_8 upper,lower;
    ch = arr[point];
    upper = (uint_8)(GetHexValue(ch));
    if(upper == 0xFF) 
    {
        /* Not a proper S19 file */
        S19FileDone = TRUE;
    } /* EndIf */
    upper = (uint_8)(upper << 4);
    ch=arr[point+1];
    lower= (uint_8)(GetHexValue(ch));
    if(lower == 0xFF) 
    {
        /* Not a proper S19 file */
        S19FileDone = TRUE;
    } /* EndIf */
    return (uint_8)(upper | lower);
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : CheckAddressValid
* Returned Value : unsigned char, TRUE or FALSE if valid
* Comments       : Checks if Address of S-Record is valid for device
*     
*END*--------------------------------------------------------------------*/
static uint_8 CheckAddressValid
    (
        /* [IN] the address to check */
        uint_32 Address
    )
{
    /* Body */
    if((Address >= MIN_FLASH1_ADDRESS) && (Address <= MAX_FLASH1_ADDRESS))
        return TRUE;
    else if ((Address >= MIN_RAM1_ADDRESS) && (Address <= MAX_RAM1_ADDRESS))
        return TRUE;
    else
        return FALSE;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : FlashArrayCW
* Returned Value : 0 if successful, other value if error
* Comments       : Get full lines from an array and flash them
*     
*END*--------------------------------------------------------------------*/
static uint_8 FlashArrayCW
    (
        /* [IN] the array to parse */
        uint_8 *Array,
        /* [IN] data length of the array */
        uint_32 size_of_array,
        /* [IN] a allocated buffer */
        uint_8 *Line
    )
{
    /* Body */
    uint_8 result;
    uint_16 i,j;
    uint_32 data_length;
    uint_8 c;
    static uint_32 curL,totalL;             /* current Length , taltal Length */
    static uint_8 newline = 0;

    for (i = 0 ; i<size_of_array;i++)
    {
        c = Array[i];
        Line[curL] = c;
        curL++;
        if(curL == 8) 
        {
            data_length = get_uint32(Line,4);
            /* Get total length of current line */
            totalL = data_length + 8;
            /* start a new line with address and data length */
            newline = 1;
        } /* EndIf */
        if(newline)
        {
            if (curL>=totalL)                   /* Get full line */
            {
                /* Parse and flash current line */
                result = FlashLineCW(Line);
                /* Reset line and other variables */
                for (j=0 ; j<totalL;j++)
                {
                    Line[j]=0xFF;
                } /* EndFor */
                curL=0;
                totalL=0;
                newline=0;
            } /* EndIf */
        } /* EndIf */
    } /* EndFor */
    return result;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : FlashLineCW
* Returned Value : 0 if successful, other value if error
* Comments       : Parse and flash a full line
*     
*END*--------------------------------------------------------------------*/
static uint_8 FlashLineCW
    (
        /* [IN] the Line to parse */
        uint_8 *Line
    )
{
    /* Body */
    uint_8  result = FLASH_IMAGE_SUCCESS;
    uint_32 write_addr;
    uint_32 data_length;
    write_addr = get_uint32(Line,0);    /* address to flash */
    data_length = get_uint32(Line,4);   /* length of data */
    if((write_addr >= (uint_32)FLASH_PROTECTED_ADDRESS) && (write_addr <= MAX_FLASH1_ADDRESS)) 
    {
        DisableInterrupts;
#if (!defined __MK_xxx_H__)
        if (Flash_ByteProgram((uint_32*)(write_addr),(uint_32 *)(Line + 8),data_length))
            result = FLASH_IMAGE_ERROR;
#else
        if (Flash_ByteProgram(write_addr,(uint_32 *)(Line + 8),data_length))
            result = FLASH_IMAGE_ERROR;
#endif
        EnableInterrupts;
    } /* EndIf */
    return result;
} /* EndBody */

/*FUNCTION*----------------------------------------------------------------
*
* Function Name  : get_uint32
* Returned Value : result
* Comments       : get a unsign long number from an array
*
*END*--------------------------------------------------------------------*/
static uint_32 get_uint32 
    (
        /* [IN] the array */
        uint_8* arr,
        /* [IN] the index of array */
        uint_32 index
    )
{
    uint_32 result;
    result = *(uint_32*)(arr + index);
    return result ;
} /* EndBody */

/* EOF */
