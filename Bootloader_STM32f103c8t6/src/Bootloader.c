/*
 * Bootloader.c
 *
 *  Created on: May 24, 2020
 *      Author: Mohamed Nafea
 */

#include "STD_TYPES.h"
#include "Debug.h"
#include "HUART_interface.h"
#include <string.h>


#include "CRC.h"
#include "Flash.h"
#include "Bootloader.h"

#ifndef  SCB_BASE_ADDRESS
#define  SCB_BASE_ADDRESS       		0xE000ED00
#endif
#define  SCB_VTOR		 				*((volatile u32*)(SCB_BASE_ADDRESS+0x008))

#define  FLASH_BOOTLDR_BASE_ADDRESS		0x08000000
#define  FLASH_USR_APP_BASE_ADDRESS		0x08008000

#define BL_RX_LEN 						200
u8      bl_rx_buffer[BL_RX_LEN]={0};

/*Boot-loader supported commands*/
#define BL_GET_VER						0X51	/*This command is used to read the boot-loader version from the MCU*/
#define BL_GET_HELP				        0X52    /*This command is used to know what are the commands supported by the bootloader*/
#define BL_GET_CID			            0X53    /*This command is used to read the MCU chip identification number*/
#define BL_GET_RDP_STATUS	            0X54    /*This command is used to read the FLASH Read Protection level.*/
#define BL_GO_TO_ADDR		            0X55    /*This command is used to jump boot-loader to specified address.*/
#define BL_FLASH_ERASE		            0X56    /*This command is used to mass erase or sector erase of the user flash .*/
#define BL_MEM_WRITE	                0X57    /*This command is used to write data in to different memories of the MCU*/
#define BL_EN_R_W_PROTECT		        0X58    /*This command is used to enable read/write protect on different sectors of the user flash .*/
#define BL_MEM_READ				        0X59    /*This command is used to read data from different memories of the microcontroller.*/
#define BL_READ_SECTOR_STATUS		    0X5A    /*This command is used to read all the sector protection status.*/
#define BL_OTP_READ				        0X5B    /*This command is used to read the OTP contents.*/
#define BL_DIS_R_W_PROTECT		        0X5C    /*This command is used to disable read/write protection on different sectors of the user flash . This command takes the protection status to default state.*/

/*ACK and NACK bytes*/
#define BL_ACK							0xA5
#define BL_NACK							0x7F

/*Boot-Loader version*/
#define BL_VERSION						0x10

/*CRC Verification return*/
#define VERIFY_CRC_SUCCESS				0u
#define VERIFY_CRC_FAIL					1u

/*MCU Chip ID*/
#define DBGMCU_IDCODE 					*((volatile u32*)0xE0042000)
#define DEV_ID_MASK						0x00000FFF
#define REV_ID_MASK						0xFFFF0000

/*Commands handle functions prototypes*/
void bootloader_handle_getver_cmd				(u8* bl_rx_buffer);
void bootloader_handle_gethelp_cmd				(u8* bl_rx_buffer);
void bootloader_handle_getcid_cmd				(u8* bl_rx_buffer);
void bootloader_handle_getrdp_cmd				(u8* bl_rx_buffer);
void bootloader_handle_goto_address_cmd			(u8* bl_rx_buffer);
void bootloader_handle_flash_erase_cmd			(u8* bl_rx_buffer);
void bootloader_handle_mem_write_cmd			(u8* bl_rx_buffer);
void bootloader_handle_en_rw_protect_cmd		(u8* bl_rx_buffer);
void bootloader_handle_mem_read_cmd				(u8* bl_rx_buffer);
void bootloader_handle_read_sector_status_cmd	(u8* bl_rx_buffer);
void bootloader_handle_read_otp_cmd				(u8* bl_rx_buffer);
void bootloader_handle_dis_rw_protect_cmd		(u8* bl_rx_buffer);

/*Helper functions prototypes*/
void bootloader_send_ack(u8 follow_len);
void bootloader_send_nack(void);
u8   bootloader_verify_crc(u8* pData, u32 len, u32 crc_host);
u8   supported_commands[] = {
							BL_GET_VER				,
							BL_GET_HELP				,
							BL_GET_CID			    ,
							BL_GET_RDP_STATUS	    ,
							BL_GO_TO_ADDR		    ,
							BL_FLASH_ERASE		    ,
							BL_MEM_WRITE	        ,
						//	BL_EN_R_W_PROTECT		,
						//	BL_MEM_READ				,
							BL_READ_SECTOR_STATUS	,
						//	BL_OTP_READ				,
						//	BL_DIS_R_W_PROTECT
							};


