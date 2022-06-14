#include "ethernet.h"
#define PRINT(...) sendstr(__VA_ARGS__)

void Ports_Init(void)
{
    PORT_InitTypeDef gpio;
    RST_CLK_PCLKcmd (RST_CLK_PCLK_PORTF, ENABLE);
    PORT_StructInit (&gpio);
    gpio.PORT_Pin   = PORT_Pin_13 | PORT_Pin_14;
    gpio.PORT_OE    = PORT_OE_OUT;
    gpio.PORT_SPEED = PORT_SPEED_SLOW;
    gpio.PORT_MODE  = PORT_MODE_DIGITAL;
    PORT_Init(MDR_PORTF, &gpio);
}

int eth_init(MAC &srcMAC){
	ETH_ClockDeInit();
	RST_CLK_PCLKcmd(RST_CLK_PCLK_DMA, ENABLE); // Dma here now, idk.
	RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTF, ENABLE);
	
	RST_CLK_HSE2config(RST_CLK_HSE2_ON);
	if(RST_CLK_HSE2status() == ERROR) return -1;
	
	ETH_PHY_ClockConfig(ETH_PHY_CLOCK_SOURCE_HSE2, ETH_PHY_HCLKdiv1);
	ETH_BRGInit(ETH_HCLKdiv1);
	ETH_ClockCMD(ETH_CLK1, ENABLE);
	ETH_DeInit(MDR_ETHERNET1);
	
	
	ETH_InitTypeDef  ETH_InitStruct;
	ETH_StructInit((ETH_InitTypeDef * ) &ETH_InitStruct);
	
	ETH_InitStruct.ETH_PHY_Mode = ETH_PHY_MODE_AutoNegotiation;
	ETH_InitStruct.ETH_Transmitter_RST = SET;
	ETH_InitStruct.ETH_Receiver_RST = SET;
	ETH_InitStruct.ETH_Automatic_Preamble = ENABLE;
	ETH_InitStruct.ETH_Automatic_CRC_Strip = ENABLE;
	ETH_InitStruct.ETH_Buffer_Mode = ETH_BUFFER_MODE_AUTOMATIC_CHANGE_POINTERS;
	ETH_InitStruct.ETH_Receive_All_Packets = ENABLE;                                                         
    ETH_InitStruct.ETH_Source_Addr_HASH_Filter = DISABLE;
	//ETH_InitStruct.ETH_Loopback_Mode = ENABLE;
	
	ETH_InitStruct.ETH_MAC_Address[2] = ((int)srcMAC[0]<<8)| (int)srcMAC[1];
	ETH_InitStruct.ETH_MAC_Address[1] = ((int)srcMAC[2]<<8)| (int)srcMAC[3];
	ETH_InitStruct.ETH_MAC_Address[0] = ((int)srcMAC[4]<<8)| (int)srcMAC[5];
	MDR_ETHERNET1->PHY_Status |= 0 << 1;
	ETH_InitStruct.ETH_Dilimiter = 0x1000;
	
	ETH_Init(MDR_ETHERNET1, (ETH_InitTypeDef *) &ETH_InitStruct);
	ETH_PHYCmd(MDR_ETHERNET1, ENABLE);
	ETH_Start(MDR_ETHERNET1);
	Ports_Init();
	PRINT("ETH INIT");
	return 1;
}


void sendto(U32 * packet){
	ETH_SendFrame(MDR_ETHERNET1, (U32 *) packet, *(U32*)&packet[0]);
}

U16 recvto(U32 * packet, TsNs &UTC_Recv){
	ETH_StatusPacketReceptionTypeDef ETH_StatusPacketReceptionStruct;
	
	UTC_Recv.renew();
	ETH_StatusPacketReceptionStruct.Status = ETH_ReceivedFrame(MDR_ETHERNET1, packet);
	return ETH_StatusPacketReceptionStruct.Fields.Length;
}