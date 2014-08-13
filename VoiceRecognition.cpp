/*===========================================================

VoiceRecognition.cpp——语音识别库文件。

上海智位机器人有限公司 版权所有
http://www.dfrobot.com.cn

2014-8-11

=============================================================
*/
#include "utility/PinMap.h"
#include "VoiceRecognition.h"
#include "Arduino.h"
/*================================*/
uint8_t g_Mic;
int RSTB=9;//RSTB引脚定义
int CS=4;//RSTB引脚定义
uint8_t MIC_VOL=0x55;//ADC增益初始值
uint8_t speech_endpoint=0x10;//语音端点检测初始值
uint8_t speech_start_time=0x08;//语音端点检测开始时间初始值
uint8_t speech_end_time=0x10;//语音端点检测结束时间初始值
uint8_t voice_max_length=0xC3;//最长语音段时间，默认20秒
uint8_t noise_time=0x02;//忽略上电噪声时间
//uint8_t ASR_time最长时间
int readflag=0;
int readnum=0;

VoiceRecognition::VoiceRecognition(){}

int VoiceRecognition::Read()//识别结果读取
{
	if(readflag==1)
	{	
		readflag=0;
		return readnum;
	}
	return -1;
}
void update()//中断服务函数
{ 
	uint8_t Asr_Count=0;
	if((ReadReg(0x2b) & 0x10) && ReadReg(0xb2)==0x21 && ReadReg(0xbf)==0x35)//如果有语音识别中断、DSP闲、ASR正常结束
	{
		WriteReg(0x29,0) ;///////////关中断
		WriteReg(0x02,0) ;/////////////关FIFO中断
		Asr_Count = ReadReg(0xba);//读中断辅助信息
		if(Asr_Count>0 && Asr_Count<4) //////如果有识别结果
		{
			readnum=ReadReg(0xc5);
			readflag=1;
		}	
		WriteReg(0x2b,0);//////清楚中断编号
		WriteReg(0x1C,0);////////貌似关麦克风啊~~为毛
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
	WriteReg(0x1c, 0x09);////////麦克风设置保留
	WriteReg(0xbd, 0x20);/////////保留设置
	WriteReg(0x08, 0x01);///////////→清除FIFO_DATA
	delay( 1);
	WriteReg(0x08, 0x00);////////////清除指定FIFO后再写入一次00H
	delay( 1);
	WriteReg(0xb2, 0xff);////////给0xB2写FF
	WriteReg(0x37, 0x06);////////开始识别
	delay( 5 );
	WriteReg(0x1c, g_Mic);////////选择麦克风
	WriteReg(0x29, 0x10);////////开同步中断
	WriteReg(0xbd, 0x00);/////////启动为语音识别
}	
void CSHigh() {//CS拉高
  digitalWrite(CS, HIGH);
}
void CSLow() {//CS脚拉低
  digitalWrite(CS, LOW);
}
void WriteReg(unsigned char address,unsigned char value)////////写寄存器，参数（寄存器地址，数据）
{	
	CSLow();////拉低CS
	delay(10);
	Transfer(0x04);////////////写指令
	Transfer(address);
	Transfer(value);
	CSHigh();////拉高CS
 }

unsigned char ReadReg(unsigned char address)///读寄存器，参数（寄存器地址）
{ 
	unsigned char result;
	CSLow();////拉低CS
	delay(10);
	Transfer(0x05);///////////读指令
	Transfer(address);
	result=Transfer(0x00);
	CSHigh();///拉高CS
	return(result);
 }
byte Transfer(byte _data) /////////////////SPI数据交换
{
	SPDR = _data;
	while (!(SPSR & _BV(SPIF)));
	return SPDR;
}

void VoiceRecognition::Init(uint8_t mic)////////模块启用，参数为麦克风选择（MIC/MONO）与丝印对照,在SETUP中调用
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
	  digitalWrite(SS_PIN, HIGH); // disable any SPI device using hardware SS 拉高ss
	  // Enable SPI, Master, clock rate f_osc/128
	  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);/////初始化SPI寄存器
	  // clear double speed
	  SPSR &= ~(1 << SPI2X);//2倍速
	#endif  // SOFTWARE_SPI	
	
	SPCR = (SPCR & ~SPI_MODE_MASK) | 0x08;//设置SCK常态电平与取样时间，0x08为SCK常态为高电平，下降沿有效
	Reset();//LD3320复位操作
	attachInterrupt(0,update,LOW);//开中断	
	ASR_Init();///语音识别初始化函数	
}

