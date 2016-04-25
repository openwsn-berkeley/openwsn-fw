/**
 * \brief Declaration of the "openmemory" interface.
 *
 * Implements a very simple memory manager.
 * How it works:
 * Memory is segmented in blocks of the FRAME_DATA_TOTAL (130B) size.
 * It uses two arrays to manage how memory is used
 *  - buffer, the memory array
 *  - map, an occupation array
 * The map array contains zero if segment is not assigned or a positive
 * value. This number indicates how many segments are asigned, including
 * itself, to start. Example: next is a 10 segments map; starting addresses
 * correspond to: 0, 130, 260, 390, 520, 650, 780, 910, 1040 and 1170
 * +---+---+---+---+---+---+---+---+---+---+
 * | 0 | 0 | 0 | 0 | 0 | 3 | 1 | 0 | 1 | 1 |
 * +---+---+---+---+---+---+---+---+---+---+
 *   0   1   2   3   4   5   6   7   8   9
 * - Position 9 contains 1: memory area from byte 1170 is reserved
 * - Position 8 contains 1: reserved from byte 1040 to 1169
 * - Position 7 contains 0: free     from byte  910 to 1039
 * - Position 6 contains 1: reserved from byte  780 to  909
 * - Position 5 contains 3: reserved from byte  390 to  779
 * - Segments 0, 1 and 2 are free:   from byte    0 to  389
 *
 * It does not include control mechanisms for overlapping, fragmentation
 * or other possible errors.
 *
 * \author Antonio Cepero <cpro@uoc.edu>, March 2016.
 */

#include "opendefs.h"
#include "openmemory.h"

//=========================== variables =======================================

openmemory_vars_t openmemory_vars;

//=========================== prototypes ======================================

bool openmemory_segmentAddr(uint8_t* address, uint8_t** first, uint8_t** last);

//=========================== public ==========================================

//======= admin

/**
\brief Initialize this module.
*/
void openmemory_init() {
   uint8_t i;

   for ( i = 0; i < FRAME_DATA_SEGMENTS; i++ ) {
      openmemory_vars.memory.map[i] = 0;
   }
//   memset(&openmemory_vars.memory.buffer[0], 0, REAL_MEMORY_SIZE);
//   openmemory_vars.used = 0;
}

/**
 * \brief Reserve a memory area to allocate the required size.
 *
 * \param size The amount of bytes that are required to allocate.
 *
 * \note  It reserves as many segments as needed for the amount of bytes
 *        in "size" and returns the address of the first segment.
 *        If "size" is a multiple of FRAME_DATA_TOTAL, an excess segment
 *        is acquired.
 *
 * \returns A pointer to the start of a memory area, if successful.
 * \returns NULL when reservation was unsuccessful.
 */
uint8_t*  openmemory_getMemory(uint16_t size)
{
   uint8_t nsegments;
   uint8_t i;
   uint8_t j;

   nsegments = size / FRAME_DATA_TOTAL + 1;

   for ( i = FRAME_DATA_SEGMENTS - 1; i >= 0; ) {
   // search for free space
      if ( openmemory_vars.memory.map[i] == 0 ) {
	 j = 0;
	 // check if there exist enough segments
         while ( nsegments > j && i-j >= 0
              && openmemory_vars.memory.map[i-j] == 0 )
            j++;
	 if ( j == nsegments ) {
            openmemory_vars.memory.map[i] = nsegments;
            openmemory_vars.used += nsegments;
	    return &openmemory_vars.memory.buffer[(i-j+1) * FRAME_DATA_TOTAL];
	 } else
            i -= j; // advance to next occupied segment
      } else { // go to next segment
         i -= openmemory_vars.memory.map[i];
      }
   }

   // There is no available segment
   return NULL;
}

/**
 * \brief Free a previously-allocated memory area.
 *
 * \param address A pointer to an address included in the previsouly-allocated
 *                memory area.
 *
 * \returns E_SUCCESS when the freeing was successful.
 * \returns E_FAIL    when the freeing was unsuccessful.
 */
owerror_t openmemory_freeMemory(uint8_t* address)
{
   uint8_t  end;
   uint8_t* first;
   uint8_t* last;

   if ( ! openmemory_segmentAddr(address, &first, &last) ) {
      return E_FAIL;
   }

   end = (last - &openmemory_vars.memory.buffer[0]) / FRAME_DATA_TOTAL;
   openmemory_vars.used -= openmemory_vars.memory.map[end];
   openmemory_vars.memory.map[end] = 0;
//   memset(first, 0, (size_t)(last-first)+1);
   return E_SUCCESS;
}

/**
 * \brief Increase a reserved memory area, maintaining its contents
 *
 * \param address A pointer to an address included in a previously-allocated
 *                memory area.
 * \param size    The new (total) size in bytes.
 *
 * \note  It tries to get the adjacent previous segments or reserve a new
 *        area. If a new area is needed it copies contents from "address"
 *        to the end of it.
 *
 * \returns A pointer to the same address in the (newly?) allocated memory
 *          area, if successful.
 * \returns NULL when reservation was unsuccessful.
 */
