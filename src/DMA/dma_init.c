#include <dma_init.h>

int fdTemp =-1 ;
uint32_t* tx_packet;				// A pointer to tx buffer area.
uint32_t* rx_packet;				// A pointer to rx buffer area.
unsigned long FrameCounter=0;
int rec_len = 0;
int ber_num = 0;


int dma_tramsmit_test()
{
	printf("\n********************DMA MM2S S2MM Test(build date:190823.1456)************\n");
	printf("MM2S means DDR 2 PL.\n");
	printf("S2MM means PL 2 DDR.\n");
	printf("****************************************************\n");
	fdTemp = Open_mem();
	TxRxMemoryMap(fdTemp);							 // Memory map from physical address to logical address
	AXIDMABaseMemoryMap(fdTemp);
	InitialAXIDMA();
//	Xil_Out32(XPAR_AXI_GPIO_0_BASEADDR,0xffff);      //just needed for zynq_dma_test2 bit file
	printf("AXI DMA Initialization completed3.\n");

	int i;
	for (i = 0; i < TX_PKT_SIZE / 4; i++) tx_packet[i]= i;

	signal(SIGALRM, (void *)SpeedCalculate);//��⵽SIGALRM�źž�ִ��SpeedCalculate()����
	alarm(1);//��ʱ�� ÿ��1s����һ��SIGALRM�ź�
	while(1)
	{
		StartDMATransfer();						// Start DMA transfer.
		FlushReceivingBuffer(FrameCounter);			// Check DMA result.
		FrameCounter++;
	}
	Close_mem();
	return 0;
}


void InitialAXIDMA()
{
	ResetAXIDMAS2MM();
	ResetAXIDMAMM2S();
	//printf("MM2S and S2MM has been reset.\n");
	//PrintAXIDMAStatus();
	StopAXIDMAS2MM();
	StopAXIDMAMM2S();
	//printf("MM2S and S2MM has been halted.\n");
	//PrintAXIDMAStatus();
	SetAXIDMAMM2SBufferAddress(AXIDMAMM2SBufferPhysicalAddress);
	SetAXIDMAS2MMBufferAddress(AXIDMAS2MMBufferPhysicalAddress);
	//printf("Source and destination addresses have been set.\n");
	//PrintAXIDMAStatus();
	EnableAXIDMAMM2SIrq();
	EnableAXIDMAS2MMIrq();
	//printf("MM2S and S2MM has been started.\n");
	//PrintAXIDMAStatus();
}
/*****************************************************************************/
/**
 * Memory map from physical address to logical address(Linux only)
 * @fd	None
 *
 ******************************************************************************/
void TxRxMemoryMap(int fd)
{
	tx_packet = (u32*)malloc(TX_PKT_SIZE);
	// Memory map tx buffer base address.
	tx_packet = mmap(tx_packet, TX_PKT_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, AXIDMAMM2SBufferPhysicalAddress);
	rx_packet = (u32*)malloc(RX_PKT_SIZE);
	// Memory map rx buffer base address.
	rx_packet = mmap(rx_packet, RX_PKT_SIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, fd, AXIDMAS2MMBufferPhysicalAddress);
	memset(tx_packet, 0, TX_PKT_SIZE);
	memset(rx_packet, 0, RX_PKT_SIZE);
	printf("The DMA buffer size is %d Bytes\n",TX_PKT_SIZE);
}
// Start DMA transfer.



void dma_tramsmit(uint16_t * Send_data,uint32_t send_num)
{
	ResetAXIDMAMM2S();
	for (int i = 0; i < send_num; i++)
		{
			tx_packet[2*i]= Send_data[i];
			tx_packet[2*i + 1] = 0;
			//printf("tx_packet[%d] = %d\n",2*i,tx_packet[2*i]);
		}
	printf("DMA start to send\n");
	ClearAXIDMAMM2SInterrupt();
	SetAXIDMAMM2STransferingBufferSize(send_num*8);
	//IsMM2SIOC();
	printf("DMA send done\n");
}

