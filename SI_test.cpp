#include "tvk02061.h"

#pragma pack(1)

#define myDEBUG		1	// режим отладки

typedef  struct {
DWORD  param;	//код параметра
DWORD  timer;	// показани€ таймера при поступлении параметра
UCHAR  error;	//ошибка приема параметра
} INPUTPARAM, *pINPUTPARAM;
#define myDEBUG		1	// режим отладки

extern	HANDLE	hECE0206;
extern	DWORD nOutput;
extern	DWORD tick;
extern	BOOL result;

ULONG	outBufULONG[2];


void BUF256x32_write(ULONG * paramArray);
void frequency_printf(UCHAR siFreq,UCHAR soFreq);
void SI_pusk(UCHAR ChanNumber, UCHAR Mode, UCHAR Parity, UCHAR Freq);
void SO_pusk(UCHAR Err_en, UCHAR Parity, UCHAR Freq, UCHAR ArrayDim, UCHAR Delay, UCHAR ArrayNumber);
void SI_clear_array(UCHAR ChanNumber);
void SI_stop(UCHAR ChanNumber);
BOOL	inputParamCodeCheck(UCHAR siChanNumber,unsigned int soArrayDim_int, ULONG * outputParam);
void SO_stop(void);


///////////////////////////////////////////////////////////////////////
//
//  “≈—“  входных каналов
//
//////////////////////////////////////////////////////////////////////
BOOL inputChannelTest()
{
	
	ULONG	param[256];
	DWORD tick1;

	unsigned int	ii;
  	for( ii = 0; ii<256; ii++)
	{
		param[ii] = 0x70000000 + (ii<<16) + (((~ii)<<8)&0xff00) + ii;
	}

	BUF256x32_write( param);

	// параметры выходного канала
	UCHAR  soErr_en = 0;	// = 0  - выдача 32-битных слов; =1 выдача 33-битных слов;  = 255 - выдача 31-битных слов
	UCHAR  soParity = 1;	//= 0  - без формировани€ нечетности; = 1 - формирование бита нечетности
	UCHAR  soFreq;	//выдача на частотах 
						//	= 0    12,5 √ц;
						//	 = 1    50 √ц;
						//	 = 2    100  √ц;
	UCHAR  soArrayDim = 0;	//размер массива (1..255слов или 0 дл€ массива размером 256 слов);
	UCHAR  soDelay = 0;	//дл€ многократной и циклической выдачи: интервал между массивами = 0, 1, 2, .., 255 (что соответствует  0  10,24  20,48  ..  2611,2мс);  дл€ однократной выдачи = 0
	UCHAR  soArrayNumber = 0;	// дл€ однократной выдачи = 1;                          дл€ многократной выдачи - количество                                    выдаваемых массивов (2..255);                          дл€ циклической выдачи = 0. 


	// параметры входных каналов
	UCHAR  siChanNumber;	//номер канала (1..4) 
	UCHAR  siMode;	// = 0  - рабочий режим; = 1 - режим самоконтрол€
	UCHAR  siParity = 1;	//= 0  - без контрол€ нечетности; = 1 - контроль нечетности
	UCHAR  siFreq;	//=0 - прием на частотах 36-100 √ц; = 1 - прием на частотах 11-14,5  √ц

	
	DeviceIoControl (
		hECE0206
		,ECE02061_XP_SET_SHORT_MODE
		,NULL
		,0
		,NULL
		,0
		,&nOutput 
		,NULL  );

	for( siChanNumber =1; siChanNumber<=4; siChanNumber++)
	{
		printf("Channel # %d\n",siChanNumber);

		//	режим самоконтроль/с заглушкой 

		for( siMode = 1; siMode <=1; siMode--)
		{
			printf("   %s\n",siMode ? "Self-checking mode" : "Operating mode (with stub)");

			// проверка на всех диапазонах

			for( soFreq = 0; soFreq <=2; soFreq++)
			{
				siFreq = soFreq ? 0 : 1;

				frequency_printf(siFreq, soFreq);

				// пуск входных каналов
				for(UCHAR siChanNumber1 =1; siChanNumber1<=4; siChanNumber1++)
				{


					SI_pusk( siChanNumber1, siMode, siParity, siFreq);
					tick = GetTickCount();
					while (( GetTickCount()- tick)<100) {};
					SI_clear_array(siChanNumber1);
				}

				// пуск выходного канала

				SO_pusk(soErr_en, soParity, soFreq, soArrayDim, soDelay, soArrayNumber);

				tick = GetTickCount();
				do
				{
					tick1 = GetTickCount();
					printf("%d \r",(tick1 - tick));
				}while ((tick1 - tick)<5000) ;


				if(inputParamCodeCheck( siChanNumber
						,256
						,param))
				{

					SO_stop();
					SI_stop(1);
					SI_stop(2);
					SI_stop(3);
					SI_stop(4);

					return 1;
				}


				SO_stop();
				SI_stop(1);
				SI_stop(2);
				SI_stop(3);
				SI_stop(4);

			}//soFreq 
		}// siMode = 1/0
	}//for( siChanNumber =1; siChanNumber<=4; siChanNumber++)

	return 0;
}