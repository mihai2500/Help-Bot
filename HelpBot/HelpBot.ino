#include <Keypad.h>
#include <DHT_Async.h>

#include <LiquidCrystal_I2C.h>

//LCD displays
LiquidCrystal_I2C lcd(0x27,16,2);
LiquidCrystal_I2C lcd1(0x26,16,2);

//ip address of the esp-32 CAM
String ip = "192.168.174.75";

//DHT11 tmperature and humidity sensor
#define DHT_SENSOR_TYPE DHT_TYPE_11
static const int DHT_SENSOR_PIN = 13;
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

//MQ2 - gas sensor
#define MQ2PIN A2
int MQValue;
int MQValueThreshold = 400; 
bool gas = false;

//Microphone
#define MicPin A3
int MicValue;
int MicValueThreshold = 300; 

//LCD print position
int row = 1;          
int column = 3;

const byte ROWS = 4; 
const byte COLS = 4; 

//SIM800l and messages
#include <SoftwareSerial.h>                    
SoftwareSerial sim(14, 15);
int _timeout;
String _buffer;
int count;
char number[9];
String phoneNumber;
String contactNumber;
bool validPhoneNumber = false;

char confirmKey = '*';
char resetKey = '#';

char hexaKeys[ROWS][COLS] = {                  //The available characters of the keypad
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

//Keypad's rows and columns pins
byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad numberKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

//variables for displaying temperature and humidity
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

//the DSM501A detecting detects fine particles as small as 1μm and up to 2.5μm
float conPM1;
float conPM25;

float ratio;
float ratioThreshold = 0.97;

int temperatureThreshold = 30;
int humidityThreshold = 60; 

void setup(){                              
  Serial.begin(9600);
  _buffer.reserve(50);
  sim.begin(9600);
  setupLCDs();
  setupDSM();

  lcd.print("Initializing...");
  lcd1.print("Please Wait");
  delay(20000);   //time for MQ to warm up and for SIM800l to connect to network

  lcd.clear();
  lcd.print("Phone Number:");
  lcd.setCursor(0,1);
  lcd.print("+40");
  lcd.setCursor(column,row);
  displayIp();

}

void setupLCDs(){
  lcd.init();
  lcd1.init();
  lcd.clear();         
  lcd1.clear();         
  lcd.backlight();
  lcd1.backlight();

  
}

void setupDSM() {
  pinMode(PM1PIN,INPUT);
  pinMode(PM25PIN,INPUT);
}
  
void loop(){
  inputKey();
  readTemp();
  readAnalogSensors();
  checkGas();
}

 void inputKey(){
  char key = numberKeypad.getKey();                   //Receive the pressed key
  
  if (key){
    Serial.println(key);
    if(key == confirmKey){
      if(count == 9){
        Serial.println();
        phoneNumber = number;
        phoneNumber.trim();                           //Cut any blank characters so there is no error while using the AT commands
        Serial.println(phoneNumber);
        validPhoneNumber = true;
        displayPhoneNumber(validPhoneNumber);
        sendMessage("Phone number successfully associated with HelpBot");        //Send message to confirmed number
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
        displayPhoneNumber(validPhoneNumber);         //Output error message if the phone number is too short
        count = 0;
      }
    }
    else if(key != 'A' && key != 'B' && key != 'C' && key !='D' && key != resetKey && validPhoneNumber == false){          //Change interface if the input phone number digits exceed the number of digits of a valid phone number
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
    else if(key == 'A'){                       //Display the phone number/The input interface and the ip address of the esp-32 CAM
      displayPhoneNumber(validPhoneNumber);
      displayIp();
    }
    else if(key == 'B'){               //Display the temperature and humidity menu
      displayTemperature(lcd);
      displayHumidity(lcd1);
    }
    else if(key == 'C'){               //Display the gas and smoke menu
      displayGas(lcd);
      displaySmoke(lcd1);
    }
    else if(key == 'D'){               //Display the dust menu
      displayDust(lcd);

    }
    else if(key == resetKey){          //Reset the phone number
      lcd.clear();
      lcd.print("Reseting");
      lcd.setCursor(0, 1);
      lcd.print("Phone number");
      delay(500);
      for(int i = 0; i <= 9; i++){
        number[i] = ' ';
      }
      count = 0;
      sendMessage("Reseting the phone number associated with HelpBot has been successful");
      validPhoneNumber = false;
      displayPhoneNumber(validPhoneNumber);
    }
  }
 }
 
 void displayPhoneNumber(boolean valid){           //Displaying the confirmed phone number or the interface to input a phone number
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

static bool measure_environment(float *temperature, float *humidity) {      //measure temperature and humidity every 200ms
    static unsigned long measurement_timestamp = millis();
    if (millis() - measurement_timestamp > 200ul) {
        if (dht_sensor.measure(temperature, humidity)) {
            measurement_timestamp = millis();
            return (true);
        }
    }
    return (false);
}

void readTemp(){                                            //Measures temperature and humidity
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
  if(temperature > temperatureThreshold){
    sendMessage("High temperature detected");       //Sending message if the temperature surpases the threshold level
    checkGas();
  }
  if(humidity > humidityThreshold){
    sendMessage("High humidity detected");
  }
}

void readAnalogSensors(){                 //Reading input from MQ and microphone pins
  MQValue = analogRead(MQ2PIN);
  MicValue = analogRead(MicPin);
  if(MQValue < MQValueThreshold)
    Serial.println(MQValue);
  else
    Serial.println("GAZ");
  if(MicValue < MicValueThreshold)
    Serial.println(MicValue);
  else{
    sendMessage("Noise heared in the baby's room");       //Sending message if loud noises occur
  }
}

void checkGas(){                 //Checking if there is any gas present
  MQValue = analogRead(MQ2PIN);
  if(MQValue > MQValueThreshold){
    gas = true;
    checkDust();
    checkSmoke();
    if(checkSmoke() == false)
      sendMessage("Gas alert");       //Sending mesage if there is gas detected
  }
  else
    gas = false;
}

bool checkDust(){            //Checking the level of dust in the air
  readDSM();
  if(ratio < ratioThreshold)
    return false;
  else
    return true; 
}

bool checkSmoke(){            //Chechking if there is smoke present in the air
  checkDust();
  if(checkDust == true && checkGas == true){
    sendMessage("Smoke alert");
    return true;
  }
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
  
  while (elapsedtime < endtime) //Calculating the ration only after the set time elapsed, in our case
  {
    durationPM1 = pulseIn(PM1PIN, LOW);
    durationPM25 = pulseIn(PM25PIN, LOW);

    lowpulseoccupancyPM1 += durationPM1;
    lowpulseoccupancyPM25 += durationPM25;

    elapsedtime = millis();    
  }
    conPM1 = calculateConcentration(lowpulseoccupancyPM1,0.01);
    conPM25 = calculateConcentration(lowpulseoccupancyPM25,0.01);
    Serial.print("PM1 ");
    Serial.print(conPM1);
    Serial.print("  PM25 ");
    Serial.println(conPM25);
}

float calculateConcentration(long lowpulseInMicroSeconds, long durationinSeconds){
  
  ratio = (lowpulseInMicroSeconds/1000000.0)/30.0*100.0;   //Calculate the ratio
  float concentration = 0.001915 * pow(ratio,2) + 0.09522 * ratio - 0.04884;   //Calculate the mg/m3
  Serial.print("lowpulseoccupancy:");
  Serial.print(lowpulseInMicroSeconds);
  Serial.print("    ratio:");
  Serial.print(ratio);
  Serial.print("    Concentration:");
  Serial.println(concentration);
  return concentration;
}

void displayTemperature(LiquidCrystal_I2C screen){     //Display the temperature
  screen.clear();
  screen.print("Temperature");
  screen.setCursor(0, 1);
  screen.print(String(t) + "°C");
}

void displayHumidity(LiquidCrystal_I2C screen){      //Display the humidity
  screen.clear();
  screen.print("Humidity");
  screen.setCursor(0, 1);
  screen.print(String(h) + "%");
}

void displayGas(LiquidCrystal_I2C screen){       //Display on lcd if there is any gas present
  screen.clear();
  screen.print("Gas");
  readAnalogSensors();
  screen.setCursor(0,1);
    if(gas == false)
      screen.print("NO GAS");
    else
      screen.print("GAS DETECTED");
}

void displaySmoke(LiquidCrystal_I2C screen){    //Display if there is smoke detected
  screen.clear();
  screen.print("Smoke");
  screen.setCursor(0, 1);
    if(checkSmoke() == true){
      screen.print("Yes");
    }
    else
      screen.print("No");
}

void displayDust(LiquidCrystal_I2C screen){       //Display dust level on the lcd
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

void displayIp(){        //Display the ip address of the Esp-32 CAM
  lcd1.clear();
  lcd1.print("IP Address:");
  lcd1.setCursor(0,1);
  lcd1.print(ip);
  lcd1.setCursor(column,row);
}

void sendMessage(String SMS)
{
  if(validPhoneNumber){                      //Check if any phone number is associated with HelpBot
  Serial.println ("Sending Message");
  sim.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);
  sim.println("AT+CMGS=\"" + phoneNumber + "\"\r"); //Mobile phone number to send message
  delay(1000);
  sim.println(SMS);
  delay(100);
  sim.println((char)26);     //ASCII code of CTRL+Z in order to send message
  delay(1000);
  _buffer = _readSerial();
  }
}
String _readSerial() {
  _timeout = 0;
  while  (!sim.available() && _timeout < 8000  )          
  {
    delay(13);
    _timeout++;
  }
  if (sim.available()) {
    return sim.readString();
  }
}
