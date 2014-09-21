/** 
\brief Register definitions for the Texas Instruments CC1101 radio chip.

\author Adilla Susungi <adilla.susungi@etu.unistra.fr>, August 2013.
*/


#ifndef __CC1101_H
#define __CC1101_H

//====================== Spi flags for header byte ==============================

// These values (given in datasheet) are obtained by adding :
// - R/W bit (1 or 0) 
// - Burst access bit (1 or 0) 
// - Address (only for RX and TX here) 


#define CC1101_WRITE_SINGLE      0x00
#define CC1101_WRITE_BURST       0x40

#define CC1101_READ_SINGLE       0x80
#define CC1101_READ_BURST        0xC0

#define CC1101_TX_SINGLE         0x3F
#define CC1101_TX_BURST          0x7F

#define CC1101_RX_SINGLE         0xBF
#define CC1101_RX_BURST          0xFF

//====================== Status Byte ============================

typedef struct {
  uint8_t FIFO_BYTES_AVAILABLE:4;
  uint8_t STATE:3;
  uint8_t CHIP_RDYn:1;
} cc1101_status_t;

//===================== Command Strobes =========================

#define CC1101_SRES                      0x30 // reset chip 
#define CC1101_SFSTXON                   0x31 // enable and calibrate frequency synthesizer 
#define CC1101_SXOFF                     0x32 // turn off crystal oscillator 
#define CC1101_SCAL                      0x33 // calibrate frequency and turn it off 
#define CC1101_SRX                       0x34 // enable rx 
#define CC1101_STX                       0x35 // enable tx 
#define CC1101_SIDLE                     0x36 // exit rx/tx 
#define CC1101_SWOR                      0x38 // start rx polling sequence 
#define CC1101_SPWD                      0x39 // power down mode 
#define CC1101_SFRX                      0x3A // flush rx fifo 
#define CC1101_SFTX                      0x3B // flush tx fifo
#define CC1101_SWORRST                   0x3C // reset real time clock 
#define CC1101_SNOP                      0x3D // no operation  



//====================== Configuration Registers ================

#define CC1101_IOCFG2                    0x00 // [R/W] GDO2 output pin config 
typedef struct {
  uint8_t GDO2_CFG:6;
  uint8_t GDO2_INV:1;
  uint8_t unused_r0:1;
} cc1101_IOCFG2_reg_t;

#define CC1101_IOCFG1                    0x01 // [R/W] GDO1 output pin config 
typedef struct {
  uint8_t GDO1_CFG:6;
  uint8_t GDO1_INV:1;
  uint8_t GDO_DS:1;
} cc1101_IOCFG1_reg_t;


#define CC1101_IOCFG0                    0x02 // [R/W] GDO0 output pin config 
typedef struct {
  uint8_t GDO0_CFG:6;
  uint8_t GDO0_INV:1;
  uint8_t TEMP_SENSOR_ENABLE:1;
} cc1101_IOCFG0_reg_t;


#define CC1101_FIFOTHR                   0x03 // [R/W] rx fifo and tx fifo thresholds 
typedef struct {
  uint8_t FIFO_THR:4;
  uint8_t CLOSE_IN_RX:2;
  uint8_t ADC_RETENTION:1;
  uint8_t reserved_rw:1;
} cc1101_FIFOTHR_reg_t;

#define CC1101_SYNC1                     0x04 // [R/W] sync word, high byte 
typedef struct {
  uint8_t SYNC1:8;
} cc1101_SYNC1_reg_t;

#define CC1101_SYNC0                     0x05 // [R/W] sync word, low byte
typedef struct {
  uint8_t SYNC0:8;
} cc1101_SYNC0_reg_t;

#define CC1101_PKTLEN                    0x06 // [R/W] packet length 
typedef struct {
  uint8_t PACKET_LENGTH:8;
} cc1101_PKTLEN_reg_t;

#define CC1101_PKTCTRL1                  0x07 // [R/W] packet automation control 
typedef struct {
  uint8_t ADR_CHK:2;
  uint8_t APPEND_STATUS:1;
  uint8_t CRC_AUTOFLUSH:1;
  uint8_t unused_r0:1;
  uint8_t PQT:3;
} cc1101_PKTCTRL1_reg_t;

