/*
 * AXI_DMA.c
 * Using Xilinx AXI Direct Memory Access v7.1
 * Simple DMA Handle
 * Direct Register Mode
 *  Created on: 2019/01/23
 *      Author: FC
 */
#include "AXI_DMA.h"

/*****************************************************************************/
/**
 * Reset AXIDMA S2MM Engine
 * @note	None
 *
 ******************************************************************************/
void ResetAXIDMAS2MM()
{
	dma_set(S2MMControlRegister, 0x4);
}
/*****************************************************************************/
/**
 * Reset AXIDMA MM2S Engine
 * @note	None
 *
 ******************************************************************************/
void ResetAXIDMAMM2S()
{
	dma_set( MM2SControlRegister, 0x4);
}
/*****************************************************************************/
/**
 * Stop AXIDMA S2MM Engine
 * @note	None
 *
 ******************************************************************************/
void StopAXIDMAS2MM()
{
	dma_set(S2MMControlRegister, 0x0);
}
/*****************************************************************************/
/**
 * Stop AXIDMA MM2S Engine
 * @note	None
 *
 ******************************************************************************/
void StopAXIDMAMM2S()
{
	dma_set(MM2SControlRegister, 0x0);
}
/*****************************************************************************/
/**
 * Set AXIDMA MM2S buffer physical address
 * @param	DMAMM2SBufferAddress is S2MM physical address
 * @note	None
 *
 ******************************************************************************/
void SetAXIDMAMM2SBufferAddress(unsigned int DMAMM2SBufferAddress)
{
	dma_set(MM2SSourceAddressRegister, DMAMM2SBufferAddress);
}
/*****************************************************************************/
/**
 * Set AXIDMA S2MM buffer physical address
 * @param	DMAS2MMBufferAddress is S2MM physical address
 * @note	None
 *
 ******************************************************************************/
void SetAXIDMAS2MMBufferAddress(unsigned int DMAS2MMBufferAddress)
{
	dma_set( S2MMDestinationAddressRegister, DMAS2MMBufferAddress);
}
/*****************************************************************************/
/**
 * Enable AXIDMA MM2S interrupt on complete(IOC) Interrupt and Interrupt on Error Interrupt
 * @note	None
 *
 ******************************************************************************/
void EnableAXIDMAMM2SIrq()
{
	//dma_set( MM2SControlRegister, 0x5001);
	dma_set( MM2SControlRegister, 0x0001); //ybq 2020.12.23   //很离谱，IOC中断置0有效，但是DMA7.1的手册上说的置1有效
}
/*****************************************************************************/
/**
 * Enable AXIDMA S2MM interrupt on complete(IOC) Interrupt and Interrupt on Error Interrupt
 * @note	None
 *
 ******************************************************************************/
void EnableAXIDMAS2MMIrq()
{
	//dma_set(S2MMControlRegister, 0x5001);
	dma_set(S2MMControlRegister, 0x0001); //ybq 2020.12.23
}
/*****************************************************************************/
/**
 * Start AXIDMA S2MM engine
 * @note	None
 *
 ******************************************************************************/
void RunAXIDMAS2MM()
{
	dma_set(S2MMControlRegister, 0x01);
}
/*****************************************************************************/
/**
 * Start AXIDMA MM2S engine
 * @note	None
 *
 ******************************************************************************/
void RunAXIDMAMM2S()
{
	dma_set(MM2SControlRegister, 0x01);
}

/*****************************************************************************/
/**
 * Clear AXIDMA S2MM Interrupt
 * @note	None
 *
 ******************************************************************************/
void ClearAXIDMAS2MMInterrupt()
{
	u32 s2mm_sr = dma_get(S2MMStatusRegister);
	dma_set( S2MMStatusRegister, s2mm_sr | 0x7000);
}
/*****************************************************************************/
/**
 * Clear AXIDMA MM2S Interrupt
 * @note	None
 *
 ******************************************************************************/
void ClearAXIDMAMM2SInterrupt()
{
	u32 mm2s_sr = dma_get(MM2SStatusRegister);
	dma_set( MM2SStatusRegister, mm2s_sr | 0x7000);
}
/*****************************************************************************/
/**
 * Set AXIDMA S2MM buffer size and start to transfer data
 * @param	value is buffer size(byte)
 * @note	None
 *
 ******************************************************************************/
void SetAXIDMAS2MMTransferingBufferSize(unsigned int value)
{
	dma_set( S2MMTransferLengthRegister, value);
}
/*****************************************************************************/
/**
 * Set AXIDMA MM2S buffer size and start to transfer data
 * @param	value is buffer size(byte)
 * @note	None
 *
 ******************************************************************************/
