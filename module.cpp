#include "tvk02061.h"
#include <stdio.h>
#include <dos.h>
#include <windows.h>

#define THREADS_NUMBER 10
#define ITERATIONS_NUMBER 100
#define PAUSE 10 /* ms */

DWORD dwCounter = 0;
#pragma pack(1)
#define myDEBUG		1			// режим отладки

typedef  struct 
{
DWORD  param;					//код параметра
DWORD  timer;					// показания таймера при поступлении параметра
UCHAR  error;					//ошибка приема параметра
} INPUTPARAM, *pINPUTPARAM;
#define myDEBUG		1			// режим отладки
DWORD WINAPI OutputChanelFunction(CONST LPVOID lpParam); 
DWORD WINAPI InputChanelFunction(CONST LPVOID lpParam);
extern	HANDLE	hECE0206;
extern	DWORD nOutput;
extern	DWORD tick;
extern	BOOL result;

void BUF256x32_write(ULONG * paramArray);
void frequency_printf(UCHAR siFreq,UCHAR soFreq);

void SI_pusk(UCHAR ChanNumber, UCHAR Mode, UCHAR Parity, UCHAR Freq);
void SI_stop(UCHAR ChanNumber);
void SI_clear_array(UCHAR ChanNumber);

void SO_pusk(UCHAR Err_en, UCHAR Parity, UCHAR Freq, UCHAR ArrayDim, UCHAR Delay, UCHAR ArrayNumber);
void SO_stop(void);

BOOL inputParamCodeCheck(UCHAR siChanNumber,unsigned int soArrayDim_int, ULONG * outputParam);

