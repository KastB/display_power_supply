
/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8,7,6, 5, 4, 3, 2);
boolean debug = false;
double pMax = 0;
double vA2Sum=0.0, vShuntsSum=0.0, vMinusSum=0.0, vPlusSum=0.0, vV2Sum=0.0, vFlugSum=0.0;
int numHighCurrent=0, numLowCurrent=0;
int counter=0;
int numLoops = 1000;
boolean highCurrent = false;

int pShunts = A2 ,
pMinus = A4 ,
pPlus = A3,
pV2 = A1,
pA2 = A0,
pVflug = A5;

int ledState = LOW;
int  ledPin = 13;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
//  lcd.print("hello, world!");
  Serial.begin(9600);      // open the serial port at 9600 bps:
  analogReference(INTERNAL);
    pinMode(ledPin, OUTPUT);
}

double fmap(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
 // lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  //lcd.print(millis()/1000);
  double v, vSet;
  double a, aSet;

  double p;

  double vA2, vShunts, vMinus, vPlus, vV2, vFlug;

  vShunts = (double)analogRead(pShunts) * 1.075268817; //*1.1V/1023*1000mV/V
  vMinus = (double)analogRead(pMinus) * 1.075268817; //*1.1V/1023*1000mV/V
  vPlus = (double)analogRead(pPlus) * 19.83339241; //*1.1V/1023*1000mV/V / 3,6*(3.6+63.5)
  vV2 = (double)analogRead(pV2) * 1.574500768; //*1.1V/1023*1000mV/V / 3.64*(1.79+3.64)
  vA2 = (double)analogRead(pA2) * 1.075268817; //*1.1V/1023*1000mV/V
  vFlug = (double)analogRead(pVflug) * 19.83339241; //*1.1V/1023*1000mV/V / 3,6*(3.6+63.5)


  //ist der Schalter für hohe Ströme gelegt?
  if(vShunts > 1.5 || vMinus > 1.5)
  {
    if(vShunts <= vMinus / 2.0 )
    {
       numHighCurrent++;
    }
    else
    {
       numLowCurrent++;
    }
  }
  vShuntsSum +=  vShunts;
  vMinusSum += vMinus;
  vPlusSum += vPlus;
  vV2Sum += vV2;
  vA2Sum += vA2;
  vFlugSum += vFlug;
  counter ++;

  if(counter > numLoops)
  {

      vShunts = vShuntsSum / counter;
      vMinus = vMinusSum / counter;
      vPlus = vPlusSum / counter;
      vV2 = vV2Sum / counter;
      vA2 = vA2Sum / counter;
      vFlug = vFlugSum / counter;

      if(numLowCurrent > counter/2)
      {
        highCurrent = false;
      }
      if(numHighCurrent > counter/2)
      {
         highCurrent = true;
      }

      vShuntsSum = 0.0;
      vMinusSum = 0.0;
      vPlusSum = 0.0;
      vV2Sum = 0.0;
      vA2Sum = 0.0;
      vFlugSum = 0.0;
      numLowCurrent = 0;
      numHighCurrent = 0;
      counter = 0;

      double relA =  1-(((vA2 - vMinus) * 0.0018934) - 0.3);
      relA = fmap(relA, 0.05, 0.97,0,1);
      if(relA < 0.0)
      {
         relA = 0.0;
      }
      if(relA > 1.0)
      {
         relA = 1.0;
      }

      //Je nachdem werden die Ströme anders berechnet - bei den Spannungen bleibt alles gleich
      if(highCurrent)
      {
         a = (double)vMinus *0.03 * 0.762; // I=U/R => /(0.1/3) * 1000
         aSet = relA * 10.08;
      }
      else
      {
         a = vMinus*1.625;
         aSet = relA * 780;
      }
      v = (vPlus - vMinus)/1000.0;

      double vSetRel;
      if(vPlus > 1000.0)
      {
         vSetRel = (double)vPlus/(double)vV2/11.0;
         vSet = fmap(vSetRel, 0.11, 1.0, 2.0, 17.5);
      }
      else
      {
       vSetRel = 2.0 - (vPlus - 5000.0) / (vFlug - 5000.0);
       vSet = fmap(vSetRel, 0.15, 1.0, 2.0, 21.0);
      }
      if(v < 0.0)
      {
        v= 0.0;
      }

      if(debug)
      {
        Serial.print("A: ");
        Serial.print(a);
        if(highCurrent)
        {
              Serial.println("A");
        }
        else
        {
             Serial.println("mA");
        }
         Serial.print("V: ");
        Serial.println(v);

        Serial.print("aSet");
         Serial.print(aSet);
        if(highCurrent)
        {
              Serial.println("A");
        }
        else
        {
             Serial.println("mA");
        }
        Serial.print("vSet");
         Serial.println(vSet);
       }

      lcd.setCursor(0,0);
      lcd.print(v);
      lcd.setCursor(4,0);
      lcd.print("V ");
      lcd.print(a);
      lcd.setCursor(10,0);
      //hier hab ich einen Bug behoben, der noch nicht im Netzteil eingespielt ist: einheitenfehler
      if(highCurrent)
      {
            lcd.setCursor(10,0);
            lcd.print("A ");
            p = v*a;
      }
      else
      {
           lcd.setCursor(9,0);
           lcd.print("mA ");
           p = v*a/1000;
      }
      lcd.setCursor(12,0);
      lcd.print(p);
      lcd.setCursor(15,0);
      lcd.print("W");

      lcd.setCursor(0,1);
      lcd.print(vSet);
      lcd.setCursor(4,1);
      lcd.print("V ");
      lcd.print(aSet);


      if(highCurrent)
      {
            lcd.setCursor(10,1);
            lcd.print("A ");
      }
      else
      {
           lcd.setCursor(9,1);
           lcd.print("mA ");
      }
      lcd.setCursor(12,1);
      if(pMax < p)
      {
        pMax = p;
      }
      lcd.print(pMax);
      lcd.setCursor(15,1);
      lcd.print("W");

          if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}


