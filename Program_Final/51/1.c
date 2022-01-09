/***************************************************
Copyright (C) 2021 CSY && SYQ. All rights reserved.

请在编译前在reg52头文件中添加：
sfr IAP_CONTR = 0xE7;
***************************************************/
#include "reg52.h"	 

#define u8 unsigned char
#define u16 unsigned int

sbit Mood_light1 = P0^0;  //氛围灯
sbit Mood_light2 = P0^1;
sbit Mood_light3 = P0^2;
sbit Mood_light4 = P0^3;
sbit ESP1 = P0^6;  //IO16
sbit ESP2 = P0^7;  //IO13
sbit BUTTON1 = P1^2;
sbit BUTTON2 = P1^1;
sbit BUTTON3 = P1^0;
sbit LED6 = P2^0;  //秒钟
sbit LED5 = P2^1;  //空挡
sbit LED1 = P2^2;  //1
sbit LED2 = P2^3;  //2
sbit LED3 = P2^4;  //3
sbit LED4 = P2^5;  //持续
sbit WATER = P2^6;  //喷雾
sbit LIGHT = P2^7;  //小灯

//u8 code tab[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};
//u8 code tab[] = {0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e};
u8 Level = 0,Mood_lightLevel = 0;
u16 FenZhong = 0,MiaoZhong = 0;
u16 num;
/******************************/
void initT0();  //中断初始化
void Tick();   //中断服务程序
void Change_Level();  //功率显示
void Miao_To_Fen();  //秒钟转分钟
void Delay_ms(u16 time);  //延时函数
void ESP8266_Receive();  //接收8266指令
void Mood_light();  //氛围灯挡位
/******************************/
int main(){
	u8 key_up1 = 1,key_up2 = 1;  //按键按下标志位
	initT0();
	BUTTON1 = 1;BUTTON2 = 1;BUTTON3 = 1;
	ESP1 = 1;ESP2 = 1;
	LIGHT = 0;WATER = 0;
	LED1 = 0;LED2 = 0;LED3 = 0;LED4 = 0;LED5 = 0;LED6 = 0;
	Mood_light1 = 0;Mood_light2 = 0;Mood_light3 = 0;Mood_light4 = 0;
	while(1){
		Change_Level();
		ESP8266_Receive();
		Miao_To_Fen();
		Mood_light();
/**************************************/
		if(BUTTON1 == 0 && key_up1 == 1){   //按键一按下+消抖
			Delay_ms(15);
			key_up1 = 0;
			if(BUTTON1 == 0)
				Level++;
		}
		else if(BUTTON1 == 1)
			key_up1 = 1;
		
		if(BUTTON2 == 0)    //按键二按下
			Level = 5;
		
		if(BUTTON3 == 0 && key_up2 == 1){    //按键三按下+消抖
			Delay_ms(15);
			key_up2 = 0;
			if(BUTTON3 == 0){
				//USB = ~USB;
				IAP_CONTR = 0x60;
			}
		}
		else if(BUTTON3 == 1)
			key_up2 = 1;
	}
}

/***************************************/
void ESP8266_Receive(){
	if(ESP1 == 0 && ESP2 == 0)  //当P0^6=0 P0^7=0时，初始化氛围灯挡位
		Mood_lightLevel = 0;
	if(ESP1 == 0 && ESP2 == 1){  //当P0^6=0 P0^7=1时，进入消抖
		Delay_ms(30);
		if(ESP1 == 1 && ESP2 == 1)  //当P0^6=1 P0^7=1时，调整氛围灯挡位
			Mood_lightLevel++;
		else if(ESP1 == 1 && ESP2 == 0){
			Mood_lightLevel = 10;
		}
	}
/***************************************/
	if(ESP1 == 1 && ESP2 == 0){  //当P0^6=0 P0^7=1时，进入消抖
		Delay_ms(30);
		if(ESP1 == 1 && ESP2 == 1)  //当P0^6=1 P0^7=1时，开关小灯
			Level++;
		else if(ESP1 == 0 && ESP2 == 1){
			//LED4 = 1;
		}
		else if(ESP1 == 0 && ESP2 == 0){
			Level = 0;
		}
	}
}

void Mood_light(){  //四种氛围灯挡位，默认氛围灯全关
	switch(Mood_lightLevel){
	case 1:
		Mood_light1 = 1;Mood_light2 = 0;Mood_light3 = 0;Mood_light4 = 0;
	break;
	case 2:
		Mood_light1 = 0;Mood_light2 = 1;Mood_light3 = 0;Mood_light4 = 0;
	break;
	case 3:
		Mood_light1 = 0;Mood_light2 = 0;Mood_light3 = 1;Mood_light4 = 0;
	break;
	case 4:
		Mood_light1 = 0;Mood_light2 = 0;Mood_light3 = 0;Mood_light4 = 1;
	break;
	case 10:
		Mood_light1 = 1;Mood_light2 = 1;Mood_light3 = 1;Mood_light4 = 1;
	break;
	default:
		Mood_light1 = 0;Mood_light2 = 0;Mood_light3 = 0;Mood_light4 = 0;
	}
	if(Mood_lightLevel == 5)
		Mood_lightLevel = 0;
}

void Change_Level(){  //加湿器功率显示
	if(Level == 0){
		LED1 = 0;LED2 = 0;LED3 = 0;LED4 = 0;LED5 = 1;
	}
	
	else if(Level == 1){
		LED1 = 1;LED2 = 0;LED3 = 0;LED4 = 0;LED5 = 0;
		//Mood_light1 = 0;Mood_light2 = 1;Mood_light3 = 0;Mood_light4 = 0;
		if(FenZhong >= 15){
			FenZhong = 0;
			Delay_ms(1500);
		}
	}
	
	else if(Level == 2){
		LED1 = 0;LED2 = 1;LED3 = 0;LED4 = 0;LED5 = 0;
		//Mood_light1 = 0;Mood_light2 = 0;Mood_light3 = 1;Mood_light4 = 0;
		if(FenZhong >= 15){
			FenZhong = 0;
			Delay_ms(3000);
		}
	}
	
	else if(Level == 3){
		LED1 = 0;LED2 = 0;LED3 = 1;LED4 = 0;LED5 = 0;
		//Mood_light1 = 0;Mood_light2 = 0;Mood_light3 = 0;Mood_light4 = 1;
		if(FenZhong >= 15){
			FenZhong = 0;
			Delay_ms(6000);
		}
	}
	
	else if(Level == 4){
		LED1 = 0;LED2 = 0;LED3 = 0;LED4 = 1;LED5 = 0;
		
	}
	
	else if(Level == 5){
		Level = 0;
	}
}

void Miao_To_Fen(){
	if(MiaoZhong >= 60){
		MiaoZhong = 0;
		FenZhong++;
	}
}

void initT0(){
	TMOD = 0x01;
	TH0 = (65536 - 10000) / 256;
	TL0 = (65536 - 10000) % 256;
	EA = 1;
	ET0 = 1;
	TR0 = 1;
}

void Tick() interrupt 1 {
	TH0 = (65536 - 10000) / 256;
	TL0 = (65536 - 10000) % 256;
	num++;
	if(num == 100){
		num = 0;
		MiaoZhong++;
		//Mood_lightLevel++;
		LED6 = ~LED6;
	}
}
/***********************Delay****************************************/
void Delay_ms(u16 time){
	u16 i,j;
	for(i = 0;i < time;i++)
		for(j = 0;j < 110;j++);
}