/*************************************************************************************************/
//Класс входного канала 
class InputChanel
{
private:
	 /*
	siChanNumber
	Номер канала (1..4) 
	----------------------------------
	siMode
	0  - рабочий режим;
	1 - режим самоконтроля.
	----------------------------------
	siParity
	0  - без контроля нечетности; 
	1 - контроль нечетности.
	----------------------------------
	siFreq
	0 - прием на частотах  36-100КГц; 
	1 - прием на частотах 11-14,5 КГц.
	*/
	UCHAR  siChanNumber;
	UCHAR  siMode;
	UCHAR  siParity;
	UCHAR  siFreq;

public:
	 INPUTPARAM  bufOutput[256];
	 InputChanel()
	 {
		//Конструктор класса 
		 siChanNumber=1;
		 siMode=0;
		 siParity=0;
		 siFreq=1;
	 }
	void setInputChanelMode(UCHAR siChanNumberGet, UCHAR siModeGet, UCHAR siParityGet, UCHAR siFreqGet)
	{
		//Изменение режима работы входного канала
		siChanNumber=siChanNumberGet;
		siMode=siModeGet;
		siParity=siParityGet;
		siFreq=siFreqGet;
	}
	void openInputChanelNumber(int numberChanel)
	{
		//Запуск входного канала
		SI_pusk( numberChanel, siMode, siParity, siFreq);
	}
	void startAllChanel()
	{
		for(UCHAR siChanNumber1 =1; siChanNumber1<=4; siChanNumber1++) // пуск входных каналов
		{
			openInputChanelNumber(siChanNumber1);
			tick = GetTickCount();
			while (( GetTickCount()- tick)<100) {};
			clearArray(siChanNumber1);
		}
	}
	void clearArray(int numberChanel)
	{
		//Очистка массива буфера по номеру канала
		SI_clear_array(numberChanel);
	}
	void ReadBuffer()
	{
		//Чтение входного канала 
		//Возвращает буфер канала в свойство bufOutput
		DeviceIoControl (hECE0206,ECE02061_XP_READ_ARRAY_AP1,NULL,0,&bufOutput,2304,&nOutput,NULL);
	}
	void StopAllChanel()
	{
		for(int i=1; i<=4; i++)
		{
			SI_stop(i);
		}
	}
	void StopChanel(int NumberOfChanel)
	{
		SI_stop(NumberOfChanel);
	}
};
/*************************************************************************************************/
//Класс выходного канала
class OutputChanel
{
private:
	/*
	soErr_en
	0 - выдача 32-битных слов; 
	1 - выдача 33-битных слов;  
	255 - выдача 31-битных слов.
	----------------------------------
	soParity
	0  - без формирования нечетности; 
	1 - формирование бита нечетности
	----------------------------------
	soFreq
	Выдача на частотах: 
		0 - 12,5КГц;
		1 - 50КГц;
		2 - 100 КГц;
	----------------------------------
	soArrayDim
	Размер массива (1..255слов или 0 для массива размером 256 слов);
	----------------------------------
	soDelay
	Для многократной и циклической выдачи: интервал между массивами = 0, 1, 2, .., 255 (что соответствует  0  10,24  20,48  ..  2611,2мс);  
	Для однократной выдачи = 0
	----------------------------------
	soArrayNumber
	для однократной выдачи = 1;                          
	для многократной выдачи - количество выдаваемых массивов (2..255);                          
	для циклической выдачи = 0.
	*/
	UCHAR  soErr_en;
	UCHAR  soParity;
	UCHAR  soFreq;
	UCHAR  soArrayDim;
	UCHAR  soDelay;	
	UCHAR  soArrayNumber;

public:
	INPUTPARAM  bufOutput[256]; //Буфер канала 
	OutputChanel()
	{
		//Конструктор класса
		soErr_en=0;
		soParity=0;
		soFreq=0;
		soArrayDim=0;
		soDelay=0;
		soArrayNumber=0;
	}
	void SetOutputChanelMode(UCHAR  soErr_enGet, UCHAR  soParityGet, UCHAR  soFreqGet, UCHAR  soArrayDimGet,UCHAR soDelayGet, UCHAR soArrayNumberGet)
	{
		//Установка режима работы канала
		soErr_en=soErr_enGet;
		soParity=soParityGet;
		soFreq=soFreqGet;
		soArrayDim=soArrayDim;
		soDelay=soDelayGet;
		soArrayNumber=soArrayNumberGet;
	}
	void openInputChanel()
	{
		//Открытие канала
		SO_pusk(soErr_en, soParity, soFreq, soArrayDim, soDelay, soArrayNumber);
	}
	void WriteBuffer(ULONG param[256])
	{
	//Метод записи данных в буфер выходного канала
	//Записывает данные:
	//1. В атрибут bufOutput
	//2. Физически на устройство 
	//В качестве аргументов принимает массив данных param[256];
		for (int i=0; i<256; i++)
		{
			bufOutput[i].param=param[i];
		}
		BUF256x32_write(param);
	}
	void ReadBuffer()
	{
	//Метод чтения данных в буфере выходного канала
	//Возвращает массив данных в атрибут bufOutput
		struct 
		{
		UCHAR ParamNumber;
		UCHAR Comm;
		} bufInput47;
		ULONG param47;
		for(int i=0; i<256; i++)
		{
			bufInput47.ParamNumber = i;
			bufInput47.Comm = 0x40;
			param47 = 0;
			unsigned int templ = ( i&1)? 0x55555555 : 0xaaaaaaaa;
			do
			{
				result = DeviceIoControl (hECE0206,ECE02061_XP_READ_PARAM_1,&bufInput47,2,&param47,4,&nOutput,NULL);
			}
			while(nOutput==0);
			bufOutput[i].param=param47;
		}
	}
	void StopChanel()
	{
		SO_stop();
	}
	double Percent(InputChanel chanel)
	{
	//Функция расчета процента правильно переданных параметров между выходным и входным буферами
	//В качестве аргументов принимает данные входного канала InputChanel
		int rightAnswer=0;
		for(int i=0; i<256; i++)
		{
			if(bufOutput[i].param==chanel.bufOutput[i].param)
			{
				rightAnswer++;
			}
		}
		double RightPercent=((double)rightAnswer/256)*100;
		return RightPercent;
	}
};
/*************************************************************************************************/
//Перегрузка оператора сравнения 
bool operator ==(const InputChanel& left, const OutputChanel& right) 
{
	//Сравнивает значения буфера входного и выходного канала
	bool result=false;
	int CountCorrectBuffer=0;
	for (int i=0; i<256; i++)
	{
		if(right.bufOutput[i].param==left.bufOutput[i].param)
		{
			CountCorrectBuffer++;
		}
	}
	if(CountCorrectBuffer==256)
	{
		result=true;
	}
	return result;
}
/*************************************************************************************************/
    InputChanel inputChanel;			//Входной канал
	OutputChanel outputChanel;			//Выходной канал
