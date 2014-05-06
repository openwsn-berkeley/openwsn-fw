#include "spi.h"
#include "radio.h"
#include "at86rf231.h"
#include "configureRadio.h"

void radio_configure()
{   
    spi_init();
    radio_init();
}