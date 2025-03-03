/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      temperature.c
 *  Author:         Tomas Dolak
 *  Date:           11.02.2025
 *  Description:    Implements The Logic For Temperature Measurement.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           temperature.c
 *  @author         Tomas Dolak
 *  @date           11.02.2025
 *  @brief          Implements The Logic For Temperature Measurement.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "temperature.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * @brief 	I2C DMA Channel For Transmission.
 */
#define LPI2C_TX_DMA_CHANNEL       	0U

/**
 * @brief 	I2C DMA Channel For Reception.
 */
#define LPI2C_RX_DMA_CHANNEL    	1U

/**
 * @brief	Connection Between DMA Channel 0 and LP_FLEXCOMM5 Tx.
 */
#define LPI2C_TX_CHANNEL 			kDma0RequestMuxLpFlexcomm5Tx

/**
 * @brief	Connection Between DMA Channel 0 and LP_FLEXCOMM5 Rx.
 */
#define LPI2C_RX_EDMA_CHANNEL  		kDma0RequestMuxLpFlexcomm5Rx

/**
 * @brief Points To I2C Peripheral Unit (Specifically LPI2C5 Instance).
 */
#define I2C_MASTER 					((LPI2C_Type *)LPI2C5_BASE)

/**
 * @brief I2C Address of P3T1755 Temperature Sensor.
 */
#define P3T1755_ADDR_7BIT 			0x48U

/**
 * @brief 	Desired Baud Rate For I2C.
 * @details Frequency - 100kHz.
 */
#define I2C_BAUDRATE               	100000U
/**
 * @brief Enables Printout of P3T1755 Register.
 */
#define PRINT_REG_OUTPUT			true

/**
 * @brief Receive Buffer Size.
 */
#define BUFF_SIZE					2

/**
 * @brief Temperature Register Address of P3T1755.
 */
#define REGISTER_TEMPERATURE		0x00

/**
 * @brief Configuration Register Address of P3T1755.
 */
#define REGISTER_CONFIG				0x01

/*
 * @brief Temperature Low Register Address of P3T1755.
 */
#define REGISTER_THVST				0x02
/*
 * @brief Temperature High Register Address of P3T1755.
 */
#define REGISTER_TOS				0x03

/*******************************************************************************
 * Structures
 ******************************************************************************/


/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/**
 * @brief	Reception Buffer.
 * @details	Must Be In Non-Cacheable Memory Due To Usage of DMA.
 */
AT_NONCACHEABLE_SECTION(uint8_t gRxBuff[BUFF_SIZE]);

/**
 * @brief	eDMA Driver Handle Used For Non-Blocking DMA Transfer.
 */
AT_NONCACHEABLE_SECTION(lpi2c_master_edma_handle_t gEdmaHandle);

/**
 * @brief	Tx eDMA Handle.
 */
edma_handle_t gEdmaTxHandle;

/**
 * @brief	Rx eDMA Handle.
 */
edma_handle_t gEdmaRxHandle;

/**
 * @brief	Flag Indicating Whether The Transfer Has Finished.
 */
volatile bool gCompletionFlag 	= false;

lpi2c_master_transfer_t xfer 	= {0};

/*******************************************************************************
 * Callback Functions
 ******************************************************************************/
static void lpi2c_callback(LPI2C_Type *base, lpi2c_master_edma_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
	if (kStatus_Success == status)
    {
    	gCompletionFlag = true;
    }
}

/*******************************************************************************
 * Functions
 ******************************************************************************/
static void PrintRegisterContentBin(uint8_t value)
{
    PRINTF("Register Content (BIN): ");
    for (int i = 7; i >= 0; i--) 						// From MSB To LSB
    {
        PRINTF("%c", (value & (1U << i)) ? '1' : '0'); 	// Check And Print The Bits

        if (0 == i % 4)
        {
            PRINTF(" ");
        }
    }
    PRINTF("\r\n");
}


static void PrintRegisterContentHex(uint8_t reg[], uint8_t size)
{
	PRINTF("Register Content (HEX): ");
	for (uint8_t i = 0; i < size; i++)
	{
		PRINTF("0x%02X ", reg[i]);
	}
	PRINTF("\r\n");
}