/******************* Implementation of Boot-loader application functions **********************/
u8 i=0;
/*Reads the command packet which comes from the host application*/
extern void bootloader_voidUARTReadData (void)
{
	u8 rcv_len = 0;

	//u8 var= 0x05;
	printmsg1("BL_DEBUG_MSG: Button is pressed .. going to BL mode\r\n");
	while(1)
	{

        //memset(bl_rx_buffer,0,200);
		/*here we will read and and decode the commands coming from host
		 * Frist read only one byte from the host, which is the "length" field of the command packet*/

		/**********Code that should be executed***************/
		//HUART_u8ReceiveSync(HUART_USART2,bl_rx_buffer,1); //reading the first byte from host
		//rcv_len = bl_rx_buffer[0];
		//HUART_u8ReceiveSync(HUART_USART2,&bl_rx_buffer[1],rcv_len);

		HUART_u8ReceiveAsync(HUART_USART2,bl_rx_buffer,1);
		rcv_len = bl_rx_buffer[0];
        HUART_u8ReceiveAsync(HUART_USART2,&bl_rx_buffer[1],rcv_len);

		/*****************************FOR DEBUGGING*****************************/
		//Array that should be received from BL_GET_VER
		//bl_rx_buffer[0]=	0x05;
		//bl_rx_buffer[1]=	0x51;
		//bl_rx_buffer[2]=	0xe7;
		//bl_rx_buffer[3]=	0xe9;
		//bl_rx_buffer[4]=	0xab;
		//bl_rx_buffer[5]=	0x7c;
		/************************************************************************/
		//Array that should be received from BL_GET_HELP
		//bl_rx_buffer[0]=	0x05;
		//bl_rx_buffer[1]=	0x52;
		//bl_rx_buffer[2]=	0x3e;
		//bl_rx_buffer[3]=	0xcf;
		//bl_rx_buffer[4]=	0xe8;
		//bl_rx_buffer[5]=	0x71;
		/************************************************************************/
		//Array that should be received from BL_GET_CID
		//bl_rx_buffer[0]=	0x05;
		//bl_rx_buffer[1]=	0x53;
		//bl_rx_buffer[2]=	0x89;
		//bl_rx_buffer[3]=	0xd2;
		//bl_rx_buffer[4]=	0x29;
		//bl_rx_buffer[5]=	0x75;

		HUART_u8SendSync(HUART_USART1,bl_rx_buffer,6);
		for(i=0;i<6;i++){
		printmsg1("0x%x\r\n",bl_rx_buffer[i]);
		}

		/***************************************************************************/
		switch(bl_rx_buffer[1]) //checking for the received command and then executing its code
		{
			case BL_GET_VER:
				bootloader_handle_getver_cmd(bl_rx_buffer);
				break;
			case BL_GET_HELP:
				bootloader_handle_gethelp_cmd(bl_rx_buffer);
				break;
			case BL_GET_CID:
				bootloader_handle_getcid_cmd(bl_rx_buffer);
				break;
			case BL_GET_RDP_STATUS:
				bootloader_handle_getrdp_cmd(bl_rx_buffer);
				break;
			case BL_GO_TO_ADDR:
				bootloader_handle_goto_address_cmd(bl_rx_buffer);
				break;
			case BL_FLASH_ERASE:
				bootloader_handle_flash_erase_cmd(bl_rx_buffer);
				break;
			case BL_MEM_WRITE:
				bootloader_handle_mem_write_cmd(bl_rx_buffer);
				break;
			case BL_EN_R_W_PROTECT:
				bootloader_handle_en_rw_protect_cmd(bl_rx_buffer);
				break;
			case BL_MEM_READ:
				bootloader_handle_mem_read_cmd(bl_rx_buffer);
				break;
			case BL_READ_SECTOR_STATUS:
				bootloader_handle_read_sector_status_cmd(bl_rx_buffer);
				break;
			case BL_OTP_READ:
				bootloader_handle_read_otp_cmd(bl_rx_buffer);
				break;
			case BL_DIS_R_W_PROTECT:
				bootloader_handle_dis_rw_protect_cmd(bl_rx_buffer);
				break;
			default:
				printmsg1("BL_DEBUG_MSG: Invalid command code received from host \r\n");
				break;
		}
	}
}

