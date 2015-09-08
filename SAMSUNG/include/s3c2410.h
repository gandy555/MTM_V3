/*

*/

#ifndef __S3C2410_DEF_H__
#define __S3C2410_DEF_H__

//-----------------------------------------------------------------------------
// S3C2410 Timer Constant
//-----------------------------------------------------------------------------
#define S2410FCLK       	(266 * 1000 * 1000)		// 266MHz (FCLK).
#define PCLKDIV         	4						// P-clock (PCLK) divisor.
#define S2410PCLK       	(S2410FCLK / PCLKDIV)	// PCLK.
#define S2410UCLK       	50331648				// 48MHz - for serial UARTs.


//-----------------------------------------------------------------------------
// S3C2410 Memory Map ( suppose OM[1:0] == 00 : NAND Flash for boot ROM )
//-----------------------------------------------------------------------------
#define BOOT_FLASH_BASE_PHYSICAL		0x00000000
#define CS1_BASE_PHYSICAL				0x08000000
#define CS2_BASE_PHYSICAL				0x10000000
#define CS3_BASE_PHYSICAL				0x18000000
#define CS4_BASE_PHYSICAL				0x20000000
#define CS5_BASE_PHYSICAL				0x28000000
#define SDRAM1_BASE_PHYSICAL			0x30000000
#define SDRAM2_BASE_PHYSICAL			0x38000000
#define SFR_BASE_PHYSICAL				0x48000000	//Special Function Register


//-----------------------------------------------------------------------------
// S3C2410 Special Function Register (Subset)
//-----------------------------------------------------------------------------
#define MEM_CTRL_REG_OFFSET				0x0			//0x48000000
#define INT_CTRL_REG_OFFSET				0x2000000	//0x4A000000
#define CLKPWR_MANAGE_REG_OFFSET		0x4000000	//0x4C000000
#define LCD_REG_OFFSET					0x5000000	//0x4D000000
#define WDT_TIMER_REG_OFFSET			0xB000000	//0x53000000
#define GPIO_REG_OFFSET					0xE000000	//0x56000000
#define RTC_REG_OFFSET					0xF000000	//0x57000000
#define ADCNV_REG_OFFSET				0x10000000	//0x58000000
#define SPI_REG_OFFSET					0x11000000  //0x59000000


#define MEM_CTRL_REG_PHYSICAL			(SFR_BASE_PHYSICAL+MEM_CTRL_REG_OFFSET)
#define INT_CTRL_REG_PHYSICAL			(SFR_BASE_PHYSICAL+INT_CTRL_REG_OFFSET)
#define CLKPWR_MANAGE_REG_PHYSICAL		(SFR_BASE_PHYSICAL+CLKPWR_MANAGE_REG_OFFSET)
#define LCD_REG_PHYSICAL				(SFR_BASE_PHYSICAL+LCD_REG_OFFSET)
#define WDT_TIMER_REG_PHYSICAL			(SFR_BASE_PHYSICAL+WDT_TIMER_REG_OFFSET)
#define GPIO_REG_PHYSICAL				(SFR_BASE_PHYSICAL+GPIO_REG_OFFSET)
#define RTC_REG_PHYSICAL				(SFR_BASE_PHYSICAL+RTC_REG_OFFSET)
#define ADCNV_REG_PHYSICAL				(SFR_BASE_PHYSICAL+ADCNV_REG_OFFSET)
#define SPI_REG_PHYSICAL				(SFR_BASE_PHYSICAL+SPI_REG_OFFSET)


//
// Memory Controller
//
#define BWSCON							0x48000000	//	R/W	Bus Width & Wait Status Control
#define BANKCON0						0x48000004	//	Boot ROM Control
#define BANKCON1						0x48000008	//	BANK1 Control
#define BANKCON2						0x4800000C	//	BANK2 Control
#define BANKCON3						0x48000010	//	BANK3 Control
#define BANKCON4						0x48000004	//	BANK4 Control
#define BANKCON5						0x48000008	//	BANK5 Control
#define BANKCON6						0x4800000C	//	BANK6 Control
#define BANKCON7						0x48000020	//	BANK7 Control
#define REFRESH							0x48000024	//	DRAM/SDRAM Refresh Control
#define BANKSIZE						0x48000028	//	Flexible Bank Size
#define MRSRB6							0x4800002C	//	Mode register set for SDRAM
#define MRSRB7							0x48000030	//	Mode register set for SDRAM

