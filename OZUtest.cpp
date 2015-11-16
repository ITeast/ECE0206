#include "tvk02061.h"


extern	HANDLE	hECE0206;
extern	DWORD nOutput;
extern	DWORD tick;
extern	BOOL result;

#pragma pack(1)



void BUF256x32_write(ULONG * paramArray);


///////////////////////////////////////////////////////////////////////
//
//  ТЕСТ буфера выходного канала
//
//////////////////////////////////////////////////////////////////////

BOOL tstOZU()
{
	ULONG   param [256];	//массив выходных параметров

//	заполнение буфера кодами  0xaaaaaaaa/0x55555555

	int i = 256;
	do{
		i--;
		param[i] = 0x55555555;
		i--;
		param[i] = 0xaaaaaaaa;
	}while(i);

	BUF256x32_write( param);//запись в выходной канал



//	чтение параметров из буфера и сравнение
//		с эталоном

	struct {
		UCHAR ParamNumber;	//номер параметра в массиве
		UCHAR Comm;	// = 0x40
		} bufInput47;
	ULONG     param47;	//код выходного параметра

	for(i=0; i<256; i++)
	{
		bufInput47.ParamNumber = i;
		bufInput47.Comm = 0x40;
		param47 = 0;

		unsigned int templ = ( i&1)? 0x55555555 : 0xaaaaaaaa;


		do{
			result = DeviceIoControl (
				hECE0206
				,ECE02061_XP_READ_PARAM_1 //чтение выходного канала
				,&bufInput47
				,2                                  
				,&param47 
				,4 
				,&nOutput 
				,NULL  );

		}while(nOutput==0);

		if(param47 != templ)
		{
			printf("ECE02061_XP_READ_PARAM_1 error: result=%x  i=%03d nOutput=%d 0x%08x (0x%08x)\n"
									,result
									, i
									, nOutput
									, param47
									,templ);
			return 1;
		}
	}


//	заполнение буфера кодами 0x55555555/0xaaaaaaaa

	i = 256;
	do{
		i--;
		param[i] = 0xaaaaaaaa;
		i--;
		param[i] = 0x55555555;
	}while(i);

	BUF256x32_write( param);//запись в выходной канал



//	чтение параметров из буфера и сравнение
//		с эталоном

	for(i=0; i<256; i++)
	{
		bufInput47.ParamNumber = i;
		bufInput47.Comm = 0x40;
		param47 = 0;

		unsigned int templ = ( i&1)? 0xaaaaaaaa : 0x55555555;

		do{
			result = DeviceIoControl (
				hECE0206
				,ECE02061_XP_READ_PARAM_1//чтение выходного буфера
				,&bufInput47
				,2                                  
				,&param47 
				,4 
				,&nOutput 
				,NULL  );

		}while(nOutput==0);

		if(param47 != templ)
		{
			printf("ECE02061_XP_READ_PARAM_1 error: result=%x  i=%03d nOutput=%d 0x%08x (0x%08x)\n"
									,result
									, i
									, nOutput
									, param47
									,templ);
			return 1;
		}
	}


	return 0;
}