/*Jumps to the user application code if there is no Boot-loader request*/
extern void bootloader_voidJumpToUserApp(void)
{
	printmsg1("BL_DEBUG_MSG: Button is not pressed .. executing user app \r\n");

	//just a function to hold the address of the reset handler of the user app
	void (*app_reset_handler)(void);

	/*1. configure the MSP by reading the value from the base address of the FLASH sector
	 * that contains the user app*/
	u32 msp_value = *(volatile u32*)FLASH_USR_APP_BASE_ADDRESS;  //getting the user app flash sector
	printmsg1("BL_DEBUG_MSG: MSP value = 0x%x\r\n",msp_value);

	//This function comes from CMSIS
	//__set_MSP(msp_value);										//forcing sp to go to the user app flash sector
	asm volatile ("MSR msp, %0\n" : : "r" (msp_value) : "sp");
	SCB_VTOR = FLASH_USR_APP_BASE_ADDRESS;				//vector table relocation

	/* 2. Now fetch the reset handler address of the user application
	 * from the location FLASH_USR_APP_BASE_ADDRESS
	 * Here, we gave the address of the reset handler to a pointer to function, so that
	 * this pointer to function (app_reset_handler)_ can trigger the user application's reset handler
	 * which jumps to the user's (main) function through (_start) function in (startup.c)
	 * */
	app_reset_handler = (void*)(*((volatile u32*)(FLASH_USR_APP_BASE_ADDRESS+0x04)));

	/*3. Jump to reset handler of the user application*/
	app_reset_handler();
}


/******************* Implementation of Boot-loader Command Handle Functions *******************/

/*Helper function to handle BL_GET_VER command*/
void bootloader_handle_getver_cmd				(u8* bl_rx_buffer)
{

	u8  bl_version;											/*variable to store BL version*/
	u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
	u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
	u32 crc_host;
	crc_host= *((u32*)(bl_rx_buffer+command_packet-4));          /*Extract the CRC32 sent by host*/

	printmsg1("BL_DEBUG_MSG: bootloader_handle_getver_cmd \r\n");
	// 1) verify the checksum
	if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
	{
		//checksum is correct
		printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
		//Stating that a reply of one byte is going to be sent
		bootloader_send_ack(1);
		//Sending boot-loader version
		bl_version = BL_VERSION;
		printmsg1("BL_DEBUG_MSG: BL_VER : 0x%x \r\n",bl_version);
		HUART_u8SendSync(HUART_USART2,&bl_version,1); //sending version to host
	}
	else
	{
		//checksum is wrong send nack
		printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
		bootloader_send_nack();
	}

}

/*Handle function to handle BL_GET_HELP command
 * Bootloader sends out all supported command codes*/
void bootloader_handle_gethelp_cmd				(u8* bl_rx_buffer)
{
		u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
		u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
		u32 crc_host;
		crc_host= *((u32*)(bl_rx_buffer+command_packet-4));          /*Extract the CRC32 sent by host*/

		printmsg1("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd \r\n");
		// 1) verify the checksum
		if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
		{
			//checksum is correct
			printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
			//Stating that a reply of eight bytes is going to be sent
			bootloader_send_ack(8);
			//Sending boot-loader supported commands
			HUART_u8SendSync(HUART_USART2,supported_commands,sizeof(supported_commands));
			for(i=0;i<sizeof(supported_commands);i++)
			printmsg1("0x%x\r\n",supported_commands[i]);//debugging
		}
		else
		{
			//checksum is wrong send nack
			printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
			bootloader_send_nack();
		}

}

/*Handle function to handle BL_GET_CID command*/
void bootloader_handle_getcid_cmd				(u8* bl_rx_buffer)
{
	u16 device_id 	= DBGMCU_IDCODE & DEV_ID_MASK;
	u16 revision_id = DBGMCU_IDCODE & REV_ID_MASK;
	u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
	u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
	u32 crc_host;
	crc_host= *((u32*)(bl_rx_buffer+command_packet-4));     /*Extract the CRC32 sent by host*/

	printmsg1("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd \r\n");
	// 1) verify the checksum
	if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
	{
		//checksum is correct
		printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
		//Stating that a reply of four bytes is going to be sent
		bootloader_send_ack(4);
		//Sending device ID and Revision ID
		HUART_u8SendSync(HUART_USART2,(u8*)&device_id,2);
		HUART_u8SendSync(HUART_USART2,(u8*)&revision_id,2);
		//for debugging
		printmsg1("Device ID: 0x%x\r\n",device_id);
		printmsg1("Revision ID: %x\r\n",revision_id);

	}
	else
	{
		//checksum is wrong send nack
		printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
		bootloader_send_nack();
	}
}

