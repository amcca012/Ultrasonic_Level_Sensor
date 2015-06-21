#include <PString.h>
char MESSAGE [160] = "" ;
const int powerpin = 31;//cycle for software start gprs
const int txpin = 18;//pins to comm with gprs shield
const int rxpin = 19;
const int pwPin = A3;//pulsewidth ranging pin
const int analogIn = A2; //analog ranging pin
const int pingPermit = 22; //digital 22 high >20us=range
const int temperatureIn = A1;//Temp sensor lm35 speed sound correction
#include <stdio.h>//Standard inout
//#include<Wire.h>//TWI/IC2
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
int Irange = 0;//dist to surface
int Ilevel = 0 ;//level as level approachs detector from  Izero/ causes level to be initially neg
int ts = 0; //switch for troubleshooting verbosity
int tc = 1; //switch for temperature correction
int cal = 0; //switch for cal 1=on 0 = off
char phone1[] = "+17573248688"; //AT command cmgs requires quotation marks around text string
char phone2[] = "+17575256148";//
//char phone[] = "+17579151048";
char timeStamp[23] = "15/01/20,13:15:10-20";//This format is for AT command, is altered for MESSAGE
PString str( MESSAGE, 160 );//PString(buffer, sizeof(buffer), pi);
PString str1( timeStamp, 23);//PString(buffer, sizeof(buffer), pi);

void setup()//........................................................SETUP......................
{
  Serial1.begin(9600);
  Serial.begin(9600);  //Serial1.begin(9600);//Mega shield extra UARTs, ensure correct board is selected
  delay(100);
  pinMode(pingPermit, OUTPUT);//d22 range  on HIGH>22us
  digitalWrite(pingPermit, HIGH); //May just leave on for better readings
  pinMode(powerpin, OUTPUT);//soft ware on/off gprs
  pinMode(txpin, OUTPUT);//HWserial1
  pinMode(rxpin, INPUT);//HWSerial1
  pinMode(pwPin, INPUT); //Measure range using pw method
  pinMode(analogIn, INPUT);
  Serial.println("  4/28");
  powerup();
  smsMode();//clears message
  if (cal)
  {
    Serial.println(" Calibrate Mode on");
  }
  if (tc == 0)
  {
    Serial.println("Temperature compensation is off");
  }
  clearMessage();
  //error(2);
  Serial.println("     Start Loop");
}

void loop()//.........................................................LOOP...............
{
  calib();
  clearMessage();
  takeTime();
  hourlyReport();
  takeTime();//get timestamp ready
  SendTextMessage(phone1);//runs log assembler (logdata) and transmits
  SendTextMessage(phone2);//add as many phones as desiredSendTestMessage(phone3) etc
}
void calib()
{
  while (cal == 1)
  {
    delay(5000);
    takeReading();
    Serial.print(Irange);
    Serial.println(" CM  Analog output");
    Serial.print(PWrange);
    Serial.println(" CM pulse width output");
  }
}
void clearMessage()//function to empty message buffer...............clearMessage..........
{
  if (ts == 1)
  {
    Serial.println("      clearMessage     ");//fills message with \0, terminator in pos 160
  }
  int x;
  for (x = 0; x < 160; x++)
  {
    MESSAGE[x] = '\0';
  }
  MESSAGE[160] = '\0'; //string terminator, null ascii 0
}


