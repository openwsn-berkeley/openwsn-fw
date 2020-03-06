# OpenMoteb SubGHz slot configuration

For this realease to work, you need to configure the ATMEL radio to use 2 byte CRC:
* Go to bsp/chips/at86rf215/radio.h then make sure LENGTH_CRC is configured to 2


To speed up the synchronization for this release (for testing and debugging purposes), make sure to apply the following modifications:

Configuration | Location
------------ | -------------
fix_channel | add scons compile option fix_channel = 11 (for example)
EB_PORTION 4 | openstack\02a-MAClow\IEEE802154E.h
SLOTFRAME_LENGTH 11 | openstack\02b-MAChigh\schedule.h
