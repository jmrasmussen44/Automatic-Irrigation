#include "RTClib.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <stdio.h>
#include <string.h>
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
const int pumpPin = 8;
const int buzzPin = 9;

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
int appendDay = 0; // Determines location of array to append day selections
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
bool arrows;
bool restart;
bool isToday;
bool isDayLightSavings;
bool isWet = false;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 450; // Adjust debounce delay as needed (in milliseconds)

String hourString; // Converted setHour to a String
String minString; // Converted setMin to a String
String timeString; // Concatenation of hourString:minString
String amPm; // Holds result of amPmInitialize, either AM or PM


int setTime[2];
bool selectedDays[7] = {false, false, false, false, false, false, false}; // Array that holds user's selected day indexes 
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



void readButtons() // Read current value for each button
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

void hapticFeedback()
{
  digitalWrite(buzzPin, HIGH);
  delay(10);
  digitalWrite(buzzPin, LOW);
}

bool isUpButton() // check if top button is pressed
{
  hapticFeedback();
  return (topState == HIGH);
}

bool isDownButton() // check if bottom button is pressed
{
  hapticFeedback();
  return (bottomState == HIGH);
}

bool isLeftButton() // check if left button is pressed
{
  hapticFeedback();
  return (leftState == HIGH);
}

bool isRightButton() // check if right button is pressed
{
  hapticFeedback();
  return (rightState == HIGH);
}

bool isMiddleButton() // check if middle button is pressed
{
  hapticFeedback();
  return (middleState == HIGH);
}

bool anyKey() // check if any button is pressed
{
  return (topState == HIGH || bottomState || leftState == HIGH || rightState == HIGH || middleState == HIGH);
}

