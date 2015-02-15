#include <PString.h>
char MESSAGE [160] = "" ;
//char smsbuffer[160];
int Zero = 10000; //CM to pavementwill restatrt to level=0
const int timeDelaySeconds = 600;//Set time between readings
const int powerpin = 31;//cycle for software start gprs
const int txpin = 18;//pins to comm with gprs shield
const int rxpin = 19;
const int pwPin = A3;//pulsewidth ranging pin

const int analogIn = A2; //analog ranging pin
const int pingPermit = 22; //digital 22 high >20us=range
const int temperatureIn = A1;//Temp sensor lm35 speed sound correction
#include <stdio.h>//Standard inout
#include<Wire.h>//TWI/IC2
//#include <RTClib.h>
unsigned int Izero = 0;//Start program with new zero
unsigned long int PWrange = 0;
int Iday = 0;
int Imonth = 0;
int Iyear = 0;
int Ihour = 0;
int Iminute = 0;
int Iminutememory = 0;
int Isecond = 0;
int Irange = 0;
int Ilevel = 0 ;//level as level approachs detector from  Izero/ causes level to be initially neg
int IVsound;
int Ifahrenheit = 100;
//int I5v = 0;
//RTC_DS1307 RTC; // define the Real Time Clock object
char phone1[] = "+17577598935";
//char phone1[] = "+17573248688"; //AT command cmgs requires quotation marks around text string
char phone2[] = "+17579151048";
char timeStamp[21] = ("15/01/20,13:15:10-20");
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
  //pinMode(Vsense, INPUT);
  Serial.println("I work + dec13");
  powerup();
  smsMode();
  Serial1.println("ATE0");
  int centigrade = analogRead(temperatureIn) * 0.488;
  Ifahrenheit = (centigrade) * 9 / 4 + 32;
  IVsound = 331.3 + (centigrade * .6);
  Izero= analogRead(analogIn) * (331.3 / IVsound);
  Serial.print(" initial temp=> ");
  Serial.println(Ifahrenheit);
  Serial1.print("AT+CCLK=");//try to set modem clock=doesnt work
  Serial1.print('"');
  Serial1.print(timeStamp);
  Serial1.println('"');
  Serial.print("AT+CCLK=");
  Serial.print('"');
  Serial.print(timeStamp);
  Serial.println('"');
  delay(500);
  WhatsINSerial1();//function to recycle message buffer read/empty Serial1 buffer
  clearMessage();//function to empty message buffer
  Serial.println(MESSAGE); 
  Serial.println("Start Loop");
}

void loop()//*************************************
{
 //takeVoltage();//need standard
  takeTemp();
  takeReading();
  takeTime();
  logdata();
  SendTextMessage();
  delay(timeDelaySeconds * 1000);//Will need tweak
}

void takeTime()//should remove+CCLK: then get date and time before LF
{
   clearMessage();
   Serial.println("takeTime");
  int x = 0;
  WhatsINSerial1();//function to recycle message buffer read Serial1 buffer
  Serial1.println("AT+");
  Serial1.println("AT+CCLK?");
  delay(500);
  Serial1.flush();
   if (Serial1.available())
  {
   WhatsINSerial1();//function to recycle message buffer read Serial1 buffer
   }//MESSAGE now contains response from AT+CCLK+
  else
  {
    Serial.println("Nothing in buffer");
  }
  Serial.println("response to cclk=>");
  if (MESSAGE[2]=='E')
  {
    Serial.println("ERROR insert dummy timestamp ");
    logdata();//will lead with comma, add timestamp etc to MESSAGE
  }
  Iyear = ((MESSAGE[1] - 48) * 10) + (MESSAGE[2] - 48);
  Imonth = ((MESSAGE[4] - 48) * 10) + (MESSAGE[5] - 48);
  Iday = ((MESSAGE[7] - 48) * 10) + (MESSAGE[8] - 48);
  Ihour = ((MESSAGE[10] - 48) * 10) + (MESSAGE[11] - 48);
  Iminute = ((MESSAGE[13] - 48) * 10) + (MESSAGE[14] - 48);
  Isecond = ((MESSAGE[15] - 48) * 10) + (MESSAGE[16] - 48);
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
  PString str( timeStamp, 21 );//PString(buffer, sizeof(buffer), pi);
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
  clearMessage();//puts a bunch of terminatorsi
  int x = 0;
  int t = 0;
  char empty[65] = "";
  //empty[65] = (Serial1.read());
  WhatsINSerial1();//fills MESSAGE with serial1 contents
  Serial.print("powerup -stuff found in serial buffer=> ");
  Serial.println(MESSAGE);
  Serial1.println("AT+"); // shoot in test signal, let Serial1Response wait for signal
  //Serial.println("AT+");//troubleshooting
  delay(1000);
  if (Serial1.available())
  {
  for(x=0; x = Serial1.available();  x++)
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
  clearMessage();
  Serial.println("logging data");
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
  PWrange = ( pulseIn(pwPin, HIGH) / 58) * (333.1 / IVsound)*(191/170) ; //58us/cm, temperature correction applied
  I[x]=Irange;
   PW[x]=PWrange;
     }
     if(I[0] > I[1] && I[1] > I[2] || I[0] < I[1] && I[1] < I[2])//find median value
    {
      Irange=I[1];
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
    }
  }
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
      }
    }
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
  char empty[65] = "";
  Serial1.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(500);
  if (Serial1.available())
  {
  for(x=0; x = Serial1.available();  x++)
  {
    empty[x] = Serial1.read();
  }   
  }
  Serial.print("Response to sms mode =>");
  Serial.println(empty);
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
      if (!Serial1.available())
      {
        Serial.print("waiting on modem to respond to at+cmgs=...");
      }
      while (!Serial1.available())//wait till modem responds with cr
      {
        delay(1000);
      }
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
void WhatsINSerial1()//function to recycle message buffer read/empty Serial1 buffer
{
  int x = 0;
  for ( x = 0; x < 64; x++)
  {
    MESSAGE[x] = Serial1.read();
  }
}
void clearMessage()//function to empty message buffer
{
  int x;
  for (x=0; x==160; x++)
  {
    MESSAGE[x]='\0';
  }
  MESSAGE[160]='\0';//string terminator, null ascii 0
}