/*Handle function to handle BL_GET_RDP_STATUS command*/
void bootloader_handle_getrdp_cmd				(u8* bl_rx_buffer)
{
	u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
	u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
	u32 crc_host;
	crc_host= *((u32*)(bl_rx_buffer+command_packet-4));          /*Extract the CRC32 sent by host*/

	printmsg1("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd \r\n");
	// 1) verify the checksum
	if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
	{
		//checksum is correct
		printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
		//Stating that a reply of one byte is going to be sent
		bootloader_send_ack(1);
		//processing
		HUART_u8SendSync(HUART_USART2,supported_commands,sizeof(supported_commands));

	}
	else
	{
		//checksum is wrong send nack
		printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
		bootloader_send_nack();
	}
}

/*Handle function to handle BL_GO_TO_ADDR command*/
void bootloader_handle_goto_address_cmd			(u8* bl_rx_buffer)
{
	u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
	u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
	u32 crc_host;
	crc_host= *((u32*)(bl_rx_buffer+command_packet-4));          /*Extract the CRC32 sent by host*/

	printmsg1("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd \r\n");
	// 1) verify the checksum
	if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
	{
		//checksum is correct
		printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
		//Stating that a reply of one byte is going to be sent
		bootloader_send_ack(1);
		//processing
		HUART_u8SendSync(HUART_USART2,supported_commands,sizeof(supported_commands));

	}
	else
	{
		//checksum is wrong send nack
		printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
		bootloader_send_nack();
	}
}

/*Handle function to handle BL_FLASH_ERASE command*/
void bootloader_handle_flash_erase_cmd			(u8* bl_rx_buffer)
{
	u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
	u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
	u32 crc_host;
	crc_host= *((u32*)(bl_rx_buffer+command_packet-4));          /*Extract the CRC32 sent by host*/

	printmsg1("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd \r\n");
	// 1) verify the checksum
	if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
	{
		//checksum is correct
		printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
		//Stating that a reply of one byte is going to be sent
		bootloader_send_ack(1);
		//processing
		HUART_u8SendSync(HUART_USART2,supported_commands,sizeof(supported_commands));

	}
	else
	{
		//checksum is wrong send nack
		printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
		bootloader_send_nack();
	}
}

/*Handle function to handle BL_MEM_WRITE command*/
void bootloader_handle_mem_write_cmd			(u8* bl_rx_buffer)
{
	u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
	u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
	u32 crc_host;
	crc_host= *((u32*)(bl_rx_buffer+command_packet-4));          /*Extract the CRC32 sent by host*/

	printmsg1("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd \r\n");
	// 1) verify the checksum
	if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
	{
		//checksum is correct
		printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
		//Stating that a reply of one byte is going to be sent
		bootloader_send_ack(1);
		//processing
		HUART_u8SendSync(HUART_USART2,supported_commands,sizeof(supported_commands));

	}
	else
	{
		//checksum is wrong send nack
		printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
		bootloader_send_nack();
	}
}

/*Handle function to handle BL_EN_R_W_PROTECT command*/
void bootloader_handle_en_rw_protect_cmd		(u8* bl_rx_buffer)
{
	u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
	u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
	u32 crc_host;
	crc_host= *((u32*)(bl_rx_buffer+command_packet-4));          /*Extract the CRC32 sent by host*/

	printmsg1("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd \r\n");
	// 1) verify the checksum
	if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
	{
		//checksum is correct
		printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
		//Stating that a reply of one byte is going to be sent
		bootloader_send_ack(1);
		//processing
		HUART_u8SendSync(HUART_USART2,supported_commands,sizeof(supported_commands));

	}
	else
	{
		//checksum is wrong send nack
		printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
		bootloader_send_nack();
	}
}

