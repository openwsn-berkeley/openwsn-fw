/**
  \brief Crypto driver initialization
  
  \author Marcelo Barros de Almeida <marcelobarrosalmeida@gmail.com>
 */
#include <stdint.h>
#include <string.h>
#include "crypto_driver.h"
#include "crypto_driver_board.h"

static crypto_driver_t crypto_driver;

static int translate_addr(open_addr_t *saddr, uint8_t _saddr[8])
{
   int n;
   int addr_size;

   // copy source addr
   switch (saddr->type) {
      case ADDR_16B:
      case ADDR_PANID:
         addr_size = 2;
         break;
      case ADDR_64B:
      case ADDR_PREFIX:
         addr_size = 8;
         break;
      case ADDR_128B:
         addr_size = 16;
         break;
      default:
         addr_size = -1;
         break;
   }

   if (addr_size == -1)
      return -1;

   if (addr_size > 8)
      addr_size = 8;

   for (n = 0; n < addr_size; n++)
      _saddr[n] = saddr->addr_128b[n];

   if (addr_size < 8)
      memset(&_saddr[addr_size], 0, 8 - addr_size);

   return 0;
}

static int translate_asn(asn_t *asn, uint8_t _asn[5])
{
   _asn[0] = (uint8_t) (asn->bytes0and1 >> 8);
   _asn[1] = (uint8_t) asn->bytes0and1;
   _asn[2] = (uint8_t) (asn->bytes2and3 >> 8);
   _asn[3] = (uint8_t) asn->bytes2and3;
   _asn[4] = (uint8_t) asn->byte4;

   return 0;
}

int crypto_driver_init(void)
{
   crypto_driver_board_init(&crypto_driver);

   return 0;
}

const crypto_driver_t* crypto_driver_get(void)
{
   return &crypto_driver;
}