void dma_transmit_32(uint32_t * Send_data,uint32_t send_num)
{
	for (int i = 0; i < send_num; i++)
		{
			tx_packet[i]= Send_data[i];
		}
	printf("DMA start to send\n");
	ClearAXIDMAMM2SInterrupt();
	SetAXIDMAMM2STransferingBufferSize(send_num*4);
	IsMM2SIOC();
	printf("DMA send done\n");
}

void dma_tramsmit_DecOr(uint32_t * Send_data , uint32_t total_num)
{
	for (int i = 0; i < total_num; i++)
		{
			tx_packet[2*i]= Send_data[i];
			tx_packet[2*i + 1] = 0;
		}
	//printf("DMA start to send\n");
	ClearAXIDMAMM2SInterrupt();
	SetAXIDMAMM2STransferingBufferSize(total_num * 8);
	IsMM2SIOC();
}

void dma_tramsmit_init()
{
	fdTemp = Open_mem();
	TxRxMemoryMap(fdTemp);							 // Memory map from physical address to logical address
	AXIDMABaseMemoryMap(fdTemp);
	InitialAXIDMA();
}


//void dma_recv(uint16_t  * Recv_data)
//{
//	int recv_len = 0;
//	start_read_pulse();
//	memset(rx_packet, 0, RX_PKT_SIZE);
//	ClearAXIDMAS2MMInterrupt();
//	SetAXIDMAS2MMTransferingBufferSize(RX_PKT_SIZE);
////	IsS2MMIOC();
//
//	recv_len = dma_get( S2MMTransferLengthRegister);
//	printf("recv done!recv_len = %d\n",recv_len);
//
//	for (int i = 0; i < 160; i++)
//		Recv_data[i] = rx_packet[i];
//
//}
int dma_recv_32(uint32_t  * Recv_data)	//返回字节数
{
	int recv_len = 0;
	start_read_pulse();
	memset(rx_packet, 0, RX_PKT_SIZE);
	ClearAXIDMAS2MMInterrupt();
	SetAXIDMAS2MMTransferingBufferSize(RX_PKT_SIZE);
	IsS2MMIOC();

	recv_len = dma_get( S2MMTransferLengthRegister);
	printf("recv done!recv_len = %d\n",recv_len);
	uint32_t words_received = recv_len / sizeof(uint32_t);
	for (int i = 0; i < words_received; i++)
		Recv_data[i] = rx_packet[i];
	return recv_len;
}


void StartDMATransfer()
{
	ClearAXIDMAS2MMInterrupt();
	ClearAXIDMAMM2SInterrupt();
	// Start MM2S transfer by setting length registers.
	SetAXIDMAMM2STransferingBufferSize(TX_PKT_SIZE);
	// Start S2MM transfer by setting length registers.
	SetAXIDMAS2MMTransferingBufferSize(RX_PKT_SIZE);
	//IsMM2SIdle();
//printf("INTO IsMM2SIOC()\n");
	IsMM2SIOC();
	//IsS2MMIdle();
//printf("INTO IsS2MMIOC()\n");

	IsS2MMIOC();
}
// Check DMA result.
void FlushReceivingBuffer(unsigned long Index)
{
    int i;
	rec_len = dma_get(S2MMTransferLengthRegister);
//    unsigned char *rx_buff = (unsigned char *)rx_packet;
	//printf("rx_len = %d,rx_packet[] : \n",rec_len);
    for (i = 0; i < rec_len/4; i++)
    {
    	if (rx_packet[i] != tx_packet[i])
    	{
    		printf("Wrong result at rx_packet[%d]\n", i);
    		ber_num++;
    		//return;
    	}
    }
    //printf("DMA transferring received data ber is %d\% \n",ber_num/rec_len*100);
	//printf("\n");
    memset(rx_packet, 0, RX_PKT_SIZE);
}

void SpeedCalculate(void)
{
	printf("DMA transferring speed: %.2f MB/s\n", (float)FrameCounter * TX_PKT_SIZE / Size_1M);
//	printf("DMA transferring received data ber is %d\% \n",ber_num/rec_len*100);
	for (int i = 0; i < 20; i++)
	{
	    printf("tx_packet[%d] is %d\n", i,tx_packet[i]);
	    printf("rx_packet[%d] is %d\n", i,rx_packet[i]);
	}
	FrameCounter = 0;
	alarm(1);
}
