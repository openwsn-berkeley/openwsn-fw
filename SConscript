import os
import threading
import subprocess
import distutils.sysconfig
import re
import datetime

Import('env')

# directory where we put object and linked files
# WARNING: -c (clean) removes the VARDIR, so it cannot be blank
env['VARDIR']  = os.path.join('#','build','{0}_{1}'.format(env['board'],env['toolchain']))

# common include paths
if env['board']!='python':
    env.Append(
        CPPPATH = [
            os.path.join('#','firmware','openos','openwsn'),
            os.path.join('#','firmware','openos','bsp','boards'),
            os.path.join('#','firmware','openos','bsp','chips'),
            os.path.join('#','firmware','openos','kernel','openos'),
            os.path.join('#','firmware','openos','drivers','common'),
        ]
    )

#============================ toolchain =======================================

# dummy
dummyFunc = Builder(
    action = '',
    suffix = '.ihex',
)

if   env['toolchain']=='mspgcc':
    if env['board'] not in ['telosb','gina','z1']:
        raise SystemError('toolchain {0} can not be used for board {1}'.format(env['toolchain'],env['board']))
    
    # compiler
    env.Replace(CC           = 'msp430-gcc')
    env.Append(CCFLAGS       = '')
    # archiver
    env.Replace(AR           = 'msp430-ar')
    env.Append(ARFLAGS       = '')
    env.Replace(RANLIB       = 'msp430-ranlib')
    env.Append(RANLIBFLAGS   = '')
    # linker
    env.Replace(LINK         = 'msp430-gcc')
    env.Append(LINKFLAGS     = '')
    
    # convert ELF to iHex
    elf2iHexFunc = Builder(
       action = 'msp430-objcopy --output-target=ihex $SOURCE $TARGET',
       suffix = '.ihex',
    )
    env.Append(BUILDERS = {'Elf2iHex' : elf2iHexFunc})
    
    # convert ELF to bin
    elf2BinFunc = Builder(
       action = 'msp430-objcopy --output-target=binary $SOURCE $TARGET',
       suffix = '.bin',
    )
    env.Append(BUILDERS = {'Elf2iBin' : elf2BinFunc})
    
    # print sizes
    printSizeFunc = Builder(
        action = 'msp430-size $SOURCE',
        suffix = '.phonysize',
    )
    env.Append(BUILDERS = {'PrintSize' : printSizeFunc})

elif env['toolchain']=='iar':
    if env['board'] not in ['telosb','gina','z1']:
        raise SystemError('toolchain {0} can not be used for board {1}'.format(env['toolchain'],env['board']))
    
    try:
        iarEw430BinDir          = os.path.join(os.environ['IAR_EW430_INSTALLDIR'],'430','bin')
    except KeyError as err:
        print 'You need to install environment variable IAR_EW430_INSTALLDIR which points to the installation directory of IAR Embedded Workbench for MSP430. Example: C:\Program Files\IAR Systems\Embedded Workbench 6.4'
        raise
    
    # compiler
    env.Replace(CC                = '"'+os.path.join(iarEw430BinDir,'icc430')+'"')
    env.Append(CCFLAGS            = '--no_cse')
    env.Append(CCFLAGS            = '--no_unroll')
    env.Append(CCFLAGS            = '--no_inline')
    env.Append(CCFLAGS            = '--no_code_motion')
    env.Append(CCFLAGS            = '--no_tbaa')
    env.Append(CCFLAGS            = '--debug')
    env.Append(CCFLAGS            = '-e')
    env.Append(CCFLAGS            = '--double=32 ')
    env.Append(CCFLAGS            = '--dlib_config "C:\\Program Files\\IAR Systems\\Embedded Workbench 6.4\\430\\LIB\\DLIB\\dl430fn.h"')
    env.Append(CCFLAGS            = '--library_module')
    env.Append(CCFLAGS            = '-Ol ')
    env.Append(CCFLAGS            = '--multiplier=16')
    env.Replace(INCPREFIX         = '-I ')
    env.Replace(CCCOM             = '$CC $SOURCES -o $TARGET $CFLAGS $CCFLAGS $_CCCOMCOM')
    env.Replace(RANLIBCOM         = '')
    # archiver
    env.Replace(AR                = '"'+os.path.join(iarEw430BinDir,'xar')+'"')
    env.Replace(ARCOM             = '$AR $SOURCES -o $TARGET')
    env.Append(ARFLAGS            = '')
    # linker
    env.Replace(LINK              = '"'+os.path.join(iarEw430BinDir,'xlink')+'"')
    env.Replace(PROGSUFFIX        = '.exe')
    env.Replace(LIBDIRPREFIX      = '-I')
    env.Replace(LIBLINKDIRPREFIX  = '-I')
    env.Replace(LIBLINKPREFIX     = 'lib')
    env.Replace(LIBLINKSUFFIX     = '.a')
    env.Append(LINKFLAGS          = '-f "C:\\Program Files\\IAR Systems\\Embedded Workbench 6.4\\430\\config\\multiplier.xcl"')
    env.Append(LINKFLAGS          = '-D_STACK_SIZE=50')
    env.Append(LINKFLAGS          = '-rt "C:\\Program Files\\IAR Systems\\Embedded Workbench 6.4\\430\\LIB\\DLIB\\dl430fn.r43"')
    env.Append(LINKFLAGS          = '-e_PrintfLarge=_Printf')
    env.Append(LINKFLAGS          = '-e_ScanfLarge=_Scanf ')
    env.Append(LINKFLAGS          = '-D_DATA16_HEAP_SIZE=50')
    env.Append(LINKFLAGS          = '-s __program_start')
    env.Append(LINKFLAGS          = '-D_DATA20_HEAP_SIZE=50')
    env.Append(LINKFLAGS          = '-S')
    env.Append(LINKFLAGS          = '-Ointel-standard')
    env.Replace(LINKCOM           = '$LINK $SOURCES -o $TARGET $LINKFLAGS $__RPATH $_LIBDIRFLAGS $_LIBFLAGS')
    
    # converts ELF to iHex
    def changeExtensionFunction(target, source, env):
        baseName      = str(target[0]).split('.')[0]
        fromExtension = '.a43'
        toExtension   = '.ihex'
        print 'change extension {0} {1}->{2}'.format(baseName,fromExtension,toExtension)
        os.rename(
            baseName+fromExtension,
            baseName+toExtension,
        )
    changeExtensionBuilder = Builder(
        action = changeExtensionFunction,
        suffix = '.ihex'
    )
    env.Append(BUILDERS = {'Elf2iHex'  : changeExtensionBuilder})
    
    # convert ELF to bin
    env.Append(BUILDERS = {'Elf2iBin'  : dummyFunc})
    
    # print sizes
    env.Append(BUILDERS = {'PrintSize' : dummyFunc})

