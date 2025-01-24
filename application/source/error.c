/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      error.c
 *  Author:         Tomas Dolak
 *  Date:           22.01.2025
 *  Description:    Implements Error Handling Logic.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           ctimer.c
 *  @author         Tomas Dolak
 *  @date           22.01.2025
 *  @brief          Implements Error Handling Logic.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "error.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
error_t error;
/*******************************************************************************
 * Code
 ******************************************************************************/
void ERR_HandleError()
{
	while(1)
	{
		; /* Error State */
	}
}

void ERR_Init()
{
	error = ERROR_NONE;
}

void ERR_SetState(error_t err)
{
	error = err;
}
