/**
\brief Definitions for the Atmel AT86RF231 radio chip.
*/
 
#ifndef __ATMEL_H
#define __ATMEL_H

//=========================== define ==========================================

enum radio_antennaselection_enum {
   RADIO_UFL_ANTENNA              = 0x06, ///< Use the antenna connected by U.FL.
   RADIO_CHIP_ANTENNA             = 0x05, ///< Use the on-board chip antenna.
};

/**
\brief Possible values for the status of the radio.

After you get an interrupt from the radio, read the status register
(<tt>RG_IRQ_STATUS</tt>) to know what type it is, amoung the following.
*/
enum radio_irqstatus_enum {
   AT_IRQ_BAT_LOW                 = 0x80,   //< Supply voltage below the programmed threshold.
   AT_IRQ_TRX_UR                  = 0x40,   ///< Frame buffer access violation.
   AT_IRQ_AMI                     = 0x20,   ///< Address matching.
   AT_IRQ_CCA_ED_DONE             = 0x10,   ///< End of a CCA or ED measurement.
   AT_IRQ_TRX_END                 = 0x08,   ///< Completion of a frame transmission/reception.
   AT_IRQ_RX_START                = 0x04,   ///< Start of a PSDU reception.
   AT_IRQ_PLL_UNLOCK              = 0x02,   ///< PLL unlock.
   AT_IRQ_PLL_LOCK                = 0x01,   ///< PLL lock.
};

#define HAVE_REGISTER_MAP (1)
/** Offset for register TRX_STATUS
 * @ingroup apiHalPHY230Reg
 */
#define RG_TRX_STATUS                    (0x01)
  /** Access parameters for sub-register CCA_DONE in register @ref RG_TRX_STATUS
   * @ingroup apiHalPHY230Sreg
   */
# define SR_CCA_DONE                  0x01, 0x80, 7
  /** Access parameters for sub-register CCA_STATUS in register @ref RG_TRX_STATUS
   * @ingroup apiHalPHY230Sreg
   */
# define SR_CCA_STATUS                0x01, 0x40, 6
# define SR_reserved_01_3             0x01, 0x20, 5
  /** Access parameters for sub-register TRX_STATUS in register @ref RG_TRX_STATUS
   * @ingroup apiHalPHY230Sreg
   */
# define SR_TRX_STATUS                0x01, 0x1f, 0
   /** Constant P_ON for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define P_ON                     (0)
   /** Constant BUSY_RX for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define BUSY_RX                  (1)
   /** Constant BUSY_TX for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define BUSY_TX                  (2)
   /** Constant RX_ON for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define RX_ON                    (6)
   /** Constant TRX_OFF for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define TRX_OFF                  (8)
   /** Constant PLL_ON for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define PLL_ON                   (9)
   /** Constant SLEEP for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define SLEEP                    (15)
   /** Constant BUSY_RX_AACK for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define BUSY_RX_AACK             (17)
   /** Constant BUSY_TX_ARET for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define BUSY_TX_ARET             (18)
   /** Constant RX_AACK_ON for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define RX_AACK_ON               (22)
   /** Constant TX_ARET_ON for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define TX_ARET_ON               (25)
   /** Constant RX_ON_NOCLK for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define RX_ON_NOCLK              (28)
   /** Constant RX_AACK_ON_NOCLK for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define RX_AACK_ON_NOCLK         (29)
   /** Constant BUSY_RX_AACK_NOCLK for sub-register @ref SR_TRX_STATUS
    * @ingroup apiHalPHY230Const
    */
#  define BUSY_RX_AACK_NOCLK       (30)

/** Offset for register TRX_STATE
 * @ingroup apiHalPHY230Reg
 */
#define RG_TRX_STATE                     (0x02)
  /** Access parameters for sub-register TRAC_STATUS in register @ref RG_TRX_STATE
   * @ingroup apiHalPHY230Sreg
   */
