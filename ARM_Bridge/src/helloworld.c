/*
 * Copyright (c) 2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#include <dma_init.h>
#include "fc_hal.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <arpa/inet.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//连接上位机的TCP接口
//调用socket函数返回的文件描述符
int serverSocket;
//声明两个套接字sockaddr_in结构体变量，分别表示客户端和服务器
struct sockaddr_in server_addr;
struct sockaddr_in clientAddr;
int addr_len = sizeof(clientAddr);
int client;
int serveport;
char data_880b[110];
int recv_upper_DataNum = 0;
int recv_upper_count  = 0;
int recv_upper_Bytes = 0;
int recv_upper_32bitNum = 0;
int iDataNum;
int tcp_send_len = 0;

uint32_t recv_upper_data[1600];
uint32_t recv_data[1600];
uint32_t reset;
uint32_t reg_val;

uint32_t total_send_count = 0; // 要发送的 uint32_t 个数
uint32_t total_send_bytes = 0; // 总发送字节数

#define END_FLAG 0
#define BUFFER_SIZE 10  // 环形缓冲区大小
uint32_t temp_data;  // 临时存储接收的32位数据

// 环形缓冲区结构体
typedef struct {
    uint32_t buffer[BUFFER_SIZE];  // 存储数据的数组
    int read_idx;                  // 读指针（下一个待读取的索引）
    int write_idx;                 // 写指针（下一个待写入的索引）
    int count;                     // 当前数据个数
} RingBuffer;

// 初始化环形缓冲区
void ring_buffer_init(RingBuffer* rb) {
    rb->read_idx = 0;
    rb->write_idx = 0;
    rb->count = 0;
}

// 写入数据（返回是否成功）
_Bool ring_buffer_write(RingBuffer* rb, uint32_t data) {
    if (rb->count >= BUFFER_SIZE) {
        return false;  // 缓冲区满，写入失败
    }
    rb->buffer[rb->write_idx] = data;               		// 写入数据
    rb->write_idx = (rb->write_idx + 1) % BUFFER_SIZE;  	// 写指针后移（循环）
    rb->count++;                                    		// 计数+1
    return true;
}

// 读取数据（返回是否成功，数据通过out参数传出）
_Bool ring_buffer_read(RingBuffer* rb, uint32_t* out) {
    if (rb->count == 0) {
        return false;  // 缓冲区空，读取失败
    }
    *out = rb->buffer[rb->read_idx];                // 读取数据
    rb->read_idx = (rb->read_idx + 1) % BUFFER_SIZE;   // 读指针后移（循环）
    rb->count--;                                    // 计数-1
    return true;
}





/**
 * @brief 将32位无符号整数以二进制格式打印，每8位添加空格分隔
 * @param num 待打印的32位无符号整数
 * @note 用于寄存器值的二进制可视化调试，从最高位（31位）到最低位（0位）打印
 */
void print_binary(uint32_t num) {
    for (int i = 31; i >= 0; i--) {
        printf("%d", (num >> i) & 1);  // 从最高位到最低位逐位打印
        if (i % 8 == 0 && i != 0)      // 每 8 位加空格，方便阅读
            printf(" ");
    }
    printf("\n");
}

/**
 * @brief 向FPGA写入数据块数量指令，控制FPGA单次处理的数据块规模
 * @param num 数据块数量（最大值限制为128，超出则报错）
 * @note 指令格式：低7位存储数据块数量（二进制），第7位（bit7）置1作为指令标志位
 *       最终通过set_Reg函数写入FPGA控制寄存器
 */
void w_instruct(uint32_t num){	//num数据块个数
	if(num > 128){
		printf("error");
	}
	uint32_t instruct = 0;
	uint32_t binary = 0;
	int bitPos = 0;
	while (num > 0 && bitPos < 7) {  // 限制7位
		binary |= (num % 2) << bitPos;  // 按位写入
		num /= 2;
		bitPos++;
	}
	instruct = binary + (1<<7);
	set_Reg(instruct);
}

/**
 * @brief 测试FPGA在不同数据块大小下的DMA传输速度及响应性能
 * @param recv_upper_data 指向从TCP接收的上位机数据缓冲区（用于向FPGA发送测试数据）
 * @note 测试逻辑：
 *       1. 遍历预设的15种数据块数量（speed_table），依次进行测试
 *       2. 每次测试前复位FPGA，确保初始状态一致
 *       3. 向FPGA发送当前测试的数据块数量指令
 *       4. 根据FPGA寄存器状态，通过DMA发送/接收数据，直至完成一次交互
 */
