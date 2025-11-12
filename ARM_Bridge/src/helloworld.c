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
#define BUFFER_SIZE 1024  // 环形缓冲区大小
uint32_t temp_data;  // 临时存储接收的32位数据

//通信指令码
#define PC_ACTIVATE  0x01
#define ARM_RESPONSE 0x02
#define ARM_RESQUEST 0x03
#define PC_DATA      0x04
#define ARM_RESPONSE_ERROR      0xEE

// 环形缓冲区结构体
typedef struct {
    uint8_t buffer[BUFFER_SIZE]; // 实际存储数据的数组
    uint16_t write_idx;          // 写指针（索引）
    uint16_t read_idx;           // 读指针（索引）
    uint16_t count;              // 当前缓冲区中的数据量
} RingBuffer;


// 初始化环形缓冲区
void ring_buffer_init(RingBuffer* rb) {
    rb->read_idx = 0;
    rb->write_idx = 0;
    rb->count = 0;
}

// 环形缓冲区批量写入数据（返回是否成功）
_Bool ring_buffer_write(RingBuffer* rb, const uint8_t* data_buffer, uint16_t data_length) {
    if (rb == NULL || data_buffer == NULL || data_length == 0) {
        return false;
    }
    //检查是否有足够的空间写入所有数据
    if (rb->count + data_length > BUFFER_SIZE) {
        return false; // 空间不足，写入失败
    }
    //处理数据写入，分两种情况
    // a. 数据不需要换行，从 write_idx 写到缓冲区末尾
    uint16_t space_to_end = BUFFER_SIZE - rb->write_idx;

    if (data_length <= space_to_end) {
        memcpy(&rb->buffer[rb->write_idx], data_buffer, data_length);
        rb->write_idx += data_length;
    }
    // b. 数据需要换行，分两部分写入
    else {
        // 第一部分：从 write_idx 写到缓冲区末尾
        memcpy(&rb->buffer[rb->write_idx], data_buffer, space_to_end);
        // 第二部分：从缓冲区开头写到剩余数据的末尾
        uint16_t remaining_data = data_length - space_to_end;
        memcpy(&rb->buffer[0], data_buffer + space_to_end, remaining_data);

        rb->write_idx = remaining_data;
    }
    rb->count += data_length;
    return true;
}




// 环形缓冲区读取数据（返回是否成功，数据通过out参数传出）
_Bool ring_buffer_read(RingBuffer* rb, uint8_t* out) {
    if (rb->count == 0) {
        return false;  // 缓冲区空，读取失败
    }
    *out = rb->buffer[rb->read_idx];                // 读取数据
    rb->read_idx = (rb->read_idx + 1) % BUFFER_SIZE;   // 读指针后移（循环）
    rb->count--;                                    // 计数-1
    return true;
}

_Bool ring_buffer_read_batch(RingBuffer* rb, uint8_t* out_buffer, uint16_t data_length) {
    // 1. 参数合法性检查
    if (rb == NULL || out_buffer == NULL || data_length == 0) {
        return false;
    }

    // 2. 检查缓冲区是否有足够的数据
    if (rb->count < data_length) {
        printf("Error: Not enough data in ring buffer. Available: %d, Requested: %d\n", rb->count, data_length);
        return false; // 数据不足，读取失败
    }

    // 3. 处理数据读取，分两种情况
    // a. 数据不需要换行，从 read_idx 读到缓冲区末尾
    uint16_t data_to_end = BUFFER_SIZE - rb->read_idx;

    if (data_length <= data_to_end) {
        memcpy(out_buffer, &rb->buffer[rb->read_idx], data_length);
        rb->read_idx += data_length;
    }
    // b. 数据需要换行，分两部分读取
    else {
        // 第一部分：从 read_idx 读到缓冲区末尾
        memcpy(out_buffer, &rb->buffer[rb->read_idx], data_to_end);

        // 第二部分：从缓冲区开头读到剩余数据的末尾
        uint16_t remaining_data = data_length - data_to_end;
        memcpy(out_buffer + data_to_end, &rb->buffer[0], remaining_data);

        rb->read_idx = remaining_data;
    }

    // 4. 更新数据计数
    rb->count -= data_length;

    return true;
}

/**
 * @brief 将缓冲区内容以十六进制格式打印，字节之间用空格分隔。
 *
 * @param buffer 指向要打印的数据缓冲区的指针。
 * @param length 要打印的数据长度（字节数）。
 * @param prefix 一个字符串，将在打印数据前输出（用于区分不同的打印内容）。
 */
