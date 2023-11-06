#include "RTClib.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#define sensorPower 7 
#define sensorPin A0
RTC_DS3231 rtc;

LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C address 0x27, 20 column and 4 rows

const int middleButton = 3;  // the number of the pushbutton pin
const int bottomButton = 4;  // the number of the pushbutton pin
const int rightButton = 5;  // the number of the pushbutton pin
const int leftButton = 6;  // the number of the pushbutton pin
const int topButton = 2;  // the number of the pushbutton pin
const int ledPin = 13;

// variables will change:
int middleState = 0;  // variable for reading the pushbutton status
int bottomState = 0;  // variable for reading the pushbutton status
int rightState = 0;  // variable for reading the pushbutton status
int leftState = 0;  // variable for reading the pushbutton status
int topState = 0; 

int waterVal = 0; 
int delayTime;
int dayCounter; // Iterates through days array
int prevDayCounter; // Checker for counter adjustments and limits lcd prints
int appendDay = 0; // Determines location of array to append additonal day selections
int timeCounter; // Iterates through maxTimes for timeInitialization
int prevTimeCounter; // Checker for counter adjustments and limits lcd prints
int amPmCounter; // Iterates through meridiem array
int prevAmPmCounter; // Checker for counter adjustments and limits lcd prints
int setHour; // Holds hour result of timeInitialize, between 1 and 12
int setMin; // Holds Minute result of timeInitialize, between 0 and 59
int setSec = 30;

bool isDaySet;
bool isArrayReady;
bool isSetReady;
bool isTimeSet;
bool isHourSet;
bool isMinSet;
bool isAmPmSet;
bool isWet = false;
bool error;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 450; // Adjust debounce delay as needed (in milliseconds)

String hourString; // Converted setHour to a String
String minString; // Converted setMin to a String
String timeString; // Concatenation of hourString:minString
String amPm; // Holds result of amPmInitialize, either AM or PM
String setDay; // Holds result of dayInitialize, one day of the week

int dayIndexArray[7] = {8, 8, 8, 8, 8, 8, 8};
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"}; // Array holding days of the Week with a range between 0 and 6 
String abrevWeek[7] = {"Su", "M", "Tu", "W", "Th", "F", "Sa"};
String sortedWeek[7];
String meridiem[2] = {"AM", "PM"}; // Array holding AM and PM with a range between 0 and 1

// Custom Arrow Chars
byte rightArrow[] = {
  B00000,
  B00000,
  B00100,
  B00110,
  B11111,
  B00110,
  B00100,
  B00000
}; 

byte leftArrow[] = {
  B00000,
  B00000,
  B00100,
  B01100,
  B11111,
  B01100,
  B00100,
  B00000
};

byte downArrow[] = {
  B00000,
  B00000,
  B00000,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100
};

byte upArrow[] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00000,
  B00000,
  B00000
};



void readButtons()
{
  middleState = digitalRead(middleButton);
  bottomState = digitalRead(bottomButton);
  rightState = digitalRead(rightButton);
  leftState = digitalRead(leftButton);
  topState = digitalRead(topButton);
}

int readSensor() 
{
	digitalWrite(sensorPower, HIGH);	// Turn the sensor ON
	delay(10);							// wait 10 milliseconds
	waterVal = analogRead(sensorPin);		// Read the analog value form sensor
	digitalWrite(sensorPower, LOW);		// Turn the sensor OFF
	return waterVal;							// send current reading
}

bool anyKey() 
{
  return (topState == HIGH || bottomState || leftState == HIGH || rightState == HIGH || middleState == HIGH);
}

bool isUpButton()
{
  return (topState == HIGH);
}

bool isDownButton()
{
  return (bottomState == HIGH);
}

bool isLeftButton()
{
  return (leftState == HIGH);
}

bool isRightButton()
{
  return (rightState == HIGH);
}

bool isMiddleButton()
{
  return (middleState == HIGH);
}

void pressAny()
{
  lcd.print("Press Any Key");
  while (true) {
  readButtons();
  if (anyKey())
  {
    lcd.clear();
    break;
  }
  }
}


