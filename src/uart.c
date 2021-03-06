/*
 * uart.c
 *
 *  Created on: 22 cze 2015
 *      Author: mike
 */

/*  uart.c  */


#define NULL					0

#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "uart.h"
#include "string.h"
#include "stm32f4_discovery.h"

// =======================================================================================================
// Zmienne globalne
extern UART_HandleTypeDef	huart4;
extern char 				Hal_RxBuff[1];
extern char					Hal_TxBuff[1];
char						cOdebranyZnak;
tsRecieverBuffer			sRecieverBuffer;
tsTransmiterBuffer 			sTransmiterBuffer;

//teRecieverStatus			eRecieverStatus;



// =======================================================================================================
void UART_InitWithInt(uint32_t MyBaudRate){
	huart4.Instance = UART4;
	huart4.Init.BaudRate = MyBaudRate;
	huart4.Init.WordLength = UART_WORDLENGTH_8B;
	huart4.Init.StopBits = UART_STOPBITS_1;
	huart4.Init.Parity = UART_PARITY_NONE;
	huart4.Init.Mode = UART_MODE_TX_RX;
	huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart4.Init.OverSampling = UART_OVERSAMPLING_16;

	huart4.pRxBuffPtr = Hal_RxBuff;
	huart4.pTxBuffPtr = Hal_TxBuff;
	huart4.RxXferSize = 1;
	huart4.TxXferSize = 1;
	HAL_UART_Init(&huart4);
}



// =======================================================================================================
void Reciever_PutCharacterToBuffer(char cCharacter)
{
	if(sRecieverBuffer.ucCharCtr == UART_RECIEVER_SIZE)
	{
		sRecieverBuffer.eStatus = OVERFLOW;
		sRecieverBuffer.ucCharCtr = 0;
	}
	else if(cCharacter == TERMINATOR)
	{
		sRecieverBuffer.cData[sRecieverBuffer.ucCharCtr] = NULL;
		sRecieverBuffer.eStatus = READY;
		sRecieverBuffer.ucCharCtr = 0;
	}
	else if(cCharacter == BACKSPACE){
		if(0 != sRecieverBuffer.ucCharCtr){
			sRecieverBuffer.ucCharCtr--;
			sRecieverBuffer.cData[sRecieverBuffer.ucCharCtr] = NULL;
		}
	}
	else
	{
		sRecieverBuffer.cData[sRecieverBuffer.ucCharCtr] = cCharacter;
		sRecieverBuffer.ucCharCtr++;
	}
}


// =======================================================================================================
teRecieverStatus eReciever_GetStatus(void)
{
	return sRecieverBuffer.eStatus;
}

// =======================================================================================================
void Reciever_GetStringCopy(char * ucDestination)
{
	CopyString(sRecieverBuffer.cData, ucDestination);
	sRecieverBuffer.eStatus = EMPTY;
}



// =======================================================================================================
 char Transmiter_GetCharacterFromBuffer(void)
{
	if (0 == sTransmiterBuffer.fLastCharacter)
	{
		if(NULL != sTransmiterBuffer.cData[sTransmiterBuffer.cCharCtr])
		{
			return sTransmiterBuffer.cData[sTransmiterBuffer.cCharCtr++];
		}
		else
		{
			sTransmiterBuffer.fLastCharacter = 1;
			return TERMINATOR;
		}
	}
	else
	{
		__HAL_UART_DISABLE_IT(&huart4,UART_IT_TXE);		// Po nadaniu ostatniego znaku wyłączamy przerwanie.
		sTransmiterBuffer.eStatus = FREE;
		return NULL;
	}
}

uint8_t Transmiter_GetRawByteFromBuffer(void){
	if( (sTransmiterBuffer.u8FrameDataLen-1) > sTransmiterBuffer.cCharCtr){
		return sTransmiterBuffer.cData[sTransmiterBuffer.cCharCtr++];
	}
	else{
		__HAL_UART_DISABLE_IT(&huart4,UART_IT_TXE);		// Po nadaniu ostatniego znaku wyłączamy przerwanie.
		sTransmiterBuffer.eStatus = FREE;
		return sTransmiterBuffer.cData[sTransmiterBuffer.cCharCtr];
	}
	/*
	if(0 == sTransmiterBuffer.fLastCharacter){
			if(sTransmiterBuffer.u8FrameDataLen > sTransmiterBuffer.cCharCtr){			// jeżeli jeszcze nie doszedł do końca
				return sTransmiterBuffer.cData[sTransmiterBuffer.cCharCtr++];
			}
			else{
				sTransmiterBuffer.fLastCharacter = 1;
				return 0xAA;
			}
		}
		else{
			__HAL_UART_DISABLE_IT(&huart4,UART_IT_TXE);		// Po nadaniu ostatniego znaku wyłączamy przerwanie.
			sTransmiterBuffer.eStatus = FREE;
			return 0x55;
		}

	*/
}



// =======================================================================================================
void Transmiter_SendString( char cString[])
{
	CopyString(cString,sTransmiterBuffer.cData);
	sTransmiterBuffer.eStatus 			= BUSY;
	sTransmiterBuffer.fLastCharacter 	= 0;
	sTransmiterBuffer.cCharCtr 			= 0;
	sTransmiterBuffer.u8FrameID 		= 0;
	sTransmiterBuffer.u8FrameDataLen 	= 0;

	huart4.Instance->DR = Transmiter_GetCharacterFromBuffer();
	__HAL_UART_ENABLE_IT(&huart4,UART_IT_TXE);						// Włączamy przerwanie. Wyłączymy je po nadaniu ostatniego znaku.
}

void Transmiter_SendFrame(char * pu8Frame){
	/**
	 * Funkcja wysyła pakiet danych.
	 * Pierwszy bajt to numer ramki
	 * Drugi bajt to liczba bajtów danych
	 * Kolejne bajty to dane w kodzie binarnym
	 */
	uint8_t	u8ByteCounter;

	sTransmiterBuffer.cCharCtr 			= 0;
	sTransmiterBuffer.eStatus 			= BUSY;
	sTransmiterBuffer.fLastCharacter 	= 0;
	sTransmiterBuffer.u8FrameID 		= pu8Frame[0];
	sTransmiterBuffer.u8FrameDataLen 	= pu8Frame[1];

	for(u8ByteCounter = 0; u8ByteCounter < sTransmiterBuffer.u8FrameDataLen; u8ByteCounter++){
		sTransmiterBuffer.cData[u8ByteCounter] = pu8Frame[u8ByteCounter];
	}

	huart4.Instance->DR = Transmiter_GetRawByteFromBuffer();
	__HAL_UART_ENABLE_IT(&huart4,UART_IT_TXE);						// Włączamy przerwanie. Wyłączymy je po nadaniu ostatniego znaku.

}



// =======================================================================================================
enum eTransmiterStatus eTransmiter_GetStatus(void)
{
	return sTransmiterBuffer.eStatus;
}


uint8_t	Transmiter_GetFrameID(void){
	return sTransmiterBuffer.u8FrameID;
}

uint8_t Transmiter_GetFrameDataLen(void){
	return sTransmiterBuffer.u8FrameDataLen;
}



uint8_t Transmiter_CheckIfDataLeft(void){
	if(sTransmiterBuffer.u8FrameDataLen > sTransmiterBuffer.cCharCtr){
		return 1;
	}
	else{
		return 0;
	}
}


