#include <Keypad.h>
#include <SoftwareWire.h>

const int sda=A0, scl=A1; 
SoftwareWire Wire(sda, scl);

#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <DHT_Async.h>

#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 13;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

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

float t;

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
  readTemp();
  Serial.println(analogRead(A0));

  
}

 void inputKey(){
  char key = customKeypad.getKey();
  
  if (key){
    Serial.println(key);
    if(key == confirmKey){
      if(count == 9){
        Serial.println();
        phoneNumber = number + '\0';
        Serial.println(phoneNumber);
        validPhoneNumber = true;
        displayPhoneNumber(validPhoneNumber);
      }
      else{
        Serial.println("Invalid phone number");
        lcd.clear();
        lcd.print("Invalid phone number");
        delay(500);
          for(int i = 0; i <= 9; i++){
            number[i] = ' ';
        }
        validPhoneNumber = false;
        displayPhoneNumber(validPhoneNumber);
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
        validPhoneNumber = false;
        displayPhoneNumber(validPhoneNumber);
      }
    }
    else if(key == 'A'){
      displayPhoneNumber(validPhoneNumber);
    }
    else if(key == 'B'){
      lcd.clear();
      lcd.print("Temperature");
      lcd.setCursor(0, 1);
      lcd.print(String(t) + "°C");
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
 
 void displayPhoneNumber(boolean valid){
  if(valid){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("My phone nr:");
    lcd.setCursor(0,1);
    lcd.print("+40" + phoneNumber);
    Serial.println("+40" + phoneNumber);
  }
  else{
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
  
 }

static bool measure_environment(float *temperature, float *humidity) {
    static unsigned long measurement_timestamp = millis();

    /* Measure once every four seconds. */
    if (millis() - measurement_timestamp > 200ul) {
        if (dht_sensor.measure(temperature, humidity)) {
            measurement_timestamp = millis();
            return (true);
        }
    }

    return (false);
}

void readTemp(){
  float temperature;
    float humidity;

    /* Measure temperature and humidity.  If the functions returns
       true, then a measurement is available. */
    if (measure_environment(&temperature, &humidity)) {
        Serial.print("T = ");
        Serial.print(temperature, 1);
        t = temperature;
        Serial.print(" deg. C, H = ");
        Serial.print(humidity, 1);
        Serial.println("%");
    }
}