uint8_t Write(uint8_t regAddress, uint8_t val[])
{
	uint8_t retVal 					= 1;

    xfer.slaveAddress   = P3T1755_ADDR_7BIT;			/* Slave Address			*/
    xfer.direction      = kLPI2C_Write;					/* Direction (Read/Write)	*/
    xfer.subaddressSize = 0;							/* Sub-Address Size 		*/
    xfer.data           = val;							/* Transmitted Data			*/
    xfer.dataSize       = 1;							/* Size of Transmitted Data	*/
    xfer.flags          = kLPI2C_TransferDefaultFlag;	/* Flags (e.g. Stop Flag) 	*/

    /* Send master non-blocking data to slave */
    retVal = LPI2C_MasterTransferEDMA(I2C_MASTER, &gEdmaHandle, &xfer);
    if (kStatus_Success != retVal)
    {
        return (0xFF);
    }

    /*  Wait for transfer completed. */
	while (false == gCompletionFlag)
	{
		; /* Wait For Slave Complete */
	}
    gCompletionFlag = false;							// Reset
    return 0;
}

uint16_t Read(uint8_t regAddress)
{
	uint16_t rxVal					= 0;
	uint8_t retVal 					= 1;

	xfer.slaveAddress   = P3T1755_ADDR_7BIT;			/* Slave Address			*/
	xfer.direction      = kLPI2C_Read;					/* Direction (Read/Write)	*/
	xfer.subaddressSize = 0;							/* Sub-Address Size 		*/
	xfer.data           = gRxBuff;						/* Transmitted Data			*/
	xfer.dataSize       = 2;							/* Size of Transmitted Data	*/
	xfer.flags          = kLPI2C_TransferDefaultFlag;	/* Flags (e.g. Stop Flag) 	*/

    /* Receive non-blocking data from slave */
    retVal = LPI2C_MasterTransferEDMA(I2C_MASTER, &gEdmaHandle, &xfer);
    if (kStatus_Success != retVal)
    {
        return (0xFFFF);
    }

	while (false == gCompletionFlag)
	{
		; /* Wait For Slave Complete */
	}

    gCompletionFlag = false;							// Reset

#if (true == PRINT_REG_OUTPUT)
	PrintRegisterContentHex(gRxBuff, 2);
#endif /* (true == PRINT_REG_OUTPUT) */

	rxVal = (((uint16_t)gRxBuff[0]) << 8 | gRxBuff[1]);
    return rxVal;
}

uint8_t TMP_Init()
{
    lpi2c_master_config_t masterCfg;
    edma_config_t userCfg;
    status_t retVal = kStatus_Fail;
//TODO DMA1    CLOCK_EnableClock(kCLOCK_Dma0);
    EDMA_GetDefaultConfig(&userCfg);
    EDMA_Init(DMA0, &userCfg);

    LPI2C_MasterGetDefaultConfig(&masterCfg);
    masterCfg.baudRate_Hz = I2C_BAUDRATE;

    /* Initialize The LPI2C Master */
    LPI2C_MasterInit(I2C_MASTER, &masterCfg, CLOCK_GetLPFlexCommClkFreq(5u));

    /* Create The EDMA Channel Handles */
    EDMA_CreateHandle(&gEdmaTxHandle, DMA0, LPI2C_TX_DMA_CHANNEL);
    EDMA_CreateHandle(&gEdmaRxHandle, DMA0, LPI2C_RX_DMA_CHANNEL);
    EDMA_SetChannelMux(DMA0, LPI2C_TX_DMA_CHANNEL, LPI2C_TX_CHANNEL);
    EDMA_SetChannelMux(DMA0, LPI2C_RX_DMA_CHANNEL, LPI2C_RX_EDMA_CHANNEL);

    /* Create LPI2C Master DMA Driver Handle */
    LPI2C_MasterCreateEDMAHandle(I2C_MASTER, &gEdmaHandle, &gEdmaRxHandle, &gEdmaTxHandle,
                                 lpi2c_callback, NULL);

    tmp = Read(REGISTER_TEMPERATURE);
    if (0xFFFF == tmp)
    {
    	PRINTF("ERR: Tx Transfer Failed\r\n");
    }

}
