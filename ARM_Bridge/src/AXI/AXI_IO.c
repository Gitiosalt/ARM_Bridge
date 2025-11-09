/*******************************************************************************
***   File Name: AXI_IO.c
***   Description: The interface of AXI
***   Create Date: 2018-02-01
***   Revision History:
***   Author: FC
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include"xparameters.h"
#include "AXI_IO.h"
int fd =-1 ;
int Open_mem()
{
  if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
	{
		perror("open /dev/mem:");
		return fd;
	}
	return fd;
}

void Close_mem()
{
	 close(fd);
}

void Mmap()
{
	printf("PAGE_SIZE:%d\n\r",PAGE_SIZE);
	/*map0base = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, XPAR_AXI_FIFO_0_BASEADDR );


	map1base = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
					fd, XPAR_AXI_FIFO_1_BASEADDR );
*/
	if(map0base == MAP_FAILED||map1base == MAP_FAILED)
	{
		perror("mmap:");
	}
	else
	{
		printf("mmap done.\n\r");
	}
}

void Munmap()
{
	munmap((void *)map0base, PAGE_SIZE);

	munmap((void *)map1base, PAGE_SIZE);
}
void fifo0_out32(unsigned int addroffset, unsigned int value)
{
	*(volatile unsigned int *)(map0base + addroffset) = value;
}
 int fifo0_in32(unsigned int addroffset)
{
	int val;
	val = *(volatile unsigned int *)(map0base + addroffset);

	return val;
}
void fifo1_out32(unsigned int addroffset, unsigned int value)
{
	*(volatile unsigned int *)(map1base + addroffset) = value;
}
 int fifo1_in32(unsigned int addroffset)
{
	int val;
	val = *(volatile unsigned int *)(map1base + addroffset);

	return val;
}
void Out32(unsigned int phyaddr,  int val)
{
	volatile unsigned char *map_base;
	unsigned int base = phyaddr & PAGE_MASK;
	unsigned int pgoffset = phyaddr & (~PAGE_MASK);
	map_base = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, base);
	if(map_base == MAP_FAILED)
	{
		perror("mmap:");
	}
	*(volatile unsigned int *)(map_base + pgoffset) = val;
	munmap((void *)map_base, PAGE_SIZE);
}

 int In32(unsigned int phyaddr)
{
	int val;
	volatile unsigned char *map_base;
	unsigned int base = phyaddr & PAGE_MASK;
	unsigned int pgoffset = phyaddr & (~PAGE_MASK);
	map_base = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, base);
	if(map_base == MAP_FAILED)
	{
		perror("mmap:");
	}
	val = *(volatile unsigned int *)(map_base + pgoffset);
	munmap((void *)map_base, PAGE_SIZE);
	return val;
}

void Xil_Out32(unsigned int addrBase, int value)
{
    Out32(addrBase, value);
}

 int Xil_In32(unsigned int addrBase)
{
	int ans=0;
    ans=In32(addrBase);
    return ans;
}

///--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
void mmap_base_0()
{
	unsigned int base = XPAR_AXI_FIFO_0_BASEADDR & PAGE_MASK;
	map_base_0 = mmap(NULL, 0x30, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, base);
	if(map_base_0 == MAP_FAILED)
	{
		perror("mmap0:");
	}
}


int readFifo_baseAddr_0(unsigned int offset)
{
	unsigned int pgoffset = offset & (~PAGE_MASK);
	return (volatile unsigned int *)(map_base_0 + pgoffset);
}

int writeFifo_baseAddr_0(unsigned int offset,int val)
{
	unsigned int pgoffset = offset & (~PAGE_MASK);
	*(volatile unsigned int *)(map_base_0 + pgoffset) = val;
	return 1;
}
//baseaddr1
void mmap_base_1()
{
	unsigned int base = XPAR_AXI_FIFO_1_BASEADDR & PAGE_MASK;
	map_base_1 = mmap(NULL, 0x30, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd, base);
	if(map_base_0 == MAP_FAILED)
	{
		perror("mmap1:");
	}
}
int readFifo_baseAddr_1(unsigned int offset)
{
	unsigned int pgoffset = offset & (~PAGE_MASK);
	return (volatile unsigned int *)(map_base_1 + pgoffset);
}

int writeFifo_baseAddr_1(unsigned int offset,int val)
{
	unsigned int pgoffset = offset & (~PAGE_MASK);
	*(volatile unsigned int *)(map_base_1 + pgoffset) = val;
	return 1;
}
*/