void print_hex_buffer(uint8_t* buffer, uint16_t length, const char* prefix) {
    if (buffer == NULL || length == 0) {
        if (prefix) printf("%s: No data to print.\n", prefix);
        return;
    }

    if (prefix) printf("%s:", prefix);
    for (uint16_t i = 0; i < length; i++) {
        printf(" %02X", buffer[i]);
    }
    printf("\n"); // 最后换行
}


/*
* 命令帧格式结构体
*    [1byte]   [2byte]
*   [指令类型]   [数据长度]
*/
struct command_struct
{
    uint8_t command_type;
    uint16_t command_length;
}__attribute__((packed));;   //禁用内存对齐

/*
* 数据帧格式结构体
*    [1byte]   [2byte]   [*byte]
*   [指令类型]   [数据长度]  [数据内容]
*/
struct data_struct
{
    uint8_t command_type;
    uint16_t command_length;
    uint8_t frame_data[] ;          //数据帧内容，柔性数组
}__attribute__((packed));;   //禁用内存对齐

//创建一个命令帧,在堆上分配内存！
struct command_struct* create_command_frame(uint8_t command_type,uint16_t command_length){

    struct command_struct* command_frame = (struct command_struct*)malloc(sizeof(struct command_struct));
    command_frame->command_type = command_type;
    command_frame->command_length = command_length;
    return command_frame;
}


int respond_pc(void) {
	struct command_struct pc_command_frame;
	int recvLen = recv(client,(uint8_t*)&pc_command_frame, sizeof(struct command_struct), 0);
	if (recvLen <= 0) {
		if (recvLen == 0) {
			printf("Connection closed by peer\n");
		} else {
			printf("Failed to receive the PC's response instruction! \n");
			perror("recv failed\n");
		}
		return -1;
	}

	printf("Received an instruction of %d Byte in length: %d\n", recvLen, pc_command_frame.command_type);
	if (pc_command_frame.command_type == PC_ACTIVATE) {
		printf("Activation successful!\n");
		struct command_struct* arm_response_frame = create_command_frame(ARM_RESPONSE, 0);
		int sendLen = send(client, (uint8_t*)arm_response_frame, sizeof(struct command_struct), 0);
		free(arm_response_frame);    //释放堆内存
		if (sendLen <= 0 ) {
			perror("send failed");
			return -2;
		}
		return 0;
	}
}

//向PC请求数据
int resquest_data(uint16_t data_length){
	struct command_struct* request_data_frame = create_command_frame(ARM_RESQUEST, data_length);
	int sendLen = send(client, (uint8_t*)request_data_frame, sizeof(struct command_struct), 0);
	printf("Request for %hu pieces of data\n",data_length);
	free(request_data_frame);    //释放堆内存
	if (sendLen <= 0 ) {
        perror("send failed");
		return -2;
	}
    return 0;
}

////接收来自PC的数据
//uint8_t* receive_data(uint16_t data_length){
//	uint8_t* pc_data_buffer = (uint8_t *)malloc(sizeof(struct command_struct)+data_length);
//	int recvLen = recv(client, (char*)pc_data_buffer, sizeof(struct command_struct)+data_length, 0);
//	if (recvLen <= 0 ) {
//        perror("send failed");
//		return -2;
//	}
//
//	pc_data_buffer = pc_data_buffer + sizeof(struct command_struct);
//	free(pc_data_buffer);    //释放堆内存
//    return pc_data_buffer;
//}

