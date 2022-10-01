#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <dht.h> 

//---Mapeamento de Hardware---

#define    thermistor         A0
#define    DHTSensor01         5
#define    DHTSensor02         7
#define    lamp01              8
#define    lamp02              9

//-----------------------------------------Thermistor Settings---------------------------------------------

#define NominalThermistorResistance 100000      
#define NominalTemperature 25   
#define SAMPLING 100
#define BCOEFFICIENT 4149
#define SerieResistance 98000  

int Sampling[SAMPLING];
int i;

int BetaFactor(){
   float average,
         temperature;
   
 //equaçao funcionamento thermisor
   for (i=0; i< SAMPLING; i++) {
    Sampling[i] = analogRead(thermistor);
    delay(10);
   }
 
   average = 0;
   
   for (i=0; i< SAMPLING; i++){
    average += Sampling[i];
   }
   
   average /= SAMPLING;
   
   average = 1023 / average - 1;
   average = SerieResistance / average;
    
   temperature = average / NominalThermistorResistance;     // (R/Ro)
   temperature = log(temperature); // ln(R/Ro)
   temperature /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
   temperature += 1.0 / (NominalTemperature + 273.15); // + (1/To)
   temperature = 1.0 / temperature;                 // Inverte o valor
   temperature -= 273.15;

   return int(temperature);
}

   
//--------------------------------------Humidity Sensor Parameters-----------------------------------------
dht DHT01;
dht DHT02;
int DHT01temperature = 0x00,
    DHT02temperature = 0x00,
    DHT01Humidity    = 0x00,
    DHT02Humidity    = 0x00;



int AverageInternalTemperature(int thermistorTemperature){
  

  DHT01.read11(DHTSensor01);
  DHT02.read11(DHTSensor02);
  DHT01temperature   = DHT01.temperature;
  DHT02temperature   = DHT02.temperature;

  int thermistorTemperatureFiltered,
      thermistorNumericWeight,
      DHT01temperatureFiltered,
      DHT01NumericWeight,
      DHT02temperatureFiltered,
      DHT02NumericWeight,
      averageTemperature,
      divider;

  if(DHT01temperature != 0){
    DHT01temperatureFiltered = DHT01temperature;
    DHT01NumericWeight = 1;
  }else{
    DHT01temperatureFiltered = 0;
    DHT01NumericWeight = 0;
  }
  
  if(DHT02temperature != 0){
    DHT02temperatureFiltered = DHT02temperature;
    DHT02NumericWeight = 1;
  }else{
    DHT02temperatureFiltered = 0;
    DHT02NumericWeight = 0;
  }

  if(thermistorTemperature != 0){
    thermistorTemperatureFiltered = thermistorTemperature;
    thermistorNumericWeight = 1;
  }else{
    thermistorTemperatureFiltered = 0;
    thermistorNumericWeight = 0;
  }

  divider = DHT01NumericWeight + DHT02NumericWeight + thermistorNumericWeight;
  averageTemperature = ((DHT01temperatureFiltered + DHT02temperatureFiltered+ thermistorTemperatureFiltered)/divider);

  return averageTemperature;
}//end AverageInternalTemperature()



  

int AverageInternalHumidity(){
  
  DHT01.read11(DHTSensor01);
  DHT02.read11(DHTSensor02);
  DHT01Humidity = DHT01.humidity;
  DHT02Humidity = DHT02.humidity;

  int DHT01HumidityFiltered,
      DHT02HumidityFiltered,
      DHT01NumericWeight,
      DHT02NumericWeight,
      averageHumidity,
      divider;

  if(DHT01Humidity != 0){
    DHT01HumidityFiltered = DHT01Humidity;
    DHT01NumericWeight = 1;
  }else{
    DHT01HumidityFiltered = 0;
    DHT01NumericWeight = 0;
  }
  
  if(DHT02Humidity != 0){
    DHT02HumidityFiltered = DHT02Humidity;
    DHT02NumericWeight = 1;
  }else{
    DHT02HumidityFiltered = 0;
    DHT02NumericWeight = 0;
  }

  divider = DHT01NumericWeight + DHT02NumericWeight;

  averageHumidity= ((DHT01HumidityFiltered + DHT02HumidityFiltered)/divider);

  if(averageHumidity <= 20){
    averageHumidity = 20;
  }else if(averageHumidity >=80){
    averageHumidity = 80;
  }

  return averageHumidity;
}//end AverageInternalHumidity()



//-----------------------------------------display parameters----------------------------------------------

//------------------------------------------Display settings-----------------------------------------------

#define    Address    0x27
#define    lines      2
#define    columns    16

LiquidCrystal_I2C lcd(Address, lines, columns);

void ScreenInformationStructure(){
  
   lcd.init();
   lcd.backlight();
   lcd.clear();
   lcd.print("Heated: ");
   lcd.setCursor(14,0);
   lcd.write(B11011111);
   lcd.print("C");
   lcd.setCursor(0,1);
   lcd.print("Humidity: ");
   lcd.setCursor(15,1);
   lcd.print("%"); 
}


void ShowInformationOnLCD(int temperature, int humidity){
  
   lcd.setCursor(12,0);
   lcd.print(temperature);
   lcd.setCursor(12,1);
   lcd.print(humidity); 
}

//-----------------------------------------temperature parameters------------------------------------------
#define MinimumTemperature 50
#define MaximumTemperature 56

void lampInit(){
   pinMode(lamp01,OUTPUT);
   pinMode(lamp02,OUTPUT);
}

void lampONOFF(int internalTemperature, int minTemperature, int maxTemperature ){
  
   if(internalTemperature < minTemperature){
    digitalWrite(lamp01,LOW);
    digitalWrite(lamp02,LOW);
   }else if(internalTemperature >= maxTemperature){
    digitalWrite(lamp01,HIGH);
    digitalWrite(lamp02,HIGH);
   }
  
}

void setup() {
  
  Serial.begin(9600);
  ScreenInformationStructure();
  lampInit();

}

void loop() {

  int thermistor,
      temperature,
      Humidity;
  
  BetaFactor(); 
  thermistor = BetaFactor();  
  AverageInternalTemperature(thermistor); 
  AverageInternalHumidity();  
  temperature = AverageInternalTemperature(thermistor);  
  Humidity    = AverageInternalHumidity(); 
  ShowInformationOnLCD(temperature,Humidity);

  if(Humidity>=21){
    lampONOFF(temperature,MinimumTemperature,MaximumTemperature); 
  }else{
    digitalWrite(lamp01,HIGH);
    digitalWrite(lamp02,HIGH);
  }
}