elif env['toolchain']=='iar-proj':
    if env['board'] not in ['telosb','gina','z1']:
        raise SystemError('toolchain {0} can not be used for board {1}'.format(env['toolchain'],env['board']))
    
    try:
        iarEw430CommonBinDir      = os.path.join(os.environ['IAR_EW430_INSTALLDIR'],'common','bin')
    except KeyError as err:
        print 'You need to install environment variable IAR_EW430_INSTALLDIR which points to the installation directory of IAR Embedded Workbench for MSP430. Example: C:\Program Files\IAR Systems\Embedded Workbench 6.4'
        raise
    
    iarProjBuilderFunction = Builder(
        action = '"{0}" $SOURCE Debug'.format(
                    os.path.join(iarEw430CommonBinDir,'IarBuild')
                ),
        src_suffix  = '.ewp',
    )
    env.Append(BUILDERS = {'iarProjBuilder' : iarProjBuilderFunction})
    
    # converts ELF to iHex
    env.Append(BUILDERS = {'Elf2iHex'  : dummyFunc})
    
    # convert ELF to bin
    env.Append(BUILDERS = {'Elf2iBin'  : dummyFunc})
    
    # print sizes
    env.Append(BUILDERS = {'PrintSize' : dummyFunc})
    
else:
    if env['board'] in ['telosb','gina','z1']:
        raise SystemError('toolchain {0} can not be used for board {1}'.format(env['toolchain'],env['board']))
    
    # converts ELF to iHex
    env.Append(BUILDERS = {'Elf2iHex'  : dummyFunc})
    
    # convert ELF to bin
    env.Append(BUILDERS = {'Elf2iBin'  : dummyFunc})
    
    # print sizes
    env.Append(BUILDERS = {'PrintSize' : dummyFunc})

#============================ upload over JTAG ================================

def jtagUploadFunc(location):
    if   env['fet_version']==2:
        # MSP-FET430uif is running v2 Firmware
        return Builder(
            action      = 'mspdebug -d {0} -j uif "prog $SOURCE"'.format(location),
            suffix      = '.phonyupload',
            src_suffix  = '.ihex',
        )
    elif env['fet_version']==3:
        # MSP-FET430uif is running v2 Firmware
        return Builder(
            action      = 'mspdebug tilib -d {0} "prog $SOURCE"'.format(location),
            suffix      = '.phonyupload',
            src_suffix  = '.ihex',
        )
    else:
        raise SystemError('fet_version={0} unsupported.'.format(fet_version))
if env['jtag']:
    env.Append(BUILDERS = {'JtagUpload' : jtagUploadFunc(env['jtag'])})

#============================ objectify functions =============================

#===== ObjectifiedFilename

def ObjectifiedFilename(env,source):
    dir       = os.path.split(source)[0]
    file      = os.path.split(source)[1]
    filebase  = file.split('.')[0]
    fileext   = file.split('.')[1]
    return os.path.join(dir,'{0}_obj.{1}'.format(filebase,fileext))

env.AddMethod(ObjectifiedFilename, 'ObjectifiedFilename')

#===== Objectify

varsToChange = [
    'openserial_vars',
    'opentimers_vars',
    'scheduler_vars',
    'scheduler_dbg',
    'ieee154e_vars',
    'ieee154e_stats',
    'ieee154e_dbg',
    'neighbors_vars',
    'res_vars',
    'schedule_vars',
    'schedule_dbg',
    'icmpv6echo_vars',
    'icmpv6rpl_vars',
    'opencoap_vars',
    'tcp_vars',
    'ohlone_vars',
    'tcpinject_vars',
    'idmanager_vars',
    'openqueue_vars',
    'random_vars',
    'r6tus_vars',
]

returnTypes = [
    'int',
    'void',
    'error_t',
    'uint8_t',
    'uint16_t',
    'uint32_t',
    'bool',
    'opentimer_id_t',
    'PORT_TIMER_WIDTH',
    'dagrank_t',
    'open_addr_t*',
    'slotOffset_t',
    'frameLength_t',
    'cellType_t',
    'channelOffset_t',
    'ipv6_header_iht',
    'OpenQueueEntry_t*',
    'kick_scheduler_t',
]