void speedTest(uint32_t *recv_upper_data){
	uint32_t reset;
	uint32_t reg_val;
	uint32_t recv_data;
	int recv_bytes;
	int speed_table[15] = {75,60,50,45,40,36,30,24,20,15,12,8,4,2,1};

	for(int testnum = 0 ;testnum < 15; testnum++){
		printf("test No. %d\n",testnum);
		reset = (1 << 9) ;
		set_Reg(reset);	//复位fpga
		int datablock = speed_table[testnum];
		printf("send datablock num %d\n",datablock);
		w_instruct(datablock);
		reg_val = read_Reg();
		if((reg_val & (1U << 7)) == 0){	//第8位是0，可以dma写
//			printf("ready to dma write\n");
			dma_transmit_32(recv_upper_data,datablock*20);
		}else{
			if((reg_val & (1U << 8)) != 0){	//第9位是1，可以dma读
//				printf("ready to dma read\n");
				dma_recv_32(recv_data);
			}
		}
		while(1){
			reg_val = read_Reg();
			print_binary(reg_val);
			if((reg_val & (1U << 8)) != 0){	//第9位是1，可以dma读
//				printf("ready to dma read\n");
				recv_bytes = dma_recv_32(recv_data);
				printf("recv datablock num %d\n",recv_bytes/80);
				break;
			}
		}
	}
}

// 发送一个32位整数（确保完整发送）
int sendInt(int sock, uint32_t data) {
    int total_sent = 0;
    int left = sizeof(uint32_t);
    char* buf = (char*)&data;
    while (total_sent < left) {
        int sent = send(sock, buf + total_sent, left - total_sent, 0);
        if (sent == -1) {
            perror("send failed");
            return -1;
        }
        total_sent += sent;
    }
    return 0;
}

// 接收一个32位整数（确保完整接收）
int recvInt(int sock, uint32_t* data) {
    int total_recv = 0;
    int left = sizeof(uint32_t);
    char* buf = (char*)data;
    while (total_recv < left) {
        int recv_len = recv(sock, buf + total_recv, left - total_recv, 0);
        if (recv_len == -1) {
            perror("recv failed");
            return -1;
        } else if (recv_len == 0) {
            printf("客户端已断开连接\n");
            return -1;
        }
        total_recv += recv_len;
    }
    return 0;
}

