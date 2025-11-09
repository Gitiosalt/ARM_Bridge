/*
 * main.h
 * AXI_DMA test using AXI_DMA & AXI_IO
 * Simple DMA Handle example
 * Direct Register Mode
 *  Created on: 2019��1��25��
 *      Author: FC
 */
#ifndef DMA_INIT_H_
#define DMA_INIT_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/mman.h>
#include <signal.h>
#include "AXI_DMA.h"
#include "AXI_IO.h"
typedef unsigned int u32;
#define TX_PKT_SIZE						2048 * 4 	// Size of TX Packet(0~0x7FFFFF)
#define RX_PKT_SIZE					    2048 * 4	// Size of RX Packet(0~0x7FFFFF)
//#define RX_PKT_SIZE						1024	// Size of RX Packet(0~0x7FFFFF)

#define Size_1K								0x400
#define Size_1M								0x100000

void InitialAXIDMA();
void TxRxMemoryMap(int fd);
void StartDMATransfer();
void FlushReceivingBuffer(unsigned long Index);
void SpeedCalculate(void);
int dma_tramsmit_test();
void dma_tramsmit_init();
void dma_tramsmit(uint16_t * Send_data,uint32_t send_num);
void dma_tramsmit_32(uint32_t * Send_data,uint32_t send_num);
#endif /* DMA_INIT_H_ */
