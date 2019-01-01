/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "stm32f10x.h"
#include "bsp_sdio_sdcard.h"

#define BLOCK_SIZE            512 /* Block Size in Bytes */



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	SD_Error  Status;
	/* Supports only single drive */
	if (drv)
	{
		return STA_NOINIT;
	}
/*-------------------------- SD Init ----------------------------- */
  Status = SD_Init();
	if (Status!=SD_OK )
	{
		return STA_NOINIT;
	}
	else
	{
		return RES_OK;
	}

}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                 */
#include <string.h>
#define SD_BLOCK_SIZE            512 /* Block Size in Bytes */
__align(4) uint8_t align_buffer[SD_BLOCK_SIZE];						  

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{

	if((uint32_t)buff%4 != 0)    //传入的buff数据地址不是四字节对齐，需要额外处理
	{
		uint8_t i;
		for(i=0;i<count;i++)
		{
			SD_ReadBlock(align_buffer, sector*SD_BLOCK_SIZE+SD_BLOCK_SIZE*i, SD_BLOCK_SIZE);

				/* Check if the Transfer is finished */
			SD_WaitReadOperation();  //循环查询dma传输是否结束

			/* Wait until end of DMA transfer */
			while(SD_GetStatus() != SD_TRANSFER_OK);

			memcpy(buff,align_buffer,SD_BLOCK_SIZE);

			buff+=SD_BLOCK_SIZE;
		}
	}
	
	else
	{
		if (count > 1)
		{
			SD_ReadMultiBlocks(buff, sector*BLOCK_SIZE, BLOCK_SIZE, count);
		
			/* Check if the Transfer is finished */
			SD_WaitReadOperation();  //循环查询dma传输是否结束

			/* Wait until end of DMA transfer */
			while(SD_GetStatus() != SD_TRANSFER_OK);

		}
		else
		{
			
			SD_ReadBlock(buff, sector*BLOCK_SIZE, BLOCK_SIZE);

			/* Check if the Transfer is finished */
			SD_WaitReadOperation();  //循环查询dma传输是否结束

			/* Wait until end of DMA transfer */
			while(SD_GetStatus() != SD_TRANSFER_OK);

		}		
		
	}
							
//	if (count > 1)
//	{
//		SD_ReadMultiBlocks(buff, sector*BLOCK_SIZE, BLOCK_SIZE, count);
//	
//			  /* Check if the Transfer is finished */
//	     SD_WaitReadOperation();  //循环查询dma传输是否结束
//	
//	    /* Wait until end of DMA transfer */
//	    while(SD_GetStatus() != SD_TRANSFER_OK);

//	}
//	else
//	{
//		
//		SD_ReadBlock(buff, sector*BLOCK_SIZE, BLOCK_SIZE);

//			  /* Check if the Transfer is finished */
//	     SD_WaitReadOperation();  //循环查询dma传输是否结束
//	
//	    /* Wait until end of DMA transfer */
//	    while(SD_GetStatus() != SD_TRANSFER_OK);

//	}
	return RES_OK;
	
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	if((uint32_t)buff%4 != 0)   //若传入的buff地址不是4字节对齐，需要额外处理
	{
		uint8_t i;
		
		for(i=0;i<count;i++)
		{
			memcpy(align_buffer,buff,SD_BLOCK_SIZE);
			SD_WriteBlock(align_buffer,sector*SD_BLOCK_SIZE+SD_BLOCK_SIZE*i,SD_BLOCK_SIZE);//单个sector的写操作

			/* Check if the Transfer is finished */
			SD_WaitWriteOperation();	                  //等待dma传输结束
			while(SD_GetStatus() != SD_TRANSFER_OK); //等待sdio到sd卡传输结束

			buff+=SD_BLOCK_SIZE;
		}
	}	
	
	else
	{
		if (count > 1)
		{
			SD_WriteMultiBlocks((uint8_t *)buff, sector*BLOCK_SIZE, BLOCK_SIZE, count);

			/* Check if the Transfer is finished */
			SD_WaitWriteOperation();	   //等待dma传输结束
			while(SD_GetStatus() != SD_TRANSFER_OK); //等待sdio到sd卡传输结束
		}
		
		else
		{
			SD_WriteBlock((uint8_t *)buff,sector*BLOCK_SIZE, BLOCK_SIZE);

			/* Check if the Transfer is finished */
			SD_WaitWriteOperation();	   //等待dma传输结束
			while(SD_GetStatus() != SD_TRANSFER_OK); //等待sdio到sd卡传输结束
		}	
	}

//	if (count > 1)
//	{
//		SD_WriteMultiBlocks((uint8_t *)buff, sector*BLOCK_SIZE, BLOCK_SIZE, count);
//		
//		  /* Check if the Transfer is finished */
//	  	 SD_WaitWriteOperation();	   //等待dma传输结束
//	    while(SD_GetStatus() != SD_TRANSFER_OK); //等待sdio到sd卡传输结束
//	}
//	else
//	{
//		SD_WriteBlock((uint8_t *)buff,sector*BLOCK_SIZE, BLOCK_SIZE);
//		
//		  /* Check if the Transfer is finished */
//	   		SD_WaitWriteOperation();	   //等待dma传输结束
//	    while(SD_GetStatus() != SD_TRANSFER_OK); //等待sdio到sd卡传输结束
//	}
	return RES_OK;
	
}
#endif /* _READONLY */




/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	return RES_OK;
}
							 
/*-----------------------------------------------------------------------*/
/* Get current time                                                      */
/*-----------------------------------------------------------------------*/ 
DWORD get_fattime(void)
{

 	return 0;

} 
