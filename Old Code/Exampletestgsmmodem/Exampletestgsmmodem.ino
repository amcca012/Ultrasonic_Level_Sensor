/*

 This example test if your modem is working correctly.

 Circuit:
 * GSM shield attached (using digital pins 2, 3, and 7)

 Created 12 Jun 2012
 by David del Peral
 modified 21 Nov 2012
 by Tom Igoe
 */

// libraries
#include <GSM.h>

// modem verification object
GSMModem modem;

// IMEI variable
String IMEI = "";

void setup()
{
  // initialize serial communications
  Serial.begin(9600);

  // start modem test (reset and check response)
  Serial.print("Starting modem test...");
  if(modem.begin()) 
    Serial.println("modem.begin() succeeded");
  else
    Serial.println("ERROR, no modem answer.");
}

void loop()
{
  // get modem IMEI
  Serial.print("Checking IMEI...");
  IMEI = modem.getIMEI();

  // check IMEI response
  if(IMEI != NULL)
  {
    // show IMEI in serial monitor
    Serial.println("Modem's IMEI: " + IMEI);
    // reset modem to check booting:
    Serial.print("Reseting modem...");
    modem.begin();
    // get and check IMEI one more time
    if(modem.getIMEI() != NULL)
    {
      Serial.println("Modem is functoning properly");
    }
    else
    {
      Serial.println("Error: getIMEI() failed after modem.begin()");
    }
  }
  else
  {
    Serial.println("Error: Could not get IMEI");
  }
  // do nothing:
  while(true);
}


