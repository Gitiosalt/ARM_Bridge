/*
 * fc_hal.h
 *
 *  Created on: 2019��1��3��
 *      Author: XXXX
 */
#include "xparameters.h"
#ifndef fc_HAL_H_
#define fc_HAL_H_
//*******************************************************************************************//
//type define
//*******************************************************************************************//
typedef struct
{
    uint8_t  spiDevice;
}ADCDevice_t;
typedef enum
{
	false = 0,
	true = 1
}BoolValue_t;
typedef enum
{
	clockDevice = 0,
	adcDevice   = 1
}SpiDevIndx_t;

//*******************************************************************************************//
#define   fc_ps_ctrl_itf_addr           0x43c00000
#define   fc_ps_ctrl_ift_clock          100e6

//*******************************************************************************************//
//----------------ps2pl interface-------------//
//*******************************************************************************************//
#define	  reg_r_w_addr						0x00
#define	  reg_slot_addr						0x01
//#define   fpga_logic_reset_waddr            0x01
//#define   IterNum_addr            			0x02
//#define   sigma_addr            			0x03
//#define   start_addr            			0x04
//#define   DecodeGroupNum_addr            	0x05
//#define   LayerNumInLastGroup_addr          0x06
//#define   start_send_res_addr            	0x07
//#define   sign_num_addr            			0x08


//*******************************************************************************************//
//----------------pl2ps interface--------------//
//*******************************************************************************************//
#define   decode_end_raddr                  0x00
#define   DecodeSucc_raddr               	0x01
#define   dma_wr_data_num_raddr             0x02
#define   res_fifo_data_num_raddr           0x03
//*******************************************************************************************//
//bit location
//*******************************************************************************************//
//ADC
//#define		ADC_RX2_ENABLE		5
//#define		ADC_RX1_ENABLE		4
//#define		ADC_RESETN			3
#define		ADC_SPI_CLK		    2
#define		ADC_SPI_CS			1
#define		ADC_SPI_DIN		    0
////AD9528
//#define		AD9528_RESETN		4
//#define		AD9528_SYS_REQ		3
#define		AD9528_SPI_CLK		2
#define		AD9528_SPI_CS	    1
#define		AD9528_SPI_DIN		0
//*******************************************************************************************//
//common
//*******************************************************************************************//
//void usleep(uint32_t delay);
void IOWR_32BIT(uint32_t offsetaddr,uint32_t value);
uint32_t IORD_32BIT(uint32_t offsetaddr);

void pstopl_StartSendDecRes();

void set_Reg(uint32_t val);
//void set_IterNum(uint32_t val);
//void set_sigma(uint32_t val);
//void set_DecodeGroupNum(uint32_t val);
//void set_LayerNumInLastGroup(uint32_t val);
//void set_sign_num(uint32_t val);
void start_decode();
void reset_pl();
void start_send_res();
void start_decode();

uint32_t dma_wr_data_num_read();
uint32_t res_fifo_data_num_read();
uint32_t read_Reg();
#endif /* fc_HAL_H_ */