#define CC1101_PKTCTRL0                  0x08 // [R/W] packet automation control 
typedef struct {
  uint8_t LENGTH_CONFIG:2;
  uint8_t CRC_EN:1;
  uint8_t unused_r0_2:1;
  uint8_t PKT_FORMAT:2;
  uint8_t WHITE_DATA:1;
  uint8_t unused_r0_1:1;
} cc1101_PKTCTRL0_reg_t;

#define CC1101_ADDR                      0x09 // [R/W] device address 
typedef struct {
  uint8_t DEVICE_ADDR:8;
} cc1101_ADDR_reg_t;

#define CC1101_CHANNR                    0x0A // [R/W] channel number 
typedef struct {
  uint8_t CHAN:8;
} cc1101_CHANNR_reg_t;


#define CC1101_FSCTRL1                   0x0B // [R/W] frequency synthesizer control 
typedef struct {
  uint8_t FREQ_IF:5;
  uint8_t reserved_rw:1;
  uint8_t unused_r0:2;
} cc1101_FSCTRL1_reg_t;

#define CC1101_FSCTRL0                   0x0C // [R/W] Frequency synthesizer control 
typedef struct {
  uint8_t FREQOFF:8;
} cc1101_FSCTRL0_reg_t;

#define CC1101_FREQ2                     0x0D // [R/W] frequency control word, high byte
typedef struct {
  uint8_t FREQ_2:6;
  uint8_t FREQ_1:2;
} cc1101_FREQ2_reg_t;

#define CC1101_FREQ1                     0x0E // [R/W] frequency control word, middle byte
typedef struct {
  uint8_t FREQ:8;
} cc1101_FREQ1_reg_t;

#define CC1101_FREQ0                     0x0F // [R/W] frequency control word, low byte
typedef struct {
  uint8_t FREQ:8;
} cc1101_FREQ0_reg_t;

#define CC1101_MDMCFG4                   0x10 // [R/W] modem configuration
typedef struct {
  uint8_t DRATE_E:4;
  uint8_t CHANBW_M:2;
  uint8_t CHANBW_E:2;
} cc1101_MDMCFG4_reg_t;


#define CC1101_MDMCFG3                   0x11 // [R/W] modem configuration
typedef struct {
  uint8_t DRATE_M:8;
} cc1101_MDMCFG3_reg_t;


#define CC1101_MDMCFG2                   0x12 
typedef struct {
  uint8_t SYNC_MODE:3;
  uint8_t MANCHESTER_EN:1;
  uint8_t MOD_FORMAT:3;
  uint8_t DEM_DCFILT_OFF:1;
} cc1101_MDMCFG2_reg_t;

#define CC1101_MDMCFG1                   0x13
typedef struct {
  uint8_t CHANSPC_E:2;
  uint8_t unused_r0:2;
  uint8_t NUM_PREAMBLE:3;
  uint8_t FEC_EN:1;
} cc1101_MDMCFG1_reg_t;

#define CC1101_MDMCFG0                   0x14
typedef struct {
  uint8_t CHANSPC_M:8;
} cc1101_MDMCFG0_reg_t;

#define CC1101_DEVIATN                   0x15 // [R/W] modem deviation setting
typedef struct {
  uint8_t DEVIATION_M:3;
  uint8_t unused_r0_2:1;
  uint8_t DEVIATION_E:3;
  uint8_t unused_r0_1:1;
} cc1101_DEVIATN_reg_t;

#define CC1101_MCSM2                     0x16 // [R/W] main radio control state machine config
typedef struct {
  uint8_t RX_TIME:3;
  uint8_t RX_TIME_QUAL:1;
  uint8_t RX_TIME_RSSI:1;
  uint8_t unused_r0:3;
} cc1101_MCSM2_reg_t;

#define CC1101_MCSM1                     0x17
typedef struct {
  uint8_t TXOFF_MODE:2;
  uint8_t RXOFF_MODE:2;
  uint8_t CCA_MODE:2;
  uint8_t unused_r0:2;
} cc1101_MCSM1_reg_t;

#define CC1101_MCSM0                     0x18
typedef struct {
  uint8_t XOSC_FORCE_ON:1;
  uint8_t PIN_CTRL_EN:1;
  uint8_t PO_TIMEOUT:2;
  uint8_t FS_AUTOCAL:2;
  uint8_t unused_r0:2;
} cc1101_MCSM0_reg_t;

#define CC1101_FOCCFG                    0x19 // [R/W] frequency offset compensation config
typedef struct {
  uint8_t FOC_LIMIT:2;
  uint8_t FOC_POST_K:1;
  uint8_t FOC_PRE_K:2;
  uint8_t FOC_BS_CS_GATE:1;
  uint8_t unused_r0:2;
} cc1101_FOCCFG_reg_t;