/////////////////////////////////////
uint8_t* receive_data(RingBuffer* rb,uint16_t data_length)
{
    // 1. 为整个数据包（头部 + 数据）分配一块临时缓冲区
    size_t total_packet_size = sizeof(struct command_struct) + data_length;
    uint8_t* temp_packet_buffer = (uint8_t*)malloc(total_packet_size);
    if (temp_packet_buffer == NULL) {
        perror("malloc for temp packet buffer failed");
        return NULL;
    }

    // 2. 接收数据
    printf("Attempting to receive %zu bytes (header + data)...\n", total_packet_size);
    int recvLen = recv(client, temp_packet_buffer, total_packet_size, 0);

    if (recvLen <= 0) {
        perror("recv failed");
        free(temp_packet_buffer); // 释放已分配的临时缓冲区
        return NULL; // 失败时返回 NULL
    } else if (recvLen < (int)total_packet_size) {
        fprintf(stderr, "Warning: Received only %d bytes out of %zu expected. Data may be incomplete.\n", recvLen, total_packet_size);
    }

    // 3. 为要返回的数据部分分配内存
    uint8_t* data_buffer = (uint8_t*)malloc(data_length);
    if (data_buffer == NULL) {
        perror("malloc for data buffer failed");
        free(temp_packet_buffer); // 释放临时缓冲区
        return NULL;
    }

    // 4. 将数据部分从临时缓冲区拷贝到新的缓冲区
    memcpy(data_buffer, temp_packet_buffer + sizeof(struct command_struct), data_length);

    // 5. 释放临时缓冲区（使用原始地址）
    free(temp_packet_buffer);

    ring_buffer_write(rb, data_buffer, data_length);  //写入环形缓冲区
    // 6. 返回指向数据部分的缓冲区
    return data_buffer;
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

/**
 * @brief 向指定套接字完整发送一个32位无符号整数
 * @param sock 已连接的套接字文件描述符
 * @param data 待发送的32位无符号整数
 * @note 发送逻辑：
 *       1. 由于底层send调用可能只发送部分数据，函数内部会循环调用send
 *       2. 每次发送从上次未发送完成的位置开始，直至所有4字节数据发送完毕
 *       3. 若发送过程中出现错误，立即打印错误信息并返回-1
 */
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

int send_command(int sock, struct command_struct* conmmand) {
    int total_sent = 0;
    int left = sizeof(struct command_struct*);
    char* buf = (char*)conmmand;
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

/**
 * @brief 从指定套接字完整接收一个command_struct
 * @param sock 已连接的套接字文件描述符
 * @param data 指向用于存储接收数据的command_struct无符号整数的指针
 * @note 接收逻辑：
 *       1. 由于底层recv调用可能只接收部分数据，函数内部会循环调用recv
 *       2. 每次接收数据存放到缓冲区的对应位置，直至接收完4字节数据
 *       3. 若接收过程中出现错误或客户端断开连接，打印信息并返回-1
 */
int recv_command(int sock, struct command_struct* conmmand) {
    int total_recv = 0;
    int left = sizeof(struct command_struct);
    char* buf = (char*)conmmand;
    while (total_recv < left) {
        int recv_len = recv(sock, buf + total_recv, left - total_recv, 0);
        if (recv_len == -1) {
            perror("recv failed");
            return -1;
        } else if (recv_len == 0) {
            printf("The client has disconnected from the connection.\n");
            return -1;
        }
        total_recv += recv_len;
    }
    return 0;
}

/**
 * @brief 初始化服务器套接字，使其绑定到指定IP和端口并进入监听状态
 * @note 初始化逻辑：
 *       1. 创建一个IPv4 (AF_INET)的TCP (SOCK_STREAM)套接字
 *       2. 配置服务器地址信息，包括地址族、端口(8889)和IP地址(192.168.8.3)
 *       3. 将创建的套接字与配置的IP地址和端口进行绑定
 *       4. 将绑定后的套接字设置为监听模式，最大等待连接数为5
 *       5. 若以上任一环节失败，打印错误信息并退出程序
 * @warning 函数依赖全局变量 serverSocket, server_addr 和 serveport，且绑定的IP地址和端口为硬编码
 */
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

void socket_connect(){
	client = accept(serverSocket, (struct sockaddr*)&clientAddr, (socklen_t*)&addr_len);
	if(client < 0)
	{
		perror("accept");
		close(serverSocket); // 失败时关闭服务器 socket
		return -1;
	}
	printf("\nrecv client data...\n");
	//inet_ntoa   ip地址转换函数，将网络字节序IP转换为点分十进制IP
	//表达式：char *inet_ntoa (struct in_addr);
	printf("IP is %s\n", inet_ntoa(clientAddr.sin_addr));
	printf("Port is %d\n", htons(clientAddr.sin_port));
}


////等待PC发送授权命令并确认
//int wait_pc()
//{
//	struct command_struct command_frame;
//	recv_command(client, &command_frame);
//	if(command_frame.frame_head == 0xAA && command_frame.frame_tail == 0x55){
//		if(command_frame.command_type == 0x01){	//激活指令,后续添加CRC校验功能
//			printf("The startup instruction from the PC has been received.\n");
//			command_frame.command_type == 0x02;
//			//后续添加生成CRC校验码
//			send_command(client, &command_frame);	//向PC发送确认指令
//			return 0;
//		}
//	}
//	else{
//		command_frame.command_type == 0xEE;		//激活失败指令
//		//后续添加生成CRC校验码
//		send_command(client, &command_frame);	//向PC发送激活失败指令
//		printf("Frame format error!\n");
//		return -1;
//	}
//}


/**
 * @brief 程序主入口，负责TCP服务器初始化、上位机数据接收、调用速度测试函数
 * @return 0：程序正常退出；非0：异常退出
 * @note 核心流程：
 *       1. 初始化DMA传输模块
 *       2. 创建TCP服务器，绑定端口8889，监听上位机连接
 *       3. .........
 */
int main()
{
	dma_tramsmit_init();	//DMA初始化
	socket_init();
	socket_connect();
	RingBuffer rb;
	ring_buffer_init(&rb);

	respond_pc();
	resquest_data(255);	//最大值65535
	receive_data(&rb,255);

	// 假设想读取并打印 16 个字节
	uint16_t bytes_to_read = 255;
	uint8_t* read_buffer = (uint8_t*)malloc(bytes_to_read); // 为读取的数据分配内存

	if (read_buffer == NULL) {
		perror("malloc for read buffer failed");
		// 在这里处理内存分配失败的情况
	} else {
		// 从环形缓冲区读取 16 字节数据
		if (ring_buffer_read_batch(&rb, read_buffer, bytes_to_read)) {
			// 读取成功，调用打印函数
			print_hex_buffer(read_buffer, bytes_to_read, "Data read from ring buffer");
		} else {
			printf("Failed to read %d bytes from ring buffer.\n", bytes_to_read);
		}

		free(read_buffer); // 用完后释放内存
	}


///////////////////////////////////////////////////////////////////
//			// 初始化随机数种子
//			srand(time(NULL));
//
//			int send_count = rand() % 3 + 3;
//			uint32_t n = rand() % 5 + 1;
//			printf("send_count:%d,  n:%d\n",send_count,n);
//			for(send_count;send_count >0;send_count--){
//				//发送长度指令n（1-5之间）
//				n = rand() % 5 + 1;
//				if (sendInt(client, n) == -1) {
//					printf("sendInt failed!\n");
//					return 1;
//				}
//				printf("Application data:%d\n",n);
//
//				//接收数据
//				for(int m = n;m > 0; m--)
//				{
//					int bytes_received = recv(client,
//											  (char*)&temp_data,  // 存储到当前索引位置
//											  sizeof(uint32_t),
//											  0);
//					//存入环形缓冲区
//					if (ring_buffer_write(&rb, temp_data)) {
//						// 打印当前接收的uint32_t
//						printf("Received[%d]: 0x%08X\n", recv_upper_count, temp_data);
//					} else {
//						printf("The circular buffer is full,Discard data:0x%08X\n", temp_data);
//					}
//
//					if (bytes_received <= 0) {
//						if (bytes_received == 0) {
//							printf("Connection closed by peer\n");
//						} else {
//							perror("recv failed\n");
//						}
//							break;
//					}
//
//					if (bytes_received != sizeof(uint32_t)) {
//						printf("Received incomplete data\n");
//						continue;
//						}
//
//					recv_upper_count++;  // 移动到下一个存储位置
//				}
//			}
//			//结束通信
//			if (sendInt(client, END_FLAG) == -1) {
//				printf("sendInt failed");
//				return 1;
//			}
//
//			printf("recv_upper_count = %d\n",recv_upper_count);	//有效字节数
//			recv_upper_32bitNum = recv_upper_count;	//记录接收到的字节数
//
//			total_send_count = recv_upper_count;
//			total_send_bytes = total_send_count * sizeof(uint32_t); // 总发送字节数
//
//
//			//人为创造误码
//			for(int error_bit = 0;error_bit < 10;error_bit ++)
//			{
//				uint32_t error_data_id = rand() % recv_upper_count;
//				uint32_t error_bit_id = rand() % 32;
//				recv_upper_data[error_data_id]  ^= 1 << error_bit_id;
//				printf("error_data[%d]:%d\n",error_data_id,error_bit_id);
//			}
//
//
//			recv_upper_count = 0;	//将本轮接收器归零代表接收结束
/////////////////////////////////////////////////////////////////////////////
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
