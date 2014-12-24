

char MESSAGE [160]= "" ;
int Zero=10000;//CM to pavementwill restatrt to level=0
const int timeDelaySeconds = 600;//Set time between readings
#include <PString.h>
#include <stdio.h>//Standard inout
#include<Wire.h>//TWI/IC2
//#include <RTClib.h>
char smsbuffer[160];
unsigned int Izero = 0;//Start program with new zero
int Irange=0;
int Ilevel = 0 ;//level as level approachs detector from  Izero/ causes level to be initially neg
int IVsound;
int Ifahrenheit=100;
//RTC_DS1307 RTC; // define the Real Time Clock object 
char phone1[]="+17573248688";//AT command cmgs requires quotation marks arount text string
 char phone2[]="+17579151048";
void setup()
{
  Serial1.begin(9600);
  Serial.begin(9600);
  //Serial1.begin(9600);//Mega shield extra UARTs, ensure correct board is selected
}
void loop()
{
  takeTemp();
  takeReading();
  logdata();

  // if(started) 
  //Read if there are messages on SIM card and print them. I don't care rem this out
  // if(gsm.readSMS(smsbuffer, 160, n, 20)) {
  //      Serial.println(n);
  //      Serial.println(smsbuffer);
  //      }
  delay(timeDelaySeconds * 10000);//This gives 1 minute between readings
}
void takeTemp()

{
  int centigrade =analogRead(A3) * 0.488;
  Ifahrenheit=(centigrade)*9/4 + 32;
  IVsound = 331.3 + (centigrade * .6);// Corrected V sound m/sec
  Serial.print(Ifahrenheit);
  Serial.println( " F"); 
  Serial.print(centigrade);
  Serial.println( " c");
} 

void logdata()
{
  Serial.print("logging data");
  // DateTime now = RTC.now();
  
  PString str( MESSAGE, 160  );
  str.print( ",");
  str.print(Irange);
  str.print(",");
  str.print(Ifahrenheit);
  str.print( ",");
  str.print(Ilevel);
  str.print(",");
  str.print(Izero);
  str.print(",");
  str.print("Phone1=");
  str.print(phone1);
  str.print("Phone2=");
  str.print(phone2);
  str.print(",");
  Serial.println(str);
  Serial.println(str);
  Serial.print("The string's length is ");
  Serial.println(str.length());
  Serial.print("MESSAGE=");
  Serial.println(MESSAGE);
  SendTextMessage();
 }
float takeReading()
{
  // Irange = 0;
  
  Irange =(analogRead(A1)/(2*.49))* (331.33/IVsound);
  Serial.print(Irange);
  Serial.println(" CM");
  // if(Izero==10000)
  //  {
  //   Izero=Irange;
  //  }
  Ilevel =Izero - Irange;//  Irange Ilevel &  IzeroIzero global no return statement
}





void SendTextMessage()
{
  Serial1.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
  Serial1.println("AT + CMGS = \phone1[]");//send sms message, be careful need to add a country code before the cellphone number
   Serial1.println("AT + CMGS = \"phone2[]");//send sms message, be careful need to add a country code before the cellphone number
  
  delay(100);
  Serial1.println(MESSAGE);//the content of the message
  delay(100);
  Serial1.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  Serial1.println();
}

