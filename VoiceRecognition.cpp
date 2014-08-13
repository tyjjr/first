/*===========================================================

VoiceRecognition.cpp��������ʶ����ļ���

�Ϻ���λ���������޹�˾ ��Ȩ����
http://www.dfrobot.com.cn

2014-8-11

=============================================================
*/
#include "utility/PinMap.h"
#include "VoiceRecognition.h"
#include "Arduino.h"
/*================================*/
uint8_t g_Mic;
int RSTB=9;//RSTB���Ŷ���
int CS=4;//RSTB���Ŷ���
uint8_t MIC_VOL=0x55;//ADC�����ʼֵ
uint8_t speech_endpoint=0x10;//�����˵����ʼֵ
uint8_t speech_start_time=0x08;//�����˵��⿪ʼʱ���ʼֵ
uint8_t speech_end_time=0x10;//�����˵������ʱ���ʼֵ
uint8_t voice_max_length=0xC3;//�������ʱ�䣬Ĭ��20��
uint8_t noise_time=0x02;//�����ϵ�����ʱ��
//uint8_t ASR_time�ʱ��
int readflag=0;
int readnum=0;

VoiceRecognition::VoiceRecognition(){}

int VoiceRecognition::Read()//ʶ������ȡ
{
	if(readflag==1)
	{	
		readflag=0;
		return readnum;
	}
	return -1;
}
void update()//�жϷ�����
{ 
	uint8_t Asr_Count=0;
	if((ReadReg(0x2b) & 0x10) && ReadReg(0xb2)==0x21 && ReadReg(0xbf)==0x35)//���������ʶ���жϡ�DSP�С�ASR��������
	{
		WriteReg(0x29,0) ;///////////���ж�
		WriteReg(0x02,0) ;/////////////��FIFO�ж�
		Asr_Count = ReadReg(0xba);//���жϸ�����Ϣ
		if(Asr_Count>0 && Asr_Count<4) //////�����ʶ����
		{
			readnum=ReadReg(0xc5);
			readflag=1;
		}	
		WriteReg(0x2b,0);//////����жϱ��
		WriteReg(0x1C,0);////////ò�ƹ���˷簡~~Ϊë
	}
	ReadReg(0x06);  
	delay(10);
	ReadReg(0x06);  
	WriteReg(0x89, 0x03);  
	delay(5);
	WriteReg(0xcf, 0x43);  
	delay(5);
	WriteReg(0xcb, 0x02);
	WriteReg(0x11, PLL_11);  
	WriteReg(0x1e,0x00);
	WriteReg(0x19, PLL_ASR_19); 
	WriteReg(0x1b, PLL_ASR_1B);	
	WriteReg(0x1d, PLL_ASR_1D);
	delay(10);
	WriteReg(0xcd, 0x04);
	WriteReg(0x17, 0x4c); 
	delay(5);
	WriteReg(0xcf, 0x4f);  
	WriteReg(0xbd, 0x00);
	WriteReg(0x17, 0x48);
	delay(10);
	WriteReg(0x3c, 0x80);  
	WriteReg(0x3e, 0x07);
	WriteReg(0x38, 0xff);  
	WriteReg(0x3a, 0x07);
	WriteReg(0x40, 0);   
	WriteReg(0x42, 8);
	WriteReg(0x44, 0); 
	WriteReg(0x46, 8); 
	delay(1);	
	WriteReg(0x1c, 0x09);////////��˷����ñ���
	WriteReg(0xbd, 0x20);/////////��������
	WriteReg(0x08, 0x01);///////////�����FIFO_DATA
	delay( 1);
	WriteReg(0x08, 0x00);////////////���ָ��FIFO����д��һ��00H
	delay( 1);
	WriteReg(0xb2, 0xff);////////��0xB2дFF
	WriteReg(0x37, 0x06);////////��ʼʶ��
	delay( 5 );
	WriteReg(0x1c, g_Mic);////////ѡ����˷�
	WriteReg(0x29, 0x10);////////��ͬ���ж�
	WriteReg(0xbd, 0x00);/////////����Ϊ����ʶ��
}	
void CSHigh() {//CS����
  digitalWrite(CS, HIGH);
}
void CSLow() {//CS������
  digitalWrite(CS, LOW);
}
void WriteReg(unsigned char address,unsigned char value)////////д�Ĵ������������Ĵ�����ַ�����ݣ�
{	
	CSLow();////����CS
	delay(10);
	Transfer(0x04);////////////дָ��
	Transfer(address);
	Transfer(value);
	CSHigh();////����CS
 }

