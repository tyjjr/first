/*==================================================================

VoiceRecognition.h——语音识别头文件。

上海智位机器人有限公司 版权所有
http://www.dfrobot.com.cn

2014-8-11

====================================================================
*/
#ifndef _VoiceRecognition_H_
#define _VoiceRecognition_H_
#include <utility/PinMap.h>
#include <Arduino.h>

#define uint8 unsigned char
#define SPI_MODE_MASK 0x0C  // CPOL = bit 3, CPHA = bit 2 on SPCR
#define MIC 0x0b
#define MONO 0x23
#define uint8 unsigned char
#define CLK_IN   	     24///频率
#define PLL_11			(uint8)((CLK_IN/2.0)-1)
#define PLL_ASR_19 		(uint8)(CLK_IN*32.0/(PLL_11+1) - 0.51)
#define PLL_ASR_1B 		0x48
#define PLL_ASR_1D 		0x1f


#if MEGA_SOFT_SPI && (defined(__AVR_ATmega1280__)||defined(__AVR_ATmega2560__))
#define SOFTWARE_SPI
#endif  // MEGA_SOFT_SPI
//------------------------------------------------------------------------------
// SPI pin definitions
//
#ifndef SOFTWARE_SPI
// hardware pin defs

/** The default chip select pin for the VOICE is SS. */
uint8_t const  LD_CHIP_SELECT_PIN = SS_PIN;
// The following three pins must not be redefined for hardware SPI.
/** SPI Master Out Slave In pin */
uint8_t const  SPI_MOSI_PIN = MOSI_PIN;
/** SPI Master In Slave Out pin */
uint8_t const  SPI_MISO_PIN = MISO_PIN;
/** SPI Clock pin */
uint8_t const  SPI_SCK_PIN = SCK_PIN;

/** optimize loops for hardware SPI */
#define OPTIMIZE_HARDWARE_SPI

#else  // SOFTWARE_SPI
// define software SPI pins 
/** SPI chip select pin */
uint8_t const LD_CHIP_SELECT_PIN = 10;
/** SPI Master Out Slave In pin */
uint8_t const SPI_MOSI_PIN = 11;
/** SPI Master In Slave Out pin */
uint8_t const SPI_MISO_PIN = 12;
/** SPI Clock pin */
uint8_t const SPI_SCK_PIN = 13;

#endif  // SOFTWARE_SPI

class VoiceRecognition
{

public:
	VoiceRecognition();

	void Reset();
	void Init(uint8_t mic=MIC);
	void ASR_Init();
	unsigned char Start();
	void AddCommand(char *pass,int num);
	int Read();
	
	void MicVol(uint8_t vol);
	void SpeechEndpoint(uint8_t speech_endpoint_);
	void SpeechStartTime(uint8_t speech_start_time_);
	void SpeechEndTime(uint8_t speech_end_time_);
	void VoiceMaxLength(uint8_t voice_max_length_);
	void NoiseTime(uint8_t noise_time_);
 private:

};

	void WriteReg(unsigned char address,unsigned char value);
	unsigned char ReadReg(unsigned char address);
	byte Transfer(byte _data);
	void CSHigh(void); 
	void CS_Low(void);
	void update();
	int Check_b2();
#endif