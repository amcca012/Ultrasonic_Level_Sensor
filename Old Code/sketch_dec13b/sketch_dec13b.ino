#include <PString.h>
char MESSAGE [160] = "" ;
char smsbuffer[160];
int Zero = 10000; //CM to pavementwill restatrt to level=0
const int timeDelaySeconds = 600;//Set time between readings
 const int powerpin = 22;//cycle for software satr gprsd
 const int txpin = 18;//pins to comm with gprs shield
 const int rxpin = 19;
 const int pwPin = A2;//pulsewidth pin
 const int analogIn =A1;//analog pin
 const int pingPermit = 22; //digital 22 high >20us=range
 const int temperatureIn =A3;
#include <stdio.h>//Standard inout
#include<Wire.h>//TWI/IC2
//#include <RTClib.h>
unsigned int Izero = 0;//Start program with new zero
long int PWrange=0;
int Irange = 0;
int Ilevel = 0 ;//level as level approachs detector from  Izero/ causes level to be initially neg
int IVsound;
int Ifahrenheit = 100;
//RTC_DS1307 RTC; // define the Real Time Clock object
char phone1[] = "+17573248688"; //AT command cmgs requires quotation marks around text string
char phone2[] = "+17579151048";
void setup()//*****************************************
{
  Serial1.begin(9600);
  Serial.begin(9600);  //Serial1.begin(9600);//Mega shield extra UARTs, ensure correct board is selected
  
  pinMode(pingPermit, OUTPUT);//d22 range  on HIGH>22us
  pinMode(powerpin, OUTPUT);//soft ware on/off gprs
  pinMode(txpin, OUTPUT);//HWserial1
  pinMode(rxpin, INPUT);//HWSerial1
  pinMode(pwPin,INPUT); //Measur range using pw method
  
  powerup();
  smsMode();
}

void loop()//*************************************
{

  takeTemp();
  takeReading();
  logdata();
  SendTextMessage();

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
  int centigrade = analogRead(temperatureIn) * 0.488;
  Ifahrenheit = (centigrade) * 9 / 4 + 32;
  IVsound = 331.3 + (centigrade * .6);// Corrected V sound m/sec
  Serial.print(Ifahrenheit);
  Serial.println( " F");
  Serial.print(centigrade);
  Serial.println( " c");
}

void powerup()// two tries at powering up
{
  int x = 0;
  for (int t = 0; t == 2; t++)//exit on t=2
  {
    Serial1.println("AT+"); // shoot in test signal, let Serial1Response wait for signal
    Serial1.flush();//Makes sure command is in before try reading
    x = Serial1Response(2); //if x=0 zero, test if alive for 2 sec, returns numberof bytes available
    if (x == 0) //something should come back, if it doesnt, power up
    {
      digitalWrite(powerpin, LOW);
      delay(1000);
      digitalWrite(powerpin, HIGH);
      delay(2000);
      digitalWrite(powerpin, LOW);
      delay(3000);
    }
  }
}

int Serial1Response(int maxsec )//waits "maxsec" sec maximum for serial1 (default 0 ), returns bytes available
{
  long int x = 0;
  for ( x = 0; x == (100 * maxsec); x++)//100*delay of 10 usec*maxsec
  {
    delay(10);
  }
  x = Serial1.available();
  return (x);//return some number bytes available
}
void logdata()
{
  Serial.print("logging data");
  // DateTime now = RTC.now();

  PString str( MESSAGE, 160  );
  str.print( ",");
  str.print(PWrange);
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
  
}
float takeReading()//stores range in global variables
{
  analogWrite(pingPermit,255);//Hold high to range
  delay(25); //hold >20 us to range
  int x=0;
  PWrange=0;
  Irange = 0;
  for (x=1; x==10; x++)//simple traffic or noise filter
  {
  PWrange =PWrange+ (pulseIn(pwPin,HIGH)/58);//58us/cm
  Irange = Irange + (analogRead(analogIn) / (2 * .49)) * (331.33 / IVsound);
  delay(100);
  }
  analogWrite(pingPermit,0);//Hold high to range
  PWrange=PWrange/10;
  Irange=Irange/10;
  Serial.print(Irange);
  Serial.println(" CM  Analog output");
  Serial.print(PWrange);
  Serial.println(" CM pulse width output");
  Ilevel = Izero - Irange; //  Irange Ilevel &  IzeroIzero global no return statement
}
void smsMode()
{
  Serial1.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
    delay(500);
}

void SendTextMessage()
{
  for (int x = 0; x < 2; x++)
  {
        if (x == 0 )
    {
      Serial1.flush();
      Serial1.print("AT+CMGS =");
      Serial.print("AT+CMGS =");
      Serial1.println(phone1);
      Serial.println(phone1);
      Serial.println("Flush");
      
     while (!Serial1.available())//wait till modem responds with cr
     {
       for (int y=0; y=500; y++)
       {
       }
     
        Serial.print("waiting on modem to respond to at+cmgs=...");
    //Im just going to assume the cr is what is in the buffer 
//delay(1500);//modem responds with> not needed with while
     }
    }
      Serial1.print(MESSAGE);//the content of the message
      Serial.print("Sending");
      Serial.print( MESSAGE);
      Serial1.println(char(26));
    //}
   // else
    //{
     // Serial1.print("AT + CMGS = "+phone2[]);//send sms message, be careful need to add a country code before the cellphone number
     // Serial1.println(MESSAGE);//the content of the message
    //}
  }
  delay(100);
  Serial1.println(char(26));//the ASCII code of the ctrl+z is 26
  delay(100);

}