callbackFunctionsToChange = [
    #===== bsp
    # supply
    # board
    # bsp_timer
    'cb',
    # debugpins
    # eui64
    # leds
    # radio
    'startFrame_cb',
    'endFrame_cb',
    # radiotimer
    'overflow_cb',
    'compare_cb',
    # sctimer
    # uart
    'txCb',
    'rxCb',
    #===== drivers
    # openserial
    # opentimers
    'callback',
    #===== kernel
    # scheduler
    #===== openwsn
    # IEEE802154
    # IEEE802154E
    # topology
    # neighbors
    # res
    # schedule
    # iphc
    # openbridge
    # forwarding
    # icmpv6
    # icmpv6echo
    # icmpv6rpl
    # opencoap
    'callbackRx',
    'callbackSendDone',
    # opentcp
    # openudp
    # rsvp
    # layerdebug
    # ohlone
    # rex
    # rinfo
    # rleds
    # rreg
    # rwellknown
    # tcpecho
    # tcpinject
    # tcpprint
    # udpecho
    # udpinject
    # udplatency
    # udpprint
    # udprand
    # udpstorm
    # idmanager
    # openqueue
    # openrandom
    # packetfunctions
]

functionsToChange = [
    #===== bsp
    'mote_main',
    # supply
    'supply_init',
    'supply_on',
    'supply_off',
    # board
    'board_init',
    'board_sleep',
    'board_reset',
    # bsp_timer
    'bsp_timer_init',
    'bsp_timer_set_callback',
    'bsp_timer_reset',
    'bsp_timer_scheduleIn',
    'bsp_timer_cancel_schedule',
    'bsp_timer_get_currentValue',
    'bsp_timer_isr',
    # debugpins
    'debugpins_init',
    'debugpins_frame_toggle',
    'debugpins_frame_clr',
    'debugpins_frame_set',
    'debugpins_slot_toggle',
    'debugpins_slot_clr',
    'debugpins_slot_set',
    'debugpins_fsm_toggle',
    'debugpins_fsm_clr',
    'debugpins_fsm_set',
    'debugpins_task_toggle',
    'debugpins_task_clr',
    'debugpins_task_set',
    'debugpins_isr_toggle',
    'debugpins_isr_clr',
    'debugpins_isr_set',
    'debugpins_radio_toggle',
    'debugpins_radio_clr',
    'debugpins_radio_set',
    # eui64
    'eui64_get',
    # leds
    'leds_init',
    'leds_error_on',
    'leds_error_off',
    'leds_error_toggle',
    'leds_error_isOn',
    'leds_error_blink',
    'leds_radio_on',
    'leds_radio_off',
    'leds_radio_toggle',
    'leds_radio_isOn',
    'leds_sync_on',
    'leds_sync_off',
    'leds_sync_toggle',
    'leds_sync_isOn',
    'leds_debug_on',
    'leds_debug_off',
    'leds_debug_toggle',
    'leds_debug_isOn',
    'leds_all_on',
    'leds_all_off',
    'leds_all_toggle',
    'leds_circular_shift',
    'leds_increment',
    # radio
    'radio_init',
    'radio_setOverflowCb',
    'radio_setCompareCb',
    'radio_setStartFrameCb',
    'radio_setEndFrameCb',
    'radio_reset',
    'radio_startTimer',
    'radio_getTimerValue',
    'radio_setTimerPeriod',
    'radio_getTimerPeriod',
    'radio_setFrequency',
    'radio_rfOn',
    'radio_rfOff',
    'radio_loadPacket',
    'radio_txEnable',
    'radio_txNow',
    'radio_rxEnable',
    'radio_rxNow',
    'radio_getReceivedFrame',
    'radio_isr',
    'radio_intr_startOfFrame',
    'radio_intr_endOfFrame',
    # radiotimer
    'radiotimer_init',
    'radiotimer_setOverflowCb',
    'radiotimer_setCompareCb',
    'radiotimer_setStartFrameCb',
    'radiotimer_setEndFrameCb',
    'radiotimer_start',
    'radiotimer_getValue',
    'radiotimer_setPeriod',
    'radiotimer_getPeriod',
    'radiotimer_schedule',
    'radiotimer_cancel',
    'radiotimer_getCapturedTime',
    'radiotimer_isr',
    'radiotimer_intr_compare',
    'radiotimer_intr_overflow',
    # sctimer
    'sctimer_init',
    'sctimer_stop',
    'sctimer_schedule',
    'sctimer_getValue',
    'sctimer_setCb',
    'sctimer_clearISR',
    'sctimer_reset',
    # uart
    'uart_init',
    'uart_setCallbacks',
    'uart_enableInterrupts',
    'uart_disableInterrupts',
    'uart_clearRxInterrupts',
    'uart_clearTxInterrupts',
    'uart_writeByte',
    'uart_readByte',
    'uart_tx_isr',
    'uart_rx_isr',
    #===== drivers
    # openserial
    'openserial_init',
    'openserial_printStatus',
    'openserial_printInfoErrorCritical',
    'openserial_printData',
    'openserial_printInfo',
    'openserial_printError',
    'openserial_printCritical',
    'openserial_getNumDataBytes',
    'openserial_getInputBuffer',
    'openserial_startInput',
    'openserial_startOutput',
    'openserial_stop',
    'debugPrint_outBufferIndexes',
    'openserial_echo',
    'outputHdlcOpen',
    'outputHdlcWrite',
    'outputHdlcClose',
    'inputHdlcOpen',
    'inputHdlcWrite',
    'inputHdlcClose',
    'isr_openserial_tx',
    'isr_openserial_rx',
    # opentimers
    'opentimers_init',
    'opentimers_start',
    'opentimers_setPeriod',
    'opentimers_stop',
    'opentimers_restart',
    'opentimers_timer_callback',
    #===== kernel
    # scheduler
    'scheduler_init',
    'scheduler_start',
    'scheduler_push_task',
    #===== openwsn
    'openwsn_init',
    # IEEE802154
    'ieee802154_prependHeader',
    'ieee802154_retrieveHeader',
    # IEEE802154E
    'ieee154e_init',
    'ieee154e_asnDiff',
    'isr_ieee154e_newSlot',
    'isr_ieee154e_timer',
    'ieee154e_startOfFrame',
    'ieee154e_endOfFrame',
    'debugPrint_asn',
    'debugPrint_isSync',
    'debugPrint_macStats',
    'activity_synchronize_newSlot',
    'activity_synchronize_startOfFrame',
    'activity_synchronize_endOfFrame',
    'activity_ti1ORri1',
    'activity_ti2',
    'activity_tie1',
    'activity_ti3',
    'activity_tie2',
    'activity_ti4',
    'activity_tie3',
    'activity_ti5',
    'activity_ti6',
    'activity_tie4',
    'activity_ti7',
    'activity_tie5',
    'activity_ti8',
    'activity_tie6',
    'activity_ti9',
    'activity_ri2',
    'activity_rie1',
    'activity_ri3',
    'activity_rie2',
    'activity_ri4',
    'activity_rie3',
    'activity_ri5',
    'activity_ri6',
    'activity_rie4',
    'activity_ri7',
    'activity_rie5',
    'activity_ri8',
    'activity_rie6',
    'activity_ri9',
    'isValidAdv',
    'isValidRxFrame',
    'isValidAck',
    'incrementAsnOffset',
    'asnWriteToAdv',
    'ieee154e_getAsn',
    'asnWriteToSerial',
    'asnStoreFromAdv',
    'synchronizePacket',
    'synchronizeAck',
    'changeIsSync',
    'notif_sendDone',
    'notif_receive',
    'resetStats',
    'updateStats',
    'calculateFrequency',
    'changeState',
    'endSlot',
    'ieee154e_isSynch',
    # topology
    'topology_isAcceptablePacket',
    # neighbors
    'neighbors_init',
    'neighbors_getMyDAGrank',
    'neighbors_getNumNeighbors',
    'neighbors_getPreferredParentEui64',
    'neighbors_getKANeighbor',
    'neighbors_isStableNeighbor',
    'neighbors_isPreferredParent',
    'neighbors_isNeighborWithLowerDAGrank',
    'neighbors_isNeighborWithHigherDAGrank',
    'neighbors_indicateRx',
    'neighbors_indicateTx',
    'neighbors_indicateRxDIO',
    'neighbors_getNeighbor',
    'neighbors_updateMyDAGrankAndNeighborPreference',
    'debugPrint_neighbors',
    'debugNetPrint_neighbors',
    'registerNewNeighbor',
    'isNeighbor',
    'removeNeighbor',
    'isThisRowMatching',
    # res
    'res_init',
    'debugPrint_myDAGrank',
    'res_send',
    'task_resNotifSendDone',
    'task_resNotifReceive',
    'timers_res_fired',
    'res_send_internal',
    'sendAdv',
    'sendKa',
    'res_timer_cb',
    # schedule
    'schedule_init',
    'debugPrint_schedule',
    'debugPrint_backoff',
    'schedule_setFrameLength',
    'schedule_getSlotInfo',
    'schedule_addActiveSlot',
    'schedule_removeActiveSlot',
    'schedule_syncSlotOffset',
    'schedule_advanceSlot',
    'schedule_getNextActiveSlotOffset',
    'schedule_getFrameLength',
    'schedule_getType',
    'schedule_getNeighbor',
    'schedule_getChannelOffset',
    'schedule_getOkToSend',
    'schedule_resetBackoff',
    'schedule_indicateRx',
    'schedule_indicateTx',
    'schedule_getNetDebugInfo',
    'schedule_resetEntry',
    # iphc
    'iphc_init',
    'iphc_sendFromForwarding',
    'iphc_sendFromBridge',
    'iphc_sendDone',
    'iphc_receive',
    'prependIPv6Header',
    'retrieveIPv6Header',
    # openbridge
    'openbridge_init',
    'openbridge_triggerData',
    'openbridge_sendDone',
    'openbridge_receive',
    # forwarding
    'forwarding_init',
    'forwarding_send',
    'forwarding_sendDone',
    'forwarding_receive',
    'forwarding_send_internal_RoutingTable',
    'forwarding_send_internal_SourceRouting',
    'forwarding_getNextHop_RoutingTable',
    # icmpv6
    'icmpv6_init',
    'icmpv6_send',
    'icmpv6_sendDone',
    'icmpv6_receive',
    # icmpv6echo
    'icmpv6echo_init',
    'icmpv6echo_trigger',
    'icmpv6echo_sendDone',
    'icmpv6echo_receive',
    # icmpv6rpl
    'icmpv6rpl_init',
    'icmpv6rpl_sendDone',
    'icmpv6rpl_receive',
    'icmpv6rpl_timer_DIO_cb',
    'icmpv6rpl_timer_DIO_task',
    'sendDIO',
    'icmpv6rpl_timer_DAO_cb',
    'icmpv6rpl_timer_DAO_task',
    'sendDAO',
    # opencoap
    'opencoap_init',
    'opencoap_receive',
    'opencoap_sendDone',
    'timers_coap_fired',
    'opencoap_writeLinks',
    'opencoap_register',
    'opencoap_send',
    'icmpv6coap_timer_cb',
    # opentcp
    'opentcp_init',
    'opentcp_connect',
    'opentcp_send',
    'opentcp_sendDone',
    'opentcp_receive',
    'opentcp_close',
    'opentcp_debugPrint',
    'timers_tcp_fired',
    'prependTCPHeader',
    'containsControlBits',
    'opentcp_reset',
    'tcp_change_state',
    'opentcp_timer_cb',
    # openudp
    'openudp_init',
    'openudp_send',
    'openudp_sendDone',
    'openudp_receive',
    'openudp_debugPrint',
    # rsvp
    'rsvp_qos_request',
    'rsvp_timer_cb',
    # layerdebug
    'layerdebug_init',
    'layerdebug_timer_schedule_cb',
    'layerdebug_timer_neighbors_cb',
    'layerdebug_task_schedule_cb',
    'layerdebug_task_neighbors_cb',
    'layerdebug_sendDone',
    'layerdebug_schedule_receive',
    'layerdebug_neighbors_receive',
    # ohlone
    'ohlone_init',
    'ohlone_shouldIlisten',
    'ohlone_sendpkt',
    'ohlone_check4chars',
    'ohlone_receive',
    'ohlone_sendDone',
    'ohlone_connectDone',
    'ohlone_debugPrint',
    # r6tus
    'r6tus_init',
    'r6tus_receive',
    'r6tus_sendDone',
    # rex
    'rex_init',
    'rex_receive',
    'rex_timer_cb',
    'rex_task_cb',
    'rex_sendDone',
    # rinfo
    'rinfo_init',
    'rinfo_receive',
    'rinfo_sendDone',
    # rleds
    'rleds__init',
    'rleds_receive',
    'rleds_sendDone',
    # rreg
    'rreg_init',
    'rreg_receive',
    'rreg_timer',
    'rreg_sendDone',
    'hexToAscii',
    # rwellknown
    'rwellknown_init',
    'rwellknown_receive',
    'rwellknown_sendDone',
    # tcpecho
    'tcpecho_init',
    'tcpecho_shouldIlisten',
    'tcpecho_receive',
    'tcpecho_sendDone',
    'tcpecho_connectDone',
    'tcpecho_debugPrint',
    # tcpinject
    'tcpinject_init',
    'tcpinject_shouldIlisten',
    'tcpinject_trigger',
    'tcpinject_connectDone',
    'tcpinject_sendDone',
    'tcpinject_receive',
    'tcpinject_debugPrint',
    # tcpprint
    'tcpprint_init',
    'tcpprint_shouldIlisten',
    'tcpprint_receive',
    'tcpprint_connectDone',
    'tcpprint_sendDone',
    'tcpprint_debugPrint',
    # udpecho
    'udpecho_init',
    'udpecho_receive',
    'udpecho_sendDone',
    'udpecho_debugPrint',
    # udpinject
    'udpinject_init',
    'udpinject_trigger',
    'udpinject_sendDone',
    'udpinject_receive',
    'udpinject_debugPrint',
    # udplatency
    'udplatency_init',
    'udplatency_task',
    'udplatency_timer',
    'udplatency_sendDone',
    'udplatency_receive',
    # udpprint
    'udpprint_init',
    'udpprint_sendDone',
    'udpprint_receive',
    'udpprint_debugPrint',
    # udprand
    'udprand_init',
    'udprand_task',
    'udprand_timer',
    'udprand_sendDone',
    'udprand_receive',
    # udpstorm
    'udpstorm_init',
    'udpstorm_receive',
    'udpstorm_timer_cb',
    'udpstorm_task_cb',
    'udpstorm_sendDone',
    # idmanager
    'idmanager_init',
    'idmanager_getIsDAGroot',
    'idmanager_setIsDAGroot',
    'idmanager_getIsBridge',
    'idmanager_setIsBridge',
    'idmanager_getMyID',
    'idmanager_setMyID',
    'idmanager_isMyAddress',
    'idmanager_triggerAboutRoot',
    'idmanager_triggerAboutBridge',
    'debugPrint_id',
    # openqueue
    'openqueue_init',
    'debugPrint_queue',
    'openqueue_getFreePacketBuffer',
    'openqueue_freePacketBuffer',
    'openqueue_removeAllCreatedBy',
    'openqueue_removeAllOwnedBy',
    'openqueue_resGetSentPacket',
    'openqueue_resGetReceivedPacket',
    'openqueue_macGetDataPacket',
    'openqueue_macGetAdvPacket',
    'openqueue_reset_entry',
    # openrandom
    'openrandom_init',
    'openrandom_get16b',
    # packetfunctions
    'packetfunctions_ip128bToMac64b',
    'packetfunctions_mac64bToIp128b',
    'packetfunctions_mac64bToMac16b',
    'packetfunctions_mac16bToMac64b',
    'packetfunctions_isBroadcastMulticast',
    'packetfunctions_isAllRoutersMulticast',
    'packetfunctions_isAllHostsMulticast',
    'packetfunctions_sameAddress',
    'packetfunctions_readAddress',
    'packetfunctions_writeAddress',
    'packetfunctions_reserveHeaderSize',
    'packetfunctions_tossHeader',
    'packetfunctions_reserveFooterSize',
    'packetfunctions_tossFooter',
    'packetfunctions_calculateCRC',
    'packetfunctions_checkCRC',
    'packetfunctions_calculateChecksum',
    'onesComplementSum',
    'packetfunctions_htons',
    'packetfunctions_ntohs',
    'packetfunctions_htonl',
]

