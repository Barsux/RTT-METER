#include "time.h"
#define UNIX_CONST 951868800
U32 global_high = 0;
U32 cntstamp = 0;
U16 divider = SystemCoreClock / 1000000;

void time_init(){
	TIMER_CntInitTypeDef timer_struct;
	RST_CLK_PCLKcmd(RST_CLK_PCLK_TIMER1, ENABLE);
	TIMER_BRGInit(MDR_TIMER1, TIMER_HCLKdiv1 );
	timer_struct.TIMER_ARR_UpdateMode 	= TIMER_ARR_Update_Immediately;
	timer_struct.TIMER_BRK_Polarity 		= TIMER_BRKPolarity_NonInverted;
	timer_struct.TIMER_CounterDirection = TIMER_CntDir_Up;
	timer_struct.TIMER_CounterMode 			= TIMER_CntMode_ClkFixedDir;
	timer_struct.TIMER_ETR_FilterConf 	= TIMER_Filter_1FF_at_TIMER_CLK;
	timer_struct.TIMER_ETR_Polarity 		= TIMER_ETRPolarity_NonInverted;
	timer_struct.TIMER_ETR_Prescaler 		= TIMER_ETR_Prescaler_None;
	timer_struct.TIMER_EventSource 			= TIMER_EvSrc_None;
	timer_struct.TIMER_FilterSampling 	= TIMER_FDTS_TIMER_CLK_div_1;
	timer_struct.TIMER_IniCounter 			= 0;
	timer_struct.TIMER_Period 					= 0xFFFFFFFF;
	timer_struct.TIMER_Prescaler 				= 0;
	
	TIMER_CntInit(MDR_TIMER1, &timer_struct);
	
	TIMER_Cmd(MDR_TIMER1, ENABLE);
}




const uint16_t day_offset[12] = {0, 31, 61,92, 122, 153, 184, 214, 245, 275,306, 337};

uint32_t datetime2seconds(char * date, char * seconds)
{
        int day = 0, month = 0, year = 0;
        sscanf(date, "%02i%02i%02i", &day, &month, &year);
        uint8_t a = month < 3; 
        int16_t y = year - a;  
        uint8_t m = month + 12 * a - 3; 
        return (day - 1 + day_offset[m] + y * 365 + y / 4 - y / 100 + y / 400) * 86400 + UNIX_CONST;
}

uint32_t get_high(){
	return global_high;
}
	

void check_overloading(){
	if(MDR_TIMER1->CNT <= cntstamp){
		global_high++;
	}
	cntstamp = MDR_TIMER1->CNT;
}

void set_from_u64(U64 time){
	global_high = (U32)(time >> 32);
	MDR_TIMER1->CNT = (volatile uint32_t)time;
}

void set_from_utc(U64 time){
	time *= (divider * 1000);
	set_from_u64(time);
}

void set_from_datetime(char * date, char * seconds){
	U32 s = datetime2seconds(date, seconds);
	set_from_utc(s);
}

void add_time(U64 added_time){
	uint64_t time = ((uint64_t)global_high << 32) | ((uint64_t)MDR_TIMER1->CNT);
	time += added_time;
	set_from_u64(time);
}



TsNs::TsNs(){
	check_overloading();
	high = global_high;
	low = MDR_TIMER1->CNT;
}

TsNs::TsNs(U64 ticks){
	check_overloading();
	high = (U32)(ticks >> 32);
	low = (U32)(ticks);
}

void TsNs::renew(){
	check_overloading();
	high = global_high;
	low = MDR_TIMER1->CNT;
}

U64 TsNs::toUTC(){
	/*
	U64 UTE = (((uint64_t)high << 32) | ((uint64_t)low));
	PRINT("%llu", UTE);
	PRINT("%lu", SystemCoreClock);
	*/
	return (((uint64_t)high << 32) | ((uint64_t)low)) / divider;
}

U64 TsNs::toU64(){
	return ((uint64_t)high << 32) | ((uint64_t)low);
}

U64 TsNs::operator - (TsNs ts2){
	U64 time = ((uint64_t)(this->high - ts2.high) << 32) + this->low - ts2.low;
	return time / divider;
}
		
U64 TsNs::operator + (TsNs ts2){
	return (((U64)(this->high + ts2.high)<<32)|((U64)(this->low + ts2.low))) / divider;
}
		
bool TsNs::operator == (TsNs ts2){
	return this->high == ts2.high && this->low == ts2.low;
}
		
bool TsNs::operator != (TsNs ts2){
	return (this->high != ts2.high || this->low != ts2.low);
}
		
bool TsNs::operator > (TsNs ts2){
	return ((this->high > ts2.high) || (this->high == ts2.high && this->low > ts2.low));
}
		
bool TsNs::operator < (TsNs ts2){
	return ((this->high < ts2.high) || (this->high == ts2.high && this->low < ts2.low));
}