# define SR_TRAC_STATUS               0x02, 0xe0, 5
  /** Access parameters for sub-register TRX_CMD in register @ref RG_TRX_STATE
   * @ingroup apiHalPHY230Sreg
   */
# define SR_TRX_CMD                   0x02, 0x1f, 0
   /** Constant CMD_NOP for sub-register @ref SR_TRX_CMD
    * @ingroup apiHalPHY230Const
    */
#  define CMD_NOP                  (0)
   /** Constant CMD_TX_START for sub-register @ref SR_TRX_CMD
    * @ingroup apiHalPHY230Const
    */
#  define CMD_TX_START             (2)
   /** Constant CMD_FORCE_TRX_OFF for sub-register @ref SR_TRX_CMD
    * @ingroup apiHalPHY230Const
    */
#  define CMD_FORCE_TRX_OFF        (3)
   /** Constant CMD_RX_ON for sub-register @ref SR_TRX_CMD
    * @ingroup apiHalPHY230Const
    */
#  define CMD_RX_ON                (6)
   /** Constant CMD_TRX_OFF for sub-register @ref SR_TRX_CMD
    * @ingroup apiHalPHY230Const
    */
#  define CMD_TRX_OFF              (8)
   /** Constant CMD_PLL_ON for sub-register @ref SR_TRX_CMD
    * @ingroup apiHalPHY230Const
    */
#  define CMD_PLL_ON               (9)
   /** Constant CMD_RX_AACK_ON for sub-register @ref SR_TRX_CMD
    * @ingroup apiHalPHY230Const
    */
#  define CMD_RX_AACK_ON           (22)
   /** Constant CMD_TX_ARET_ON for sub-register @ref SR_TRX_CMD
    * @ingroup apiHalPHY230Const
    */
#  define CMD_TX_ARET_ON           (25)

/** Offset for register TRX_CTRL_0
 * @ingroup apiHalPHY230Reg
 */
#define RG_TRX_CTRL_0                    (0x03)
  /** Access parameters for sub-register PAD_IO in register @ref RG_TRX_CTRL_0
   * @ingroup apiHalPHY230Sreg
   */
# define SR_PAD_IO                    0x03, 0xc0, 6
  /** Access parameters for sub-register PAD_IO_CLKM in register @ref RG_TRX_CTRL_0
   * @ingroup apiHalPHY230Sreg
   */
# define SR_PAD_IO_CLKM               0x03, 0x30, 4
   /** Constant CLKM_2mA for sub-register @ref SR_PAD_IO_CLKM
    * @ingroup apiHalPHY230Const
    */
#  define CLKM_2mA                 (0)
   /** Constant CLKM_4mA for sub-register @ref SR_PAD_IO_CLKM
    * @ingroup apiHalPHY230Const
    */
#  define CLKM_4mA                 (1)
   /** Constant CLKM_6mA for sub-register @ref SR_PAD_IO_CLKM
    * @ingroup apiHalPHY230Const
    */
#  define CLKM_6mA                 (2)
   /** Constant CLKM_8mA for sub-register @ref SR_PAD_IO_CLKM
    * @ingroup apiHalPHY230Const
    */
#  define CLKM_8mA                 (3)
  /** Access parameters for sub-register CLKM_SHA_SEL in register @ref RG_TRX_CTRL_0
   * @ingroup apiHalPHY230Sreg
   */
# define SR_CLKM_SHA_SEL              0x03, 0x08, 3
  /** Access parameters for sub-register CLKM_CTRL in register @ref RG_TRX_CTRL_0
   * @ingroup apiHalPHY230Sreg
   */
# define SR_CLKM_CTRL                 0x03, 0x07, 0
   /** Constant CLKM_no_clock for sub-register @ref SR_CLKM_CTRL
    * @ingroup apiHalPHY230Const
    */
