#include <xc.h>
#include <conio.h>
#include <stdio.h>
#include <stdint.h>
#define _XTAL_FREQ 20000000
#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON      // Brown-out Reset Enable bit (BOR disabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)
typedef struct {
    uint8_t ngay;
    uint8_t thang;
    uint8_t nam;
    uint8_t gio;
    uint8_t phut;
    uint8_t giay;
} datetime;
uint8_t Decimal2BCD(uint8_t num)
{
return (num/10)<<4|(num%10);
}
uint8_t BCD2Decimal(uint8_t num)
{
return (num>>4)*10+(num&0x0F);
}
void I2C_Master_Init(unsigned long clock)
{
    SSPCON = 0x28; 
    SSPCON2 = 0x00;
    SSPSTAT = 0x00; 
    SSPADD = (_XTAL_FREQ /(4*clock))-1; 
    TRISC3 = 1; 
    TRISC4 = 1;}
void I2C_Master_Wait()
{
while((SSPSTAT & 0x04) || (SSPCON2 & 0x1F));
}
void I2C_Master_Start()
{
    I2C_Master_Wait();
    SEN = 1; 
}

void I2C_Master_Stop()
{
    I2C_Master_Wait();
    PEN = 1;
}

void  I2C_Master_RepeatedStart()
{
    I2C_Master_Wait();
    RSEN = 1;
}
void I2C_Master_Write(unsigned char data)
{
    I2C_Master_Wait();
    SSPBUF = data;
}
unsigned char I2C_Master_Read(unsigned char ack)
{
    unsigned char temp;
    I2C_Master_Wait();
    RCEN = 1;
    I2C_Master_Wait();
    temp = SSPBUF; 
    I2C_Master_Wait();
    if(ack==1) SSPCON2bits.ACKDT=0;
    else SSPCON2bits.ACKDT=1; 
    SSPCON2bits.ACKEN=1;
    return temp;
}

void Lcd_Port(char a)
{if(a & 1) D4 = 1;else D4 = 0;
if(a & 2) D5 = 1;else D5 = 0;
if(a & 4) D6 = 1;else D6 = 0;
if(a & 8) D7 = 1;else D7 = 0;}
void Lcd_Cmd(char a)
{RS = 0;
Lcd_Port(a);
EN  = 1;
__delay_ms(4);
EN  = 0;}
void Lcd_Init()
{Lcd_Port(0x00);
__delay_ms(20);
Lcd_Cmd(0x03);
__delay_ms(5);
Lcd_Cmd(0x03);
__delay_ms(11);
Lcd_Cmd(0x03);
Lcd_Cmd(0x02);
Lcd_Cmd(0x02);
Lcd_Cmd(0x08);
Lcd_Cmd(0x00);
Lcd_Cmd(0x0C);
Lcd_Cmd(0x00);
Lcd_Cmd(0x06);}
void Lcd_Clear(){
    Lcd_Cmd(0);
    Lcd_Cmd(1);
}
void Lcd_Set_Cursor(char a, char b)
{char temp,z,y;
if(a == 1)
{temp = 0x80 + b -1;
z = temp>>4;
y = temp & 0x0F;
Lcd_Cmd(z);
Lcd_Cmd(y);
}
else if(a == 2)
{temp = 0xC0 + b -1;
z = temp>>4;
y = temp & 0x0F;
Lcd_Cmd(z);
Lcd_Cmd(y);}}
 void Lcd_Write_Char(char a){
    char temp,y;
    temp = a&0x0F;y = a&0xF0;RS = 1; 
    Lcd_Port(y>>4);
    EN = 1;__delay_us(40);EN = 0;
    Lcd_Port(temp);EN = 1;
    __delay_us(40);EN = 0;}
void Lcd_Write_String(char *a){
    int i;
    for(i=0;a[i]!='\0';i++)Lcd_Write_Char(a[i]);}

void rtc_get_datetime(datetime* add)
{
  uint8_t buff[8];
  I2C_Master_Start();
  I2C_Master_Write(0xD0);
  I2C_Master_Write(0x00);
  I2C_Master_Wait();
  I2C_Master_RepeatedStart();
  I2C_Master_Write(0xD1);
  I2C_Master_Wait();
  for(int i=0;i<7;i++)
  {
     buff[i]=I2C_Master_Read(1);
  }
  buff[7] = I2C_Master_Read(0);
  I2C_Master_Stop();
  add->ngay= BCD2Decimal(buff[4]);
  add->thang = BCD2Decimal(buff[5]);
  add->nam = BCD2Decimal(buff[6]);
  add->gio = BCD2Decimal(buff[2]);
  add->phut = BCD2Decimal(buff[1]);
  add->giay = BCD2Decimal(buff[0]);
}

void rtc_set_datetime(datetime* dt){
    uint8_t buff[8];
    int i;
    buff[0]=Decimal2BCD(dt->giay);
    buff[1]=Decimal2BCD(dt->phut);
    buff[2]=Decimal2BCD(dt->gio);
    buff[4]=Decimal2BCD(dt->ngay);
    buff[5]=Decimal2BCD(dt->thang);
    buff[6]=Decimal2BCD(dt->nam);
    I2C_Master_Start();
    I2C_Master_Write(0xD0);
    I2C_Master_Wait();
    I2C_Master_Write(0x00);
    I2C_Master_Wait();
    for(i=0;i<7;i++)
    {
       I2C_Master_Write(buff[i]);
       I2C_Master_Wait();
    }
    I2C_Master_Write(0x90);
    I2C_Master_Stop();    }
void main()
{
   TRISB= 0XFF;
   TRISD= 0X00;
   RD1=0;
   datetime dt;
   I2C_Master_Init(100000);
   Lcd_Init();
   Lcd_Clear();
   char a[10];
    char b[10];
   while(1)
   {
      rtc_get_datetime(&dt);
      sprintf(a,"%2d/%2d/%2d ",dt.ngay,dt.thang,dt.nam);
      sprintf(b,"%2d/%2d/%2d ",dt.gio,dt.phut,dt.giay);
      Lcd_Set_Cursor(1,3);
      Lcd_Write_String(b);
      Lcd_Set_Cursor(2,3);
      Lcd_Write_String(a);
      if(dt.gio==23 && dt.phut==52 && dt.giay==0 ) RD1=1;
      if(RB7==0){
      __delay_ms(15);
      if(RB7==0) RD1=1;
     }
      __delay_ms(5);
    }
}      
