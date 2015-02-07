#include <PString.h>
char MESSAGE [160] = "" ;
//char smsbuffer[160];
int Zero = 10000; //CM to pavementwill restatrt to level=0
const int timeDelaySeconds = 600;//Set time between readings
const int powerpin = 31;//cycle for software start gprs
const int txpin = 18;//pins to comm with gprs shield
const int rxpin = 19;
const int pwPin = A2;//pulsewidth ranging pin
const int analogIn = A1; //analog ranging pin
const int pingPermit = 22; //digital 22 high >20us=range
const int temperatureIn = A3;
#include <stdio.h>//Standard inout
#include<Wire.h>//TWI/IC2
//#include <RTClib.h>
unsigned int Izero = 0;//Start program with new zero
unsigned long int PWrange = 0;
int Iday = 0;
int Imonth =0;
int Iyear = 0;
int Ihour = 0;
int Iminute = 0;
int Isecond = 0;
int Irange = 0;
int Ilevel = 0 ;//level as level approachs detector from  Izero/ causes level to be initially neg
int IVsound;
int Ifahrenheit = 100;
char phone2[] = "+17573248688";
//char phone1[] = "+17573248688"; //AT command cmgs requires quotation marks around text string
char phone1[] = "+17579151048";
char timeStamp[24] = ("15/02/07,10:00:00:10-20");
void setup()//*****************************************
{
  Serial1.begin(9600);
  Serial.begin(9600);  //Serial1.begin(9600);//Mega shield extra UARTs, ensure correct board is selected
  pinMode(pingPermit, OUTPUT);//d22 range  on HIGH>22us
  digitalWrite(pingPermit, HIGH); //May just leave on for better readings
  pinMode(powerpin, OUTPUT);//soft ware on/off gprs
  pinMode(txpin, OUTPUT);//HWserial1
  pinMode(rxpin, INPUT);//HWSerial1
  pinMode(pwPin, INPUT); //Measure range using pw method
  pinMode(analogIn, INPUT);
  powerup();
  smsMode();
  Serial1.println("ATE0");
  int centigrade = analogRead(temperatureIn) * 0.488;
  Ifahrenheit = (centigrade) * 9 / 4 + 32;
  IVsound = 331.3 + (centigrade * .6);
  Izero= analogRead(analogIn) * (331.3 / IVsound);
  //Izero=Ilevel;
  Serial1.print("AT+cclk=");
  Serial1.print('"');
  Serial1.print("15/02/07,10:18:00-20");
  Serial1.println('"');
  Serial.print("AT+CCLK=");
  Serial.print('"');
  Serial.print("15/02/07,10:18:00-20");
  Serial.println('"');
  delay(500);
  WhatsINSerial1();
  Serial.println(MESSAGE);
}

void loop()//*************************************
{
  takeTemp();
  takeReading();
  takeTime();
  logdata();
  SendTextMessage();
  delay(timeDelaySeconds * 1000);//Will need tweak
}

