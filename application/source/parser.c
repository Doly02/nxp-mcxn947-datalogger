/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      parser.c
 *  Author:         Tomas Dolak
 *  Date:           16.04.2025
 *  Description:    Includes Implementation of Configuration File Parser.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           parser.c
 *  @author         Tomas Dolak
 *  @date           16.04.2025
 *  @brief          Includes Implementation of Configuration File Parser.
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

uint32_t PARSER_GetFreeSpaceLimitMB(void)
{
	return g_config.free_space_limit_mb;
}

uint32_t PARSER_GetMaxBytes(void)
{
	return g_config.max_bytes;
}

void PARSER_ClearConfig(void)
{
	g_config.baudrate  	= DEFAULT_BAUDRATE;
	g_config.max_bytes 	= 0UL;
	g_config.version 	= WCT_UNKOWN;

	g_config.parity 	= DEFAULT_PARITY;
	g_config.stop_bits 	= DEFAULT_STOP_BITS;
	g_config.data_bits 	= DEFAULT_DATA_BITS;

	g_config.free_space_limit_mb = DEFAULT_FREE_SPACE;
}

error_t PARSER_ParseBaudrate(const char *chContent)
{
    const char *chKey = "baudrate=";
    char *chFound = strstr(chContent, chKey);
    if (NULL == chFound)
    {
        PRINTF("ERR: Key 'baudrate=' not found.\r\n");
        return ERROR_READ;
    }

    chFound += strlen(chKey);
    /* MISRA Deviation Note:
     * Rule: MISRA 2012 Rule 21.7
     * Justification: Use of 'atoi' is intentional in controlled context.
     * The input string 'chFound' should contain only numeric characters - baudrate.
     */
    /*lint -e586 MISRA Deviation: Use of 'atoi' is intentional and input is trusted. */
    uint32_t u32Baudrate = (uint32_t)atoi(chFound);			// Convert Value To INT
    /*lint +e586 */

    if (0UL == u32Baudrate)
    {
        PRINTF("ERR: Invalid baudrate value.\r\n");
        return ERROR_READ;
    }

    switch (u32Baudrate)
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
            PRINTF("ERR: Unsupported baudrate value: %d\r\n", u32Baudrate);
            return ERROR_CONFIG;
        }
        /*lint +e9077 +e9090 +e9042 */
    }

    g_config.baudrate = u32Baudrate;
    return ERROR_NONE;
}

error_t PARSER_ParseFileSize(const char *chContent)
{
    const char *chKey = "file_size=";
    uint32_t u32Value = DEFAULT_MAX_FILESIZE;

    char *chFound = strstr(chContent, chKey);

    if (NULL == chFound)
    {
        PRINTF("ERR: Key 'file_size=' not found.\r\n");
        /* Continue With Default */
    }
    else
    {
		chFound += strlen(chKey);
		/* MISRA Deviation Note:
		 * Rule: MISRA 2012 Rule 21.7
		 * Justification: Use of 'atoi' is intentional in controlled context.
		 * The input string 'chFound' should contain only numeric characters - file size.
		 */
		/*lint -e586 MISRA Deviation: Use of 'atoi' is intentional and input is trusted. */
		u32Value = (uint32_t)atoi(chFound);			// Convert Value To INT
		/*lint +e586 */

		if (0UL == u32Value)
		{
			PRINTF("ERR: Invalid file size value.\r\n");
			return ERROR_READ;
		}
    }

    if (0UL != (u32Value % 512UL))
    {
        uint32_t u32Rounded = ((u32Value + 511UL) / 512UL) * 512UL;
        PRINTF("INFO: File size %u is not multiple of 512. Rounding up to %u.\r\n", u32Value, u32Rounded);
        u32Value = u32Rounded;
    }

    g_config.size = u32Value;
    g_config.max_bytes = (g_config.baudrate / 1000UL) * RECORD_LED_TIME_INTERVAL;

    PRINTF("DEBUG: File size set to %u bytes.\r\n", g_config.size);
    return ERROR_NONE;
}


error_t PARSER_ParseParity(const char *chContent)
{
    const char *chKeyParity = "parity=";
    char *chFound = strstr(chContent, chKeyParity);
    if (NULL == chFound)
    {
        PRINTF("INFO: Key 'parity=' not found. Using default.\r\n");
        return ERROR_NONE;
    }

    chFound += strlen(chKeyParity);

    if (0 == strncmp(chFound, "none", 4))
    {
        g_config.parity = kLPUART_ParityDisabled;
    }
    else if (0 == strncmp(chFound, "even", 4))
    {
        g_config.parity = kLPUART_ParityEven;
    }
    else if (0 == strncmp(chFound, "odd", 3))
    {
        g_config.parity = kLPUART_ParityOdd;
    }
    else
    {
        PRINTF("ERR: Invalid value for 'parity': %s\r\n", chFound);
        return ERROR_READ;
    }

    return ERROR_NONE;
}

