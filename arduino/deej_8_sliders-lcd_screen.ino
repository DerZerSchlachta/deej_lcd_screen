#include <string.h>
#include <Arduino.h>
#include <LiquidCrystal.h>
#include <ezButton.h>
#include <LowPower.h>

using namespace std;



// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int NUM_SLIDERS = 8;        //Input the Total amount of sliders/ rotary potentiometers
const int analogInputs[NUM_SLIDERS] = {A0, A1, A2, A3, A4, A5, A6, A7};   //list of the pins, the pots are connected to
int potValues[NUM_SLIDERS] = {0, 0, 0, 0, 0, 0, 0, 0};                 //output value of the pots, this one is used only for the build-in lcd display
int analogSliderValues[NUM_SLIDERS];                //same as above, but used for communication with pc, may or may not be synchronized with "potValues"

String audiosource[NUM_SLIDERS] = {
"Master:     ", 
"Focus:      ", 
"Microphone: ", 
"Sys.-Sounds:",
"Discord:    ",
"Spotify:    ",
"Firefox:    ",
"unmapped:   " };                   // Array of the different Audiosources we want to control, used as text for the lcd to display

String vollevel;                    // adjustable string-to-be-displayed by the lcd, containing the value of the pot as a percentage from 0 to 100%, where 100% corresponds to 1020 of the analogue voltage reading  
String volumebar = "/";             // adjustable string-to-be-displayed by the lcd in the second row, visualising the vollevel as a "loading bar"
int volumebarcount = 0;             // controlls the amount of slashes that make up the volumebar

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  
  // initiate a basic, entirely cosmetical startup sequenz to demonstrate the loading bar functionality
  lcd.setCursor(0,0);
  lcd.print("starting up...");
  
  for (int i = 0; i <= 16; i++)   //for-loop that iterates through all 16 lcd-slots in the second row
  {
    lcd.setCursor(0,1);           //start in the second row, first slot
    volumebar=volumebar + "/";    // take the previous iterations volume bar and add another slash to it
    lcd.print(volumebar);         // print newly assembled volumebar
    delay(100);                   // wait a short amount before lengthening the bar ones more
  }
  
  
  lcd.clear();                    // clear all parts of the startup/ demo sequence so that the real fun can begin.
  

  for (int i = 0; i < NUM_SLIDERS; i++) {         // intiates the pins used for the potentiometers
    pinMode(analogInputs[i], INPUT);        
  }

  for (int i = 0; i < NUM_SLIDERS; i++)     // for-loop through all the  available pots
  {
    int newInput = analogRead(analogInputs[i]);   //read the analogue values, e.i. what is the starting position of each pot
    potValues[i] = newInput * -1 + 1023;                      // store that value inside our potvalue list, and inverts it, because I failed hooking gnd to the right pin, lol
  }

  Serial.begin(9600);                              // startup of the serial monitor

 
}

void updateSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
     analogSliderValues[i] = analogRead(analogInputs[i]);
  }
}

void sendSliderValues() {
  String builtString = String("");

  for (int i = 0; i < NUM_SLIDERS; i++) {
    builtString += String((int)analogSliderValues[i]);

    if (i < NUM_SLIDERS - 1) {
      builtString += String("|");
    }
  }
  
  Serial.println(builtString);
}

void printSliderValues() {
  for (int i = 0; i < NUM_SLIDERS; i++) {
    String printedString = String("Slider #") + String(i + 1) + String(": ") + String(analogSliderValues[i]) + String(" mV");
    //Serial.write(printedString.c_str());

    if (i < NUM_SLIDERS - 1) {
      //Serial.write(" | ");
    } else {
      //Serial.write("\n");
    }
  }
}
  





void ChangeVolume(int ChangedPot, int ChangedValue)       // a function to-be-called when there is a change in the position of a pot is detected which requires the volume display on the lcd to be changed
{
  int potVal = ChangedValue/10.2;       //converts the Pots voltage value into a more usable format (volate Values range from zero to 1012, we convert this to 0 to 100, e.i. the repective percentage)
  vollevel = String(potVal);            // converts the integer to a string to better edit and display it on the lcd
  
  if(potVal <10)                        // checks, wether the number is on digit
  {
    vollevel = "  " + vollevel + "%";    // adds two spaces in front, to allow for better display, e.i. the number stay centered and properly replaces previous numbers
  }
  else if(potVal >9 && potVal < 100)      // checks if the number is two digits...
  {
    vollevel = " " + vollevel + "%";      // ... in which case only one space has to be added
  }
  else                                    // for the single remaining option, that potVal is equal to 100, e.i. has three digit...
  {
    vollevel = vollevel + "%";            // no space has to be added, merely a percentage sign at the end
  }

  lcd.setCursor(0,0);               // set the cursor to the begining of line 1
  lcd.print(audiosource[ChangedPot]);   //print the audiosource corresponding to the ChangedPot value given by ChangeDetectr() function
  lcd.setCursor(12,0);                      //sets cursor to the end of line 1
  lcd.print(vollevel);                      // prints the assembled vollevel

  volumebarcount = potVal/6.25;             // dividing potVal by 6.25 will give us the amount of slahes corresponding to the percentage value
  volumebar = "";                           // resets the volumebar for further assembly
  for (int i =0; i <= volumebarcount; i++)  // iterates through the amount of slahes needed as of the calculation made above
  {
    volumebar = volumebar + "/";            // adds one slash for each completed iteration
  }
  int emptybar = 16 - volumebarcount;       // by subtracting the amount of slashes from the maximum amount of lcd-slots, we get the amount of empty slots needed to complete the volumebar string
  for (int i = 0; i <= emptybar; i++)       // iterates through the number of empty slots
  {
    volumebar = volumebar + " ";            // adds one empty slot (space) for each iteration to our volumebar string
  }

  lcd.setCursor(0,1);             // set the cursor to the begining of the second row
  lcd.print(volumebar);           // prints the fully assembled, 16 digits long volumebar string, overwriting any potentially still existing characters on the lcd
  //loop();                         // calls loop again, which will also ready the arduino for the next loop through the entire sequence
}



void loop() {
  
  //forloop to check whether any slider value has been changed
  for (int i = 0; i < NUM_SLIDERS; i++)   // loops through all available pins
  {
    int newInput = analogRead(analogInputs[i]) * -1 + 1023; // stores value of the current pin, and inverts it again, because I am not a good technician it appears

    if(newInput > potValues[i]+1 || newInput < potValues[i] -1)   // checks, if the current pins value is different than the prebiously saved value, with a +-1 area of tolerance to account for unavoidable volate fluctuations
    {
      potValues[i] = newInput;    //changes the old value to the new value
      ChangeVolume(i, newInput);   // calls the ChangeVolume() function with the number of the pin and its new value
    }
    else        
    {
      continue;                     //does nothing, when the values are the same, e.i. when the pot has not been moved/adjusted
    }
  }
  updateSliderValues(); 
  sendSliderValues();  // Actually send data (all the time)
  // printSliderValues(); // For debug
  delay(30);

  
}