void iniCH()
{
		outputChanel.SetOutputChanelMode(255,1,1,0,0,0);
	//outputChanel.SetOutputChanelMode(0,1,1,0,0,0); 
	inputChanel.setInputChanelMode(1,1,1,0);
	inputChanel.startAllChanel();
}
DWORD WINAPI ThreadProc(CONST LPVOID lpParam) {
  CONST HANDLE hMutex = (CONST HANDLE)lpParam;
  DWORD i;
  for(i = 0; i < ITERATIONS_NUMBER; i++) {
    WaitForSingleObject(hMutex, INFINITE);
    dwCounter++;
    ReleaseMutex(hMutex);
    Sleep(PAUSE);
  }
  ExitThread(0);
}

void StartProcess()
{
	TCHAR szMessage[256];
  DWORD dwTemp, i;
  HANDLE hThreads[THREADS_NUMBER];
  CONST HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
  CONST HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);

   for(i = 0; i < THREADS_NUMBER; i++) {
    hThreads[i] = CreateThread(NULL, 0, &ThreadProc, hMutex, 0, NULL);

  }

  HANDLE InputChanel;
  HANDLE OutputChanel;





	iniCH(); //Инициализация
	//outCH(); //Запуск выходного канала
	//inputCH(); //Запуск и прием входного канала

   InputChanel = CreateThread(NULL, 0, &OutputChanelFunction, hMutex, 0, NULL);
   OutputChanel= CreateThread(NULL, 0, &InputChanelFunction, hMutex, 0, NULL);

   WaitForSingleObject(InputChanel,INFINITE);
   WaitForSingleObject(OutputChanel,INFINITE);


	/*Остановка и закрытие всех каналов*/
	inputChanel.StopAllChanel();
	outputChanel.StopChanel();
}
DWORD WINAPI OutputChanelFunction(CONST LPVOID lpParam)
{
	printf("OpenOutputChanel Thread\n");
	ULONG	param[256];					//Локальный буфер
	for(int j=1; j<4; j++)
	{
		Sleep(2000);
		for( int i = 0; i<256; i++)			//Записываем информацию в локальный буфер выходного канал
		{
			param[i] =i*j;
		}								 
		outputChanel.WriteBuffer(param);
		DeviceIoControl (hECE0206,ECE02061_XP_SET_SHORT_MODE,NULL,0,NULL,0,&nOutput ,NULL);

		outputChanel.openInputChanel();		//Пуск выходного канала
		Sleep(1000);
	}
	ExitThread(0);
}
DWORD WINAPI InputChanelFunction(CONST LPVOID lpParam)
{
	printf("OpenInputChanel Thread\n");
	FILE *output=fopen("out.txt","w");	//Файл вывода информации
	ULONG	param[256];					//Локальный буфер
	for(int j=1; j<4; j++)
	{
		Sleep(5000);
		for( int i = 0; i<256; i++)			//Записываем информацию в локальный буфер выходного канал
		{
			param[i] =i*j;
		}	

	
		inputChanel.ReadBuffer();			//Чтение буфера входного канала
		Sleep(1000);

		//Вывод информации в файл 
		fprintf(output,"Процент правильного получения: %f итерации %d \n\n",outputChanel.Percent(inputChanel),j);
		for(int i=0; i<256; i++)
		{
			fprintf(output,"in=%d, out=%d, time=%d\n",param[i],inputChanel.bufOutput[i].param,inputChanel.bufOutput[i].timer);
		}
		fprintf(output,"\n\n\n");
	}
	fclose(output);
	ExitThread(0);
}