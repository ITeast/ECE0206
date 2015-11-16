// tvk02061.cpp 

#include "tvk02061.h"


HANDLE OpenDeviceByIndex(  DWORD Index, PDWORD pError );
BOOL tstOZU( void);
BOOL outputChannelTest( void);
BOOL inputChannelTest( void);
void SI_stop(UCHAR ChanNumber);
void SO_stop(void);
void StartProcess();

extern	HANDLE	hECE0206;



int main(int argc, char* argv[])
{
	//Принцип работы 
	/*
	Запускаем входной и выходной канал 
	1. Делаем настройки режимов работы канала
	2. Выполняем запись информации в выходной канал 
	3. Выполняем чтение с входного канала
	*/
	DWORD	Error;
	//Подключение к устройству по его номеру 
	hECE0206  = OpenDeviceByIndex(0, &Error);
	

	if (hECE0206 == INVALID_HANDLE_VALUE) //Проверка 
	{
		printf("ERROR opening device: (%0x) \n",Error);
		return 0;
	}
	//Остановка всего и вся
					SO_stop(); //Выходной канал 
					SI_stop(1); //Входной канал 1
					SI_stop(2); //Входной канал 2
					SI_stop(3); //Входной канал 3
					SI_stop(4); //Входной канал 4
	
	//Вывод указателя на подключенное устройство 
	printf("hECE0206 = 0x%0x\n",hECE0206);

	//Выполнение тестирования
	printf("\nOZU test\n");
	//Тест ОЗУ
	/*if( tstOZU()) //Тестирование методов записи и чтения выходного буфера
	{
		printf("OZU test ERROR\n");

		return 0;
	}
	printf("OK\n");*/


/*
	printf("\nOutput Channel (SO) Test \n");
	//Тест выходного канала
	if( outputChannelTest())
	{
		printf("OutputChannelTest ERROR\n");
		return 0;
	}

	printf("\r            \r");
	printf("OK\n");*/
/*
	printf("\nInput Channels (SI) Test \n");
	//Тест входного канала 
	if( inputChannelTest())
	{
		printf("InputChannelTest ERROR\n");
		return 0;
	}

	printf("\r            \r");
	printf("OK\n");*/
	//Закрытие работы с утройством 
	StartProcess();
	CloseHandle ( hECE0206 );
	
	return 0;
}