void hourlyReport()//1=timer for logginghourly or level change
{
  Serial.println(' ');
  Serial.println("................Hourly report............");
  float time = millis();
  float hour = 3480000;//tweaked to reduce interval from 62 to 60 minut av
  float minut = 59400;
  float ms = 4294967295;
  while (millis() + hour > ms)
  {
    time = millis();//mark time until millis function rolls over to zero
  }
  delay(2000);
  int l = Ilevel;//reset to this base level for detecting level changes between houly reports
  while (time + hour > millis()  ) //ms per hour
  {
    takeReading();
    delay(minut);//take reading every 1 min approx to trigger logging on level increase
    Serial.print(Ilevel);
    Serial.print(" CM level at ");
    Serial.print(60 - ((millis() - time ) / minut)); // should work if prevent rollover
    Serial.println(" Min.");
    if ((Ilevel - l) > 2 || (l - Ilevel) > 2) //level change >1 or millis() rolled over
    {
      delay(5000);
      takeReading();
      if ((Ilevel - l) > 2 || (l - Ilevel) > 2) //level change >1 or millis() rolled over
      {
        Serial.print("Reporting large level change of (cm) ");
        Serial.println(Ilevel - l);
        return;
      }
      return;
    }
  }

  Serial.println("..........hourly report follows......");
  return;
}

int isSerial1Filling(int counts)//(count delay)returns current size, or zero if no change, 64 if full...................
{
  if (ts == 1)
  {
    Serial.println("isSerial1Filling() is checking for increase in serial buffer length ");
  }
  Serial1.flush();
  int x = Serial1.available();
  int y = x;
  if (x == 64)
  {
    if (ts == 1)
    {
      Serial.println("Serial buffer is full");
      return 64;
    }
  }
  if (ts == 1)
  {
    Serial.print(x);
    Serial.println(" bytes initially available");
  }
  for (int c = 0; c < 4; c++)
  {
    delay(counts);//give a little time for something to happen but still be fast
    while (x < Serial1.available())//note:>0 means true
    {
      x++;//x will eventually catch up with available
      delay(5);
    }
    if (ts == 1)
    {
      Serial.print("isSerial1filling reports size change of");
      Serial.println(Serial1.available() - y);
    }
    return Serial1.available() - y;
  }
  if (ts == 1)
  {
    Serial.println("isSerial1filling()reports no activity");
  }
  return 0;
}
void logdata()//     assemble body of text message.......................LOG .... DATA....................................
{
  clearMessage();
  Serial.println("       logdata");
  PString str( MESSAGE, 160  );
  str.print("Time");
  str.print(timeStamp);//timestamp() provides leading comma
  str.print( ",");
  str.print("pulse width range (cm)=> ");
  str.print( ",");
  str.print(PWrange);
  str.print( ",");
  str.print(" analogue range =>");
  str.print( ",");
  str.print(Irange);
  str.print(",");
  str.print( "F=>");
  str.print( ",");
  str.print(takeTemp());
  str.print( ",");
  str.print( "level");
  str.print( ",");
  str.print(Ilevel);
  str.print(",");
  str.print( "zero distance");
  str.print( ",");
  str.print(Izero);
  str.print(",");
  str.print("Phone1=");
  str.print(phone1);
  str.print("  Phone2=");
  str.print(phone2);
  str.print(",");
  if (ts == 1)
  {
    Serial.print("Logdata assembles following data string= ");
    Serial.println(MESSAGE);
  }

}