/*Handle function to handle BL_MEM_READ command*/
void bootloader_handle_mem_read_cmd				(u8* bl_rx_buffer)
{
	u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
	u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
	u32 crc_host;
	crc_host= *((u32*)(bl_rx_buffer+command_packet-4));          /*Extract the CRC32 sent by host*/

	printmsg1("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd \r\n");
	// 1) verify the checksum
	if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
	{
		//checksum is correct
		printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
		//Stating that a reply of one byte is going to be sent
		bootloader_send_ack(1);
		//processing
		HUART_u8SendSync(HUART_USART2,supported_commands,sizeof(supported_commands));

	}
	else
	{
		//checksum is wrong send nack
		printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
		bootloader_send_nack();
	}
}

/*Handle function to handle BL_READ_SECTOR_STATUS command*/
void bootloader_handle_read_sector_status_cmd	(u8* bl_rx_buffer)
{
	u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
	u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
	u32 crc_host;
	crc_host= *((u32*)(bl_rx_buffer+command_packet-4));          /*Extract the CRC32 sent by host*/

	printmsg1("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd \r\n");
	// 1) verify the checksum
	if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
	{
		//checksum is correct
		printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
		//Stating that a reply of one byte is going to be sent
		bootloader_send_ack(1);
		//processing
		HUART_u8SendSync(HUART_USART2,supported_commands,sizeof(supported_commands));

	}
	else
	{
		//checksum is wrong send nack
		printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
		bootloader_send_nack();
	}
}

/*Handle function to handle BL_OTP_READ command*/
void bootloader_handle_read_otp_cmd				(u8* bl_rx_buffer)
{
	u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
	u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
	u32 crc_host;
	crc_host= *((u32*)(bl_rx_buffer+command_packet-4));          /*Extract the CRC32 sent by host*/

	printmsg1("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd \r\n");
	// 1) verify the checksum
	if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
	{
		//checksum is correct
		printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
		//Stating that a reply of one byte is going to be sent
		bootloader_send_ack(1);
		//processing
		HUART_u8SendSync(HUART_USART2,supported_commands,sizeof(supported_commands));

	}
	else
	{
		//checksum is wrong send nack
		printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
		bootloader_send_nack();
	}
}

/*Handle function to handle BL_DIS_R_W_PROTECT command*/
void bootloader_handle_dis_rw_protect_cmd		(u8* bl_rx_buffer)
{
	u8  command_packet = bl_rx_buffer[0]+1;                 /*Total length of command packet*/
	u32 command_length_without_crc = command_packet-4;      /*Length to be sent to (bl_verify_crc) function*/
	u32 crc_host;
	crc_host= *((u32*)(bl_rx_buffer+command_packet-4));          /*Extract the CRC32 sent by host*/

	printmsg1("BL_DEBUG_MSG: bootloader_handle_gethelp_cmd \r\n");
	// 1) verify the checksum
	if(! bootloader_verify_crc(bl_rx_buffer, command_length_without_crc, crc_host))
	{
		//checksum is correct
		printmsg1("BL_DEBUG_MSG: checksum success !! \r\n");
		//Stating that a reply of one byte is going to be sent
		bootloader_send_ack(1);
		//processing
		HUART_u8SendSync(HUART_USART2,supported_commands,sizeof(supported_commands));

	}
	else
	{
		//checksum is wrong send nack
		printmsg1("BL_DEBUG_MSG: checksum fail !! \r\n");
		bootloader_send_nack();
	}
}

/******************* Implementation Helper functions prototypes **********************************************/

void bootloader_send_ack(u8 follow_len)
{
	//here we send 2 bytes .. first byte is ack and the second byte is the length of the following
	//reply bytes
	u8 ack_buffer[2]={BL_ACK,follow_len};
	HUART_u8SendSync(HUART_USART2,ack_buffer,2);
}
void bootloader_send_nack(void)
{
	HUART_u8SendSync(HUART_USART2,(u8*)BL_NACK,1);
}

//This verifies the CRC of the given buffer in pData
u8 bootloader_verify_crc(u8* pData, u32 len, u32 crc_host)
{
	u32 i;
	u32 iData;
	u32 crc_rcv ;
	CRC_ResetDR();
	for(i=0;i<len;i++)
	{
		iData= pData[i];
		crc_rcv = CRC_CalcBlockCRC(&iData, 1);
	}
	if(crc_rcv==crc_host)
		return VERIFY_CRC_SUCCESS;
	return VERIFY_CRC_FAIL;
}