#  define CLKM_no_clock            (0)
   /** Constant CLKM_1MHz for sub-register @ref SR_CLKM_CTRL
    * @ingroup apiHalPHY230Const
    */
#  define CLKM_1MHz                (1)
   /** Constant CLKM_2MHz for sub-register @ref SR_CLKM_CTRL
    * @ingroup apiHalPHY230Const
    */
#  define CLKM_2MHz                (2)
   /** Constant CLKM_4MHz for sub-register @ref SR_CLKM_CTRL
    * @ingroup apiHalPHY230Const
    */
#  define CLKM_4MHz                (3)
   /** Constant CLKM_8MHz for sub-register @ref SR_CLKM_CTRL
    * @ingroup apiHalPHY230Const
    */
#  define CLKM_8MHz                (4)
   /** Constant CLKM_16MHz for sub-register @ref SR_CLKM_CTRL
    * @ingroup apiHalPHY230Const
    */
#  define CLKM_16MHz               (5)

/** Offset for register PHY_TX_PWR
 * @ingroup apiHalPHY230Reg
 */
#define RG_PHY_TX_PWR                    (0x05)
  /** Access parameters for sub-register TX_AUTO_CRC_ON in register @ref RG_PHY_TX_PWR
   * @ingroup apiHalPHY230Sreg
   */
# define SR_TX_AUTO_CRC_ON            0x05, 0x80, 7
# define SR_reserved_05_2             0x05, 0x70, 4
  /** Access parameters for sub-register TX_PWR in register @ref RG_PHY_TX_PWR
   * @ingroup apiHalPHY230Sreg
   */
# define SR_TX_PWR                    0x05, 0x0f, 0

/** Offset for register PHY_RSSI
 * @ingroup apiHalPHY230Reg
 */
#define RG_PHY_RSSI                      (0x06)
# define SR_reserved_06_1             0x06, 0xe0, 5
  /** Access parameters for sub-register RSSI in register @ref RG_PHY_RSSI
   * @ingroup apiHalPHY230Sreg
   */
# define SR_RSSI                      0x06, 0x1f, 0

/** Offset for register PHY_ED_LEVEL
 * @ingroup apiHalPHY230Reg
 */
#define RG_PHY_ED_LEVEL                  (0x07)
  /** Access parameters for sub-register ED_LEVEL in register @ref RG_PHY_ED_LEVEL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_ED_LEVEL                  0x07, 0xff, 0

/** Offset for register PHY_CC_CCA
 * @ingroup apiHalPHY230Reg
 */
#define RG_PHY_CC_CCA                    (0x08)
  /** Access parameters for sub-register CCA_REQUEST in register @ref RG_PHY_CC_CCA
   * @ingroup apiHalPHY230Sreg
   */
# define SR_CCA_REQUEST               0x08, 0x80, 7
  /** Access parameters for sub-register CCA_MODE in register @ref RG_PHY_CC_CCA
   * @ingroup apiHalPHY230Sreg
   */
# define SR_CCA_MODE                  0x08, 0x60, 5
  /** Access parameters for sub-register CHANNEL in register @ref RG_PHY_CC_CCA
   * @ingroup apiHalPHY230Sreg
   */
# define SR_CHANNEL                   0x08, 0x1f, 0

/** Offset for register CCA_THRES
 * @ingroup apiHalPHY230Reg
 */
#define RG_CCA_THRES                     (0x09)
  /** Access parameters for sub-register CCA_CS_THRES in register @ref RG_CCA_THRES
   * @ingroup apiHalPHY230Sreg
   */
# define SR_CCA_CS_THRES              0x09, 0xf0, 4
  /** Access parameters for sub-register CCA_ED_THRES in register @ref RG_CCA_THRES
   * @ingroup apiHalPHY230Sreg
   */
# define SR_CCA_ED_THRES              0x09, 0x0f, 0

/** Offset for register IRQ_MASK
 * @ingroup apiHalPHY230Reg
 */
