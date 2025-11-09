/*******************************************************************************
***   File Name: AXI_IO.h
***   Description: Definitions of AXI_IO
***   Create Date: 2018-02-01
***   Revision History:
***   Author: FC
*******************************************************************************/
#ifndef AXI_IO_H_
#define AXI_IO_H_

#define PAGE_SIZE  ((size_t)getpagesize())
#define PAGE_MASK ((unsigned long)~(PAGE_SIZE - 1))

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

volatile unsigned char *map0base;
volatile unsigned char *map1base;
unsigned int* axidma_base;			// A pointer to axidma base address.
int Open_mem();
void Close_mem();
void Mmap();
void Munmap();
void Out32(unsigned int phyaddr,  int val);
int In32(unsigned int phyaddr);
void fifo0_out32(unsigned int addroffset, unsigned int value);
int fifo0_in32(unsigned int addroffset);
void fifo1_out32(unsigned int addroffset, unsigned int value);
int fifo1_in32(unsigned int addroffset);
void TempOut32(unsigned int phyaddr, int val);
int TempIn32(unsigned int phyaddr);
void Xil_Out32(unsigned int addrBase, int value);
int Xil_In32(unsigned int addrBase);

#endif /* AXI_IO_H_ */