void takeTime()
{
  int x = 0;
  char empty[65] = "";
  Serial.println("taketime");
  /*for (x=0; x <= Serial1.available(); x++)
  {
    empty[x]=(Serial1.read());
  }*/
  WhatsINSerial1();//function to recycle message buffer read Serial1 buffer

  //Serial.println(Serial1.available());
  Serial1.println("AT+CCLK?");
  Serial.println("AT+CCLK?");
  
  delay(500);
  //Serial.print("CCLK answer ==>Serial bytes available =>");
  //Serial.println(Serial1.available());
  Serial1.flush();
   if (Serial1.available())
  {
    WhatsINSerial1();//function to recycle message buffer read Serial1 buffer
  }   
    else
  {
    Serial.println("Nothing in buffer");
  }
  Serial.println("response to cclk=>");
  Serial.println(MESSAGE);
  Iyear=((MESSAGE[10]-48)*10)+(MESSAGE[11]-48);
  Imonth=((MESSAGE[13]-48)*10)+(MESSAGE[14]-48);
  Iday=((MESSAGE[16]-48)*10)+(MESSAGE[17]-48);
  Ihour=((MESSAGE[19]-48)*10)+(MESSAGE[20]-48);
  Iminute=((MESSAGE[22]-48)*10)+(MESSAGE[23]-48);
  Isecond=((MESSAGE[25]-48)*10)+(MESSAGE[26]-48);
  Serial.print("Date=>");
 Serial.print(Iyear); 
 Serial.print("/");
 Serial.print(Imonth); 
 Serial.print("/");
 Serial.println(Iday); 
  Serial.print("Time=>");
 Serial.print(Ihour); 
 Serial.print(":");
 Serial.print(Iminute); 
 Serial.print(":");
 Serial.println(Isecond);
Serial.println(' '); 
  PString str( timeStamp, 24  );//PString(buffer, sizeof(buffer), pi);
  // str.print(',');
   str.print(Iyear); 
 str.print(',');
 str.print(Imonth); 
 str.print(',');
 str.print(Iday); 
 str.print(',');
 str.print(Ihour); 
 str.print(",");
 str.print(Iminute); 
 str.print(",");
 str.println(Isecond);
// Serial.print(timeStamp);//comma delimited to insert into final message
  
  // while (!Serial1.read() == '/r')
  //{
  //  Serial.print(" waiting resp cclk");
  //  delay(50);
  //}
 // Serial.println(Serial1.peek());
  //Serial1.readBytesUntil('/f' , timeStamp, 20); //:=58
  //Serial.println(timeStamp);
  //Serial1.readBytesUntil(10, timeStamp, 19); //lf=10
  //Serial.println(timeStamp);
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

void powerup()// 
{
  Serial.println("powerup");
  int x = 0;
  int t = 0;
  char empty[65] = "";
  //empty[65] = (Serial1.read());
  WhatsINSerial1();//fills MESSAGE with serial1 contents
  Serial.print("powerup -stuff found in serial1 buffer=> ");
  Serial.println(MESSAGE);
  Serial1.println("AT+"); // shoot in test signal, let Serial1Response wait for signal
  //Serial.println("AT+");//troubleshooting
  delay(1000);
  if (Serial1.available())
  {
  for(x=0; x < Serial1.available();  x++)
  {
    empty[x] = Serial1.read();
  }   
  }
  else //actions if nothing available(modem off)
  {
    Serial1.flush();//Makes sure command is in before try reading
    x = Serial1Response(2); //if x=0 zero, test if alive for 2 sec, returns number of bytes available
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
     Serial.println(empty);//show what was in serial1, should be response from at+
}

int Serial1Response(int maxsec )//waits "maxsec" sec maximum for serial1 (default 0 ), returns bytes available
{
  Serial.println("checking serial bytes available");
  long int x = 0;
  for ( x = 0; x == (1000 * maxsec); x++)//100*delay of 10 usec*maxsec
  {
    delay(10);
  }
  Serial.print(Serial1.available());
  Serial.println(" bytes avaiable from serial 1");
  return (Serial1.available());//return some number bytes available
}
void logdata()//               ...........................
{
  Serial.println("logging data");
  // DateTime now = RTC.now();

  PString str( MESSAGE, 160  );
  str.print( ",");
  str.print(timeStamp);
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
  //Serial.println(str);
  Serial.print("MESSAGE=");
  Serial.println(MESSAGE);

}
float takeReading()//stores range in global variables
{
  int I[4];
  int PW[4];
    for ( int x = 0; x <= 3; x++)//load three readings into array
  {
  int centigrade = (Ifahrenheit - 32) * 4 / 9;
  Irange = analogRead(analogIn) * (331.3 / IVsound); //Temperature correction applied, 1 count/cmdistance
  PWrange = ( pulseIn(pwPin, HIGH) ) * (333.1 / IVsound)*(0.0195) ; //58us/cm, temperature correction applied
  I[x]=Irange;
   PW[x]=PWrange;
     }
     if(I[0] > I[1] && I[1] > I[2] || I[0] < I[1] && I[1] < I[2])//find median value
    {
      Irange=I[1];
      //Serial.print("Median analogue 1=> ");//testing
      //Serial.println(I[1]);//testing
    }
    else
    {
     if(I[2] > I[1] && I[0] > I[2] || I[0] <I [2] && I[2] <I [1])
    {
      Irange=I[2] ;
      // Serial.print("Median analogue 2 => ");//testing
      //  Serial.println(I[2]);//testing
    }
      else
      {
        Irange = I[0];
        // Serial.print("Median analogue 0=> ");//testing
        //Serial.println(I[0]);//testing
      }}
    if(PW[0] > PW[1] && PW[1] > PW[2] || PW[0] < PW[1] && PW[1] < PW[2])
    {
      PWrange=PW[1];
    }
    else
    {
     if(PW[2] > PW[1] && PW[0] > PW[2] || PW[0] < PW[2] && PW[2] < PW[1])
    {
      PWrange=PW[2] ;
    }
      else
      {
        PWrange = PW[0];
      }}
  Serial.print(Irange);
  Serial.println(" CM  Analog output");
  Serial.print(PWrange);
  Serial.println(" CM pulse width output");
  Serial.print(Ilevel);
  Serial.println(" CM level");
  Ilevel = Izero - Irange; //  Irange Ilevel &  IzeroIzero global no return statement
}

void smsMode()
{
  int x;
  //char empty[65] = "";
  Serial1.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(500);//is this enough time?
  WhatsINSerial1();//Looking for response
  /*if (Serial1.available())
  {
  for(x=0; x < Serial1.available();  x++)
  {
    empty[x] = Serial1.read();
  }   
  }*/
  Serial.print("Response to sms mode =>");
  Serial.println(MESSAGE);
}

void SendTextMessage()
{
  Serial.println("Send TextMessage");
   for (int x = 0; x < 2; x++)
  {
    if (x == 0 )
    {
      Serial1.flush();
      Serial1.print("AT+CMGS =");
      Serial.print("AT+CMGS = ");
      Serial1.println(phone1);
      Serial.println(phone1);
      if (!Serial1.available())
      {
        Serial.print("waiting on modem to respond to at+cmgs=...");
      
      while (!Serial1.available())//wait till modem responds with cr
      {
        Serial1.print("AT+CMGS =");
         Serial1.println(phone1);
          Serial.print("AT+CMGS =");
        delay(10000);
      }}
      //Im just going to assume the cr is what is in the buffer
    }
  }
  Serial1.print(MESSAGE);//the content of the message
  Serial.print("Sending");
  Serial.print( MESSAGE);
  Serial1.println(char(26));

}
//delay(100);
//Serial1.println(char(26));//the ASCII code of the ctrl+z is 26
//delay(100);
void WhatsINSerial1()//function to recycle message buffer read Serial1 buffer
{
 int x=0;
  for ( x = 0; x < 64; x++)
  {
  MESSAGE[x]=Serial1.read();
  //MESSAGE[(2*x)+1]=MESSAGE[x];
  }
  }

