# Running OpenWSN on nRF52840

The nRF52840 port of OpenWSN uses SEGGER Embedded Studio as its IDE.
The current version doesn't support the L2_security yet.
To run an OpenWSN network, cjoin needs to be disabled by commenting out the 

        void openapps_init(void) {

           ...
           ...
           
           // cjoin_init();

           ...
           ...
        }
        
in [openapp.c](https://github.com/openwsn-berkeley/openwsn-fw/blob/develop/openapps/openapps.c#L40) file.