/*********************************************************************************************************************
 * @file     system_XMC1100.c
 * @brief    Device specific initialization for the XMC1100-Series according to CMSIS
 * @version  V1.13
 * @date     02 Dec 2019
 *
 * @cond
 *********************************************************************************************************************
 * Copyright (c) 2012-2020, Infineon Technologies AG
 * All rights reserved.                        
 *                                             
 * Boost Software License - Version 1.0 - August 17th, 2003
 * 
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 * 
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *                                                                              
 * To improve the quality of the software, users are encouraged to share 
 * modifications, enhancements or bug fixes with Infineon Technologies AG 
 * at XMCSupport@infineon.com.
 *********************************************************************************************************************
 *
 * *************************** Change history ********************************
 * V1.2, 13 Dec 2012, PKB : Created change history table
 * V1.3, 20 Dec 2012, PKB : Fixed SystemCoreClock computation
 * V1.4, 02 Feb 2013, PKB : SCU_CLOCK -> SCU_CLK
 * V1.5, 27 Nov 2013, DNE : Comments added in SystemInit function for MCLK support
 * V1.6, 19 Feb 2014, JFT : Fixed SystemCoreClock when FDIV != 0 
 * V1.7, 11 Dec 2014, JFT : SystemCoreClockSetup, SystemCoreSetup as weak functions
 * V1.8, 03 Sep 2015, JFT : Override values of CLOCK_VAL1 and CLOCK_VAL2 defined in vector table (startup.s)
 *                          MCLK = 32MHz, PCLK = 64MHz
 * V1.9, 31 Mar 2016, JFT : Fix flash wait states to 1 cycle
 * V1.10,22 Aug 2016, JFT : Update coding for fixed flash wait states using new macros in device header file
 *                          Add macro guard USE_DYNAMIC_FLASH_WS. If defined in compiler options, adaptive wait states
 *                          are used for read accesses to the flash memory. Otherwise a fixed 1 WS is used.
 * V1.11,19 Jun 2017, Rely on cmsis_compiler.h instead of defining __WEAK
 *                    Added support for ARM Compiler 6 (armclang) 
 * V1.12,29 Oct 2018, Fix variable location of SystemCoreClock for ARMCC compiler
 * V1.13,02 Dec 2019, Fix including device header file following the convention: angle brackets are used for standard includes and double quotes for everything else.
 * 
 * @endcond 
 */

/*******************************************************************************
 * HEADER FILES
 *******************************************************************************/

#include "XMC1100.h"
#include "system_XMC1100.h"

/*******************************************************************************
 * MACROS
 *******************************************************************************/
#define DCO1_FREQUENCY (64000000U)

/*******************************************************************************
 * GLOBAL VARIABLES
 *******************************************************************************/

#if defined ( __CC_ARM )
uint32_t SystemCoreClock __attribute__((at(0x20003FFC)));
#elif defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
uint32_t SystemCoreClock __attribute__((section(".bss.ARM.__at_0x20003FFC")));
#elif defined ( __ICCARM__ )
__no_init uint32_t SystemCoreClock;
#elif defined ( __GNUC__ )
uint32_t SystemCoreClock __attribute__((section(".no_init")));
#elif defined ( __TASKING__ )
uint32_t SystemCoreClock __at( 0x20003FFC );
#endif

/*******************************************************************************
 * API IMPLEMENTATION
 *******************************************************************************/

__WEAK void SystemInit(void)
{    
  SystemCoreSetup();
  SystemCoreClockSetup();
}

__WEAK void SystemCoreSetup(void)
{
#ifndef USE_DYNAMIC_FLASH_WS
  /* Fix flash wait states to 1 cycle (see DS Addendum) */
  NVM->NVMCONF |= NVM_NVMCONF_WS_Msk;
  NVM->CONFIG1 |= NVM_CONFIG1_FIXWS_Msk;
#endif
}

__WEAK void SystemCoreClockSetup(void)
{
  /* Override values of CLOCK_VAL1 and CLOCK_VAL2 defined in vector table */
  /* MCLK = 32MHz, PCLK = 64MHz */
  
  SCU_GENERAL->PASSWD = 0x000000C0UL; /* disable bit protection */
  SCU_CLK->CLKCR = 0x3FF10100UL;
  while((SCU_CLK->CLKCR & SCU_CLK_CLKCR_VDDC2LOW_Msk));
  SCU_GENERAL->PASSWD = 0x000000C3UL; /* enable bit protection */
  
  SystemCoreClockUpdate();
}

__WEAK void SystemCoreClockUpdate(void)
{
  static uint32_t IDIV, FDIV;

  IDIV = ((SCU_CLK->CLKCR) & SCU_CLK_CLKCR_IDIV_Msk) >> SCU_CLK_CLKCR_IDIV_Pos;
  FDIV = ((SCU_CLK->CLKCR) & SCU_CLK_CLKCR_FDIV_Msk) >> SCU_CLK_CLKCR_FDIV_Pos;
  
  if (IDIV != 0)
  {
    /* Fractional divider is enabled and used */
    SystemCoreClock = ((DCO1_FREQUENCY << 6U) / ((IDIV << 8) + FDIV)) << 1U;
  }
  else
  {
    /* Fractional divider bypassed. Simply divide DCO_DCLK by 2 */
    SystemCoreClock = DCO1_FREQUENCY >> 1U;
  }
}
