/*
 * AXI_DMA.h
 * Using Xilinx AXI Direct Memory Access v7.1
 * Simple DMA Handle
 * Direct Register Mode
 *  Created on: 2019/01/23
 *      Author: FC
 */
#ifndef AXI_DMA_H_
#define AXI_DMA_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "xparameters.h"
#include "AXI_IO.h"
typedef unsigned int u32;

#define AXIDMA_BASE_ADDR					XPAR_AXI_DMA_0_BASEADDR
#define AXIDMAMM2SBufferPhysicalAddress		0x10600000 //0x0F000000//0x30000000 Phy. address of TX buffer
#define AXIDMAS2MMBufferPhysicalAddress		0x10900000 //0x0F800000//0x30800000 Phy. address of RX buffer
#define MM2SControlRegister 				0x00		// MM2S DMA Control Register
#define MM2SStatusRegister 					0x04		// MM2S DMA Status Register
#define MM2SSourceAddressRegister			0x18		// MM2S DMA Source Address Register
#define MM2STransferLengthRegister 			0x28		// MM2S DMA Transfer Length Register
#define S2MMControlRegister 				0x30		// S2MM DMA Control Register
#define S2MMStatusRegister 					0x34		// S2MM DMA Status Register
#define S2MMDestinationAddressRegister		0x48		// S2MM DMA Destination Address Register
#define S2MMTransferLengthRegister 			0x58		// S2MM DMA Buffer Length Register

void ResetAXIDMAS2MM();
void ResetAXIDMAMM2S();
void StopAXIDMAS2MM();
void StopAXIDMAMM2S();
void ClearAXIDMAS2MMInterrupt();
void ClearAXIDMAMM2SInterrupt();
void SetAXIDMAMM2SBufferAddress(unsigned int DMAMM2SBufferAddress);
void SetAXIDMAS2MMBufferAddress(unsigned int DMAS2MMBufferAddress);
void SetAXIDMAS2MMTransferingBufferSize(unsigned int value);
void SetAXIDMAMM2STransferingBufferSize(unsigned int value);
void EnableAXIDMAMM2SIrq();
void EnableAXIDMAS2MMIrq();
void RunAXIDMAS2MM();
void RunAXIDMAMM2S();
void IsMM2SIdle();
void IsMM2SIOC();
void IsS2MMIdle();
void IsS2MMIOC();
void PrintAXIDMAStatus();
void AXIDMABaseMemoryMap(int fd);
void dma_set(int offset, unsigned int value);
unsigned int dma_get( int offset);

#endif /* AXI_DMA_H_ */
