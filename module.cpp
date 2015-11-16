#include "tvk02061.h"
#include <stdio.h>
#include <dos.h>
#pragma pack(1)
#define myDEBUG		1			// ����� �������

typedef  struct 
{
DWORD  param;					//��� ���������
DWORD  timer;					// ��������� ������� ��� ����������� ���������
UCHAR  error;					//������ ������ ���������
} INPUTPARAM, *pINPUTPARAM;
#define myDEBUG		1			// ����� �������

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
//����� �������� ������ 
class InputChanel
{
private:
	 /*
	siChanNumber
	����� ������ (1..4) 
	----------------------------------
	siMode
	0  - ������� �����;
	1 - ����� ������������.
	----------------------------------
	siParity
	0  - ��� �������� ����������; 
	1 - �������� ����������.
	----------------------------------
	siFreq
	0 - ����� �� ��������  36-100���; 
	1 - ����� �� �������� 11-14,5 ���.
	*/
	UCHAR  siChanNumber;
	UCHAR  siMode;
	UCHAR  siParity;
	UCHAR  siFreq;

public:
	 INPUTPARAM  bufOutput[256];
	 InputChanel()
	 {
		//����������� ������ 
		 siChanNumber=1;
		 siMode=0;
		 siParity=0;
		 siFreq=1;
	 }
	void setInputChanelMode(UCHAR siChanNumberGet, UCHAR siModeGet, UCHAR siParityGet, UCHAR siFreqGet)
	{
		//��������� ������ ������ �������� ������
		siChanNumber=siChanNumberGet;
		siMode=siModeGet;
		siParity=siParityGet;
		siFreq=siFreqGet;
	}
	void openInputChanelNumber(int numberChanel)
	{
		//������ �������� ������
		SI_pusk( numberChanel, siMode, siParity, siFreq);
	}
	void startAllChanel()
	{
		for(UCHAR siChanNumber1 =1; siChanNumber1<=4; siChanNumber1++) // ���� ������� �������
		{
			openInputChanelNumber(siChanNumber1);
			tick = GetTickCount();
			while (( GetTickCount()- tick)<100) {};
			clearArray(siChanNumber1);
		}
	}
	void clearArray(int numberChanel)
	{
		//������� ������� ������ �� ������ ������
		SI_clear_array(numberChanel);
	}
	void ReadBuffer()
	{
		//������ �������� ������ 
		//���������� ����� ������ � �������� bufOutput
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
//����� ��������� ������
class OutputChanel
{
private:
	/*
	soErr_en
	0 - ������ 32-������ ����; 
	1 - ������ 33-������ ����;  
	255 - ������ 31-������ ����.
	----------------------------------
	soParity
	0  - ��� ������������ ����������; 
	1 - ������������ ���� ����������
	----------------------------------
	soFreq
	������ �� ��������: 
		0 - 12,5���;
		1 - 50���;
		2 - 100 ���;
	----------------------------------
	soArrayDim
	������ ������� (1..255���� ��� 0 ��� ������� �������� 256 ����);
	----------------------------------
	soDelay
	��� ������������ � ����������� ������: �������� ����� ��������� = 0, 1, 2, .., 255 (��� �������������  0  10,24  20,48  ..  2611,2��);  
	��� ����������� ������ = 0
	----------------------------------
	soArrayNumber
	��� ����������� ������ = 1;                          
	��� ������������ ������ - ���������� ���������� �������� (2..255);                          
	��� ����������� ������ = 0.
	*/
	UCHAR  soErr_en;
	UCHAR  soParity;
	UCHAR  soFreq;
	UCHAR  soArrayDim;
	UCHAR  soDelay;	
	UCHAR  soArrayNumber;

public:
	INPUTPARAM  bufOutput[256]; //����� ������ 
	OutputChanel()
	{
		//����������� ������
		soErr_en=0;
		soParity=0;
		soFreq=0;
		soArrayDim=0;
		soDelay=0;
		soArrayNumber=0;
	}
	void SetOutputChanelMode(UCHAR  soErr_enGet, UCHAR  soParityGet, UCHAR  soFreqGet, UCHAR  soArrayDimGet,UCHAR soDelayGet, UCHAR soArrayNumberGet)
	{
		//��������� ������ ������ ������
		soErr_en=soErr_enGet;
		soParity=soParityGet;
		soFreq=soFreqGet;
		soArrayDim=soArrayDim;
		soDelay=soDelayGet;
		soArrayNumber=soArrayNumberGet;
	}
	void openInputChanel()
	{
		//�������� ������
		SO_pusk(soErr_en, soParity, soFreq, soArrayDim, soDelay, soArrayNumber);
	}
	void WriteBuffer(ULONG param[256])
	{
	//����� ������ ������ � ����� ��������� ������
	//���������� ������:
	//1. � ������� bufOutput
	//2. ��������� �� ���������� 
	//� �������� ���������� ��������� ������ ������ param[256];
		for (int i=0; i<256; i++)
		{
			bufOutput[i].param=param[i];
		}
		BUF256x32_write(param);
	}
	void ReadBuffer()
	{
	//����� ������ ������ � ������ ��������� ������
	//���������� ������ ������ � ������� bufOutput
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
	//������� ������� �������� ��������� ���������� ���������� ����� �������� � ������� ��������
	//� �������� ���������� ��������� ������ �������� ������ InputChanel
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
//���������� ��������� ��������� 
bool operator ==(const InputChanel& left, const OutputChanel& right) 
{
	//���������� �������� ������ �������� � ��������� ������
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
void StartProcess()
{
	FILE *output=fopen("out.txt","w");	//���� ������ ����������
	ULONG	param[256];					//��������� �����
    InputChanel inputChanel;			//������� �����
	OutputChanel outputChanel;			//�������� �����
										//��������� ������� �������� � ��������� ������ 
	outputChanel.SetOutputChanelMode(255,1,1,0,0,0);
	//outputChanel.SetOutputChanelMode(0,1,1,0,0,0); 
	inputChanel.setInputChanelMode(1,1,1,0);
	inputChanel.startAllChanel();
	for( int i = 0; i<256; i++)			//���������� ���������� � ��������� ����� ��������� �����
	{
		param[i] =i;
	}								 
	outputChanel.WriteBuffer(param);
	DeviceIoControl (hECE0206,ECE02061_XP_SET_SHORT_MODE,NULL,0,NULL,0,&nOutput ,NULL);

	outputChanel.openInputChanel();		//���� ��������� ������
	Sleep(1000);
	inputChanel.ReadBuffer();			//������ ������ �������� ������
		Sleep(1000);
	inputChanel.ReadBuffer();			//������ ������ �������� ������

	//����� ���������� � ���� 
	fprintf(output,"������� ����������� ���������: %f \n\n",outputChanel.Percent(inputChanel));
	for(int i=0; i<256; i++)
	{
		fprintf(output,"in=%d, out=%d, time=%d\n",param[i],inputChanel.bufOutput[i].param,inputChanel.bufOutput[i].timer);
	}
	fclose(output);
	inputChanel.StopAllChanel();
	outputChanel.StopChanel();
}
void ProccessOutputChanel()
{

}
void ProccessInputChanel()
{

}