headerFiles = [
    #=== libbsp
    'board',
    'bsp_timer',
    'debugpins',
    'eui64',
    'leds',
    'radio',
    'radiotimer',
    'uart',
    #=== libdrivers,
    'openhdlc',
    'openserial',
    'opentimers',
    #=== libopenos
    'scheduler',
    #=== libopenstack
    'openwsn',
    # 02a-MAClow
    'topology',
    'IEEE802154',
    'IEEE802154E',
    # 02b-MAChigh
    'neighbors',
    'res',
    'schedule',
    # 02.5-MPLS
    # TODO
    # 03a-IPHC
    'iphc',
    'openbridge',
    # 03b-IPv6
    'forwarding',
    'icmpv6',
    'icmpv6echo',
    'icmpv6rpl',
    # 04-TRAN
    'opencoap',
    'opentcp',
    'openudp',
    'rsvp',
    # 07-App
    #'heli',
    #'imu',
    'layerdebug',
    'ohlone',
    'ohlone_webpages',
    #'rex',
    'rheli',
    'rinfo',
    'rleds',
    'rreg',
    'rrube',
    #'rt',
    'rwellknown',
    #'rxl1',
    'tcpecho',
    'tcpinject',
    'tcpprint',
    'udpecho',
    'udpinject',
    'udpprint',
    'udprand',
    'udplatency',
    'udpstorm',
    # cross-layers
    'idmanager',
    'openqueue',
    'openrandom',
    'packetfunctions',
]

