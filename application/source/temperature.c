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
#include <led.h>
#include "fsl_debug_console.h"
#include "fsl_ctimer.h"

#include "temperature.h"
#include "defs.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * @brief 	I2C DMA Channel For Transmission.
 */
#define LPI2C_TX_DMA_CHANNEL       	0UL

/**
 * @brief 	I2C DMA Channel For Reception.
 */
#define LPI2C_RX_DMA_CHANNEL    	1UL

/**
 * @brief	Connection Between DMA Channel 1 and LP_FLEXCOMM5 Tx.
 */
#define LPI2C_TX_CHANNEL 			(int32_t)kDma1RequestMuxLpFlexcomm5Tx

/**
 * @brief	Connection Between DMA Channel 1 and LP_FLEXCOMM5 Rx.
 */
#define LPI2C_RX_EDMA_CHANNEL  		(int32_t)kDma1RequestMuxLpFlexcomm5Rx

/**
 * @brief Points To I2C Peripheral Unit (Specifically LPI2C5 Instance).
 */
#define I2C_MASTER_I2C5				((LPI2C_Type *)LPI2C5_BASE)

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
AT_NONCACHEABLE_SECTION(static uint8_t gRxBuff_I2C5[BUFF_SIZE]);

/**
 * @brief	eDMA Driver Handle Used For Non-Blocking DMA Transfer.
 */
AT_NONCACHEABLE_SECTION(static lpi2c_master_edma_handle_t gEdmaHandle_I2C5);

/**
 * @brief	Tx eDMA Handle.
 */
static edma_handle_t gEdmaTxHandle_I2C5;

/**
 * @brief	Rx eDMA Handle.
 */
static edma_handle_t gEdmaRxHandle_I2C5;

/**
 * @brief	Flag Indicating Whether The Transfer Has Finished.
 */
static volatile bool g_bCompletionFlag_I2C5 	= false;

static lpi2c_master_transfer_t xfer_I2C5 	= {0};

/*******************************************************************************
 * Callback Functions
 ******************************************************************************/
static void lpi2c_callback(LPI2C_Type *base, lpi2c_master_edma_handle_t *handle, status_t status, void *userData)
{
    /* Signal Transfer Success When Received Success Status */
	if (kStatus_Success == status)
    {
    	g_bCompletionFlag_I2C5 = true;
    }
}

/*******************************************************************************
 * Interrupt Handler Functions
 ******************************************************************************/
/*lint -e957 */
/* MISRA 2012 Rule 8.4:
 * Suppress: Function 'CTIMER0_IRQHandler' Defined Without a Prototype in Scope.
 * Justification: CTIMER0_IRQHandler is Declared WEAK in startup_mcxn947_cm33_core0.c and Overridden Here.
 */
void CTIMER0_IRQHandler(void)
{
	float fTemp = TMP_GetTemperature();

	if (35.0 < fTemp)
	{
#if (CONTROL_LED_ENABLED == true)
		LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */
		PRINTF("ERR: High Temperature (%f)\r\n", fTemp);
	}
	CTIMER_ClearStatusFlags(CTIMER0, (uint32_t)kCTIMER_Match0Flag);
}
/*lint +e957 */

/*******************************************************************************
 * Functions
 ******************************************************************************/
uint8_t Write(uint8_t regAddress, uint8_t val[])
{
	status_t status 			 = 1;

    xfer_I2C5.slaveAddress   = P3T1755_ADDR_7BIT;						/* Slave Address			*/
    xfer_I2C5.direction      = kLPI2C_Write;							/* Direction (Read/Write)	*/
    xfer_I2C5.subaddressSize = 0;										/* Sub-Address Size 		*/
    xfer_I2C5.data           = val;										/* Transmitted Data			*/
    xfer_I2C5.dataSize       = 1;										/* Size of Transmitted Data	*/
    xfer_I2C5.flags          = (uint32_t)kLPI2C_TransferDefaultFlag;	/* Flags (e.g. Stop Flag) 	*/

    /* Send Data To Slave */
    status = LPI2C_MasterTransferEDMA(I2C_MASTER_I2C5, &gEdmaHandle_I2C5, &xfer_I2C5);
    if (kStatus_Success != status)
    {
        return (0xFF);
    }

    /*  Wait For Transfer Completed */
	while (false == g_bCompletionFlag_I2C5)
	{
		; /* Wait For Slave Complete */
	}
    g_bCompletionFlag_I2C5 = false;							// Reset
    return 0;
}

