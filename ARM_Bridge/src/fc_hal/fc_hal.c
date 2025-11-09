/*
 * fc_hal.c
 *
 *  Created on: 2019濠德板�楁慨鐑藉磿閹达箑绠柨鐕傛嫹1闂傚倷绀侀幖顐︽偋閻愬搫绠柨鐕傛嫹3闂傚倷绀侀幖顐﹀窗濞戙垹绠柨鐕傛嫹
 *      Author: XXXX
 */
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
//#include <xil_io.h>
#include "fc_hal.h"
#include "AXI_IO.h"
#define SPI_HARDWARE_MODE//spi缂傚倷鑳堕搹搴ㄥ矗鎼淬劌绀傛繛鎴炃氶弸搴ㄦ煏瀵版盯顣︾划鎾⒑閸涘娈橀柛瀣椤ゅ嫭绻濈喊妯活潑闁割煈浜弫鎾绘寠婢跺矉绱炵紒缁㈠弿閹凤拷
//#define SPI_SOFTWARE_MODE//spi闂備礁鎼ˇ閬嶅磿閹版澘鍨傛い鏍ㄥ焹閺嬪酣鏌曞娑㈩暒缁挳姊洪崨濠庢畼闁稿绋栭·鍕節绾版ɑ顫婇柛顭戜邯閺佹捇鎸婃径宀嬬礊缂佺虎鍙忛幏锟�
//*******************************************************************//
//    common
//*******************************************************************//
//void usleep(uint32_t delay)
//{
//	while(delay--);
//}
void IOWR_32BIT(uint32_t offsetaddr,uint32_t value)
{
	Xil_Out32(fc_ps_ctrl_itf_addr+(offsetaddr<<2),value);
}
uint32_t IORD_32BIT(uint32_t offsetaddr)
{
	return Xil_In32(fc_ps_ctrl_itf_addr+(offsetaddr<<2));
}

/************************************************************************************
 * ARM TO FPGA/FPGA TO ARM : CRTLWORD
 ************************************************************************************/
//ARM TO FPGA

void set_Reg(uint32_t val)
{
	IOWR_32BIT(reg_r_w_addr,val);
}

//void set_IterNum(uint32_t val)
//{
//	IOWR_32BIT(IterNum_addr,val);
//}
//
//void set_sigma(uint32_t val)
//{
//	IOWR_32BIT(sigma_addr,val);
//}
//
//void set_DecodeGroupNum(uint32_t val)
//{
//	IOWR_32BIT(DecodeGroupNum_addr,val);
//}
//
//void set_LayerNumInLastGroup(uint32_t val)
//{
//	IOWR_32BIT(LayerNumInLastGroup_addr,val);
//}
//
//void set_sign_num(uint32_t val)
//{
//	IOWR_32BIT(sign_num_addr,val);
//}


//void start_decode()
//{
//	IOWR_32BIT(start_addr,0);
//	IOWR_32BIT(start_addr,1);
//	IOWR_32BIT(start_addr,0);
//}
//void reset_pl()
//{
//	IOWR_32BIT(fpga_logic_reset_waddr,0);
//	IOWR_32BIT(fpga_logic_reset_waddr,1);
//	IOWR_32BIT(fpga_logic_reset_waddr,0);
//}
void start_send_res()
{
	IOWR_32BIT(reg_r_w_addr,0);
	IOWR_32BIT(reg_r_w_addr,1);
	IOWR_32BIT(reg_r_w_addr,0);
}
void start_read_pulse()
{
	uint32_t signal_H;
	uint32_t signal_L;
	signal_H = (1<<8);
	signal_L = 0;
	set_Reg(signal_L);
	set_Reg(signal_H);
	set_Reg(signal_L);
}



//FPGA TO ARM
uint32_t dma_wr_data_num_read()
{
	uint32_t rdata = 0;
	rdata = IORD_32BIT(reg_r_w_addr);
	return rdata;
}

uint32_t read_Reg()
{
	uint32_t rdata = 0;
	rdata = IORD_32BIT(reg_r_w_addr);
	return rdata;
}

uint32_t read_slot()
{
	uint32_t rdata = 0;
	rdata = IORD_32BIT(reg_slot_addr);
	return rdata;
}
//uint32_t res_fifo_data_num_read()
//{
//	uint32_t rdata = 0;
//	rdata = IORD_32BIT(res_fifo_data_num_raddr);
//	return rdata;
//}