def objectify(env,target,source):
    
    assert len(target)==1
    assert len(source)==1
    
    target = target[0].abspath
    source = source[0].abspath
    
    basefilename = os.path.split(source)[1].split('.')[0]
    
    if os.path.split(source)[1].split('.')[1]=='h':
        headerFile = True
    else:
        headerFile = False
    
    output  = []
    output += ['objectify:']
    output += ['- target : {0}'.format(target)]
    output += ['- source : {0}'.format(source)]
    output  = '\n'.join(output)
    #print output
    
    #========== read
    
    f = open(source,'r')
    lines = f.read()
    f.close()
    
    #========= modify
    
    #=== all files
    
    # add banner
    banner    = []
    banner   += ['/**']
    banner   += ['DO NOT EDIT DIRECTLY!!']
    banner   += ['']
    banner   += ['This file was \'objectified\' by SCons as a pre-processing']
    banner   += ['step for the building a Python extension module.']
    banner   += ['']
    banner   += ['This was done on {0}.'.format(datetime.datetime.now())]
    banner   += ['*/']
    banner   += ['']
    banner    = '\n'.join(banner)
    
    lines     = banner+lines
    
    # update the included headers
    for v in headerFiles+[basefilename]:
        lines = re.sub(
            r'\b{0}.h\b'.format(v),
            r'{0}_obj.h'.format(v),
            lines
        )
    
    # change callback function declaration signatures
    def replaceCallbackFunctionDeclarations(matchObj):
        function        = matchObj.group(1)
        args            = matchObj.group(2)
        
        if args:
            return '{0}(OpenMote* self, {1})'.format(function, args)
        else:
            return '{0}(OpenMote* self)'.format(function)
    
    lines = re.sub(
        pattern         = r'(typedef[ \S]+_cbt\))\((.*?)\)',
        repl            = replaceCallbackFunctionDeclarations,
        string          = lines,
        flags           = re.DOTALL,
    )
    
    # comment out global variables declarations 
    if not headerFile:
        for v in varsToChange:
            lines  = re.sub(
                '{0}_t\s+{0}\s*;'.format(v),
                '// declaration of global variable _{0}_ removed during objectification.'.format(v),
                lines
            )
    
    # change global variables by self->* counterpart
    if basefilename!='openwsnmodule':
        for v in varsToChange:
            lines = re.sub(
                r'\b{0}\b'.format(v),
                r'(self->{0})'.format(v),
                lines
            )
    
    # change function signatures
    def replaceFunctions(matchObj):
        returnType      = matchObj.group(1)
        function        = matchObj.group(2)
        args            = matchObj.group(3)
        
        if returnType in returnTypes:
            if args:
                return '{0} {1}(OpenMote* self, {2})'.format(returnType,function, args)
            else:
                return '{0} {1}(OpenMote* self)'.format(returnType,function)
        else:
            if args:
                return '{0} {1}(self, {2})'.format(returnType,function, args)
            else:
                return '{0} {1}(self)'.format(returnType,function, args)
    
    if basefilename!='openwsnmodule':
        for v in functionsToChange:
            lines = re.sub(
                pattern     = r'([\w\*]*)[ \t]*({0})[ \t]*\((.*?)\)'.format(v),
                repl        = replaceFunctions,
                string      = lines,
                flags       = re.DOTALL,
            )
    
    #=== .h files only
    
    if headerFile:
        
        # include openwsnmodule
        lines = re.sub(
            r'(//[=]+ prototypes [=]+)',
            r'#include "openwsnmodule_obj.h"\ntypedef struct OpenMote OpenMote;\n\n\1',
            lines,
        )
    
    #=== .c files only
    
    if not headerFile:
        
        # change function signatures
        def replaceCallbackFunctionCalls(matchObj):
            operator        = matchObj.group(1)
            function        = matchObj.group(2)
            args            = matchObj.group(3)
            
            if args:
                return '{0}{1}(self, {2})'.format(operator,function, args)
            else:
                return '{0}{1}(self)'.format(operator,function)
        
        for v in callbackFunctionsToChange:
            lines = re.sub(
                pattern     = '(\.|->)({0})\((.*?)\)'.format(v),
                repl        = replaceCallbackFunctionCalls,
                string      = lines,
            )
        
        # modify Python module name
        assert len(BUILD_TARGETS)==1
        if basefilename=='openwsnmodule':
            lines = re.sub(
                'openwsn_generic',
                BUILD_TARGETS[0],
                lines
            )
    
    #========== write
    
    f = open(target,'w')
    f.write(''.join(lines))
    f.close()