void socket_init()
{
    //socket函数，失败返回-1
    //int socket(int domain, int type, int protocol);
    //第一个参数表示使用的地址类型，一般都是ipv4，AF_INET
    //第二个参数表示套接字类型：tcp：面向连接的稳定数据传输SOCK_STREAM
    //第三个参数设置为0
	if((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return 1;
	}

	bzero(&server_addr, sizeof(server_addr));
    //初始化服务器端的套接字，并用htons和htonl将端口和地址转成网络字节序
	server_addr.sin_family = AF_INET;
	serveport = 8889;
	server_addr.sin_port = htons(serveport);
    //ip可是是本服务器的ip，也可以用宏INADDR_ANY代替，代表0.0.0.0，表明所有地址
	server_addr.sin_addr.s_addr = inet_addr("192.168.8.3");
    //对于bind，accept之类的函数，里面套接字参数都是需要强制转换成(struct sockaddr *)
    //bind三个参数：服务器端的套接字的文件描述符，
    if(bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("connect");
		return 1;
	}
    //设置服务器上的socket为监听状态
	if(listen(serverSocket, 5) < 0)
	{
		perror("listen");
		return 1;
	}
	printf("Listening on port: %d\n", serveport);

}

/**
 * @brief 程序主入口，负责TCP服务器初始化、上位机数据接收、调用速度测试函数
 * @return 0：程序正常退出；非0：异常退出
 * @note 核心流程：
 *       1. 初始化DMA传输模块
 *       2. 创建TCP服务器，绑定端口8889，监听上位机连接
 *       3. 接收上位机数据（以0xFFFFFFFF为结束标志）
 *       4. 调用speedTest函数，使用接收的数据进行FPGA速度测试
 *       5. （注释部分）预留数据回传上位机的逻辑
 */
int main()
{
	//DMA初始化
	dma_tramsmit_init();
	socket_init();
	RingBuffer rb;
	ring_buffer_init(&rb);

//		while(1)
//		{
			client = accept(serverSocket, (struct sockaddr*)&clientAddr, (socklen_t*)&addr_len);
			if(client < 0)
			{
				perror("accept");
//				continue;
			    close(serverSocket); // 失败时关闭服务器 socket
			    return 1;
			}
			printf("\nrecv client data...\n");
	        //inet_ntoa   ip地址转换函数，将网络字节序IP转换为点分十进制IP
	        //表达式：char *inet_ntoa (struct in_addr);
			printf("IP is %s\n", inet_ntoa(clientAddr.sin_addr));
			printf("Port is %d\n", htons(clientAddr.sin_port));

			// 初始化随机数种子
			    srand(time(NULL));

			int send_count = rand() % 3 + 3;
			uint32_t n = rand() % 5 + 1;
			printf("send_count:%d,  n:%d\n",send_count,n);
			for(send_count;send_count >0;send_count--){
				//发送长度指令n（1-5之间）
				n = rand() % 5 + 1;
				if (sendInt(client, n) == -1) {
					printf("sendInt failed!\n");
					return 1;
				}
				printf("Application data:%d\n",n);

				//接收数据
				for(int m = n;m > 0; m--)
				{
					int bytes_received = recv(client,
											  (char*)&temp_data,  // 存储到当前索引位置
											  sizeof(uint32_t),
											  0);
					//存入环形缓冲区
					if (ring_buffer_write(&rb, temp_data)) {
						// 打印当前接收的uint32_t
						printf("Received[%d]: 0x%08X\n", recv_upper_count, temp_data);
					} else {
						printf("The circular buffer is full,Discard data:0x%08X\n", temp_data);
					}

					if (bytes_received <= 0) {
						if (bytes_received == 0) {
							printf("Connection closed by peer\n");
						} else {
							perror("recv failed\n");
						}
							break;
					}

					if (bytes_received != sizeof(uint32_t)) {
						printf("Received incomplete data\n");
						continue;
						}



					recv_upper_count++;  // 移动到下一个存储位置
				}
			}
			//结束通信
			if (sendInt(client, END_FLAG) == -1) {
				printf("sendInt failed");
				return 1;
			}

			printf("recv_upper_count = %d\n",recv_upper_count);	//有效字节数
			recv_upper_32bitNum = recv_upper_count;	//记录接收到的字节数

			total_send_count = recv_upper_count;
			total_send_bytes = total_send_count * sizeof(uint32_t); // 总发送字节数


			//人为创造误码
			for(int error_bit = 0;error_bit < 10;error_bit ++)
			{
				uint32_t error_data_id = rand() % recv_upper_count;
				uint32_t error_bit_id = rand() % 32;
				recv_upper_data[error_data_id]  ^= 1 << error_bit_id;
				printf("error_data[%d]:%d\n",error_data_id,error_bit_id);
			}


			recv_upper_count = 0;	//将本轮接收器归零代表接收结束

			/***************************服务器端发送数据部分**************************/
//			// 发送数据
//			int sent_bytes = send(
//			    client,                  // 客户端 socket 描述符（必须用 accept 返回的 client）
//				recv_upper_data,         // 要发送的数据缓冲区
//			    total_send_bytes,        // 要发送的总字节数
//			    0                        // 发送标志（0 表示默认阻塞发送，确保数据尽可能发送）
//			);
//
//			// 检查发送结果
//			if (sent_bytes < 0) {
//			    perror("send failed");
//			} else if (sent_bytes != total_send_bytes) {
//			    // 部分数据发送成功
//			    printf("Partial send: expected %d bytes, sent %d bytes\n", total_send_bytes, sent_bytes);
//			} else {
//			    printf("Send complete: %d bytes sent to client\n", sent_bytes);
//			}


			/*****************************************************************/
			/***************************处理数据部分**************************/
//			speedTest(recv_upper_data);
//
//			reset = (1 << 9) ;
//			set_Reg(reset);	//复位fpga
//			while(1){
//				w_instruct(1);
//				reg_val = read_Reg();
//
//				if((reg_val & (1U << 7)) == 0){	//第8位是0，可以dma写
//					printf("ready to dma write\n");
//					dma_transmit_32(recv_upper_data,recv_upper_32bitNum);
//				}else{
//					if((reg_val & (1U << 8)) != 0){	//第9位是1，可以dma读
//						printf("ready to dma read\n");
//						start_decode();
//						dma_recv(recv_data);
//					}
//				}
//			}
//
//
//			/************************************************************/
//			printf("start send\n");
//			int bytes_to_send = recv_upper_32bitNum * sizeof(uint32_t);
//			int sent_bytes = send(client, recv_upper_data, bytes_to_send, 0);
//			if (sent_bytes != bytes_to_send) {
//			    perror("[ERROR] Partial data sent");
//			    printf("Expected: %d, Actual: %d\n", bytes_to_send, sent_bytes);
//			}
//			uint32_t end_marker = 0xFFFFFFFF;
//			send(client, (char*)&end_marker, sizeof(end_marker), 0);	//发送结束标志
//			printf("finish send\n");


//		}

		close(serverSocket);
		return 0;
}