void pressAny() // if anykey is pressed, continue
{
  lcd.print("Press Any Key");
  while (true) {
  readButtons();
  if (anyKey())
  {
    hapticFeedback();
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


String listAllArray(bool array[], int size) {
  String result = "";
  for (int i = 0; i < size; i++) {
    if (array[i]) {
    result += abrevWeek[i] + " "; 
    isArrayReady = true;
    }
    else {
    continue;
    }
  }
  return result;
}

int clearArray(int array[])
{
for (int j = 0; j > appendDay; j++) {
  array[j] = 8;
}
appendDay = 0;
return appendDay;
}

void copyArray(int* array, int size, int* copy) {
  for (int i = 0; i < size; i++) {
    copy[i] = daysOfTheWeek[array[i]];
  }
}

void printIntArray(int* array, int size) { // REMOVE
  for (int i = 0; i < size; i++) {
    lcd.print(array[i]);
    lcd.print(" ");
    }
}

bool checkDay(String day) {
    for (int i = 0; i < 7; i++) {
        if (strcmp(daysOfTheWeek[i], day.c_str()) == 0) {
            return selectedDays[i];
    }
  }
}

bool checkWet(int waterLevel) {
  isWet = (waterLevel >= 100);
  return isWet;
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
        lcd.print(listAllArray(selectedDays, 7));
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
        lcd.setCursor(0,0);
        lcd.print("Add Additonal Days?");
        lcd.setCursor(0,3);
        lcd.print("Press Down to Finish");
        selectedDays[dayCounter] = !selectedDays[dayCounter];
        isArrayReady = false;
        lcd.setCursor(0, 2);
        lcd.print(listAllArray(selectedDays, 7));
        delay(1000);
      }
      else if (isDownButton() && isArrayReady) {
        if (yes_no("Is " + listAllArray(selectedDays, 7) + "Correct?")) { 
          isDaySet = true;
        }
        else
        {
          selectedDays[0] = false;
          selectedDays[1] = false;
          selectedDays[2] = false;
          selectedDays[3] = false;
          selectedDays[4] = false;
          selectedDays[5] = false;
          selectedDays[6] = false;
          isSetReady = false;
          isArrayReady = false;
          appendDay = 0;
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
  timeCounter = 0;

  // Loop until the time is set
  while (!isTimeSet) {
    // Reset flags and adjust timeCounter if restart is requested
    if (restart) {
      isSetReady = false;
      isHourSet = false;
      isMinSet = false;
      timeCounter = (timeCounter % 12) + 1;
      restart = false;
    }

    // Wait for a short duration
    delay(200);

    // Read button inputs
    readButtons();
    unsigned long currentMillis = millis();

    // Display arrows to indicate time adjustment
    if (arrows) {
      lcd.setCursor(19, 0);
      lcd.write(0);
      lcd.setCursor(19, 3);
      lcd.write(1);
    }

    // Check if enough time has passed since the last button press
    if (currentMillis - lastButtonPress >= debounceTime) {
      // Check if timeCounter has changed
      if (prevTimeCounter != timeCounter) {
        prevTimeCounter = timeCounter;
        lcd.clear();
        arrows = true;
        isSetReady = true;

        // Display appropriate message based on whether hour or minute is being set
        if (!isHourSet) {
          lcd.print("Input Hour");
          lcd.setCursor(7, 1);
          lcd.print(timeCounter);
        } else {
          lcd.print("Input Minute(s)");
          lcd.setCursor(7, 1);
          lcd.print(hourString);
          lcd.setCursor(7 + hourString.length(), 1);
          lcd.print((timeCounter < 10) ? "0" + String(timeCounter) : String(timeCounter));
        }
      }

      int maxTime = (isHourSet) ? 60 : 13; // Max value for setHour/setMin, adjusts based on isHourSet bool
      int incrementValue = (isHourSet) ? 5 : 1; // Max value for setHour/setMin, adjusts based on isHourSet bool


      // Check button states and update timeCounter accordingly
      if (isUpButton()) {
        // Up button pressed
        timeCounter = (timeCounter + incrementValue) % maxTime;
        timeCounter = (!isHourSet && timeCounter == 0) ? 1 : timeCounter;
        lcd.setCursor(19, 0);
        lcd.write(0);
      } else if (isDownButton()) {
        // Down button pressed
        timeCounter = ((timeCounter + (maxTime - incrementValue)) % maxTime);
        timeCounter = (!isHourSet && timeCounter == 0) ? 12 : timeCounter;
        lcd.setCursor(19, 3);
        lcd.write(1);
      } else if (isMiddleButton() && isSetReady) {
        // Middle button pressed
        if (!isHourSet) {
          // Set hour and prepare for minute selection
          setHour = timeCounter;
          hourString = (setHour < 10) ? "0" + String(setHour) + ":" : String(setHour) + ":";
          lcd.setCursor(0, 0);
          lcd.print("Input Minute(s)");
          lcd.setCursor(7, 1);
          lcd.print(hourString);
          isHourSet = true;
          timeCounter = 0; // Reset timeCounter for Minute selection
          delay(500);
        } else {
          // Set minute, construct timeString, and confirm with the user
          lcd.clear();
          lcd.setCursor(7, 1);
          lcd.print(hourString);
          setMin = timeCounter;
          isMinSet = true;
          String minString = (setMin < 10) ? "0" + String(setMin) : String(setMin);
          timeString = hourString + minString;

          // Confirm with the user
          if (yes_no("Is " + timeString + " Correct?")) {
            isTimeSet = true;
            setTime[0] = setHour;
            setTime[1] = setMin;

          } else {
            restart = true;
            timeInitialize(); // Restart the time setting process
          }
        }
        lastButtonPress = currentMillis; // Update the last button press time
      }
    }
  }
  isSetReady = false;
}



int amPmInitialize() {
  unsigned long debounceDelay = 50; // Debounce time in milliseconds
  unsigned long lastDebounceTime = 0; // Variable to store the last button press time
  bool isSetReady = false; // Variable to track button readiness
  bool isAmPmSet = false; // Variable to track if AM/PM is set
  int adjustedSetHour;
  
  while (!isAmPmSet) {
    delay(200);
    unsigned long currentMillis = millis();
    
    // Read buttons and debounce them
    if (currentMillis - lastDebounceTime >= debounceDelay) {
      readButtons();
      String displayAmPm = meridiem[amPmCounter];
      
      if (prevAmPmCounter != amPmCounter) {
        lcd.clear();
        lcd.write(2);
        lcd.setCursor(8, 1);
        lcd.print(displayAmPm);
        lcd.setCursor(19, 1);
        lcd.write(3);
        prevAmPmCounter = amPmCounter;
        isSetReady = true;
      }

      if (isRightButton() || isLeftButton()) {
        // Right or left button pressed
        amPmCounter = (amPmCounter + 1) % 2;
      } else if (isMiddleButton() && isSetReady) {
        if (yes_no("Is " + displayAmPm + " Correct?")) {
          isAmPmSet = true;
          amPm = displayAmPm;
          if (amPm == "AM") {
            return setTime[0];
          }
          else
          {
            adjustedSetHour = setTime[0] + 12;
            return adjustedSetHour;
            
          }
        } else {
          amPmInitialize();
        }
      }
      
      lastDebounceTime = currentMillis; // Update the last button press time
    }
  }
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
  pinMode(pumpPin, OUTPUT);
  pinMode(buzzPin, OUTPUT);

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    lcd.print("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    lcd.print("RTC lost power, let's set the time!");
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

  // Select Day
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
  delay(500);
  lcd.clear();

  // Select Time
  String up_down = "Use Up/Down Keys";
  lcd.print("Select Time to Water"); 
  lcd.setCursor(0, 2);
  pressAny();
  lcd.write(0);
  lcd.setCursor(2, 0);
  lcd.print(up_down);
  lcd.setCursor(up_down.length() + 3, 0);
  lcd.write(1);
  lcd.setCursor(0, 2);
  lcd.print("Select with Middle");
  delay(750);
  timeInitialize(); //set time to water:
  delay(500);
  lcd.clear();
  
  // Select Am/Pm
  lcd.print("Select AM / PM");
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
  setTime[0] = amPmInitialize(); //set AmPm to water:
  lcd.clear();
  lcd.setCursor(5, 1);
  lcd.print("Loading...");
}

void loop () {

    digitalWrite(pumpPin, LOW);

    while (true) {
      DateTime now = rtc.now(); // Get Current Time:
      int level = readSensor();
      int currentHour = (isDayLightSavings) ? now.hour() : (now.hour() - 1);
      currentHour = (currentHour == 0) ? 1 : currentHour;
      String currentDay = daysOfTheWeek[now.dayOfTheWeek()]; // Get current day for comparison:
      isToday = checkDay(currentDay);
      DateTime future (now + TimeSpan(7,12,30,6));
      delay(500);
      int prevLevel;

      // if (prevLevel >= level + 5 || prevLevel <= level - 5) {
    if (now.second() % 5 == 0) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("~~~~~~~~~~~~~~");
      lcd.setCursor(0,1);
      lcd.print("Water level: ");
      lcd.print(level);
      lcd.setCursor(0,2);
      lcd.print("~~~~~~~~~~~~~~");
      digitalWrite(ledPin, LOW);
      digitalWrite(pumpPin, LOW);
      }

      if (isToday && currentHour == setTime[0] && now.minute() == setTime[1] && level <= 100) {
        break;
      }
    }

  while(!isWet) {
    int level = readSensor();
    isWet = checkWet(level);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("********************");
    lcd.setCursor(0, 1);
    lcd.print("Watering in Progress");
    lcd.setCursor(0, 2);
    lcd.print("********************");
    digitalWrite(ledPin, HIGH);
    digitalWrite(pumpPin, HIGH);
    delay(3000);
    }
  }








