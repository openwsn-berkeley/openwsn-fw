//AHB Peripheral
#define     AHB_BOOTLOAD_BASE           0x01000000
#define     AHB_DATAMEM_BASE            0x20000000
#define     AHB_RF_BASE                 0x40000000
#define     AHB_DMA_BASE                0x41000000
#define     AHB_RFTIMER_BASE            0x42000000

//APB Peripheral
#define     APB_ADC_BASE                0x50000000
#define     APB_UART_BASE               0x51000000
#define     APB_ANALOG_CFG_BASE         0x52000000
#define     APB_GPIO_BASE               0x53000000



//RFCONTRLLER Registers
#define RFCONTROLLER_REG__CONTROL       *(unsigned int*)(AHB_RF_BASE + 0x00)
#define RFCONTROLLER_REG__STATUS        *(unsigned int*)(AHB_RF_BASE + 0x04)
#define RFCONTROLLER_REG__TX_DATA_ADDR  *(char**)(AHB_RF_BASE + 0x08)
#define RFCONTROLLER_REG__TX_PACK_LEN   *(unsigned int*)(AHB_RF_BASE + 0x0C)
#define RFCONTROLLER_REG__INT           *(unsigned int*)(AHB_RF_BASE + 0x10)
#define RFCONTROLLER_REG__INT_CONFIG    *(unsigned int*)(AHB_RF_BASE + 0x14)
#define RFCONTROLLER_REG__INT_CLEAR     *(unsigned int*)(AHB_RF_BASE + 0x18)
#define RFCONTROLLER_REG__ERROR         *(unsigned int*)(AHB_RF_BASE + 0x1C)
#define RFCONTROLLER_REG__ERROR_CONFIG  *(unsigned int*)(AHB_RF_BASE + 0x20)
#define RFCONTROLLER_REG__ERROR_CLEAR   *(unsigned int*)(AHB_RF_BASE + 0x24)

//RFTIMER Registers
#define RFTIMER_REG__CONTROL            *(unsigned int*)(AHB_RFTIMER_BASE + 0x00)
#define RFTIMER_REG__COUNTER            *(unsigned int*)(AHB_RFTIMER_BASE + 0x04)
#define RFTIMER_REG__MAX_COUNT          *(unsigned int*)(AHB_RFTIMER_BASE + 0x08)
#define RFTIMER_REG__COMPARE0           *(unsigned int*)(AHB_RFTIMER_BASE + 0x10)
#define RFTIMER_REG__COMPARE1           *(unsigned int*)(AHB_RFTIMER_BASE + 0x14)
#define RFTIMER_REG__COMPARE2           *(unsigned int*)(AHB_RFTIMER_BASE + 0x18)
#define RFTIMER_REG__COMPARE3           *(unsigned int*)(AHB_RFTIMER_BASE + 0x1C)
#define RFTIMER_REG__COMPARE4           *(unsigned int*)(AHB_RFTIMER_BASE + 0x20)
#define RFTIMER_REG__COMPARE5           *(unsigned int*)(AHB_RFTIMER_BASE + 0x24)
#define RFTIMER_REG__COMPARE6           *(unsigned int*)(AHB_RFTIMER_BASE + 0x28)
#define RFTIMER_REG__COMPARE7           *(unsigned int*)(AHB_RFTIMER_BASE + 0x2C)
#define RFTIMER_REG__COMPARE0_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x30)
#define RFTIMER_REG__COMPARE1_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x34)
#define RFTIMER_REG__COMPARE2_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x38)
#define RFTIMER_REG__COMPARE3_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x3C)
#define RFTIMER_REG__COMPARE4_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x40)
#define RFTIMER_REG__COMPARE5_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x44)
#define RFTIMER_REG__COMPARE6_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x48)
#define RFTIMER_REG__COMPARE7_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x4C)
#define RFTIMER_REG__CAPTURE0           *(unsigned int*)(AHB_RFTIMER_BASE + 0x50)
#define RFTIMER_REG__CAPTURE1           *(unsigned int*)(AHB_RFTIMER_BASE + 0x54)
#define RFTIMER_REG__CAPTURE2           *(unsigned int*)(AHB_RFTIMER_BASE + 0x58)
#define RFTIMER_REG__CAPTURE3           *(unsigned int*)(AHB_RFTIMER_BASE + 0x5C)
#define RFTIMER_REG__CAPTURE0_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x60)
#define RFTIMER_REG__CAPTURE1_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x64)
#define RFTIMER_REG__CAPTURE2_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x68)
#define RFTIMER_REG__CAPTURE3_CONTROL   *(unsigned int*)(AHB_RFTIMER_BASE + 0x6C)
#define RFTIMER_REG__INT                *(unsigned int*)(AHB_RFTIMER_BASE + 0x70)
#define RFTIMER_REG__INT_CLEAR          *(unsigned int*)(AHB_RFTIMER_BASE + 0x74)

//DMA Registers
#define DMA_REG__RF_RX_ADDR             *(char**)(AHB_DMA_BASE + 0x14)

//ADC Registers
#define ADC_REG__START                  *(unsigned int*)(APB_ADC_BASE + 0x000000)
#define ADC_REG__DATA                   *(unsigned int*)(APB_ADC_BASE + 0x040000)

//UART Registers
#define UART_REG__TX_DATA               *(unsigned int*)(APB_UART_BASE)
#define UART_REG__RX_DATA               *(unsigned int*)(APB_UART_BASE)
    
//GPIO Registers
#define GPIO_REG__INPUT                 *(unsigned int*)(APB_GPIO_BASE + 0x000000)
#define GPIO_REG__OUTPUT                *(unsigned int*)(APB_GPIO_BASE + 0x040000)