objectifyBuilder = Builder(action = objectify)
env.Append(BUILDERS = {'Objectify' : objectifyBuilder})

#============================ bootload ========================================

class telosb_bootloadThread(threading.Thread):
    def __init__(self,comPort,hexFile,countingSem):
        
        # store params
        self.comPort         = comPort
        self.hexFile         = hexFile
        self.countingSem     = countingSem
        
        # initialize parent class
        threading.Thread.__init__(self)
        self.name            = 'telosb_bootloadThread_{0}'.format(self.comPort)
    
    def run(self):
        print 'starting bootloading on {0}'.format(self.comPort)
        subprocess.call(
            'python '+os.path.join('firmware','openos','bootloader','telosb','bsl')+' --telosb -c {0} -r -e -I -p "{1}"'.format(self.comPort,self.hexFile)
        )
        print 'done bootloading on {0}'.format(self.comPort)
        
        # indicate done
        self.countingSem.release()

def telosb_bootload(target, source, env):
    bootloadThreads = []
    countingSem     = threading.Semaphore(0)
    # create threads
    for comPort in env['bootload'].split(','):
        bootloadThreads += [
            telosb_bootloadThread(
                comPort      = comPort,
                hexFile      = source[0],
                countingSem  = countingSem,
            )
        ]
    # start threads
    for t in bootloadThreads:
        t.start()
    # wait for threads to finish
    for t in bootloadThreads:
        countingSem.acquire()