void SetAXIDMAMM2STransferingBufferSize(unsigned int value)
{
	//printf("value=%d\n",value);
	dma_set(MM2STransferLengthRegister, value);
}
/*****************************************************************************/
/**
 * Judge MM2S transferring Idle
 * @note	None
 *
 ******************************************************************************/
void IsMM2SIdle()
{	u32 status=0;
	while(!(status & 0x00000002))
	{
		status = dma_get( MM2SStatusRegister);
	}
}
/*****************************************************************************/
/**
 * Judge MM2S Interrupt on Complete.
 * @note	None
 *
 ******************************************************************************/
void IsMM2SIOC()
{
	u32 status=0;
		while(!(status & 0x1000))
		{
		//PrintAXIDMAStatus();
			status = dma_get( MM2SStatusRegister);
		printf("dma_get( MM2SStatusRegister) = 0x%08x\n",status);
		}
}
/*****************************************************************************/
/**
 * Judge S2MM transferring Idle
 * @note	None
 *
 ******************************************************************************/
void IsS2MMIdle()
{	u32 status=0;
	while(!(status & 0x00000002))
	{
		status = dma_get( S2MMStatusRegister);
	}
}
/*****************************************************************************/
/**
 * Judge S2MM Interrupt on Complete.
 * @note	None
 *
 ******************************************************************************/
void IsS2MMIOC()
{
	u32 status=0;
	while(!(status & 0x1000))
	{
		status = dma_get( S2MMStatusRegister);
		PrintAXIDMAStatus();
	}
}
/*****************************************************************************/
/**
 * Read MM2S & S2MM status,and print it
 * @note	None
 *
 ******************************************************************************/
void PrintAXIDMAStatus()
{
	u32 status = dma_get( S2MMStatusRegister);
	printf("S2MM status (0x%08x@0x%02x):", status, S2MMStatusRegister);
	if (status & 0x00000001) printf(" halted");
	else 					 printf(" running");
    if (status & 0x00000002) printf(" idle");
    if (status & 0x00000008) printf(" SGIncld");
    if (status & 0x00000010) printf(" DMAIntErr");
    if (status & 0x00000020) printf(" DMASlvErr");
    if (status & 0x00000040) printf(" DMADecErr");
    if (status & 0x00000100) printf(" SGIntErr");
    if (status & 0x00000200) printf(" SGSlvErr");
    if (status & 0x00000400) printf(" SGDecErr");
    if (status & 0x00001000) printf(" IOC_Irq");
    if (status & 0x00002000) printf(" Dly_Irq");
    if (status & 0x00004000) printf(" Err_Irq");
    printf("\n\r");
    status = dma_get(MM2SStatusRegister);
    printf("MM2S status (0x%08x@0x%02x):", status, MM2SStatusRegister);
    if (status & 0x00000001) printf(" halted");
	else 					 printf(" running");
    if (status & 0x00000002) printf(" idle");
    if (status & 0x00000008) printf(" SGIncld");
    if (status & 0x00000010) printf(" DMAIntErr");
    if (status & 0x00000020) printf(" DMASlvErr");
    if (status & 0x00000040) printf(" DMADecErr");
    if (status & 0x00000100) printf(" SGIntErr");
    if (status & 0x00000200) printf(" SGSlvErr");
    if (status & 0x00000400) printf(" SGDecErr");
    if (status & 0x00001000) printf(" IOC_Irq");
    if (status & 0x00002000) printf(" Dly_Irq");
    if (status & 0x00004000) printf(" Err_Irq");
    printf("\n\r");
}
/*****************************************************************************/
/**
 * Map AXIDMA_BASE_ADDR
 *
 ******************************************************************************/
void AXIDMABaseMemoryMap(int fd)
{

	axidma_base = (unsigned int*)malloc(65536);
    axidma_base = mmap(axidma_base, 65536, PROT_READ | PROT_WRITE,
    		MAP_SHARED, fd, AXIDMA_BASE_ADDR);
    memset(axidma_base, 0, 65536);
    /*
    axidma_base = (unsigned int*)malloc(16*1024);
    axidma_base = mmap(axidma_base, 16*1024, PROT_READ | PROT_WRITE,
    		MAP_SHARED, fd, AXIDMA_BASE_ADDR);
    memset(axidma_base, 0, 16*1024);*/
}
/*****************************************************************************/
/**
 * Set AXIDMA Register value
 * @note	None
 *
 ******************************************************************************/
void dma_set(int offset, unsigned int value)
{
	axidma_base[offset >> 2]= value;
}
/*****************************************************************************/
/**
 * Get AXIDMA Register value
 * @note	None
 *
 ******************************************************************************/
unsigned int dma_get( int offset)
{
	return axidma_base[offset >> 2];
}

