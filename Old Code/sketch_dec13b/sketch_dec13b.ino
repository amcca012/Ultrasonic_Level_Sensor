#include <PString.h>
char MESSAGE [160] = "" ;
//char smsbuffer[160];
int Zero = 10000; //CM to pavementwill restatrt to level=0
const int timeMinutes = 1;//Set time between readings
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
int F = 0;
int Isecond = 0;
int ItimeZone = -20;
int Irange = 0;
int Ilevel = 0 ;//level as level approachs detector from  Izero/ causes level to be initially neg
int IVsound;
int Ifahrenheit = 100;
//float pwcal = 0;
char phone1[] = "+17573248688";
//char phone2[] = "+17573248688"; //AT command cmgs requires quotation marks around text string
//char phone1[] = "+17579151048";
char phone2[] = "+17579151048";
char timeStamp[23] = "15/01/20,13:15:10-20";//This format is for AT command, is altered for MESSAGE
PString str( MESSAGE, 160 );//PString(buffer, sizeof(buffer), pi);
PString str1( timeStamp, 23);//PString(buffer, sizeof(buffer), pi);

void setup()//........................................................SETUP......................
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
  Serial.println("3/2+ dec13");
  powerup();
  smsMode();//clears message
  clearMessage();
  Serial.println("Sending ATE0");
  Serial1.println("ATE0");
  delay(1000);
  WhatsINSerial1();
  Serial.println(MESSAGE);
  clearMessage();
  Serial.println("Setup initiating variables");
  takeTemp();
  //Serial.print(" AT+CCLK=(dummy timeStamp)=");
  //Serial.print('"');
  //Serial.print(timeStamp);
  //Serial.println('"');
  //Serial1.print("AT+CCLK=");//clock battery good for five years
  //Serial1.print('"');//uncomment as necessary
  //Serial1.print(timeStamp);
  //Serial1.print('"');
  //Serial1.print('\r');
  //delay(3000);
  //WhatsINSerial1();//function read/empty Serial1 buffer into message buffer
  //Serial.println(MESSAGE);
  textSelf();//goal is to update time from network ......RTC better?
  takeReading();
  //pwcal = Irange/PWrange;
  //Serial.print("PW cal factor=>");
  //Serial.println(pwcal);
  Serial.println("Start Loop");
}

void loop()//.........................................................LOOP...............
{
  //lineprint();
  takeTemp();
  takeReading();
  takeTime();
  logdata();
  SendTextMessage();
  delay(timeMinutes * 50000);
}
int isSerial1Filling()//returns zero for no change if filling returns current size...................
{
  Serial.println("isSerial1Filling() is checking for increase in serial buffer length ");
  Serial1.flush();
  int x = Serial1.available();
  Serial.print(x);
  Serial.println(" bytes initially available");
  for (int c = 0; c < 50; c++)
  {
    delay(15);
    if (x < Serial1.available())//note:>0 means true
    {
      Serial.println("isSerial1filling()reports activity");
      return Serial1.available();
    }
  }
  Serial.println("isSerial1filling()reports no activity");
  return 0;
}

void takeTime()//..................................................Take..Time...........
{
  int M = 1; //local message offset variable
  WhatsINSerial1();//empty serial1 bufferinto MESSAGE
  clearMessage();//Fill MESSAGE with null
  Serial.println("Entering takeTime function");
  int x = 0;
  Serial.println("AT+CCLK?");
  Serial1.print("AT+CCLK?");
  Serial1.print('\r');
  delay(4000);
  Serial1.flush();
  if (Serial1.available())
  {
    WhatsINSerial1();// read Serial1 buffer into MESSAGE.
  }//MESSAGE now contains response from AT+CCLK+
  else
  {
    Serial.println("Nothing in buffer");
  }
  Serial.println("Response to cclk MESSAGE =>");
  Serial.println(MESSAGE);
  for ( M = 0; MESSAGE[M] != '/'; M++)
  {
    delay(10);//trying to stop the crazy time numbers
    //Serial.println(MESSAGE[M]);
  }//Set M to position on first / which always occurs after year

  Iyear = ((MESSAGE[M - 2] - 48) * 10) + (MESSAGE[M - 1] - 48);
  Imonth = ((MESSAGE[M + 1] - 48) * 10) + (MESSAGE[M + 2] - 48);
  Iday = ((MESSAGE[M + 4] - 48) * 10) + (MESSAGE[M + 5] - 48);
  Ihour = ((MESSAGE[M + 7] - 48) * 10) + (MESSAGE[M + 8] - 48);
  Iminute = ((MESSAGE[M + 10] - 48) * 10) + (MESSAGE[M + 11] - 48);
  Isecond = ((MESSAGE[M + 13] - 48) * 10) + (MESSAGE[M + 14] - 48);
  ItimeZone = -20; //(MESSAGE[ figure how to handle neg timezone number
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
  Serial.print(Isecond);
  Serial.println(',');
  str1.begin();//resets to beginning of string,strl1 manipulates timeStamp
  str1.print(',');//provide leading comma for timestamp
  printleading0(Iyear);//adds to str uning pstring
  str1.print(',');
  printleading0(Imonth);
  str1.print(',');
  printleading0(Iday);
  str1.print(',');
  printleading0(Ihour);
  str1.print(",");
  printleading0(Iminute);
  str1.print(",");
  printleading0(Isecond);
  str1.print(',');
  str1.print(ItimeZone);
  Serial.print("formatted timestamp =>");
  Serial.println(timeStamp);//timeStamp now ready to use in logdata
}

