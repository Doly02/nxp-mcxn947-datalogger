/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      parser.c
 *  Author:         Tomas Dolak
 *  Date:           16.04.2025
 *  Description:    Includes Implementation of Task For FreeRTOS.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           parser.c
 *  @author         Tomas Dolak
 *  @date           16.04.2025
 *  @brief          Includes Implementation of Task For FreeRTOS.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "parser.h"

/*******************************************************************************
 * Local Definitions
 ******************************************************************************/
/*
 * @brief 	Time Interval Between LED Blinking.
 * @details	In Milliseconds.
 */
#define RECORD_LED_TIME_INTERVAL 	(uint32_t)(10U)

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/**
 * @brief 	Configuration of The Data Logger on The Basis of Data
 * 			Obtained From The Configuration File.
 */
static REC_config_t g_config;
/*******************************************************************************
 * Interrupt Service Routines (ISRs)
 ******************************************************************************/


/*******************************************************************************
 * Functions
 ******************************************************************************/
REC_config_t PARSER_GetConfig(void)
{
	return g_config;
}

REC_version_t PARSER_GetVersion(void)
{
	return g_config.version;
}

uint32_t PARSER_GetBaudrate(void)
{
	return g_config.baudrate;
}

uint32_t PARSER_GetFileSize(void)
{
	return g_config.size;
}

lpuart_data_bits_t PARSER_GetDataBits(void)
{
	return g_config.data_bits;
}

lpuart_parity_mode_t PARSER_GetParity(void)
{
	return g_config.parity;
}

lpuart_stop_bit_count_t PARSER_GetStopBits(void)
{
	return g_config.stop_bits;
}

uint32_t PARSER_GetMaxBytes(void)
{
	return g_config.max_bytes;
}

void PARSER_ClearConfig(void)
{
	g_config.baudrate  	= 0UL;
	g_config.max_bytes 	= 0UL;
	g_config.version 	= WCT_UNKOWN;

	g_config.parity = kLPUART_ParityDisabled;
	g_config.stop_bits = kLPUART_OneStopBit;
	g_config.data_bits = kLPUART_EightDataBits;
}

error_t PARSER_ParseBaudrate(const char *content)
{
    const char *key = "baudrate=";
    char *found = strstr(content, key);
    if (NULL == found)
    {
        PRINTF("ERR: Key 'baudrate=' not found.\r\n");
        return ERROR_READ;
    }

    found += strlen(key);
    /* MISRA Deviation Note:
     * Rule: MISRA 2012 Rule 21.7
     * Justification: Use of 'atoi' is intentional in controlled context.
     * The input string 'found' should contain only numeric characters - baudrate.
     */
    /*lint -e586 MISRA Deviation: Use of 'atoi' is intentional and input is trusted. */
    uint32_t baudrate = (uint32_t)atoi(found);			// Convert Value To INT
    /*lint +e586 */

    if (0UL == baudrate)
    {
        PRINTF("ERR: Invalid baudrate value.\r\n");
        return ERROR_READ;
    }

    switch (baudrate)
    {
        case 230400UL:
            g_config.version = WCT_AUTOS2;
            break;
        case 115200UL:
            g_config.version = WCT_AUTOS1;
            break;

		/* MISRA Deviation Note:
		 * Rule: MISRA 2012 Rule 16.1 and Rule 16.3 [Required]
		 * Justification: The `default` case ends unconditionally with a `return` statement
		 * and is enclosed in a compound block to clearly separate control flow.
		 * No fall-through is possible, and this structure is used intentionally for clarity.
		 */
		/*lint -e9077 -e9090 -e9042 MISRA Deviation: switch default case ends with unconditional return */
        default:
        {
            PRINTF("ERR: Unsupported baudrate value: %d\r\n", baudrate);
            return ERROR_CONFIG;
        }
        /*lint +e9077 +e9090 +e9042 */
    }

    g_config.baudrate = baudrate;
    return ERROR_NONE;
}