bool yes_no(String question) {
  lcd.clear();
  lcd.print(question);
  lcd.setCursor(0, 2);
  lcd.print("Right = (y)");
  lcd.setCursor(0, 3);
  lcd.print("Left = (n)");

  while (true) {
    readButtons();
    delay(200);
    if (isRightButton()) {
      return true;  // User pressed 'y'
    } else if (isLeftButton()) {
      return false; // User pressed 'n'
    }
    // You can perform other tasks here if needed.
  }
}


bool containsDay( String array[], int size, String value) {
  for (int i = 0; i < size; i++) {
    if (array[i] == value) {
      return true;
    }
  }
  return false;
}

bool containsDay(const int array[], int size, int value) {
  for (int i = 0; i < size; i++) {
    if (array[i] == value) {
      return true;
    }
  }
  return false;
}

void sortArray(String day) {


}

String listAllDays(int indexArray[], int size) {
  String result = "";
  for (int i = 0; i < size; i++) {
    if (indexArray[i] != 8) {
    result += abrevWeek[indexArray[i]] + " "; }
    else {
    continue;
    }
  }
  return result;
}

bool dupDays(int counter) {
  if (containsDay(dayIndexArray, 7, counter))
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Duplciate Day");
    lcd.setCursor(0, 1);
    lcd.print("Please Try Again");
    dayCounter = (dayCounter + 1) % 7;
    lcd.setCursor(0, 3);
    delay(2000);
    pressAny();
    dayInitialize();


    
  }
}

void clearArray(int array[])
{
for (int j = 0; j > 7; j++) {
  array[j] = 8;
}
}



void dayInitialize() { //set day to water:
  unsigned long debounceTime = 50; // Debounce time in milliseconds
  unsigned long lastButtonPress = 0; // Variable to store the last button press time

  
  while (!isDaySet) {
    delay(200);
    unsigned long currentMillis = millis();
    
    // Read buttons and debounce them
    if (currentMillis - lastDebounceTime >= debounceDelay) {
      readButtons();
      String displayDay = daysOfTheWeek[dayCounter];
      
      if (prevDayCounter != dayCounter) {
        lcd.clear();
        lcd.setCursor(0, 1);
        lcd.write(2);
        lcd.setCursor(6, 1);
        lcd.print(displayDay);
        lcd.setCursor(19, 1);
        lcd.write(3);
        lcd.setCursor(0, 2);
        lcd.print(listAllDays(dayIndexArray, 6));
        prevDayCounter = dayCounter;
        isSetReady = true;
        lcd.setCursor(0, 3);
        if (isArrayReady) {
        lcd.print("Press Down to Finish");
        }


      }

      if (isRightButton()) {
        // Right button pressed
        dayCounter = (dayCounter + 1) % 7;
      } else if (isLeftButton()) {
        // Left button pressed
        dayCounter = (dayCounter + 6) % 7;
      } else if (isMiddleButton() && isSetReady) {
        (dupDays(dayCounter));
        lcd.setCursor(0,0);
        lcd.print("Add Additonal Days?");
        lcd.setCursor(0,3);
        lcd.print("Press Down to Finish");
        dayIndexArray[appendDay] = dayCounter;
        appendDay ++;
        setDay = displayDay;
        isArrayReady = true;
        delay(1000);
      }
      else if (isDownButton() && isArrayReady) {
      if (yes_no("Is " + listAllDays(dayIndexArray, 6) + "Correct?")) { 
      isDaySet= true;
      }
      else
      {
      isSetReady = false;
      isArrayReady = false;
      dayInitialize();
      
      }
      }
      
      lastButtonPress = currentMillis; // Update the last button press time
    }
  }

  isSetReady = false;
}