#define CC1101_BSCFG                     0x1A // [R/W] bit synchronization configuration
typedef struct {
  uint8_t BS_LIMIT:2;
  uint8_t BS_POST_KP:1;
  uint8_t BS_POST_KI:1;
  uint8_t BS_PRE_KP:2;
  uint8_t BS_PRE_KI:2;
} cc1101_BSCFG_reg_t;

#define CC1101_AGCCTRL2                  0x1B // [R/W] AGC control
typedef struct {
  uint8_t MAGN_TARGET:3;
  uint8_t MAX_LNA_GAIN:3;
  uint8_t MAX_DVGA_GAIN:2;
} cc1101_AGCCTRL2_reg_t;

#define CC1101_AGCCTRL1                  0x1C
typedef struct {
  uint8_t CARRIER_SENSE_ABS_THR:4;
  uint8_t CARRIER_SENSE_REL_THR:2;
  uint8_t AGC_LNA_PRIORITY:1;
  uint8_t unused_r0:1;
} cc1101_AGCCTRL1_reg_t;


#define CC1101_AGCCTRL0                  0x1D
typedef struct {
  uint8_t FILTER_LENGTH:2;
  uint8_t AGC_FREEZE:2;
  uint8_t WAIT_TIME:2;
  uint8_t HYST_LEVEL:2;
} cc1101_AGCCTRL0_reg_t;

#define CC1101_WOREVT1                   0x1E // [R/W] high byte event 0 timeout
typedef struct {
  uint8_t EVENT0:8;
} cc1101_WOREVT1_reg_t;

#define CC1101_WOREVT0                   0x1F // [R/W] low byte event 0 timeout
typedef struct {
  uint8_t EVENT0:8;
} cc1101_WOREVT0_reg_t;

#define CC1101_WORCTRL                   0x20 // [R/W] wake on radio control
typedef struct {
  uint8_t WOR_RES:2;
  uint8_t unused_r0:1;
  uint8_t RC_CAL:1;
  uint8_t EVENT1:3;
  uint8_t RC_PD:1;
} cc1101_WORCTRL_reg_t;

#define CC1101_FREND1                    0x21 // [R/W] front end rx config
typedef struct {
  uint8_t MIX_CURRENT:2;
  uint8_t LODIV_BUF_CURRENT_RX:2;
  uint8_t LNA2MIX_CURRENT:2;
  uint8_t LNA_CURRENT:2; 
} cc1101_FREND1_reg_t;


#define CC1101_FREND0                    0x22 // [R/W] front ent tx config
typedef struct {
  uint8_t PA_POWER:3;
  uint8_t unused_r0_2:1;
  uint8_t LODIV_BUF_CURRENT_TX:2;
  uint8_t unused_r0_1:2;
} cc1101_FREND0_reg_t;

#define CC1101_FSCAL3                    0x23 // [R/W] frequency synthesizer calibration
typedef struct {
  uint8_t FSCAL3_2:2;
  uint8_t CHP_CURR_CAL_EN:2;
  uint8_t FSCAL3_1:4;
} cc1101_FSCAL3_reg_t;


#define CC1101_FSCAL2                    0x24
typedef struct {
  uint8_t FSCAL2:2;
  uint8_t VCO_CORE_H_EN:1;
  uint8_t unused:5;
} cc1101_FSCAL2_reg_t;

#define CC1101_FSCAL1                    0x25
typedef struct {
  uint8_t FSCAL1:6;
  uint8_t unused:2;
} cc1101_FSCAL1_reg_t;

#define CC1101_FSCAL0                    0x26
typedef struct {
  uint8_t FSCAL0:7;
  uint8_t unused:1;
} cc1101_FSCAL0_reg_t;

#define CC1101_RCCTRL1                   0x27 // [R/W] rc oscillator configuration*
typedef struct {
  uint8_t RCCTRL1:7;
  uint8_t unused_r0:1;
} cc1101_RCCTRL1_reg_t;

#define CC1101_RCCTRL0                   0x28 
typedef struct {
  uint8_t RCCTRL0:7;
  uint8_t unused_r0:1;
} cc1101_RCCTRL0_reg_t;

#define CC1101_FSTEST                    0x29 // [R/W] frequency synthesizer calibration control
typedef struct {
  uint8_t FSTEST:8;
} cc1101_FSTEST_reg_t;