error_t PARSER_ParseFileSize(const char *content)
{
    const char *key = "file_size=";
    char *found = strstr(content, key);
    if (NULL == found)
    {
        PRINTF("ERR: Key 'file_size=' not found.\r\n");
        return ERROR_READ;
    }

    found += strlen(key);
    /* MISRA Deviation Note:
     * Rule: MISRA 2012 Rule 21.7
     * Justification: Use of 'atoi' is intentional in controlled context.
     * The input string 'found' should contain only numeric characters - file size.
     */
    /*lint -e586 MISRA Deviation: Use of 'atoi' is intentional and input is trusted. */
    uint32_t value = (uint32_t)atoi(found);			// Convert Value To INT
    /*lint +e586 */

    if (0UL == value)
    {
        PRINTF("ERR: Invalid file size value.\r\n");
        return ERROR_READ;
    }

    if (0UL != (value % 512UL))
    {
        uint32_t rounded = ((value + 511UL) / 512UL) * 512UL;
        PRINTF("INFO: File size %u is not multiple of 512. Rounding up to %u.\r\n", value, rounded);
        value = rounded;
    }

    g_config.size = value;
    g_config.max_bytes = (g_config.baudrate / 1000UL) * RECORD_LED_TIME_INTERVAL;

    PRINTF("DEBUG: File size set to %u bytes.\r\n", g_config.size);
    return ERROR_NONE;
}


error_t PARSER_ParseParity(const char *content)
{
    const char *keyParity = "parity=";
    char *found = strstr(content, keyParity);
    if (NULL == found)
    {
        PRINTF("INFO: Key 'parity=' not found. Using default.\r\n");
        return ERROR_NONE;
    }

    found += strlen(keyParity);

    if (0 == strncmp(found, "none", 4))
    {
        g_config.parity = kLPUART_ParityDisabled;
    }
    else if (0 == strncmp(found, "even", 4))
    {
        g_config.parity = kLPUART_ParityEven;
    }
    else if (0 == strncmp(found, "odd", 3))
    {
        g_config.parity = kLPUART_ParityOdd;
    }
    else
    {
        PRINTF("ERR: Invalid value for 'parity': %s\r\n", found);
        return ERROR_READ;
    }

    return ERROR_NONE;
}

error_t PARSER_ParseStopBits(const char *content)
{
    const char *keyStopBits = "stop_bits=";
    char *found = strstr(content, keyStopBits);
    if (NULL == found)
    {
        PRINTF("INFO: Key 'stop_bits=' not found. Using default.\r\n");
        return ERROR_NONE;
    }

    found += strlen(keyStopBits);
    /* MISRA Deviation Note:
     * Rule: MISRA 2012 Rule 21.7
     * Justification: Use of 'atoi' is intentional in controlled context.
     * The input string 'found' should contain only numeric characters - stop bits.
     */
    /*lint -e586 MISRA Deviation: Use of 'atoi' is intentional and input is trusted. */
    uint32_t stopBits = (uint32_t)atoi(found);			// Convert Value To INT
    /*lint +e586 */
    if (1UL == stopBits)
    {
        g_config.stop_bits = kLPUART_OneStopBit;
        return ERROR_NONE;
    }
    else if (2UL == stopBits)
	{
		g_config.stop_bits = kLPUART_TwoStopBit;
		return ERROR_NONE;
	}
    else
    {
    	;	/* To avoid MISRA 2012 Rule 15.7 required rule */
    }

    PRINTF("ERR: Invalid value for 'stop_bits': %u\r\n", stopBits);
    return ERROR_READ;
}


error_t PARSER_ParseDataBits(const char *content)
{
    const char *keyDataBits = "data_bits=";
    char *found = strstr(content, keyDataBits);
    if (NULL == found)
    {
        PRINTF("INFO: Key 'data_bits=' not found. Using default.\r\n");
        return ERROR_NONE;
    }

    found += strlen(keyDataBits);
    /* MISRA Deviation Note:
     * Rule: MISRA 2012 Rule 21.7
     * Justification: Use of 'atoi' is intentional in controlled context.
     * The input string 'found' should contain only numeric characters - data bits.
     */
    /*lint -e586 MISRA Deviation: Use of 'atoi' is intentional and input is trusted. */
    uint32_t dataBits = (uint32_t)atoi(found);			// Convert Value To INT
    /*lint +e586 */
    if (7UL == dataBits)
    {
        g_config.data_bits = kLPUART_SevenDataBits;
        return ERROR_NONE;
    }
    else if (8UL == dataBits)
    {
        g_config.data_bits = kLPUART_EightDataBits;
        return ERROR_NONE;
    }
    else
	{
		; /* To avoid MISRA 2012 Rule 15.7 required rule */
	}

    PRINTF("ERR: Invalid value for 'data_bits': %u\r\n", dataBits);
    return ERROR_READ;
}