unsigned char ReadReg(unsigned char address)///���Ĵ������������Ĵ�����ַ��
{ 
	unsigned char result;
	CSLow();////����CS
	delay(10);
	Transfer(0x05);///////////��ָ��
	Transfer(address);
	result=Transfer(0x00);
	CSHigh();///����CS
	return(result);
 }
byte Transfer(byte _data) /////////////////SPI���ݽ���
{
	SPDR = _data;
	while (!(SPSR & _BV(SPIF)));
	return SPDR;
}

void VoiceRecognition::Init(uint8_t mic)////////ģ�����ã�����Ϊ��˷�ѡ��MIC/MONO����˿ӡ����,��SETUP�е���
{
	if(mic==MIC)
	{
		g_Mic=MIC;
	}
	else if(mic==MONO)
	{
		g_Mic=MONO;
	}
	pinMode(RSTB,OUTPUT);
	pinMode(CS, OUTPUT);
	CSHigh();
	pinMode(SPI_MISO_PIN, INPUT);
	pinMode(SPI_MOSI_PIN, OUTPUT);
	pinMode(SPI_SCK_PIN, OUTPUT);

	#ifndef SOFTWARE_SPI
	  // SS must be in output mode even it is not chip select
	  pinMode(SS_PIN, OUTPUT);
	  digitalWrite(SS_PIN, HIGH); // disable any SPI device using hardware SS ����ss
	  // Enable SPI, Master, clock rate f_osc/128
	  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);/////��ʼ��SPI�Ĵ���
	  // clear double speed
	  SPSR &= ~(1 << SPI2X);//2����
	#endif  // SOFTWARE_SPI	
	
	SPCR = (SPCR & ~SPI_MODE_MASK) | 0x08;//����SCK��̬��ƽ��ȡ��ʱ�䣬0x08ΪSCK��̬Ϊ�ߵ�ƽ���½�����Ч
	Reset();//LD3320��λ����
	attachInterrupt(0,update,LOW);//���ж�	
	ASR_Init();///����ʶ���ʼ������	
}

void VoiceRecognition::Reset()//LD3320��λ����
{
  digitalWrite(RSTB,HIGH);
  delay(1);
  digitalWrite(RSTB,LOW);
  delay(1);
  digitalWrite(RSTB,HIGH);
  delay(1);
  CSLow();
  delay(1);
  CSHigh();
  delay(1);  
  WriteReg(0xb9, 0x00);
}