int MESSAGEScanner(char pattern[], int s) //..............................pattern recognition........
{
  //returns zero or location of complete pattern, s controls how close to end of string search goes-
  if (ts == 1)
  {
    Serial.println(' ');
    Serial.println(".............Start message scanner..............................");
    //Just insert # of char in string. Careful,Zero may a be location.not a false
    Serial.print("MESSAGEScanner Looking in MESSAGE for ");
    Serial.println(pattern);
    Serial.print("Pattern size is (incl null)");
    Serial.println(s);
  }
  int y = 0;
  int location = 0;
  if (ts == 1)
  {
    Serial.print("MESSAGE contents=>");
    Serial.println(MESSAGE);
  }
  for (location = 0 ; location <= 160 - s; location++) //scan whole messageif necessary,
    // secondary scan using y for full match. note 1st false boots out of for() loop
  {
    for (y = 0; MESSAGE[location + y] == pattern[y] ; y++) //subsquent charecters match
    {
      if (ts == 1)
      {
        Serial.print("Location=>");
        Serial.println(location + y);
        Serial.print(MESSAGE[location + y]);
        Serial.print(" =Pattern[location]= ");
        Serial.println(pattern[y]);
      }
      if (y == s - 1  ) //loc0, loc1, ... locx=1st pattern char+y(=0),locx+y(=1),locx+y(=2)...locx+y(=s-1),loc,...\n(y=160)
      {
        if (ts == 1)
        {
          Serial.print("Complete pattern found (& Messagescanner returning) location=>");
          Serial.println(location);
          Serial.println(' ');
          Serial.println(".............Exit message scanner..............................");
        }
        return location;//success,exit MESSAGEScanner here
      }
    }//pattern scan ends here if no match,resume message scan
  }
  if (ts == 1)
  {
    Serial.print("pattern not found, highest location scanned is ");
    Serial.println(location);
  }
  return 0; //0 always = no match, message starts w cr lf
}


void PauseToFillSerial1(int counts)//Waits 10 times for (counts) to give chance of response to Serial1)..................
{
  //if (ts = 1) // use to wait without permanent lockup
  Serial.println("...................PauseToFillSerial1.............................");
  while (!isSerial1Filling(counts)) //attempt to allow answer to finish before moving on
  {
    for (int y = 0; y < 11; y++)
    {
      delay(counts);
      Serial.print("no activity");
      if (y == 10 || Serial1.available() == 64)
      {
        return;
      }
    }
  }
  while (isSerial1Filling(counts)) //allow to fill, but if 64 bytes, need to "read" to make room for more
  {
    Serial.print("filling");
    delay(1000);
    if (64 == Serial1.available())
    {
      Serial.println("Serial buffer is full");
      break;
    }
  }
  Serial.println("...................Exit PauseToFillSerial1.............................");
}

void powerup()// ...................................................POWER UP..........
{
  Serial.println("      Powerup");
  int x = 0;
  int y = 0;
  Serial1.println("AT"); // shoot in test signal(AT), let Serial1Response wait for signal
  Serial1.flush();
  PauseToFillSerial1(1000);//if its powered up, something should happen
  //delay(5000);
  WhatsINSerial1();
  x = MESSAGEScanner("AT", 2);//anything to say gprs is alive
  y = MESSAGEScanner("OK", 2);
  while ( x + y == 0) //o= not found, location can never be zero with gsm module
  {
    Serial.println("another try...AT");//troubleshooting
    Serial1.println("AT");
    Serial1.flush();
    PauseToFillSerial1(1000);
    WhatsINSerial1();
    x = MESSAGEScanner("AT", 2);//anything to say gprs is alive
    y = MESSAGEScanner("OK", 2);
    if (x + y > 0)
    {
      return;
    }
    delay(15000);//this sequence toggles gprs unit between off an on
    digitalWrite(powerpin, LOW);
    delay(1000);
    digitalWrite(powerpin, HIGH);
    delay(2000);
    digitalWrite(powerpin, LOW);
  }

  Serial.print('\r');
  WhatsINSerial1();
  if (ts == 1)
  {
    Serial.println(MESSAGE);//show what was in serial1, should be response from at
  }
}


void printleading0(int x)// fomats x<10 with leading 0, add to timest..................................
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