uint16_t Read(uint8_t regAddress)
{
	uint16_t u16rxVal		 = 0;
	status_t status 		 = 1;

	xfer_I2C5.slaveAddress   = P3T1755_ADDR_7BIT;						/* Slave Address			*/
	xfer_I2C5.direction      = kLPI2C_Read;								/* Direction (Read/Write)	*/
	xfer_I2C5.subaddressSize = 0;										/* Sub-Address Size 		*/
	xfer_I2C5.data           = gRxBuff_I2C5;							/* Transmitted Data			*/
	xfer_I2C5.dataSize       = 2;										/* Size of Transmitted Data	*/
	xfer_I2C5.flags          = (uint32_t)kLPI2C_TransferDefaultFlag;	/* Flags (e.g. Stop Flag) 	*/

    /* Receive non-blocking data from slave */
	status = LPI2C_MasterTransferEDMA(I2C_MASTER_I2C5, &gEdmaHandle_I2C5, &xfer_I2C5);
    if (kStatus_Success != status)
    {
        return (0xFFFF);
    }

	while (false == g_bCompletionFlag_I2C5)
	{
		; /* Wait For Slave Complete */
	}

    g_bCompletionFlag_I2C5 = false;							// Reset

    u16rxVal = (((uint16_t)gRxBuff_I2C5[0]) << 8 | gRxBuff_I2C5[1]);
    return u16rxVal;
}

float TMP_GetTemperature(void)
{
	uint16_t u16Val;

	u16Val = Read(REGISTER_TEMPERATURE);
	if (0xFFFFU == u16Val)
	{
		PRINTF("ERR: Tx Transfer Failed\r\n");
	}
	return ((float)u16Val / 256.0f);
}


uint8_t TMP_Init(void)
{
    lpi2c_master_config_t masterCfg;
    edma_config_t userCfg;

	ctimer_config_t config;
    ctimer_match_config_t matchConfig;

    EDMA_GetDefaultConfig(&userCfg);
    EDMA_Init(DMA1, &userCfg);

    LPI2C_MasterGetDefaultConfig(&masterCfg);
    masterCfg.baudRate_Hz = I2C_BAUDRATE;

    /* Initialize The LPI2C Master */
    LPI2C_MasterInit(I2C_MASTER_I2C5, &masterCfg, CLOCK_GetLPFlexCommClkFreq(5u));

    /* Create The EDMA Channel Handles */
    EDMA_CreateHandle(&gEdmaTxHandle_I2C5, DMA1, LPI2C_TX_DMA_CHANNEL);
    EDMA_CreateHandle(&gEdmaRxHandle_I2C5, DMA1, LPI2C_RX_DMA_CHANNEL);
    EDMA_SetChannelMux(DMA1, LPI2C_TX_DMA_CHANNEL, LPI2C_TX_CHANNEL);
    EDMA_SetChannelMux(DMA1, LPI2C_RX_DMA_CHANNEL, LPI2C_RX_EDMA_CHANNEL);

    /* Create LPI2C Master DMA Driver Handle */
    LPI2C_MasterCreateEDMAHandle(I2C_MASTER_I2C5, &gEdmaHandle_I2C5, &gEdmaRxHandle_I2C5, &gEdmaTxHandle_I2C5,
                                 lpi2c_callback, NULL);


    /* Initialize Timer For Periodic Interrupts */
	CTIMER_GetDefaultConfig(&config);
	config.prescale = 144 - 1; 							/* Frequency From 144 MHz To 1 MHz 						*/

	CTIMER_Init(CTIMER0, &config);

	matchConfig.enableInterrupt = true;         		/* Enable Interrupt If Match							*/
	matchConfig.enableCounterReset = true;      		/* Reset Timer If Match 								*/
	matchConfig.enableCounterStop = false;      		/* Count After Match									*/
	matchConfig.matchValue = 600000000 - 1;             /* Match Value (6M cycles -> 10 Minutes Per Interrupt) 	*/
	matchConfig.outControl = kCTIMER_Output_NoAction; 	/* Output Pin Off 										*/

	/* Setup Match Register */
	CTIMER_SetupMatch(CTIMER0, kCTIMER_Match_0, &matchConfig);

	/* Enable IRQ */
	(void)EnableIRQWithPriority(CTIMER0_IRQn, 4);

	/* Start Timer */
	CTIMER_StartTimer(CTIMER0);
}