void VoiceRecognition::Reset()//LD3320复位操作
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

void VoiceRecognition::ASR_Init()////////////初始化语音识别模式、
{
	  //添加状态标记 
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
		WriteReg(0xc1, num);//字符编号
		WriteReg(0xc3, 0 );//添加时输入00
		WriteReg(0x08, 0x04);//不清除
  
		delay(1);
		WriteReg(0x08, 0x00);//
		delay(1);
	for(i=0;i<=80;i++)
	{
		if (pass[i] == 0)
			break;
		WriteReg(0x5, pass[i]);///写入FIFO_EXT
	}
	WriteReg(0xb9, i);//写入当前添加字符串长度
	WriteReg(0xb2, 0xff);//////////B2全写ff 
	WriteReg(0x37, 0x04);//添加语句
}
unsigned char VoiceRecognition::Start()//////开始识别
{
	WriteReg(0x35, MIC_VOL);////adc增益；会影响识别范围即噪声
	
	WriteReg(0xb3, speech_endpoint);//语音端点检测控制

	WriteReg(0xb4, speech_start_time);//语音端点起始时间
	
	WriteReg(0xb5, speech_end_time);//语音结束时间

	WriteReg(0xb6, voice_max_length);//语音结束时间
	
	WriteReg(0xb7, noise_time);//噪声时间
	
	WriteReg(0x1c, 0x09);////////麦克风设置保留

	WriteReg(0xbd, 0x20);/////////保留设置
	WriteReg(0x08, 0x01);///////////→清除FIFO_DATA
	delay( 1);
	WriteReg(0x08, 0x00);////////////清除指定FIFO后再写入一次00H
	delay( 1);
	if(Check_b2() == 0)////////读取0xB2寄存器函数如果DSP没在闲状态则RETURN 0
	{
		return 0;
	}
	WriteReg(0xb2, 0xff);////////给0xB2写FF

	WriteReg(0x37, 0x06);////////开始识别
	delay( 5 );
	WriteReg(0x1c, g_Mic);////////选择麦克风
	WriteReg(0x29, 0x10);////////开同步中断
	WriteReg(0xbd, 0x00);/////////启动为语音识别

	return 1;////返回1
}

int Check_b2()////////用作检测芯片工作是否正常，或者DSP是否忙，不需用户操作，正常/闲返回1
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

void VoiceRecognition::MicVol(uint8_t vol)//调整ADC增益，参数（0x00~0xFF,建议10-60）；
{
	MIC_VOL=vol;
	WriteReg(0x35, MIC_VOL);////adc增益；会影响识别范围即噪声
}
void VoiceRecognition::SpeechEndpoint(uint8_t speech_endpoint_)//调整语音端点检测，参数（0x00~0xFF,建议10-40）；
{
	speech_endpoint=speech_endpoint_;
	WriteReg(0xb3, speech_endpoint);//语音端点检测控制
}

void VoiceRecognition::SpeechStartTime(uint8_t speech_start_time_)//调整语音端点起始时间，参数（0x00~0x30,单位10MS）；
{
	speech_start_time=speech_start_time_;
	WriteReg(0xb4, speech_start_time);//语音端点起始时间
}
/*void VoiceRecognition::SpeechEndTime(uint8_t speech_end_time_)//调整语音端点结束时间（吐字间隔时间），参数（0x00~0xC3,单位10MS）；
{
	//speech_end_time=speech_end_time_;
	//WriteReg(0xb5, speech_end_time);//语音结束时间
}*/
void VoiceRecognition::VoiceMaxLength(uint8_t voice_max_length_)//最长语音段时间，参数（0x00~0xC3,单位100MS）；
{
	voice_max_length=voice_max_length_;
	WriteReg(0xb6, voice_max_length);//语音
}
void VoiceRecognition::NoiseTime(uint8_t noise_time_)//上电噪声略过，参数（0x00~0xff,单位20MS）；
{
	noise_time=noise_time_;
	WriteReg(0xb7, noise_time);//噪声时间
}