typedef struct 
{
	unsigned long       rBWSCON;    // 0
    unsigned long       rBANKCON0;  // 4
    unsigned long       rBANKCON1;  // 8
    unsigned long       rBANKCON2;  // c
    unsigned long       rBANKCON3;  // 10
    unsigned long       rBANKCON4;  // 1c
    unsigned long       rBANKCON5;  // 18
    unsigned long       rBANKCON6;  // 1c
    unsigned long       rBANKCON7;  // 20
    unsigned long       rREFRESH;   // 24
    unsigned long       rBANKSIZE;  // 28
    unsigned long       rMRSRB6;    // 2c
    unsigned long       rMRSRB7;    // 30
} MEMreg;


//								        
// Interrupt Controller			        
//								        
#define SRCPND							0x4A000000	//	R/W	Interrupt Request Status
#define INTMOD							0x4A000004	//	 W	Interrupt Mode Control
#define INTMSK							0x4A000008	//	R/W	Interrupt Mask Control
#define PRIORITY						0x4A00000C	//	 W	IRQ Priority Control
#define INTPND							0x4A000010	//	R/W	Interrupt Request Status
#define INTOFFSET						0x4A000014	//	 R	Interrupt Request Source Offset
#define SUBSRCPND						0x4A000018	//	R/W	Sub Source Pending
#define INTSUBMSK						0x4A00001C	//	R/W	Interrupt Sub Mask

typedef struct 
{
    unsigned long		rSRCPND;
    unsigned long		rINTMOD;
    unsigned long		rINTMSK;
    unsigned long		rPRIORITY;
    unsigned long		rINTPND;
    unsigned long		rINTOFFSET;
    unsigned long		rSUBSRCPND;
    unsigned long		rINTSUBMSK;
} INTreg;


//								        
// Clock & Power Management		        
//								        
#define LOCKTIME						0x4C000000	//	R/W	PLL Lock Time Counter
#define MPLLCON							0x4C000004	//		MPLL Control
#define UPLLCON							0x4C000008	//		UPLL Control
#define CLKCON							0x4C00000C	//		Clock Generator Control
#define CLKSLOW							0x4C000010	//		Slow Clock Control
#define CLKDIVN							0x4C000014	//		Clock divider Control

typedef struct 
{
    unsigned long       rLOCKTIME;
    unsigned long       rMPLLCON;
    unsigned long       rUPLLCON;
    unsigned long       rCLKCON;
    unsigned long       rCLKSLOW;
    unsigned long       rCLKDIVN;
} CLKPWRreg;


//
// LCD Controller
//
#define LCDCON1							0X4D000000	// R/W LCD Control 1
#define LCDCON2							0X4D000004	//     LCD Control 2
#define LCDCON3							0X4D000008	//     LCD Control 3
#define LCDCON4							0X4D00000C	//     LCD Control 4
#define LCDCON5							0X4D000010	//     LCD Control 5
#define LCDSADDR1						0X4D000014	//     STN/TFT: Frame Buffer Start Address1
#define LCDSADDR2						0X4D000018	//     STN/TFT: Frame Buffer Start Address2
#define LCDSADDR3						0X4D00001C	//     STN/TFT: Virtual Screen Address Set
#define REDLUT							0X4D000020	//     STN: Red Lookup Table
#define GREENLUT						0X4D000024	//     STN: Green Lookup Table
#define BLUELUT							0X4D000028	//     STN: Blue Lookup Table
#define DITHMODE						0X4D00004C	//     STN: Dithering Mode
#define TPAL							0X4D000050	//     TFT: Temporary Palette
#define LCDINTPND						0X4D000054	//     LCD Interrupt Pending
#define LCDSRCPND						0X4D000058	//     LCD Interrupt Source
#define LCDINTMSK						0X4D00005C	//     LCD Interrupt Mask
#define LPCSEL							0X4D000060	//     LPC3600 Control

