#include<stc15f2k60s2.h>
#define uint unsigned int
#define uchar unsigned char
#define ulong unsigned long

#define coefficient				10.73741824			//系数是2^28/MCLK得到的
#define speed             1000

sbit FSYNC = P3^2;

void uart1()interrupt 4
{
	TI=0;
}

void delay_us(ulong us)										//延时函数
{
	for(;us>0;us--);
}

uint bit_inversion(uint a)							//8bit数据反转函数
{
	a=(a&0xaa)>>1|(a&0x55)<<1;						//与、或、移位运算符的巧妙运用
	a=(a&0xcc)>>2|(a&0x33)<<2;
	a=(a&0xf0)>>4|(a&0x0f)<<4;
	return a;
}

void uart1_set(uint a)										//串口发送数据函数
{
	FSYNC=0;
	SBUF=bit_inversion(a>>8);							//数据寄存器SBUF
	while(TI);
	SBUF=bit_inversion(a);
	delay_us(5);													//注意延时
	FSYNC=1;
}

void write_freq0(ulong freq)						//FREQ0写数据
{
	uint L_REG,H_REG,reg;
	reg=coefficient*freq;
	L_REG=(reg&0x3fff)|0x4000;
	H_REG=(reg>>14)|0x4000;					//通信数据处理
	uart1_set(L_REG);
	uart1_set(H_REG);
	uart1_set(0x2000);
	delay_us(speed);
}

void write_freq1(ulong freq)						//FREQ1写数据
{
	uint L_REG,H_REG,reg;
	reg=coefficient*freq;
	L_REG=(reg&0x3fff)|0x8000;
	H_REG=(reg>>14)|0x8000;					//通信数据处理
	uart1_set(L_REG);
	uart1_set(H_REG);
	uart1_set(0x2800);
	delay_us(speed);
}

void main()
{
	ulong freq=100;												//初始频率100Hz
	bit flag=0;
																				//复位值默认为模式0
	AUXR=0x10;														//速率是系统时钟的一半sysclk/2，禁止接收数据
																				//SCON.2 TI标志位，发送请求中断，中断中清零
	IE=0x90;															//开启中断EA=1,ES=1
	uart1_set(0x2100);
	uart1_set(0xC000);
	while(1)
	{
		if(!flag)
		{
			write_freq0(freq);
			flag=!flag;
		}
		else
		{
			write_freq1(freq);
			flag=!flag;
		}
		freq=freq+10;
		if(freq>10000)
			freq=100;
	}
}