# bootload
def BootloadFunc():
    if   env['board']=='telosb':
        return Builder(
            action      = telosb_bootload,
            suffix      = '.phonyupload',
            src_suffix  = '.ihex',
        )
    else:
        raise SystemError('bootloading on board={0} unsupported.'.format(env['board']))
if env['bootload']:
    env.Append(BUILDERS = {'Bootload' : BootloadFunc()})

# PostBuildExtras is a method called after a program (not a library) is built.
# You can any addition step in this function, such as converting the binary
# or copying it somewhere.
def extras(env, source):
    returnVal  = []
    returnVal += [env.PrintSize(source=source)]
    returnVal += [env.Elf2iHex(source=source)]
    returnVal += [env.Elf2iBin(source=source)]
    if   env['jtag']:
        returnVal += [env.JtagUpload(env.Elf2iHex(source))]
    elif env['bootload']:
        returnVal += [env.Bootload(env.Elf2iHex(source))]
    return returnVal
env.AddMethod(extras, 'PostBuildExtras')

#============================ helpers =========================================

def buildLibs(projectDir):
    libs_dict = {
        '00std': [                                                ],
        '01bsp': [                                        'libbsp'],
        '02drv': [                           'libdrivers','libbsp'],
        '03oos': ['libopenstack','libopenos','libdrivers','libbsp'], # this order needed for mspgcc
    }
    
    returnVal = None
    for k,v in libs_dict.items():
        if projectDir.startswith(k):
           returnVal = v
    
    assert returnVal!=None
    
    return returnVal

def buildIncludePath(projectDir,localEnv):
    if projectDir.startswith('03oos_'):
        localEnv.Append(
            CPPPATH = [
                os.path.join('#','firmware','openos','openwsn'),
                os.path.join('#','firmware','openos','openwsn','03b-IPv6'),
                os.path.join('#','firmware','openos','openwsn','02b-MAChigh'),
                os.path.join('#','firmware','openos','openwsn','02a-MAClow'),
                os.path.join('#','firmware','openos','openwsn','cross-layers'),
                os.path.join('#','firmware','openos','drivers','common'),
            ]
        )

def populateTargetGroup(localEnv,targetName):
    env['targets']['all'].append(targetName)
    for prefix in ['std','bsp','drv','oos']:
        if targetName.startswith(prefix):
            env['targets']['all_'+prefix].append(targetName)