void timeInitialize() {
  unsigned long debounceTime = 50; // Debounce time in milliseconds
  unsigned long lastButtonPress = 0; // Variable to store the last button press time
  
  while (!isTimeSet) {
    delay(200);
    readButtons();
    unsigned long currentMillis = millis();

    // Check if enough time has passed since the last button press
    if (currentMillis - lastButtonPress >= debounceTime) {
      if (prevTimeCounter != timeCounter) {
        prevTimeCounter = timeCounter;
        lcd.clear();
        isSetReady = true;

        if (!isHourSet) {
          lcd.print("Input Hour");
          lcd.setCursor(7, 1);
        } else {
          lcd.print("Input Minute(s)");
          lcd.setCursor(7, 1);
          lcd.print(hourString);
          lcd.setCursor(7 + hourString.length(), 1);
        }
        lcd.print(timeCounter);
      }

      int maxTime = (isHourSet) ? 59 : 12; // Max value that setHour/setMin can be input as, adjusts based on isHourSet bool

      if (isUpButton()) {
        // Up button pressed
        timeCounter = (timeCounter % maxTime) + 1;
      } else if (isDownButton()) {
        // Down button pressed
        timeCounter = ((timeCounter - 2 + maxTime) % maxTime) + 1;
      } else if (isMiddleButton() && isSetReady) {
        // Middle button pressed
        if (!isHourSet) {
          setHour = timeCounter;
          hourString = (setHour < 10) ? "0" + String(setHour) + ":" : String(setHour) + ":";
          lcd.setCursor(0, 0);
          lcd.print("Input Minute(s)");
          lcd.setCursor(7, 1);
          lcd.print(hourString);
          isHourSet = true;
          timeCounter = 0; // Return timeCounter back to 0 to prep for Minute selection
        } else {
          lcd.clear();
          lcd.setCursor(7, 1);
          lcd.print(hourString);
          setMin = timeCounter;
          isMinSet = true;
        }
        if (isMinSet) {
          String minString = (setMin < 10) ? "0" + String(setMin) : String(setMin);
          timeString = hourString + minString;
          if (yes_no("Is " + timeString + " Correct?")) {
            isTimeSet = true;
          } else {
            timeInitialize();
          }
        }
        lastButtonPress = currentMillis; // Update the last button press time
      }
    }
  }
  isSetReady = false;
  lcd.clear();
}


String amPmInitialize() { //set AmPm to water: 
// Intergrate this into time selection method, no need to have seperate methods for this
  unsigned long debounceTime = 50; // Debounce time in milliseconds
  unsigned long lastButtonPress = 0; // Variable to store the last button press time
  
  while (!isAmPmSet) {
    delay(200);
    unsigned long currentMillis = millis();
    
    // Read buttons and debounce them
    if (currentMillis - lastDebounceTime >= debounceDelay) {
      readButtons();
      String displayAmPm = meridiem[amPmCounter];
      
      if (prevAmPmCounter != amPmCounter) {
        lcd.clear();
        lcd.setCursor(7, 1);
        lcd.print(displayAmPm);
        prevAmPmCounter = amPmCounter;
        isSetReady = true;
      }

      if (isRightButton()) {
        // Right button pressed
        amPmCounter = (amPmCounter + 1) % 2;
      } else if (isLeftButton()) {
        // Left button pressed
        amPmCounter = (amPmCounter + 1) % 2;
      } else if (isMiddleButton() && isSetReady) {
        if (yes_no("Is " + displayAmPm + " Correct?")) {
        isAmPmSet = true;
        amPm = displayAmPm;
      }
      else
      {
        amPmInitialize();
      }
      }
      
      lastButtonPress = currentMillis; // Update the last button press time
    }
  }
  if (amPm == "AM")
  {
    setHour += 12;
  }
  lcd.clear();
  lcd.print(setHour);
  delay(700);
}