void VoiceRecognition::ASR_Init()////////////��ʼ������ʶ��ģʽ��
{
	  //���״̬��� 
	  ReadReg(0x06);  
//	  WriteReg(0x17, 0x35); 
	  delay(10);
	  ReadReg(0x06);  
	  WriteReg(0x89, 0x03);  
	  delay(5);
	  WriteReg(0xcf, 0x43);  
	  delay(5);
	  WriteReg(0xcb, 0x02);
	  WriteReg(0x11, PLL_11);  
	  WriteReg(0x1e,0x00);
	  WriteReg(0x19, PLL_ASR_19); 
	  WriteReg(0x1b, PLL_ASR_1B);	
	  WriteReg(0x1d, PLL_ASR_1D);
	  delay(10);
	  WriteReg(0xcd, 0x04);
	  WriteReg(0x17, 0x4c); 
	  delay(5);
//	  WriteReg(0xb9, 0x00);
	  WriteReg(0xcf, 0x4f);  
	  WriteReg(0xbd, 0x00);
	  WriteReg(0x17, 0x48);
	  delay(10);
	  WriteReg(0x3c, 0x80);  
	  WriteReg(0x3e, 0x07);
	  WriteReg(0x38, 0xff);  
	  WriteReg(0x3a, 0x07);
	  WriteReg(0x40, 0);   
	  WriteReg(0x42, 8);
	  WriteReg(0x44, 0); 
	  WriteReg(0x46, 8); 
	  delay(1);
}
void VoiceRecognition::AddCommand(char *pass,int num)
{

	int i;
		WriteReg(0xc1, num);//�ַ����
		WriteReg(0xc3, 0 );//���ʱ����00
		WriteReg(0x08, 0x04);//�����
  
		delay(1);
		WriteReg(0x08, 0x00);//
		delay(1);
	for(i=0;i<=80;i++)
	{
		if (pass[i] == 0)
			break;
		WriteReg(0x5, pass[i]);///д��FIFO_EXT
	}
	WriteReg(0xb9, i);//д�뵱ǰ����ַ�������
	WriteReg(0xb2, 0xff);//////////B2ȫдff 
	WriteReg(0x37, 0x04);//������
}
unsigned char VoiceRecognition::Start()//////��ʼʶ��
{
	WriteReg(0x35, MIC_VOL);////adc���棻��Ӱ��ʶ��Χ������
	
	WriteReg(0xb3, speech_endpoint);//�����˵������

	WriteReg(0xb4, speech_start_time);//�����˵���ʼʱ��
	
	WriteReg(0xb5, speech_end_time);//��������ʱ��

	WriteReg(0xb6, voice_max_length);//��������ʱ��
	
	WriteReg(0xb7, noise_time);//����ʱ��
	
	WriteReg(0x1c, 0x09);////////��˷����ñ���

	WriteReg(0xbd, 0x20);/////////��������
	WriteReg(0x08, 0x01);///////////�����FIFO_DATA
	delay( 1);
	WriteReg(0x08, 0x00);////////////���ָ��FIFO����д��һ��00H
	delay( 1);
	if(Check_b2() == 0)////////��ȡ0xB2�Ĵ����������DSPû����״̬��RETURN 0
	{
		return 0;
	}
	WriteReg(0xb2, 0xff);////////��0xB2дFF

	WriteReg(0x37, 0x06);////////��ʼʶ��
	delay( 5 );
	WriteReg(0x1c, g_Mic);////////ѡ����˷�
	WriteReg(0x29, 0x10);////////��ͬ���ж�
	WriteReg(0xbd, 0x00);/////////����Ϊ����ʶ��

	return 1;////����1
}

int Check_b2()////////�������оƬ�����Ƿ�����������DSP�Ƿ�æ�������û�����������/�з���1
{ 
  for (int j=0; j<10; j++)
	{
	  if (ReadReg(0xb2) == 0x21)
		{
			return 1;
		}
	  delay(10);		
	}
  return 0;
}

void VoiceRecognition::MicVol(uint8_t vol)//����ADC���棬������0x00~0xFF,����10-60����
{
	MIC_VOL=vol;
	WriteReg(0x35, MIC_VOL);////adc���棻��Ӱ��ʶ��Χ������
}
void VoiceRecognition::SpeechEndpoint(uint8_t speech_endpoint_)//���������˵��⣬������0x00~0xFF,����10-40����
{
	speech_endpoint=speech_endpoint_;
	WriteReg(0xb3, speech_endpoint);//�����˵������
}

void VoiceRecognition::SpeechStartTime(uint8_t speech_start_time_)//���������˵���ʼʱ�䣬������0x00~0x30,��λ10MS����
{
	speech_start_time=speech_start_time_;
	WriteReg(0xb4, speech_start_time);//�����˵���ʼʱ��
}
/*void VoiceRecognition::SpeechEndTime(uint8_t speech_end_time_)//���������˵����ʱ�䣨���ּ��ʱ�䣩��������0x00~0xC3,��λ10MS����
{
	//speech_end_time=speech_end_time_;
	//WriteReg(0xb5, speech_end_time);//��������ʱ��
}*/
void VoiceRecognition::VoiceMaxLength(uint8_t voice_max_length_)//�������ʱ�䣬������0x00~0xC3,��λ100MS����
{
	voice_max_length=voice_max_length_;
	WriteReg(0xb6, voice_max_length);//����
}
void VoiceRecognition::NoiseTime(uint8_t noise_time_)//�ϵ������Թ���������0x00~0xff,��λ20MS����
{
	noise_time=noise_time_;
	WriteReg(0xb7, noise_time);//����ʱ��
}
