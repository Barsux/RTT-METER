#ifndef TIMEH
#define TIMEH

#include "base.h"
#include "uart.h"

//Инициализация таймера
void time_init();

//Получить значение верхнего регистра времени
uint32_t get_high();

//Присвоение внутреннему счётчику старшего и младшего разрядов нового значения, на основе заданного времени в тактах / секундах / дате и секундах
void set_from_u64(uint64_t time);
void set_from_utc(uint64_t time);
void set_from_datetime(char * date, char * seconds);

//Добавить ко регистрам времени значение в тактах
void add_time(uint64_t added_time);

//Проверка на переполнение таймера
void check_overloading();



/* 
При инициализации объекта данного класса, в нём сохраняется старшее и младшее 32-битовое значение времени в машинных тактах, которым,
с помощью данного класса далее можно манипулировать, сравнивать, вычитать, преобразовывать в наносекунды
*/
class TsNs{
	public:
		TsNs();
		TsNs(uint64_t ticks);
		uint32_t high, low;
		uint16_t divider;
	
		//Записывает в объект класса новое значение времени в машинных тактах
		void renew();
		//Возвращает преобразованное время в наносекундах, ранее сохранённое в объекте
		uint64_t toUTC();
		//Возвращает преобразованное время в виде 64-битного числа в машинных тактах
		uint64_t toU64();
		uint64_t 	operator 	- 	(TsNs ts2);
		uint64_t 	operator 	+ 	(TsNs ts2);
		bool 			operator 	== 	(TsNs ts2);
		bool 			operator 	!= 	(TsNs ts2);
		bool 			operator 	> 	(TsNs ts2);
		bool 			operator 	< 	(TsNs ts2);
};

struct OTT{
	uint16_t seq;
	TsNs out_ts;
	TsNs inc_ts;
};

#endif 