void printleading0(int x)// fomats x<10 with leading 0, add to timest
{
  if (x < 10)
  {
    str1.print("0");
    str1.print(x);
    //str.print(',');
  }
  else
  {
    str1.print(x);
    //str.print(',');
  }
}

void takeTemp()//.................................................Take Temp.................
{
  Serial.println("Taking Temperature");
  int centigrade = analogRead(temperatureIn) * 0.488;
  Ifahrenheit = (centigrade) * 9 / 4 + 32;
  IVsound = 331.3 + (centigrade * .6);// Corrected V sound m/sec
  Serial.print(Ifahrenheit);
  Serial.println( " F");
  Serial.print(centigrade);
  Serial.println( " c");
}

void powerup()// ...................................................POWER UP..........
{
  Serial.println("Powerup");
  clearMessage();//puts in 160 \n in MESSAGE
  WhatsINSerial1();//empty Serial1 buffer into MESSAGE
  int x = 0;
  Serial1.println("AT"); // removed +, shoot in test signal(AT), let Serial1Response wait for signal
  Serial.println("Sending AT command to check MODEM avail");//troubleshooting
  //delay(3000);//shou
  Serial1.flush();
  if (isSerial1Filling()&& Serial1.available())//0= not filling , AT+ command is done
  {
    Serial.println("Modem responding to initial AT command");
    //(Serial1Response(.1));
  }
  else //actions if serial notfilling,ie (modem off)
  {
    Serial1.println("AT"); // shoot in test signal, let Serial1Response wait for signal
    Serial.println("2nd try...AT");//troubleshooting
    //delay(3000);
    Serial1.flush();//Makes sure command is in before try reading
    x = Serial1Response(2); //if x=0 zero, test if alive for 2 sec, returns number of bytes available
    if (x == 0) //something should come back, if it doesnt, power up
    {
      digitalWrite(powerpin, LOW);
      delay(1000);

      digitalWrite(powerpin, HIGH);
      delay(2000);
      digitalWrite(powerpin, LOW);

    }
  }
  WhatsINSerial1();
  Serial.println(MESSAGE);//show what was in serial1, should be response from at
}

