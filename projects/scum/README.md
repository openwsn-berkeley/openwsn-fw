Run 6TiSCH on SCuM
===============================================================================

As the stage of SCuM development (SCuM3C),  several adaptation need to be made to run 6TiSCH on SCuM.

Changes to make
-------------------------------------------------------------------------------

1. CoAP/App
    no CoAP or any application running, this is because of the limited 64Kb RAM 
of SCuM3C. 

    It's applied in the port of SCuM, so no changes required.

2. Schedule
T   o get SCuM synchronize all the time, the slotframe length should be set to 11
in schedule.h file, such as:

        #define SLOTFRAME_LENGTH    11 //should be 101
        #define NUMSLOTSOFF          5

3. EB interval
    To get tight synchronization, EB sending rate need to be increase by following changes in IEEE802154e.h file: 

        #define EB_PORTION                   2 // set EB on minimal cell for 1/EB_PORTION portion
        #define MAXKAPERIOD                428 // in slots: 1500@20ms per slot -> ~30 seconds. Max value used by adaptive synchronization.
        #define DESYNCTIMEOUT              500 // in slots: 1750@20ms per slot -> ~35 seconds. A larger DESYNCTIMEOUT is needed if using a larger KATIMEOUT.
    
4. calibrate the channel frequency

    The frequency needs to be pre-calibrate before running 6TiSCH on SCuM. You will need to replace the update the frequency settings in radio.c file using a 15-bit integer: e.g. 24900 (coarse 24 mid 10 fine 4). Try to use just one channel and disable channel hopping by change radio_setFrequency function in radio.c file by: 

        radio_vars.current_frequency = DEFAULT_FREQ;
    
    Do the same thing on OpenMote side as well to communicate with OpenMote
    
5. Xon/Xoff adaptation

    SCuM may need more time to support the xon/xoff feature with following changes (in IEEE802154e.h)
    
    #define DURATION_si  ieee154e_vars.slotDuration-3*SERIALINHIBITGUARD


Test
-------------------------------------------------------------------------------
With those changes, you should be able to have one OpenMote acting as dagroot (set through OV), one SCuM acting as a child of dagroot. They are communicating on channel 11 with 6TiSCH protocol. When receiving the DAO from SCuM, you should be able to ping SCuM.