error_t PARSER_ParseStopBits(const char *chContent)
{
    const char *chKeyStopBits = "stop_bits=";
    char *chFound = strstr(chContent, chKeyStopBits);
    if (NULL == chFound)
    {
        PRINTF("INFO: Key 'stop_bits=' not found. Using default.\r\n");
        return ERROR_NONE;
    }

    chFound += strlen(chKeyStopBits);
    /* MISRA Deviation Note:
     * Rule: MISRA 2012 Rule 21.7
     * Justification: Use of 'atoi' is intentional in controlled context.
     * The input string 'chFound' should contain only numeric characters - stop bits.
     */
    /*lint -e586 MISRA Deviation: Use of 'atoi' is intentional and input is trusted. */
    uint32_t u32StopBits = (uint32_t)atoi(chFound);			// Convert Value To INT
    /*lint +e586 */
    if (1UL == u32StopBits)
    {
        g_config.stop_bits = kLPUART_OneStopBit;
        return ERROR_NONE;
    }
    else if (2UL == u32StopBits)
	{
		g_config.stop_bits = kLPUART_TwoStopBit;
		return ERROR_NONE;
	}
    else
    {
    	;	/* To avoid MISRA 2012 Rule 15.7 required rule */
    		/* Selected 1 Stop Bit By Default */
    }

    PRINTF("ERR: Invalid value for 'stop_bits': %u\r\n", u32StopBits);
    return ERROR_READ;
}


error_t PARSER_ParseDataBits(const char *chContent)
{
    const char *chKeyDataBits = "data_bits=";
    char *chFound = strstr(chContent, chKeyDataBits);
    if (NULL == chFound)
    {
        PRINTF("INFO: Key 'data_bits=' not found. Using default.\r\n");
        return ERROR_NONE;
    }

    chFound += strlen(chKeyDataBits);
    /* MISRA Deviation Note:
     * Rule: MISRA 2012 Rule 21.7
     * Justification: Use of 'atoi' is intentional in controlled context.
     * The input string 'chFound' should contain only numeric characters - data bits.
     */
    /*lint -e586 MISRA Deviation: Use of 'atoi' is intentional and input is trusted. */
    uint32_t u32DataBits = (uint32_t)atoi(chFound);			// Convert Value To INT
    /*lint +e586 */
    if (7UL == u32DataBits)
    {
        g_config.data_bits = kLPUART_SevenDataBits;
        return ERROR_NONE;
    }
    else if (8UL == u32DataBits)
    {
        g_config.data_bits = kLPUART_EightDataBits;
        return ERROR_NONE;
    }
    else
	{
		; 	/* To avoid MISRA 2012 Rule 15.7 required rule */
			/* Selected 8 Data Bits By Default */
	}

    PRINTF("ERR: Invalid value for 'data_bits': %u\r\n", u32DataBits);
    return ERROR_READ;
}

error_t PARSER_ParseFreeSpace(const char *chContent)
{
    const char *chKey = "free_space=";
    char *chFound = strstr(chContent, chKey);
    if (NULL == chFound)
    {
        PRINTF("INFO: Key 'free_space=' not found. Using default.\r\n");
        g_config.free_space_limit_mb = DEFAULT_FREE_SPACE;
        return ERROR_NONE;
    }

    chFound += strlen(chKey);

    errno = 0;  // Reset errno before parsing
    char *endptr = NULL;

    /*lint -e586 MISRA Deviation: strtoul is used with trusted input */
    unsigned long ulParsedValue = strtoul(chFound, &endptr, 10);
    /*lint +e586 */

    // Conversion Errors Check (Only 32bit Number)
    if ((chFound == endptr) || (0 != errno) || (0xFFFFFFFFUL < ulParsedValue))
    {
        PRINTF("ERR: Invalid or out-of-range value for 'free_space=': %s\r\n", chFound);
        return ERROR_READ;
    }

    g_config.free_space_limit_mb = (uint32_t)ulParsedValue;

    if (0UL == g_config.free_space_limit_mb)
    {
        PRINTF("INFO: 'free_space=' set to 0 MB, threshold disabled.\r\n");
    }
    else
    {
        PRINTF("INFO: Free space LED threshold set to %lu MB.\r\n", ulParsedValue);
    }

    return ERROR_NONE;
}