int Serial1Response(int maxsec )//waits "maxsec" sec maximum for serial1 (default 0 ), returns bytes available
{
  Serial.println("Serial1Response() is checking serial bytes available");//........................Serial..Response
  long int x = 0;
  for ( x = 0; x == (1000 * maxsec); x++)//100*delay of 10 usec*maxsec
  {
    delay(10);
  }
  Serial.print(Serial1.available());
  Serial.println(" bytes avaiable from serial 1");
  return (Serial1.available());//return some number bytes available
}
void logdata()//               ...........................LOG .... DATA......
{
  clearMessage();
  Serial.println("logging data");
  PString str( MESSAGE, 160  );
  //str.print( ",");
  str.print(timeStamp);//timestamp() provides leading comma
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
float takeReading()//stores median range in global variables........................
{
  int I[4];
  long int PW[4];
  for ( int x = 0; x <= 3; x++)//load three readings into array
  {
    int centigrade = (Ifahrenheit - 32) * 4 / 9;
    Irange = analogRead(analogIn) * (331.3 / IVsound); //Temperature correction applied, 1 count/cmdistance
    PWrange = ( pulseIn(pwPin, HIGH) / 53) * (333.1 / IVsound) ; //58us/cm, temperature correction applied
    I[x] = Irange;
    PW[x] = PWrange;
  }
  if (I[0] > I[1] && I[1] > I[2] || I[0] < I[1] && I[1] < I[2]) //find median value
  {
    Irange = I[1];
  }
  else
  {
    if (I[2] > I[1] && I[0] > I[2] || I[0] < I [2] && I[2] < I [1])
    {
      Irange = I[2] ;
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
  if (PW[0] > PW[1] && PW[1] > PW[2] || PW[0] < PW[1] && PW[1] < PW[2])
  {
    PWrange = PW[1];
  }
  else
  {
    if (PW[2] > PW[1] && PW[0] > PW[2] || PW[0] < PW[2] && PW[2] < PW[1])
    {
      PWrange = PW[2] ;
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
  if (Izero == 0)
  {
    Izero = Irange;
  }
  Ilevel = Izero - Irange; //  Irange Ilevel &  IzeroIzero global no return statement
}

void smsMode()//..................................................SMS..............
{
  Serial.println("SMS text mode");
  clearMessage();
  Serial1.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(2000);
  WhatsINSerial1();
  Serial.print("Response to sms mode =>");
  Serial.println(MESSAGE);
}

void SendTextMessage()//......................................Send MESSAGE...............
{
  Serial.println("SendTextMessage");
  char M[65];//dont mess with MESSAGE
  for (int x = 0; x < 2; x++)
  {
    if (x == 0 )
    {
      isSerial1Filling();
      Serial1.flush();
      Serial1.print("AT+CMGS =\"");
      Serial.print("AT+CMGS =\"");
      Serial1.print(phone1);
      Serial.print(phone1);
      Serial1.print('\"');
      Serial1.print('\r');
      Serial.print('\r');
      delay(100);
      Serial1.print(MESSAGE);
      Serial.print(MESSAGE);
      delay(100);
      Serial1.print(char(26));
      Serial.print(char(26));
      Serial1.println();
      Serial.println();
      delay(5000);
      for ( int c = 0; c < 64; c++)
      { delay(10);
        M[c] = Serial1.read();//cant recycle MESSAGE here so cant use whatsinserial()
      }
      Serial.print("modem response to phone1 message =>");
      Serial.println(M);
    }
    else
    {
      delay(5000);
      Serial1.flush();
      Serial1.print("AT+CMGS =\"");
      Serial.print("AT+CMGS =\"");
      Serial1.print(phone2);
      Serial.print(phone2);
      Serial1.print('\"');
      Serial1.print('\r');
      Serial.print('\r');
      delay(100);
      Serial1.print(MESSAGE);
      Serial.print(MESSAGE);
      delay(100);
      Serial1.print(char(26));
      Serial.print(char(26));
      Serial1.println();
      Serial.println();
      for ( int c = 0; c < 64; c++)
      { delay(10);
        M[c] = Serial1.read();//cant recycle MESSAGE here so cant use whatsinserial()
      }
      Serial.print("modem response to phone2 message =>");
      Serial.println(M);
      //WhatsINSerial1();
      //clearMessage();
    }
    //Serial1.print(MESSAGE);//the content of the message
    //Serial.print("Sending");
    //Serial.print( MESSAGE);
    //Serial1.println(char(26))
  }
}

void textSelf()//............................................textSelf
{
  Serial.println("text self() sends AT+CSCS=GSM");// want to try to obtain time from network
  clearMessage();
  Serial1.print("AT+CSCS=");//set charecter set
  Serial1.print('"');
  Serial1.print("GSM");
  Serial1.print('"');
  Serial1.println(char(26));//generates error
  Serial1.flush();
  delay(1000);
  if (!Serial1.available())
  {
    Serial.println("waiting on modem to respond to at+cscs=gsm....");
  }
  WhatsINSerial1();
  Serial.println(MESSAGE);
}

void WhatsINSerial1()//function to recycle message buffer read/empty Serial1 buffer
{
  Serial1.flush();
  Serial.println("WhatsINserial() fills MESSAGE from Serial1");
  int x = 0;
  for ( x = 0; x < 160; x++)
  {
    MESSAGE[x] = Serial1.read();
  }
}
void clearMessage()//function to empty message buffer...............clearMessage..........
{
  Serial.println("clearMessage fills MESSAGE with nulls");
  int x;
  for (x = 0; x < 160; x++)
  {
    MESSAGE[x] = '\0';
  }
  //MESSAGE[160] = '\0'; //string terminator, null ascii 0
}