void SendTextMessage( char phone[])//......................................Send MESSAGE...............
{
  int y = 0;
  int x = 0;
  int z = 0;
  if (ts == 1)
  {
    Serial.print('\r');
    Serial.println(' ');
    Serial.print("        SendTextMessage to ");
    Serial.println(phone);
  }
  while (x + y == 0) //this will loop until message succeeds
  {
    logdata();//assembles time and data into MESSAGE
    Serial1.flush();
    Serial1.print("AT+CMGS =\"");
    Serial1.print(phone);
    Serial1.print('\"');
    Serial1.print('\r');
    while (!Serial1.available())//the > charecter appears
    {
      for (int count = 0; count < 25; count++)
      {
        delay(10);//this ill take place of delay(150)
      }
      break;//if program gets here, something is wrong
    }
    //delay(150);
    Serial1.print(MESSAGE);
    Serial1.print('\r');
    Serial1.print(char(26));//ctrlz esc ctlrl27
    if (ts == 1)
    {
      Serial.print("AT+CMGS =\"");
      Serial.print(phone);
      Serial.println('\"');
    }
    Serial.println(MESSAGE);
    while ( !isSerial1Filling(100))
    {
    }
    WhatsINSerial1();
    x = MESSAGEScanner("+cmgs", 5);//..........did message get sent?...............
    y = MESSAGEScanner("OK", 2);
    z = MESSAGEScanner("ERROR", 5);
    if (z > 0)
    {
      Serial.println(" ...............Error response first try SMGS............");
      Serial.println(MESSAGE);
      //delay(10000);
    }
  }//if get +CMGS or OK exit while loop here
}


void smsMode()//..................................................SMS..............
{
  Serial.println("       SMS text mode");
  Serial1.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  isSerial1Filling(1000);
  WhatsINSerial1();
  Serial.print("Response to sms mode =>");
  Serial.println(MESSAGE);
}

int takeTemp()//.................................................Take Temp.................
{
  float f = (analogRead(temperatureIn) * (0.488  * 9 / 4)) + 32;
  delay(10);
  f = (analogRead(temperatureIn) * (0.488  * 9 / 4)) + 32;
  F = (int)f; //use float for calc then cast to int
  if (ts == 1)
  {
    Serial.println("............Taking Temperature ............");
    Serial.print(F);
    Serial.println( " F");
  }
  return F;
}