#define RG_IRQ_MASK                      (0x0e)
  /** Access parameters for sub-register IRQ_MASK in register @ref RG_IRQ_MASK
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IRQ_MASK                  0x0e, 0xff, 0

/** Offset for register IRQ_STATUS
 * @ingroup apiHalPHY230Reg
 */
#define RG_IRQ_STATUS                    (0x0f)
  /** Access parameters for sub-register IRQ_7_BAT_LOW in register @ref RG_IRQ_STATUS
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IRQ_7_BAT_LOW             0x0f, 0x80, 7
  /** Access parameters for sub-register IRQ_6_TRX_UR in register @ref RG_IRQ_STATUS
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IRQ_6_TRX_UR              0x0f, 0x40, 6
  /** Access parameters for sub-register IRQ_5 in register @ref RG_IRQ_STATUS
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IRQ_5                     0x0f, 0x20, 5
  /** Access parameters for sub-register IRQ_4 in register @ref RG_IRQ_STATUS
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IRQ_4                     0x0f, 0x10, 4
  /** Access parameters for sub-register IRQ_3_TRX_END in register @ref RG_IRQ_STATUS
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IRQ_3_TRX_END             0x0f, 0x08, 3
  /** Access parameters for sub-register IRQ_2_RX_START in register @ref RG_IRQ_STATUS
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IRQ_2_RX_START            0x0f, 0x04, 2
  /** Access parameters for sub-register IRQ_1_PLL_UNLOCK in register @ref RG_IRQ_STATUS
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IRQ_1_PLL_UNLOCK          0x0f, 0x02, 1
  /** Access parameters for sub-register IRQ_0_PLL_LOCK in register @ref RG_IRQ_STATUS
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IRQ_0_PLL_LOCK            0x0f, 0x01, 0

/** Offset for register VREG_CTRL
 * @ingroup apiHalPHY230Reg
 */
#define RG_VREG_CTRL                     (0x10)
  /** Access parameters for sub-register AVREG_EXT in register @ref RG_VREG_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_AVREG_EXT                 0x10, 0x80, 7
  /** Access parameters for sub-register AVDD_OK in register @ref RG_VREG_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_AVDD_OK                   0x10, 0x40, 6
  /** Access parameters for sub-register AVREG_TRIM in register @ref RG_VREG_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_AVREG_TRIM                0x10, 0x30, 4
   /** Constant AVREG_1_80V for sub-register @ref SR_AVREG_TRIM
    * @ingroup apiHalPHY230Const
    */
#  define AVREG_1_80V              (0)
   /** Constant AVREG_1_75V for sub-register @ref SR_AVREG_TRIM
    * @ingroup apiHalPHY230Const
    */
#  define AVREG_1_75V              (1)
   /** Constant AVREG_1_84V for sub-register @ref SR_AVREG_TRIM
    * @ingroup apiHalPHY230Const
    */
#  define AVREG_1_84V              (2)
   /** Constant AVREG_1_88V for sub-register @ref SR_AVREG_TRIM
    * @ingroup apiHalPHY230Const
    */
#  define AVREG_1_88V              (3)
  /** Access parameters for sub-register DVREG_EXT in register @ref RG_VREG_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_DVREG_EXT                 0x10, 0x08, 3
  /** Access parameters for sub-register DVDD_OK in register @ref RG_VREG_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_DVDD_OK                   0x10, 0x04, 2
  /** Access parameters for sub-register DVREG_TRIM in register @ref RG_VREG_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_DVREG_TRIM                0x10, 0x03, 0
   /** Constant DVREG_1_80V for sub-register @ref SR_DVREG_TRIM
    * @ingroup apiHalPHY230Const
    */
#  define DVREG_1_80V              (0)
   /** Constant DVREG_1_75V for sub-register @ref SR_DVREG_TRIM
    * @ingroup apiHalPHY230Const
    */