def sconscript_scanner(localEnv):
    '''
    This function is called from the following directories:
    - projects\common\
    - projects\<board>\
    '''
    # list subdirectories
    subdirs = [name for name in os.listdir('.') if os.path.isdir(os.path.join('.', name)) ]
    
    # parse dirs and build targets
    for projectDir in subdirs:
        
        src_dir     = os.path.join(os.getcwd(),projectDir)
        variant_dir = os.path.join(env['VARDIR'],'projects',projectDir),
        
        added      = False
        targetName = projectDir[2:]
        
        if   (
                ('{0}.c'.format(projectDir) in os.listdir(projectDir)) and
                (localEnv['toolchain']!='iar-proj') and 
                (localEnv['board']!='python')
             ):
            
            localEnv.VariantDir(
                variant_dir = variant_dir,
                src_dir     = src_dir,
                duplicate   = 0,
            )
    
            target =  projectDir
            source = [os.path.join(projectDir,'{0}.c'.format(projectDir))]
            libs   = buildLibs(projectDir)
            
            buildIncludePath(projectDir,localEnv)

            #fix for problem on having the same target as directory name and failing to compile in linux. Appending something to the target solves the isse.
            target=target+"_prog"
            
            exe = localEnv.Program(
                target  = target,
                source  = source,
                LIBS    = libs,
            )
            targetAction = localEnv.PostBuildExtras(exe)
            
            Alias(targetName, [targetAction])
            added = True
        
        elif (
                ('{0}.c'.format(projectDir) in os.listdir(projectDir)) and
                (localEnv['board']=='python')
             ):
            
            # build the artifacts in a separate directory
            localEnv.VariantDir(
                variant_dir = variant_dir,
                src_dir     = src_dir,
                duplicate   = 1,
            )
            
            # build both the application's and the Python module's main files
            sources_c = [
                os.path.join(projectDir,'{0}.c'.format(projectDir)),
                os.path.join('#','firmware','openos','bsp','boards','python','openwsnmodule.c'),
            ]
            
            # objectify those two files
            for s in sources_c:
                temp = localEnv.Objectify(
                    target = localEnv.ObjectifiedFilename(s),
                    source = s,
                )
            
            # prepare environment for this build
            target = targetName
            source = [localEnv.ObjectifiedFilename(s) for s in sources_c]
            libs   = buildLibs(projectDir)
            libs  += [['python' + distutils.sysconfig.get_config_var('VERSION')]]
            
            buildIncludePath(projectDir,localEnv)
            
            # build a shared library (a Python extension module) rather than an exe
            
            targetAction = localEnv.SharedLibrary(
                target,
                source,
                LIBS           = libs,
                SHLIBPREFIX    = '',
                SHLIBSUFFIX    = distutils.sysconfig.get_config_var('SO'),
            )
            
            Alias(targetName, [targetAction])
            added = True
            
        elif (
                ('{0}.ewp'.format(projectDir) in os.listdir(projectDir)) and
                (localEnv['toolchain']=='iar-proj')
             ):
            
            VariantDir(
                variant_dir = variant_dir,
                src_dir     = src_dir,
                duplicate   = 0,
            )
            
            source = [os.path.join(projectDir,'{0}.ewp'.format(projectDir))]
        
            targetAction = localEnv.iarProjBuilder(
                source  = source,
            )
            
            Alias(targetName, [targetAction])
            added = True
        
        if added:
            populateTargetGroup(localEnv,targetName)

env.AddMethod(sconscript_scanner, 'SconscriptScanner')
    
#============================ board ===========================================

# Get build environment from platform directory
buildEnv = env.SConscript(
    os.path.join('firmware','openos','projects',env['board'],'SConscript.env'),
    exports     = ['env'],
)

#bspheader
boardsDir       = os.path.join('#','firmware','openos','bsp','boards')
boardsVarDir    = os.path.join(buildEnv['VARDIR'],'bsp','boards')
buildEnv.SConscript(
    os.path.join(boardsDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = boardsVarDir,
)

# bsp
bspDir          = os.path.join('#','firmware','openos','bsp','boards',buildEnv['BSP'])
bspVarDir       = os.path.join(buildEnv['VARDIR'],'bsp','boards',buildEnv['BSP'])
buildEnv.Append(CPPPATH = [bspDir])
buildEnv.SConscript(
    os.path.join(bspDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = bspVarDir,
)
buildEnv.Clean('libbsp', Dir(bspVarDir).abspath)
buildEnv.Append(LIBPATH = [bspVarDir])

# kernel
kernelDir       = os.path.join('#','firmware','openos','kernel','openos')
kernelVarDir    = os.path.join(buildEnv['VARDIR'],'kernel','openos')
buildEnv.SConscript(
    os.path.join(kernelDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = kernelVarDir,
)
buildEnv.Clean('libopenos', Dir(kernelVarDir).abspath)
buildEnv.Append(LIBPATH = [kernelVarDir])

# drivers
driversDir      = os.path.join('#','firmware','openos','drivers')
driversVarDir   = os.path.join(buildEnv['VARDIR'],'drivers')
buildEnv.SConscript(
    os.path.join(driversDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = driversVarDir,
)
buildEnv.Clean('libdrivers', Dir(driversVarDir).abspath)
buildEnv.Append(LIBPATH = [driversVarDir])

# openstack
openstackDir    = os.path.join('#','firmware','openos','openwsn')
openstackVarDir = os.path.join(buildEnv['VARDIR'],'openwsn')
buildEnv.SConscript(
    os.path.join(openstackDir,'SConscript'),
    exports     = {'env': buildEnv},
    variant_dir = openstackVarDir,
)
buildEnv.Clean('libopenstack', Dir(openstackVarDir).abspath)
buildEnv.Append(LIBPATH = [openstackVarDir])

# projects
buildEnv.SConscript(
    os.path.join('#','firmware','openos','projects','SConscript'),
    exports     = {'env': buildEnv},
    #variant_dir = os.path.join(env['VARDIR'],'projects'),
)