typedef struct  {
    unsigned int		rLCDCON1;
    unsigned int		rLCDCON2;
    unsigned int		rLCDCON3;
    unsigned int		rLCDCON4;
    unsigned int		rLCDCON5;
    unsigned int		rLCDSADDR1;
    unsigned int		rLCDSADDR2;
    unsigned int		rLCDSADDR3;
    unsigned int		rREDLUT;
    unsigned int		rGREENLUT;
    unsigned int		rBLUELUT;
    unsigned int		rPAD[8];
    unsigned int		rDITHMODE;
    unsigned int		rTPAL;
    unsigned int		rLCDINTPND;
    unsigned int		rLCDSRCPND;
    unsigned int		rLCDINTMSK;
    unsigned int		rLPCSEL;
} LCDreg; 


//								        
// Watchdog Timer				        
//								        
#define WTCON							0x53000000	//	R/W	Watchdog Timer Mode
#define WTDAT							0x53000004	//		Watchdog Timer Data
#define WTCNT							0x53000008	//		Watchdog Timer Count

typedef struct 
{
    unsigned long       rWTCON;
    unsigned long       rWTDAT;
    unsigned long       rWTCNT;
} WATCHreg;


//								        
// GPIO							        
//								        
#define GPACON							0x56000000	//	R/W	Port A Control
#define GPADAT							0x56000004	//		Port A Data
#define GPBCON							0x56000010	//		Port B Control
#define GPBDAT							0x56000014	//		Port B Data
#define GPBUP							0x56000018	//		Pull-up Control B
#define GPCCON							0x56000020	//		Port C Control
#define GPCDAT							0x56000024	//		Port C Data
#define GPCUP							0x56000028	//		Pull-up Control C
#define GPDCON							0x56000030	//		Port D Control
#define GPDDA1T							0x56000034	//		Port D Data
#define GPDUP							0x56000038	//		Pull-up Control D
#define GPECON							0x56000040	//		Port E Control
#define GPEDAT							0x56000044	//		Port E Data
#define GPEUP							0x56000048	//		Pull-up Control E
#define GPFCON							0x56000050	//		Port F Control
#define GPFDAT							0x56000054	//		Port F Data
#define GPFUP							0x56000058	//		Pull-up Control F
#define GPGCON							0x56000060	//		Port G Control
#define GPGDAT							0x56000064	//		Port G Data
#define GPGUP							0x56000068	//		Pull-up Control G
#define GPHCON							0x56000070	//		Port H Control
#define GPHDAT							0x56000074	//		Port H Data
#define GPHUP							0x56000078	//		Pull-up Control H

#define MISCCR							0x56000080	//	R/W	Miscellaneous Control
#define DCLKCON							0x56000084	//		DCLK0/1 Control
#define EXTINT0							0x56000088	//		External Interrupt Control Register 0
#define EXTINT1							0x5600008C	//		External Interrupt Control Register 1
#define EXTINT2							0x56000090	//		External Interrupt Control Register 2
#define EINTFLT0						0x56000094	//		Reserved
#define EINTFLT1						0x56000098	//		Reserved
#define EINTFLT2						0x5600009C	//		External Interrupt Filter Control Register 2
#define EINTFLT3						0x560000A0	//		External Interrupt Filter Control Register 3
#define EINTMASK						0x560000A4	//		External Interrupt Mask
#define EINTPEND						0x560000A8	//		External Interrupt Pending
#define GSTATUS0						0x560000AC	//		External Pin Status
#define GSTATUS1						0x560000B0	//		External Pin Status

typedef struct 
{
    unsigned long		rGPACON;		// 00
    unsigned long		rGPADAT;
    unsigned long		rPAD1[2];

    unsigned long		rGPBCON;		// 10
    unsigned long		rGPBDAT;
    unsigned long		rGPBUP;
    unsigned long		rPAD2;

    unsigned long		rGPCCON;		// 20
    unsigned long		rGPCDAT;
    unsigned long		rGPCUP;
    unsigned long		rPAD3;

    unsigned long		rGPDCON;		// 30
    unsigned long		rGPDDAT;
    unsigned long		rGPDUP; 
    unsigned long		rPAD4;

    unsigned long		rGPECON;		// 40
    unsigned long		rGPEDAT;
    unsigned long		rGPEUP;
    unsigned long		rPAD5;

    unsigned long		rGPFCON;		// 50
    unsigned long		rGPFDAT;
    unsigned long		rGPFUP; 
    unsigned long		rPAD6;

    unsigned long		rGPGCON;		// 60
    unsigned long		rGPGDAT;
    unsigned long		rGPGUP; 
    unsigned long		rPAD7;

    unsigned long		rGPHCON;		// 70
    unsigned long		rGPHDAT;
    unsigned long		rGPHUP; 
    unsigned long		rPAD8;

    unsigned long		rMISCCR;		// 80
    unsigned long		rDCKCON;		
    unsigned long		rEXTINT0;
    unsigned long		rEXTINT1;		
    unsigned long		rEXTINT2;		// 90
	unsigned long		rEINTFLT0;
	unsigned long		rEINTFLT1;
	unsigned long		rEINTFLT2;
	unsigned long		rEINTFLT3;		// A0
	unsigned long		rEINTMASK;
	unsigned long		rEINTPEND;
	unsigned long		rGSTATUS0;
	unsigned long		rGSTATUS1;		// B0
} IOPreg;  