#  define DVREG_1_75V              (1)
   /** Constant DVREG_1_84V for sub-register @ref SR_DVREG_TRIM
    * @ingroup apiHalPHY230Const
    */
#  define DVREG_1_84V              (2)
   /** Constant DVREG_1_88V for sub-register @ref SR_DVREG_TRIM
    * @ingroup apiHalPHY230Const
    */
#  define DVREG_1_88V              (3)

/** Offset for register BATMON
 * @ingroup apiHalPHY230Reg
 */
#define RG_BATMON                        (0x11)
# define SR_reserved_11_1             0x11, 0xc0, 6
  /** Access parameters for sub-register BATMON_OK in register @ref RG_BATMON
   * @ingroup apiHalPHY230Sreg
   */
# define SR_BATMON_OK                 0x11, 0x20, 5
  /** Access parameters for sub-register BATMON_HR in register @ref RG_BATMON
   * @ingroup apiHalPHY230Sreg
   */
# define SR_BATMON_HR                 0x11, 0x10, 4
  /** Access parameters for sub-register BATMON_VTH in register @ref RG_BATMON
   * @ingroup apiHalPHY230Sreg
   */
# define SR_BATMON_VTH                0x11, 0x0f, 0

/** Offset for register XOSC_CTRL
 * @ingroup apiHalPHY230Reg
 */
#define RG_XOSC_CTRL                     (0x12)
  /** Access parameters for sub-register XTAL_MODE in register @ref RG_XOSC_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_XTAL_MODE                 0x12, 0xf0, 4
  /** Access parameters for sub-register XTAL_TRIM in register @ref RG_XOSC_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_XTAL_TRIM                 0x12, 0x0f, 0

/** Offset for register FTN_CTRL
 * @ingroup apiHalPHY230Reg
 */
#define RG_FTN_CTRL                      (0x18)
  /** Access parameters for sub-register FTN_START in register @ref RG_FTN_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_FTN_START                 0x18, 0x80, 7
# define SR_reserved_18_2             0x18, 0x40, 6
  /** Access parameters for sub-register FTNV in register @ref RG_FTN_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_FTNV                      0x18, 0x3f, 0

/** Offset for register PLL_CF
 * @ingroup apiHalPHY230Reg
 */
#define RG_PLL_CF                        (0x1a)
  /** Access parameters for sub-register PLL_CF_START in register @ref RG_PLL_CF
   * @ingroup apiHalPHY230Sreg
   */
# define SR_PLL_CF_START              0x1a, 0x80, 7
# define SR_reserved_1a_2             0x1a, 0x70, 4
  /** Access parameters for sub-register PLL_CF in register @ref RG_PLL_CF
   * @ingroup apiHalPHY230Sreg
   */
# define SR_PLL_CF                    0x1a, 0x0f, 0

/** Offset for register PLL_DCU
 * @ingroup apiHalPHY230Reg
 */
#define RG_PLL_DCU                       (0x1b)
  /** Access parameters for sub-register PLL_DCU_START in register @ref RG_PLL_DCU
   * @ingroup apiHalPHY230Sreg
   */
# define SR_PLL_DCU_START             0x1b, 0x80, 7
# define SR_reserved_1b_2             0x1b, 0x40, 6
  /** Access parameters for sub-register PLL_DCUW in register @ref RG_PLL_DCU
   * @ingroup apiHalPHY230Sreg
   */
# define SR_PLL_DCUW                  0x1b, 0x3f, 0

/** Offset for register PART_NUM
 * @ingroup apiHalPHY230Reg
 */
#define RG_PART_NUM                      (0x1c)
  /** Access parameters for sub-register PART_NUM in register @ref RG_PART_NUM
   * @ingroup apiHalPHY230Sreg
   */
# define SR_PART_NUM                  0x1c, 0xff, 0
   /** Constant RF230 for sub-register @ref SR_PART_NUM
    * @ingroup apiHalPHY230Const
    */
