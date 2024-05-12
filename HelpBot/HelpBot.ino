#include <Keypad.h>
#include <SoftwareWire.h>

const int sda=A0, scl=A1; 
SoftwareWire Wire(sda, scl);

#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

hd44780_I2Cexp lcd;
int row = 1;
int column = 3;

const byte ROWS = 4; 
const byte COLS = 4; 

int count;
char number[9];
String phoneNumber;
boolean validPhoneNumber = false;

char confirmKey = '*';
char resetKey = '#';

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

void setup(){
  Serial.begin(9600);
  int istatus = lcd.begin(16,2);
  if(istatus){
    lcd.fatalError(istatus);
  }
  lcd.print("Phone Number:");
  lcd.setCursor(0,1);
  lcd.print("+40");
  lcd.setCursor(column,row);
}
  
void loop(){
  inputKey();

}

 void inputKey(){
  char key = customKeypad.getKey();
  
  if (key){
    Serial.println(key);
    if(key == confirmKey){
      if(count == 9){
        Serial.println();
        phoneNumber = number + "/r";
        Serial.println(phoneNumber);
        displayPhoneNumber(phoneNumber);
        validPhoneNumber = true;
      }
      else{
        Serial.println("Invalid phone number");
        lcd.clear();
        delay(500);
          for(int i = 0; i <= 9; i++){
            number[i] = ' ';
        }
        displayNumberInput();
        count = 0;
      }
    }
    else if(key != 'A' && key != 'B' && key != 'C' && key !='D'){
      lcd.print(key);
      number[count] = key;
      count++;
      column++;
      if(count > 9){
        Serial.println("Invalid");
        lcd.clear();
        lcd.print("Invalid");
        delay(500);
        for(int i = 0; i <= 9; i++){
            number[i] = ' ';
        }
        count = 0;
        displayNumberInput();
      }
    }
    else if(key == 'A'){
      if(validPhoneNumber)
        displayPhoneNumber(phoneNumber);
      else
        displayNumberInput();
    }
    else if(key == 'B'){
      lcd.clear();
      lcd.print("Temperature");
    }
    else if(key == 'C'){
      lcd.clear();
      lcd.print("Gas");
    }
    else if(key == 'D'){
      lcd.clear();
      lcd.print("Smoke");
    }

  }
 }
 
 void displayNumberInput(){
  lcd.clear();
  lcd.print("Phone Number:");
  Serial.println("Phone Number");
  lcd.setCursor(0,1);
  Serial.println("+40");
  lcd.print("+40");
  column = 3;
  row = 1;
  lcd.setCursor(column,row);
 }

 void displayPhoneNumber(String nr){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("My phone nr:");
    lcd.setCursor(0,1);
    lcd.print("+40" + nr);
    Serial.println("+40" + nr);
 }