#define CC1101_PTEST                     0x2A // [R/W] production test
typedef struct {
  uint8_t PTEST:8;
} cc1101_PTEST_reg_t;

#define CC1101_AGCTEST                   0x2B // [R/W] AGC test
typedef struct {
  uint8_t AGCTEST:8;
} cc1101_AGCTEST_reg_t;


#define CC1101_TEST2                     0x2C // [R/W] various test settings
typedef struct {
  uint8_t TEST2:8;
} cc1101_TEST2_reg_t;


#define CC1101_TEST1                     0x2D
typedef struct {
  uint8_t TEST1:8;
} cc1101_TEST1_reg_t;

#define CC1101_TEST0                     0x2E
typedef struct {
  uint8_t TEST0_2:1;
  uint8_t VCO_SEL_CAL_EN:1;
  uint8_t TEST0:6;
} cc1101_TEST0_reg_t;



//========================= Status Registers ====================


#define CC1101_PARTNUM                   0x30 // [R  ] part number 
typedef struct {
  uint8_t PARTNUM:8;
} cc1101_PARTNUM_reg_t;

#define CC1101_VERSION                   0x31 // [R  ] version number
typedef struct {
  uint8_t VERSION:8;
} cc1101_VERSION_reg_t;

#define CC1101_FREQEST                   0x32 // [R  ] frequency offset estimate 
typedef struct {
  uint8_t FREQOFF_EST:8;
} cc1101_FREQEST_reg_t;

#define CC1101_LQI                       0x33 // [R  ] Demodulator estimate for link quality
typedef struct {
  uint8_t LQI_EST:7;
  uint8_t CRC_OK:1;
} cc1101_LQI_reg_t;

#define CC1101_RSSI                      0x34 // [R  ] received signal strength indication
typedef struct {
  uint8_t RSSI:8;
} cc1101_RSSI_reg_t;

#define CC1101_MARCSTATE                 0x35 // [R  ] control state machine state 
typedef struct {
  uint8_t MARC_STATE:5;
  uint8_t unused:3;
} cc1101_MARCSTATE_reg_t;

#define CC1101_WORTIME1                  0x36 // [R  ] high byte of WOR timer
typedef struct {
  uint8_t TIME:8;
} cc1101_WORTIME1_reg_t;

#define CC1101_WORTIME0                  0x37 // [R  ] low byte of WOR timer
typedef struct {
  uint8_t TIME:8;
} cc1101_WORTIME0_reg_t;

#define CC1101_PKTSTATUS                 0x38 // [R  ] packet status
typedef struct {
  uint8_t GDO0:1;
  uint8_t unused:1;
  uint8_t GDO2:1;
  uint8_t SFD:1;
  uint8_t CCA:1;
  uint8_t PQT_REACHED:1;
  uint8_t CS:1;
  uint8_t CRC_OK:1;
} cc1101_PKSTATUS_reg_t;

#define CC1101_VCO_VC_DAC                0x39 // [R  ] setting from pll calibration 
typedef struct {
  uint8_t VCO_VC_DAC:8;
} cc1101_VCO_VC_DAC_reg_t;


#define CC1101_TXBYTES                   0x3A // [R  ] underflow and number of bytes in tx fifo
typedef struct {
  uint8_t NUM_TXBYTES:7;
  uint8_t TXFIFO_UNDERFLOW:1;
} cc1101_TXBYTES_reg_t;

#define CC1101_RXBYTES                   0x3B // [R  ] overflow and number of byte in rx fifo
typedef struct {
  uint8_t NUM_RXBYTES:7;
  uint8_t RXFIFO_OVERFLOW:1;
} cc1101_RXBYTES_reg_t;

#define CC1101_RCCTRL1_STATUS            0x3C // [R  ] last rc oscillator calibration result
typedef struct {
  uint8_t RCCTRL1_STATUS:7;
  uint8_t unused:1;
} cc1101_RCCTRL1_STATUS_reg_t;


#define CC1101_RCCTRL0_STATUS            0x3D
typedef struct {
  uint8_t RCCTRL0_STATUS:7;
  uint8_t unused:1;
} cc1101_RCCTRL0_STATUS_reg_t;



// PATABLE address
#define CC1101_PATABLE_ADDR              0x3E

// TXFIFO and RXFIFO address
#define CC1101_FIFO_ADDR                 0x3F


#endif