void setup () {
  lcd.begin(); // Start LCD
  lcd.backlight();  //open the backlight 
  lcd.createChar(0, upArrow);
  lcd.createChar(1, downArrow);
  lcd.createChar(2, leftArrow);
  lcd.createChar(3, rightArrow);
  pinMode(ledPin, OUTPUT);
  pinMode(sensorPower, OUTPUT);

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  String leftRight = " Use Left/Right ";

  lcd.setCursor(0, 0);
  lcd.print("Select Day to Water");
  lcd.setCursor(0, 2);
  pressAny();
  lcd.write(2);
  lcd.setCursor(1, 0);
  lcd.print(leftRight);
  lcd.setCursor(leftRight.length() + 1, 0);
  lcd.write(3);
  
  
  lcd.setCursor(1, 2);
  lcd.print("Select with Middle");
  delay(750);
  dayInitialize(); //set day to water:
  delay(750);


  lcd.print("Select Time to Water"); 
  lcd.setCursor(0, 2);
  pressAny();
  lcd.print("Use Up/Down Keys");
  lcd.setCursor(0, 2);
  lcd.print("Select with Middle");
  delay(750);
  timeInitialize(); //set time to water:
  delay(750);
  

  lcd.print("Select AM / PM");
  lcd.setCursor(0, 2);
  pressAny();
  lcd.print("Use Left/Right Keys");
  lcd.setCursor(0, 2);
  lcd.print("Select with Middle");
  delay(750);
  amPm = amPmInitialize(); //set AmPm to water:
  delay(750);

  // lcd.setCursor(4,2);
  // lcd.print(setDay + " " + timeString + amPm); // replace with dayIndexArray printing function
  // delay(1000);
}

void loop () {

    int level = readSensor(); // Get water level value readings:

    DateTime now = rtc.now(); // Get Current Time:
    String currentDay = daysOfTheWeek[now.dayOfTheWeek()]; // Get current day for comparison:

    // Serial.print(now.year(), DEC);
    // Serial.print('/');
    // Serial.print(now.month(), DEC);
    // Serial.print('/');
    // Serial.print(now.day(), DEC);
    // Serial.print(" (");
    // Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    // Serial.print(") ");
    // Serial.print(now.hour(), DEC);
    // Serial.print(':');
    // Serial.print(now.minute(), DEC);
    // Serial.print(':');
    // Serial.print(now.second(), DEC);
    // Serial.println();

    // Serial.print(" since midnight 1/1/1970 = ");
    // Serial.print(now.unixtime());
    // Serial.print("s = ");
    // Serial.print(now.unixtime() / 86400L);
    // Serial.println("d");

    // calculate a date which is 7 days, 12 hours, 30 minutes, 6 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));

    // Serial.print(" now + 7d + 12h + 30m + 6s: ");
    // Serial.print(future.year(), DEC);
    // Serial.print('/');
    // Serial.print(future.month(), DEC);
    // Serial.print('/');
    // Serial.print(future.day(), DEC);
    // Serial.print(' ');
    // Serial.print(future.hour(), DEC);
    // Serial.print(':');
    // Serial.print(future.minute(), DEC);
    // Serial.print(':');
    // Serial.print(future.second(), DEC);
    // Serial.println();

    // Serial.print("Temperature: ");
    // Serial.print(rtc.getTemperature());
    // Serial.println(" C");

    // Serial.println();




if (level >= 100) {
  lcd.clear();
  digitalWrite(ledPin, LOW);
  lcd.setCursor(0,0);
  lcd.print("~~~~~~~~~~~~~~");
  lcd.setCursor(0,1);
  lcd.print("Water level: ");
  lcd.print(level);
  // Serial.print("Water level: ");
  // Serial.println(level);
  lcd.setCursor(0,2);
  lcd.print("~~~~~~~~~~~~~~");
  isWet = true;
  
}

//(containsDay(dayIndexArray, sizeof(dayIndexArray), currentDay)
//currentDay == setDay &&
else if ( now.hour() == setHour && now.minute() == setMin && now.second() <= setSec) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("**************");
  lcd.setCursor(0, 1);
  lcd.print("Watering in Progress...");
  lcd.setCursor(0, 2);
  lcd.print("**************");
  digitalWrite(ledPin, HIGH);
  isWet = false;
} else {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("~~~~~~~~~~~~~~");
  lcd.setCursor(0, 1);
  lcd.print("Water level: ");
  lcd.print(level);
  lcd.setCursor(0, 2);
  lcd.print("~~~~~~~~~~~~~~");
  digitalWrite(ledPin, LOW);
  isWet = false;
}


    if (isWet){ 
      delayTime = 1500;
    }
    else {
      delayTime = 3000;
    }
    
    delay(delayTime);
}







