#include <Keypad.h>
#include <DHT_Async.h>

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);
LiquidCrystal_I2C lcd1(0x26,16,2);

String ip = "12345";

#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 13;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

#define MQ2PIN A2
int MQValue;
int MQValueThreshold = 200; //replace with threshold value of MQ2
bool gas = false;

#define MicPin A3
int MicValue;
int MicValueThreshold = 300; //replace with threshold value of Microphone


int row = 1;
int column = 3;

const byte ROWS = 4; 
const byte COLS = 4; 

int count;
char number[9];
String phoneNumber;
bool validPhoneNumber = false;

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
int h;

#define PM1PIN 10
#define PM25PIN 11

unsigned long durationPM1;
unsigned long durationPM25;
unsigned long elapsedtime;
unsigned long endtime;
unsigned long sampletime_ms = 10;
unsigned long lowpulseoccupancyPM1 = 0;
unsigned long lowpulseoccupancyPM25 = 0;

float conPM1;
float conPM25;

float ratio;
float ratioThreshold = 1;

void setup(){
  Serial.begin(9600);
  lcd.init();
  lcd.clear();         
  lcd.backlight();

  lcd.print("Phone Number:");
  lcd.setCursor(0,1);
  lcd.print("+40");
  lcd.setCursor(column,row);

  lcd1.init();
  lcd1.clear();         
  lcd1.backlight();

  displayIp();

  setupDSM();

}

void setupDSM() {
  pinMode(PM1PIN,INPUT);
  pinMode(PM25PIN,INPUT);
}
  
void loop(){
  inputKey();
  readTemp();
  //Serial.println(analogRead(A3));

  readAnalogSensors();
  checkGas();
  //readDSM();
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
    else if(key != 'A' && key != 'B' && key != 'C' && key !='D' && key != resetKey && validPhoneNumber == false){
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
      displayIp();
    }
    else if(key == 'B'){
      displayTemperature(lcd);
      displayHumidity(lcd1);
    }
    else if(key == 'C'){
      displayGas(lcd);
      displaySmoke(lcd1);
    }
    else if(key == 'D'){
      displayDust(lcd);

    }
    else if(key == resetKey){
      lcd.clear();
      lcd.print("Reseting");
      lcd.setCursor(0, 1);
      lcd.print("Phone number");
      delay(500);
      for(int i = 0; i <= 9; i++){
        number[i] = ' ';
      }
      count = 0;
      validPhoneNumber = false;
      displayPhoneNumber(validPhoneNumber);

    }

  }
 }
 
 void displayPhoneNumber(boolean valid){
  if(valid){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("My phone nr:");
    lcd.setCursor(0,1);
    lcd.print("+40");
    for(int i = 0; i <=8; i++)
      lcd.print(number[i]);
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
    if (measure_environment(&temperature, &humidity)) {
        Serial.print("T = ");
        Serial.print(temperature, 1);
        t = temperature;
        h = humidity;
        Serial.print(" deg. C, H = ");
        Serial.print(humidity, 1);
        Serial.println("%");
    }
}

void readAnalogSensors(){
  MQValue = analogRead(MQ2PIN);
  MicValue = analogRead(MicPin);
  if(MQValue < MQValueThreshold)
    Serial.println("MQ e bine");
  else
    Serial.println("GAZ");
  if(MicValue < MicValueThreshold)
    Serial.println(MicValue);
  else
    Serial.println("Zgomot");
}

void checkGas(){
  MQValue = analogRead(MQ2PIN);
  if(MQValue > MQValueThreshold){
    gas = true;
    checkDust();
    checkSmoke();
  }
  else
    gas = false;
}

bool checkDust(){
  readDSM();
  if(ratio < ratioThreshold)
    return false;
  else
    return true; 
}

bool checkSmoke(){
  checkDust();
  if(checkDust == true && checkGas == true)
    return true;
  else
    return false;
}

void readDSM(){
  elapsedtime = millis();
  endtime = elapsedtime + sampletime_ms;
  
  lowpulseoccupancyPM1 = 0;
  lowpulseoccupancyPM25 = 0;

  if (endtime < elapsedtime) {
    Serial.println("missing");
    return;
  }
  
  while (elapsedtime < endtime) //Only after 30s has passed we calcualte the ratio
  {
    durationPM1 = pulseIn(PM1PIN, LOW);
    durationPM25 = pulseIn(PM25PIN, LOW);

    lowpulseoccupancyPM1 += durationPM1;
    lowpulseoccupancyPM25 += durationPM25;

    elapsedtime = millis();    
  }
      /*
    ratio1 = (lowpulseoccupancy/1000000.0)/30.0*100.0; //Calculate the ratio
    Serial.print("ratio1: ");
    Serial.println(ratio1);
    
    concentration = 0.001915 * pow(ratio1,2) + 0.09522 * ratio1 - 0.04884;//Calculate the mg/m3
    */
    conPM1 = calculateConcentration(lowpulseoccupancyPM1,0.01);
    conPM25 = calculateConcentration(lowpulseoccupancyPM25,0.01);
    Serial.print("PM1 ");
    Serial.print(conPM1);
    Serial.print("  PM25 ");
    Serial.println(conPM25);
}

float calculateConcentration(long lowpulseInMicroSeconds, long durationinSeconds){
  
  ratio = (lowpulseInMicroSeconds/1000000.0)/30.0*100.0; //Calculate the ratio
  float concentration = 0.001915 * pow(ratio,2) + 0.09522 * ratio - 0.04884;//Calculate the mg/m3
  Serial.print("lowpulseoccupancy:");
  Serial.print(lowpulseInMicroSeconds);
  Serial.print("    ratio:");
  Serial.print(ratio);
  Serial.print("    Concentration:");
  Serial.println(concentration);
  return concentration;
}

void displayTemperature(LiquidCrystal_I2C screen){
  screen.clear();
  screen.print("Temperature");
  screen.setCursor(0, 1);
  screen.print(String(t) + "°C");
}

void displayHumidity(LiquidCrystal_I2C screen){
  screen.clear();
  screen.print("Humidity");
  screen.setCursor(0, 1);
  screen.print(String(h) + "%");
}

void displayGas(LiquidCrystal_I2C screen){
  screen.clear();
  screen.print("Gas");
  readAnalogSensors();
  screen.setCursor(0,1);
    if(gas == false)
      screen.print("NO GAS");
    else
      screen.print("GAS DETECTED");
}

void displaySmoke(LiquidCrystal_I2C screen){
  screen.clear();
  screen.print("Smoke");
  screen.setCursor(0, 1);
    if(checkSmoke() == true){
      screen.print("Yes");
    }
    else
      screen.print("No");
}

void displayDust(LiquidCrystal_I2C screen){
  screen.clear();
  screen.print("Dust:");
  screen.setCursor(0, 1);
  readDSM();
  if(ratio < ratioThreshold){
    screen.print("Low level of dust");
  }
  else
    screen.print("High level of dust");
}

void displayIp(){
  lcd1.clear();
  lcd1.print("IP Address:");
  lcd1.setCursor(0,1);
  lcd1.print(ip);
  lcd1.setCursor(column,row);
}