uint8_t* openmemory_increaseMemory(uint8_t* address, uint16_t size)
{
   uint8_t  j;
   uint8_t  end;
   uint8_t  old_segments;
   uint8_t  new_segments;
   uint8_t* first;
   uint8_t* last;
   uint8_t* new;

   if ( ! openmemory_segmentAddr(address, &first, &last) ) {
      return NULL;
   }

   end  = (last - &openmemory_vars.memory.buffer[0]) / FRAME_DATA_TOTAL;
   old_segments = openmemory_vars.memory.map[end];
   new_segments = size / FRAME_DATA_TOTAL + 1;

   if ( old_segments == new_segments ) {
      return address;
   }

   // Try to allocate it in previous segments
   j = old_segments;
   while ( new_segments > j && end - j >= 0
        && openmemory_vars.memory.map[end-j] == 0 ) {
      j++;
   }
   if ( j == new_segments ) {
      openmemory_vars.used += new_segments - old_segments;
      openmemory_vars.memory.map[end] = new_segments;
      return address;
   }

   // There is no adjacent space to allocate it in contiguous form
   new = openmemory_getMemory(size);
   if ( new != NULL ) {
      uint16_t nsize; // bytes to copy minus 1
      uint8_t* aux;
      uint8_t* newlast;

      nsize = (uint16_t)(last - address);
      openmemory_segmentAddr(new, &aux, &newlast);
      new = newlast - nsize;
      memcpy(new, address, nsize+1);
      openmemory_vars.used += new_segments - old_segments;
      openmemory_vars.memory.map[end] = 0; // freeing old area
//      memset(first, 0, (size_t)(last-first)+1);
   }

   return new;
}

/**
 * \brief Returns the initial address of the first segment of a memory area
 *
 * \param address A pointer to an address included in the previsouly-allocated
 *                memory area.
 *
 * \returns A pointer to the start of a memory area, if successful.
 * \returns NULL when memory does not belong to a previsouly-allocated
 *               memory area
 */
uint8_t*  openmemory_firstSegmentAddr(uint8_t* address)
{
   uint8_t* first;
   uint8_t* last;

   if ( ! openmemory_segmentAddr(address, &first, &last) ) {
      return NULL;
   }

   return first;
}

/**
 * \brief Returns the initial address of the last segment of a memory area
 *
 * \param address A pointer to an address included in the previsouly-allocated
 *                memory area.
 *
 * \returns A pointer to the start of a memory area, if successful.
 * \returns NULL when memory does not belong to a previsouly-allocated
 *               memory area
 */
uint8_t*  openmemory_lastSegmentAddr(uint8_t* address)
{
   uint8_t* first;
   uint8_t* last;

   if ( ! openmemory_segmentAddr(address, &first, &last) ) {
      return NULL;
   }

   return last - FRAME_DATA_TOTAL + 1;
}

/**
 * \brief Returns were the two address belong to the same reservation
 *
 * \param addr1 A pointer to an address included in a previsouly-allocated
 *              memory area.
 * \param addr2 A pointer to an other address.
 *
 * \returns TRUE  When both addresses belong to the same previsouly-allocated
 *                memory area.
 * \returns FALSE When not.
 */
bool openmemory_sameMemoryArea(uint8_t* addr1, uint8_t* addr2)
{
   uint8_t* first;
   uint8_t* last;

   if ( addr1 == NULL || addr2 == NULL ) {
      return FALSE;
   }

   if ( ! openmemory_segmentAddr(addr1, &first, &last) ) {
      return FALSE;
   }

   if ( first <= addr2 && addr2 <= last ) {
      return TRUE;
   }

   return FALSE;
}

//=========================== private =========================================

/**
 * \brief Returns the first and last address of a memory area
 *
 * \param address A pointer to an address included in a previsouly-allocated
 *                memory area.
 * \param first   OUTPUT Address of the first position
 * \param last    OUTPUT Address of the last position
 *
 * \returns TRUE  When address belongs to a previsouly-allocated memory area.
 * \returns FALSE When not.
 */
bool openmemory_segmentAddr(uint8_t* address, uint8_t** first, uint8_t** last)
{ 
   uint8_t  i;
   uint8_t  j;
   uint8_t  start;
   uint8_t* init;
   uint8_t* end;

   start = (address - &openmemory_vars.memory.buffer[0]) / FRAME_DATA_TOTAL;
   for ( i = start; i < FRAME_DATA_SEGMENTS
                 && openmemory_vars.memory.map[i] == 0; i++ )
      ;
   if ( i >= FRAME_DATA_SEGMENTS ) {
      return FALSE;
   } else if ( start < i ) {
      // It is memory overlapping?
      for ( j = i - openmemory_vars.memory.map[i] + 1; j < start; j++ )
         if ( j < i && openmemory_vars.memory.map[j] != 0 ) {
            openserial_printCritical(COMPONENT_OPENMEMORY,
			    ERR_MEMORY_OVERLAPS,
			    (errorparameter_t)0,
			    (errorparameter_t)0);
            return FALSE;
	 }
   }
   start = i - openmemory_vars.memory.map[i] + 1;
   init  = &openmemory_vars.memory.buffer[start * FRAME_DATA_TOTAL];
   end   = &openmemory_vars.memory.buffer[i * FRAME_DATA_TOTAL];
   end  += FRAME_DATA_TOTAL - 1;
   
   if ( address >= init && address <= end) {
      *first = init;
      *last  = end;
      return TRUE;
   }

   return FALSE;
}
