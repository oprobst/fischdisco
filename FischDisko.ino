
//6.9.10.11
#define PIN_PWM_BLUE 6
#define PIN_PWM_RED 10
#define PIN_PWM_WWHITE 11
#define PIN_PWM_CWHITE 9

#define PIN_PWM_LCD 5

#define PIN_A_IN_SCHALTER 1
#define PIN_A_IN_TASTER 2
#define PIN_A_IN_POTI 3

#define TIMOUT_DISPLAY 60000

#include <LiquidCrystal.h>
#include <Wire.h>

LiquidCrystal lcd(7, 3, 2, 4, 12, 8);
const int DS1307 = 0x68; // Address of DS1307 see data sheets


const char* days[] =
{"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char* months[] =
{"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Initializes all values:
byte second = 0;
byte minute = 0;
byte hour = 0;
byte weekday = 0;
byte monthday = 0;
byte month = 0;
byte year = 0;

long timerSinceDisplayOff = 1;

//hours of the day on a scale 0-10000;
int hourP [] = {0, 416, 833, 1250, 1666, 2083, 2500, 2916, 3333, 3750, 4166, 4583, 5000, 5416, 5833, 6250, 6666, 7083, 7500, 7916, 8333, 8750, 9166, 9583, 10000};

void setup() {


  timerSinceDisplayOff = millis() + TIMOUT_DISPLAY;

  Wire.begin();
  Serial.begin(9600);
  lcd.begin(16, 2);

  delay(2000); // This delay allows the MCU to read the current date and time.

  printTime();

  // set pins 2 through 13 as outputs:
  //for (int thisPin = lowestPin; thisPin <= highestPin; thisPin++) {
  //  pinMode(thisPin, OUTPUT);
  //}
}
int sensorValue = 0;
void shortPush() {
  Serial.println ("ShortKey");
}

void longPush () {


  Serial.println ("LongKey");
  int * time;
  for (int i = 2000; i < 10000; i += 10 ) {
    lcd.setCursor(11, 0);
    time = percentToClock (i);
    lcd.write (prettyPrintClock(time[0], time[1]));
    Serial.println (prettyPrintClock(time[0], time[1]));
    updateLEDs(i);
    delay (75);
  }
  lcd.clear();

}

int getPotiValue () {
  int in = analogRead(PIN_A_IN_POTI);
  in = in / 30;
  in = in * 30; // round down in 3 percent steps
  return 990.0 / 1023.0 * in;
}


int loopCount = 0;
void loop() {

  sensorValue = getPotiValue();


  if (! checkForSchalter ())  {

    if (checkForTaster ()) {
      timerSinceDisplayOff = millis() + TIMOUT_DISPLAY;
    }

    updateDisplay();

    byte buttonState = getButtonState();
    if (buttonState == 2) {
      shortPush();
    } else if (buttonState == 3) {
      longPush();
    }
  } else {
    maintenanceMode();
    delay (1000);
  }


  if (loopCount ++ > 10) {
    updateLEDs(timeToPercentage ());
    updateDisplay();
    loopCount = 0;
  }

  delay (100);
}

// Developed with two screaming kids on board, no quality promised...
void updateLEDs(int time) {
  int inBetween = 0;
  if (time > hourP[0] && time <= hourP[7] ) { // 0-8
    setBrightness(PIN_PWM_BLUE, calcValue (6, 7, 1 , 1, time));
    setBrightness(PIN_PWM_WWHITE, calcValue (6, 7, 0, 0, time));
    setBrightness(PIN_PWM_CWHITE, calcValue (6, 7, 0, 0, time));
  } else if (time > hourP[7] && time <= hourP[8] ) { // 8-9
    setBrightness(PIN_PWM_BLUE, calcValue (7, 8, 1, 50, time));
    setBrightness(PIN_PWM_WWHITE, calcValue (7, 8, 0, 50, time));
    setBrightness(PIN_PWM_CWHITE, calcValue (7, 8, 0, 50, time));
  } else  if (time > hourP[8] && time <= hourP[9] ) { // 8-9
    setBrightness(PIN_PWM_BLUE, calcValue (8, 9, 50, 99, time));
    setBrightness(PIN_PWM_WWHITE, calcValue (8, 9, 50, 99, time));
    setBrightness(PIN_PWM_CWHITE, calcValue (8, 9, 50, 99, time));
  } else if (time > hourP[9] && time <= hourP[20] ) { // 9-20
    setBrightness(PIN_PWM_BLUE, calcValue (9, 10, 99, 99, time));
    setBrightness(PIN_PWM_WWHITE, calcValue (9, 10, 99, 99, time));
    setBrightness(PIN_PWM_CWHITE, calcValue (9, 10, 99, 99, time));  
  } else if (time > hourP[20] && time <= hourP[21] ) { // 20-21
    setBrightness(PIN_PWM_BLUE, calcValue (20, 21, 99, 50, time));
    setBrightness(PIN_PWM_WWHITE, calcValue (20, 21, 99, 50, time));
    setBrightness(PIN_PWM_CWHITE, calcValue (20, 21, 99, 50, time));
  } else if (time > hourP[21] && time <= hourP[22]) { // 21-22
    setBrightness(PIN_PWM_BLUE, calcValue (21, 22, 50, 40, time));
    setBrightness(PIN_PWM_WWHITE, calcValue (21, 22, 50, 0, time));
    setBrightness(PIN_PWM_CWHITE, calcValue (21, 22, 50, 0, time));
  } else if (time > hourP[22] && time <= hourP[23]) { // 22-23
    setBrightness(PIN_PWM_BLUE, calcValue (22, 23, 40, 1, time));
    setBrightness(PIN_PWM_WWHITE, calcValue (22, 23, 0, 0, time));
    setBrightness(PIN_PWM_CWHITE, calcValue (22, 23, 0, 0, time));
  } else if (time > hourP[23]  ) { // 23-24
    setBrightness(PIN_PWM_BLUE, calcValue (23, 24, 1, 1, time));
    setBrightness(PIN_PWM_WWHITE, calcValue (23, 24, 0, 0, time));
    setBrightness(PIN_PWM_CWHITE, calcValue (23, 24, 0, 0, time));

  } else {
    setBrightness(PIN_PWM_BLUE, 0);
    setBrightness(PIN_PWM_WWHITE, 0);
    setBrightness(PIN_PWM_CWHITE, 0);
  }

}


byte calcValue (int timeStart, int timeEnd, int lightStart, int lightEnd, int time) {

  int timeStartP = hourP[timeStart];
  int timeEndP =  hourP[timeEnd];
  int distanceP = timeEndP - timeStartP;

  int currentP = time - timeStartP;
  int distanceLED = lightEnd - lightStart;

  float currentLED = distanceLED / (float) distanceP * (float) currentP;


  int led = currentLED + lightStart;

  int dimmLed =  getPotiValue() / 990.0 * led;

  Serial.print("Calculated led intensity (%):");
  Serial.print (led);
  Serial.print(" and dimmed: ");
  Serial.println(dimmLed);

  return 256.0 / 100.0 * dimmLed;
}

void maintenanceMode() {
  setBrightness(PIN_PWM_BLUE, 000);
  //setBrightness(PIN_PWM_RED, value);
  setBrightness(PIN_PWM_WWHITE, 250);
  setBrightness(PIN_PWM_CWHITE, 250);
}

// brightness = 0-255
void setBrightness (short output, int brightness) {

  analogWrite (output, brightness);

  if (output == PIN_PWM_BLUE) {
    lcd.setCursor(0, 1);
    lcd.print ( "b");
  } else if (output == PIN_PWM_WWHITE) {
    lcd.setCursor(6, 1);
    lcd.print ( "w");
  } else if (output == PIN_PWM_CWHITE) {
    lcd.setCursor(12, 1);
    lcd.print ( "c");
  }
  static char buffer[3];
  int percent = 99.0 / 255.0 * brightness;
  sprintf(buffer, "%02d", percent );
  lcd.write ( buffer);
  lcd.print ( "%");
}
long lastButtonFree = millis();
byte buttonWasPushed = false;

/*
* Measure how long the button was pushed and return value depending on button state.
*
* return 0 if button is not pushed,
* return 1 if button is currently pushed
* return 2 if button was released with a former push time < 0,5 sec.
* return 3 if button was longer pushed than 0,5 sec and released.
*/
byte getButtonState () {
  boolean buttonPushed = checkForTaster();

  if (buttonWasPushed && !buttonPushed) {
    buttonWasPushed = false;
    if (millis () > lastButtonFree + 500) {
      return 3;
    } else {
      return 2;
    }
  }
  if (buttonPushed) {
    buttonWasPushed = true;
    return 1;
  } else {
    lastButtonFree = millis();
    return 0;
  }

}

void printTime() {
  char buffer[3];
  const char* AMPM = 0;
  readTime();
  Serial.print(days[weekday - 1]);
  Serial.print(" ");
  Serial.print(months[month - 1]);
  Serial.print(" ");
  Serial.print(monthday);
  Serial.print(", 20");
  Serial.print(year);
  Serial.print(" ");
  if (hour > 12) {
    hour -= 12;
    AMPM = " PM";
  }
  else AMPM = " AM";
  Serial.print(hour);
  Serial.print(":");
  sprintf(buffer, "%02d", minute);
  Serial.print(buffer);

  Serial.println(AMPM);

  Serial.println(timeToPercentage());
}



boolean checkForSchalter () {
  return !(analogRead(PIN_A_IN_SCHALTER) > 512);
}

boolean checkForTaster () {
  return (analogRead(PIN_A_IN_TASTER) > 512);
}

boolean wasInMaintenanceMode = false;
void updateDisplay() {

  //maintenance mode
  if (checkForSchalter ()) {
    if (!wasInMaintenanceMode) {
      wasInMaintenanceMode = true;
      lcd.clear();
    }
    analogWrite (PIN_PWM_LCD, 1000);
    lcd.display();
    lcd.setCursor(0, 0);
    lcd.write ("Wartungsmodus");
    lcd.setCursor(1, 1);
    static char buffer[5];
    sprintf(buffer, "%0d,%d", sensorValue / 10, sensorValue % 10);
    lcd.write ( buffer);
    lcd.setCursor(5, 1);
    lcd.write ("%");
    return;
  }

  // is display on?
  if (millis() < timerSinceDisplayOff) {
    analogWrite (PIN_PWM_LCD, 1000);
    lcd.display();

    // delete maintenance message, if just changed
    if (wasInMaintenanceMode) {
      wasInMaintenanceMode = false;
      lcd.clear();
    }

    // write clock to display
    Wire.beginTransmission(DS1307);
    Wire.write(byte(0));
    Wire.endTransmission();
    Wire.requestFrom(DS1307, 7);
    second = bcdToDec(Wire.read());
    minute = bcdToDec(Wire.read());
    hour = bcdToDec(Wire.read());

    lcd.setCursor(11, 0);
    lcd.write (prettyPrintClock(hour, minute));

    //write poti to display
    lcd.setCursor(0, 0);

    static char buffer[3];
    sprintf(buffer, "%02d", sensorValue / 10);
    lcd.print ("Dim:");
    lcd.print (buffer);
    lcd.print ("%");

  } else {
    lcd.noDisplay();
    analogWrite (PIN_PWM_LCD, 0);
  }

}

char* prettyPrintClock (int hour, int minute) {
  static char buffer[5];
  sprintf(buffer, "%02d:%02d", hour, minute);
  return buffer;
}


int timeToPercentage () {
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.endTransmission();
  Wire.requestFrom(DS1307, 7);
  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read());

  return clockToPercent (hour, minute);
}

static int currentTime [2];
int * percentToClock (int percentage) {
  int minutes = 1440.0 / 10000.0 * percentage;
  int hours = minutes / 60;
  Serial.print ("percent: ");
  Serial.print (percentage);
  Serial.print (" is ");
  Serial.print (hours);
  Serial.print (":");
  Serial.println ( minutes % 60);
  minutes = minutes % 60;
  currentTime[0] = hours;
  currentTime[1] = minutes;
  return currentTime;
}

// 0-10000 in % * 100
int clockToPercent (byte hour,  byte minute) {
  int minutesSinceMidnight = hour * 60 + minute;
  return (10000.0 / 1440.0 * (minutesSinceMidnight * 1.0));
}

void readTime() {
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.endTransmission();
  Wire.requestFrom(DS1307, 7);
  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read());
  weekday = bcdToDec(Wire.read());
  monthday = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
}
byte decToBcd(byte val) {
  return ((val / 10 * 16) + (val % 10));
}
byte bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));
}