#  define RF230                    (2)

/** Offset for register VERSION_NUM
 * @ingroup apiHalPHY230Reg
 */
#define RG_VERSION_NUM                   (0x1d)
  /** Access parameters for sub-register VERSION_NUM in register @ref RG_VERSION_NUM
   * @ingroup apiHalPHY230Sreg
   */
# define SR_VERSION_NUM               0x1d, 0xff, 0

/** Offset for register MAN_ID_0
 * @ingroup apiHalPHY230Reg
 */
#define RG_MAN_ID_0                      (0x1e)
  /** Access parameters for sub-register MAN_ID_0 in register @ref RG_MAN_ID_0
   * @ingroup apiHalPHY230Sreg
   */
# define SR_MAN_ID_0                  0x1e, 0xff, 0

/** Offset for register MAN_ID_1
 * @ingroup apiHalPHY230Reg
 */
#define RG_MAN_ID_1                      (0x1f)
  /** Access parameters for sub-register MAN_ID_1 in register @ref RG_MAN_ID_1
   * @ingroup apiHalPHY230Sreg
   */
# define SR_MAN_ID_1                  0x1f, 0xff, 0

/** Offset for register SHORT_ADDR_0
 * @ingroup apiHalPHY230Reg
 */
#define RG_SHORT_ADDR_0                  (0x20)
  /** Access parameters for sub-register SHORT_ADDR_0 in register @ref RG_SHORT_ADDR_0
   * @ingroup apiHalPHY230Sreg
   */
# define SR_SHORT_ADDR_0              0x20, 0xff, 0

/** Offset for register SHORT_ADDR_1
 * @ingroup apiHalPHY230Reg
 */
#define RG_SHORT_ADDR_1                  (0x21)
  /** Access parameters for sub-register SHORT_ADDR_1 in register @ref RG_SHORT_ADDR_1
   * @ingroup apiHalPHY230Sreg
   */
# define SR_SHORT_ADDR_1              0x21, 0xff, 0

/** Offset for register PAN_ID_0
 * @ingroup apiHalPHY230Reg
 */
#define RG_PAN_ID_0                      (0x22)
  /** Access parameters for sub-register PAN_ID_0 in register @ref RG_PAN_ID_0
   * @ingroup apiHalPHY230Sreg
   */
# define SR_PAN_ID_0                  0x22, 0xff, 0

/** Offset for register PAN_ID_1
 * @ingroup apiHalPHY230Reg
 */
#define RG_PAN_ID_1                      (0x23)
  /** Access parameters for sub-register PAN_ID_1 in register @ref RG_PAN_ID_1
   * @ingroup apiHalPHY230Sreg
   */
# define SR_PAN_ID_1                  0x23, 0xff, 0

/** Offset for register IEEE_ADDR_0
 * @ingroup apiHalPHY230Reg
 */
#define RG_IEEE_ADDR_0                   (0x24)
  /** Access parameters for sub-register IEEE_ADDR_0 in register @ref RG_IEEE_ADDR_0
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IEEE_ADDR_0               0x24, 0xff, 0

/** Offset for register IEEE_ADDR_1
 * @ingroup apiHalPHY230Reg
 */
#define RG_IEEE_ADDR_1                   (0x25)
  /** Access parameters for sub-register IEEE_ADDR_1 in register @ref RG_IEEE_ADDR_1
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IEEE_ADDR_1               0x25, 0xff, 0

/** Offset for register IEEE_ADDR_2
 * @ingroup apiHalPHY230Reg
 */
#define RG_IEEE_ADDR_2                   (0x26)
  /** Access parameters for sub-register IEEE_ADDR_2 in register @ref RG_IEEE_ADDR_2
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IEEE_ADDR_2               0x26, 0xff, 0

/** Offset for register IEEE_ADDR_3
 * @ingroup apiHalPHY230Reg
 */
