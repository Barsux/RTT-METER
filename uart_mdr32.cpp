#include "uart_mdr32.h"
#ifdef __MDR32F9Qx__

#endif



class UART_MDR32: public WaitSystem::Module, public UART {
public:
  bool init, wait_pps, wait_uart, copy;
  char buff[256]; int buff_ptr;
  U32 high_ts, ts;
  class Rx: public Queue_rx {public:
    UART_MDR32 &base;
    Rx(UART_MDR32 &base): base(base) { 
    }
  } queue_rx;
  class Tx: public Queue_tx {public:
    UART_MDR32 &base;
    Tx(UART_MDR32 &base): base(base) {
    }
  } queue_tx;
	
  UART_MDR32(WaitSystem* waitSystem): WaitSystem::Module(waitSystem)
		,init(false),queue_rx(*this), queue_tx(*this), high_ts(0), ts(0)
  {
    module_debug = "UART";
    rx 		= &queue_rx; 	memset(rx->rxbuf, 0, 256); 	rx->rxlen = 0;
    tx 		= &queue_tx; 	enable_wait(tx); 						tx->txbuf = 0;
    flags = (Flags)(flags | evaluate_every_cycle);
  }
	
	void uart_init(){
		copy = false;
		wait_pps = true;
		wait_uart = false;
		init = true;
		awaited = true;
		RST_CLK_PCLKcmd(RST_CLK_PCLK_PORTD, ENABLE);

		UART_InitTypeDef uart_init;
		uart_init.UART_BaudRate 			= 115200;
		uart_init.UART_WordLength 			= UART_WordLength8b;
		uart_init.UART_StopBits 			= UART_StopBits1;
		uart_init.UART_Parity 				= UART_Parity_0;
		uart_init.UART_FIFOMode 			= UART_FIFO_OFF;
		uart_init.UART_HardwareFlowControl 	= UART_HardwareFlowControl_RXE;
		PORT_InitTypeDef uart_port;
		
		
		uart_port.PORT_Pin 				= PORT_Pin_14;
		uart_port.PORT_OE 				= PORT_OE_IN;
		uart_port.PORT_PULL_UP 			= PORT_PULL_UP_OFF;
		uart_port.PORT_PULL_DOWN 		= PORT_PULL_DOWN_OFF;
		uart_port.PORT_PD_SHM 			= PORT_PD_SHM_OFF;
		uart_port.PORT_PD 				= PORT_PD_DRIVER;
		uart_port.PORT_GFEN 			= PORT_GFEN_OFF;
		uart_port.PORT_FUNC 			= PORT_FUNC_MAIN;
		uart_port.PORT_SPEED 			= PORT_SPEED_MAXFAST;
		uart_port.PORT_MODE				= PORT_MODE_DIGITAL;
		PORT_Init(MDR_PORTD, &uart_port);
		
		uart_port.PORT_Pin 				= PORT_Pin_13;
		uart_port.PORT_OE 				= PORT_OE_OUT;
		PORT_Init(MDR_PORTD, &uart_port);
		

		uart_init.UART_HardwareFlowControl	 = UART_HardwareFlowControl_RXE | UART_HardwareFlowControl_TXE;
		RST_CLK_PCLKcmd(RST_CLK_PCLK_UART2, ENABLE);
		UART_BRGInit(MDR_UART2, UART_HCLKdiv1);
		UART_Init(MDR_UART2, &uart_init);	
		UART_Cmd(MDR_UART1, ENABLE);
		
	}
	void check(){
		//Ждём PPS
		if(wait_pps && MDR_PORTD->RXTX & (1<<15)) {
			//Блинк светодиодом
			MDR_PORTD->RXTX ^= (1<<8);
			wait_pps = false; wait_uart = true;
			//Сохраняем штамп старшего и младшего регистра, обнуляем таймер
			high_ts = get_high(); ts = MDR_TIMER1->CNT; MDR_TIMER1->CNT = 0;
		}
		//Если есть символ в буффере UART2_RX
		if(wait_uart && MDR_UART2->FR & UART_FLAG_RXFF){
			volatile U8 data = (U8)MDR_UART2->DR;
			if(data == '$') {
				copy = true;
			}
			else if(data == '\r'){
				int stat = nmea_time_set(buff, buff_ptr);
				if(stat > 0){
					wait_uart = false; wait_pps = true;
				}
				memset(buff, 0, buff_ptr + 1); buff_ptr = 0;
				copy = false;
			}
			if(copy){
				buff[buff_ptr] = data;
				buff_ptr += 1;
			}
		}
		if(wait_uart && high_ts < get_high()){
			//Если таймер переполнился и время не пришло, то всё отменяем, прибавляем к текущему времени, штамп младшего регистра
			add_time(ts);
			wait_uart = false; wait_pps = true;
			memset(buff, 0, 256); buff_ptr = 0;
		}
	}
	void evaluate() {
		if(!init)uart_init();
	}
};

UART* new_UART(WaitSystem* waitSystem) {
  return new UART_MDR32(waitSystem);
}
