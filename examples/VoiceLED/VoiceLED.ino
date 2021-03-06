/*============================================================

VoiceLED.ino——Voice Recognition模块样例程序
上海智位机器人有限公司 版权所有
http://www.dfrobot.com.cn
2014-8-11

===========================================================*/
#include <VoiceRecognition.h>
VoiceRecognition Voice;
#define Led 8//定义Led引脚为8

void setup() {
    Serial.begin(9600);
    pinMode(Led,OUTPUT);//初始化LED引脚为输出模式
    digitalWrite(Led,LOW);//LED引脚低电平
    
    Voice.Init();//初始化VoiceRecognition模块
    
    //Voice.MicVol(85);//麦克风放大增益，参数（10-100）默认85
    //Voice.SpeechEndpoint(15);//语音/背噪对比度,参数（10-50），默认15
    //Voice.SpeechStartTime(8);//背噪-语音辨别时间，参数（00~50）单位10MS，默认80ms
    //Voice.SpeechEndTime(15);//语音-背噪辨别时间，参数（00~200）单位10MS，默认150ms
    //Voice.VoiceMaxLength(200);//最长语音段时间，参数（0~200）单位100MS,默认20s
    //Voice.NoiseTime(2);//上电噪声忽略时间，参数（00~255）单位20ms,默认40ms
    
    Voice.AddCommand("kai deng",0);//添加指令，参数（指令内容，指令标签（可重复））
    Voice.AddCommand("guan deng",1);//添加指令，参数（指令内容，指令标签（可重复））

Voice.Start();//开始识别
}
void loop() {
  // put your main code here, to run repeatedly:
  // Serial.println(Voice.ASR(2,80,sRecog));
  switch(Voice.Read())//判断识别内容，在有识别结果的情况下Voice.Read()会返回该指令标签，否则返回-1
  {
    case 0://若是指令“kai deng”
	digitalWrite(Led,HIGH);//点亮LED
	Serial.println("0");//串口发送“0”
    break;
    case 1://若是指令“guan deng”
	digitalWrite(Led,LOW);//熄灭LED
	Serial.println("1");//串口发送“1”
    break;   
  }
}