//								        
// RTC							        
//								        
#define RTCCON							0x57000043	//	R/W	RTC Control
#define TICNT							0x57000047	//		Tick time count
#define RTCALM							0x57000053	//		RTC Alarm Control
#define ALMSEC							0x57000057	//		Alarm Second
#define ALMMIN							0x5700005B	//		Alarm Minute
#define ALMHOUR							0x5700005F	//		Alarm Hour
#define ALMDATE							0x57000063	//		Alarm Day
#define ALMMON							0x57000067	//		Alarm Month
#define ALMYEAR							0x5700006B	//		Alarm Year
#define RTCRST							0x5700006F	//		RTC Round Reset
#define BCDSEC							0x57000073	//		BCD Second
#define BCDMIN							0x57000077	//		BCD Minute
#define BCDHOUR							0x5700007B	//		BCD Hour
#define BCDDATE							0x5700007F	//		BCD Day
#define BCDDAY							0x57000083	//		BCD Date
#define BCDMON							0x57000087	//		BCD Month
#define BCDYEAR							0x5700008B	//		BCD Year

typedef struct {
	unsigned long		rPAD1[16]; 		// 00 - 3F
    unsigned long		rRTCCON;		// 40
    unsigned long		rTICNT;
    unsigned long		rPAD2[2];      
    unsigned long		rRTCALM;
    unsigned long		rALMSEC;
    unsigned long		rALMMIN;
    unsigned long		rALMHOUR;
    unsigned long		rALMDATE;
    unsigned long		rALMMON;
    unsigned long		rALMYEAR;
    unsigned long		rRTCRST;
    unsigned long		rBCDSEC;
    unsigned long		rBCDMIN;
    unsigned long		rBCDHOUR;
    unsigned long		rBCDDATE;
    unsigned long		rBCDDAY;
    unsigned long		rBCDMON;
    unsigned long		rBCDYEAR;
} RTCreg;

								
//								        
// A/D Converter				        
//								        
#define ADCCON							0x58000000	//	R/W	ADC Control
#define ADCTSC							0x58000004	//		ADC Touch Screen Control
#define ADCDLY							0x58000008	//		ADC Start or Interval Delay
#define ADCDAT0							0x5800000C	//	 R	ADC Conversion Data
#define ADCDAT1							0x58000010	//	ADC Conversion Data

typedef struct {
	unsigned int 		rADCCON;
	unsigned int 		rADCTSC;
	unsigned int		rADCDLY;
	unsigned int 		rADCDAT0;
	unsigned int 		rADCDAT1;
} ADCreg;


//
// SPI/SSP
//

#define SPCON0							0x59000000
#define SPSTA0							0x59000004
#define SPPIN0							0x59000008
#define SPPRE0							0x5900000C
#define SPTDAT0							0x59000010
#define SPRDAT0							0x59000014

#define SPCON1							0x59000020
#define SPSTA1							0x59000024
#define SPPIN1							0x59000028
#define SPPRE1							0x5900002C
#define SPTDAT							0x59000030
#define SPRDAT							0x59000034

typedef struct  {
    unsigned int		rSPCON0;
    unsigned int		rSPSTA0;
    unsigned int		rSPPIN0;
    unsigned int		rSPPRE0;
    unsigned int		rSPTDAT0;
    unsigned int		rSPRDAT0;
    unsigned int		rPAD[2];
    unsigned int		rSPCON1;
    unsigned int		rSPSTA1;
    unsigned int		rSPPIN1;
    unsigned int		rSPPRE1;
    unsigned int		rSPTDAT1;
    unsigned int		rSPRDAT1;
} SPIreg ; 

#endif //__S3C2410_DEF_H__