#define RG_IEEE_ADDR_3                   (0x27)
  /** Access parameters for sub-register IEEE_ADDR_3 in register @ref RG_IEEE_ADDR_3
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IEEE_ADDR_3               0x27, 0xff, 0

/** Offset for register IEEE_ADDR_4
 * @ingroup apiHalPHY230Reg
 */
#define RG_IEEE_ADDR_4                   (0x28)
  /** Access parameters for sub-register IEEE_ADDR_4 in register @ref RG_IEEE_ADDR_4
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IEEE_ADDR_4               0x28, 0xff, 0

/** Offset for register IEEE_ADDR_5
 * @ingroup apiHalPHY230Reg
 */
#define RG_IEEE_ADDR_5                   (0x29)
  /** Access parameters for sub-register IEEE_ADDR_5 in register @ref RG_IEEE_ADDR_5
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IEEE_ADDR_5               0x29, 0xff, 0

/** Offset for register IEEE_ADDR_6
 * @ingroup apiHalPHY230Reg
 */
#define RG_IEEE_ADDR_6                   (0x2a)
  /** Access parameters for sub-register IEEE_ADDR_6 in register @ref RG_IEEE_ADDR_6
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IEEE_ADDR_6               0x2a, 0xff, 0

/** Offset for register IEEE_ADDR_7
 * @ingroup apiHalPHY230Reg
 */
#define RG_IEEE_ADDR_7                   (0x2b)
  /** Access parameters for sub-register IEEE_ADDR_7 in register @ref RG_IEEE_ADDR_7
   * @ingroup apiHalPHY230Sreg
   */
# define SR_IEEE_ADDR_7               0x2b, 0xff, 0

/** Offset for register XAH_CTRL
 * @ingroup apiHalPHY230Reg
 */
#define RG_XAH_CTRL                      (0x2c)
  /** Access parameters for sub-register MAX_FRAME_RETRIES in register @ref RG_XAH_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_MAX_FRAME_RETRIES         0x2c, 0xf0, 4
  /** Access parameters for sub-register MAX_CSMA_RETRIES in register @ref RG_XAH_CTRL
   * @ingroup apiHalPHY230Sreg
   */
# define SR_MAX_CSMA_RETRIES          0x2c, 0x0e, 1
# define SR_reserved_2c_3             0x2c, 0x01, 0

/** Offset for register CSMA_SEED_0
 * @ingroup apiHalPHY230Reg
 */
#define RG_CSMA_SEED_0                   (0x2d)
  /** Access parameters for sub-register CSMA_SEED_0 in register @ref RG_CSMA_SEED_0
   * @ingroup apiHalPHY230Sreg
   */
# define SR_CSMA_SEED_0               0x2d, 0xff, 0

/** Offset for register CSMA_SEED_1
 * @ingroup apiHalPHY230Reg
 */
#define RG_CSMA_SEED_1                   (0x2e)
  /** Access parameters for sub-register MIN_BE in register @ref RG_CSMA_SEED_1
   * @ingroup apiHalPHY230Sreg
   */
# define SR_MIN_BE                    0x2e, 0xc0, 6
# define SR_reserved_2e_2             0x2e, 0x30, 4
  /** Access parameters for sub-register I_AM_COORD in register @ref RG_CSMA_SEED_1
   * @ingroup apiHalPHY230Sreg
   */
# define SR_I_AM_COORD                0x2e, 0x08, 3
  /** Access parameters for sub-register CSMA_SEED_1 in register @ref RG_CSMA_SEED_1
   * @ingroup apiHalPHY230Sreg
   */
# define SR_CSMA_SEED_1               0x2e, 0x07, 0

# define RG_ANT_DIV                      (0x0d)
//controls antenna diversity

# define RG_RX_CTRL                      (0x0a)
//controls the sensitivity of the antenna diversity mode

//=========================== typedef =========================================

//=========================== variables =======================================

//=========================== prototypes ======================================

#endif