void takeTime()//.clears MESSAGE sets time.................................................Take..Time...........
{
  int M = 1; //local message offset variable
  Serial.println("..................... Entering takeTime function..........");
  int x = 0;
  clearMessage();//fill MESSAGE w nulls
  if (ts == 1)
  {
    Serial.println("AT+CCLK?");
  }
  Serial1.print("AT+CCLK?");
  Serial1.print('\r');
  while (!Serial1.available())
  {
    Serial.print('.');
  }
  Serial.println(' ');
  isSerial1Filling(1000);
  WhatsINSerial1();// read Serial1 buffer into MESSAGE.
  if (MESSAGEScanner("error", 5))
  {
    Serial.println(MESSAGE);
  }
  if (ts == 1)
  {
    Serial.println(MESSAGE);
  }
  M = MESSAGEScanner("+CCLK:", 6) ;
  Serial.println(' ');
  Serial.print("Response to cclk =>");
  if (M == 0)
  {
    Serial.println("MESSAGE scanner finds no response");
    return;
  }
  if (MESSAGEScanner("+CCLK: \"", 8))
  {
    M = M + 1;
  }
  Iyear = ((MESSAGE[M + 7] - 48) * 10) + (MESSAGE[M + 8] - 48);
  if (ts == 1)
  { Serial.print(MESSAGE[M + 7]);
    Serial.println(MESSAGE[M + 8]);
  }
  Imonth = ((MESSAGE[M + 10] - 48) * 10) + (MESSAGE[M + 11] - 48);
  Iday = ((MESSAGE[M + 13] - 48) * 10) + (MESSAGE[M + 14] - 48);
  Ihour = ((MESSAGE[M + 16] - 48) * 10) + (MESSAGE[M + 17] - 48);
  Iminute = ((MESSAGE[M + 19] - 48) * 10) + (MESSAGE[M + 20] - 48);
  Isecond = ((MESSAGE[M + 22] - 48) * 10) + (MESSAGE[M + 23] - 48);
  ItimeZone = -20; //(MESSAGE[ figure how to handle neg timezone number
  if (ts == 1)
  {
    Serial.print(" Date=>");
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
  }
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


int takeReading()//stores median range in global variables...sets global PWrange Irange Izero..............................................
{
  float v = Vsound();
  if (ts == 1)
  {

    Serial.println("......... Take distance reading.........");
  }
  if (Irange == 0)//first time through irange will be zero
  {
    Irange = analogRead(analogIn) * ( v / 331.3) ;
    delay(10);
    Irange = analogRead(analogIn) * ( v / 331.3) ;
  }
  int oldIrange = Irange;
  int I[4];
  long int PW[4];
  for ( int x = 0; x <= 3; x++)//load three readings into array
  {
    //int centigrade = (Ifahrenheit - 32) * 4 / 9;
    Irange = analogRead(analogIn) ; //Temperature correction applied, 1 count/cmdistance
    delay(10);
    Irange = analogRead(analogIn) * (v / 331.3) ; //Temperature correction applied, 1 count/cmdistance

    //Serial.print(analogRead(analogIn));
    PWrange = (( pulseIn(pwPin, HIGH)) / 51.28) * ( v / 331.3)  ; //58us/cm, temperature correction applied
    if (oldIrange - Irange > 1 || Irange - oldIrange > 1)
    {
      Irange = analogRead(analogIn) * ( v / 331.3) ; //Temperature correction applied, 1 count/cmdistance
      delay(10);
      Irange = analogRead(analogIn) * (v / 331.3) ; //Temperature correction applied, 1 count/cmdistance
    }
    I[x] = Irange;
    PW[x] = PWrange;
  }
  delay(50);

  if (I[0] > I[1] && I[1] > I[2] || I[0] < I[1] && I[1] < I[2]) //find median value
  {
    Irange = I[1];
  }
  else
  {
    if (I[2] > I[1] && I[0] > I[2] || I[0] < I [2] && I[2] < I [1])
    {
      Irange = I[2] ;
        }
    else
    {
      Irange = I[0];
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

  if (ts == 1)
  {
    Serial.print(Irange);
    Serial.println(" CM  Analog output");
    Serial.print(PWrange);
    Serial.println(" CM pulse width output");

  }
  if (Izero == 0)
  {
    Izero = Irange;
  }
  Ilevel = Izero - Irange; //  Irange Ilevel &  IzeroIzero global no return statement
  
}


float Vsound()//.............returns  sound velocity in m/s.......................
{
  if ( tc == 1)
  {
    if (ts == 1)
    {
      Serial.println("..... Vsound calculating sound velocity....");
    }
    int Fahrenheit = takeTemp();
    float Centigrade = ( Fahrenheit - 32) * 4 / 9;
    float vsound = 331.3 + (Centigrade * .6);// Corrected V Vsound m/sec
    vsound =  (20.05 * sqrt(273.15 + Centigrade));
    if (ts == 1)
    {
      Serial.print("Vsound m/s =");
      Serial.println(vsound);
    }
    return vsound;
  }
  //Vsound=331.3;
  return 331.3;
}
void WhatsINSerial1()//function to recycle message buffer read/empty Serial1 buffer...........Whats in Serial.......
{
  clearMessage();
  Serial1.flush();
  if (ts == 1)
  {
    Serial.println("WhatsINserial() fills MESSAGE from Serial1");
  }
  int x = 0;
  delay(1000);
  int y = Serial1.available();
  MESSAGE[0] = ' '; //insert a leading space to make  zero lenght have valid meaning to messageScanner
  for ( x = 1; x <= y + 1; x++) //serialavailable is a moving target sometimes need a snapshot
  {
    MESSAGE[x] = Serial1.read();
    delay(10);
    if (ts == 1)
    {
      Serial.print(x);
      Serial.print(MESSAGE[x]);
    }
  }
  if (ts == 1)
  {
    Serial.println();
    Serial.println("Exit whats in Serial1");
  }
}





