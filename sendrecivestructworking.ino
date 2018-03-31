#include<SPI.h>
#include<RF24.h>
#include<stdlib.h>
//Setup Variables
int LastValue;
#define Offset 0;
int anchor = 4;
const int sensorPin = 0;
// ce, csn pins
RF24 radio(9, 10) ;

float getvanedir() {
  int result = 0;
  int VaneValue;// raw analog value from wind vane
  int Direction;// translated 0 - 360 direction
  int CalDirection;// converted value with offset applied
  VaneValue = analogRead(1);
  Direction = map(VaneValue, 0, 1023, 0, 360);
  CalDirection = Direction + Offset;

  if (CalDirection > 360)
    CalDirection = CalDirection - 360;
  if (CalDirection < 0)
    CalDirection = CalDirection + 360;

  // Only update the display if change greater than 2 degrees.
  if (abs(CalDirection - LastValue) > 5)
  {
    //Serial.print(VaneValue); Serial.print("\t\t");
    //Serial.print(CalDirection); Serial.print("\t\t");
    //getHeading(CalDirection);
    LastValue = CalDirection;
  }
  result = CalDirection;
  return result;
}

float getwindSpeed() {
  float windSpeed = 0; // Wind speed in meters per second (m/s)
  float voltageMin = .4; // Mininum output voltage from anemometer in mV.
  float windSpeedMin = 0; // Wind speed in meters/sec corresponding to minimum voltage
  float voltageMax = 2.0; // Maximum output voltage from anemometer in mV.
  float windSpeedMax = 32; // Wind speed in meters/sec corresponding to maximum voltage
  float voltageConversionConstant = .004882814;
  float sensorVoltage;
  float sensorValue;
  float result = 0;
  sensorValue = analogRead(sensorPin);//Get a value between 0 and 1023 from the analog pin connected to the anemometer
  //Serial.print("sensor Value ");
  Serial.println(sensorValue);
  sensorVoltage = sensorValue * voltageConversionConstant; //Convert sensor value to actual voltage
  //Serial.print("sensor Voltage");
  //Serial.println(sensorVoltage);
  //Convert voltage value to wind speed using range of max and min voltages and wind speed for the anemometer
  if (sensorVoltage <= voltageMin) {
    windSpeed = 0; //Check if voltage is below minimum value. If so, set wind speed to zero.
  } else {
    windSpeed = (sensorVoltage - voltageMin) * windSpeedMax / (voltageMax - voltageMin); //For voltages above minimum value, use the linear relationship to calculate wind speed.
  }
  result = windSpeed;
  return result;

}
// Converts compass direction to heading
void getHeading(int direction) {
  if (direction < 22)
    Serial.println("N");
  else if (direction < 67)
    Serial.println("NE");
  else if (direction < 112)
    Serial.println("E");
  else if (direction < 157)
    Serial.println("SE");
  else if (direction < 212)
    Serial.println("S");
  else if (direction < 247)
    Serial.println("SW");
  else if (direction < 292)
    Serial.println("W");
  else if (direction < 337)
    Serial.println("NW");
  else
    Serial.println("N");
}
void setup(void) {
  while (!Serial) ;
  LastValue = 1;
  Serial.begin(9600) ;
  pinMode(4, OUTPUT);
  radio.begin() ;
  radio.setPALevel(RF24_PA_MAX) ;
  radio.setChannel(0x76) ;
  radio.openWritingPipe(0xF0F0F0F0E1LL) ;
  const uint64_t pipe = 0xE8E8F0F0E1LL ;
  radio.openReadingPipe(1, pipe) ;
  radio.enableDynamicPayloads() ;
  radio.powerUp() ;
  //  Serial.println("Vane Value\tDirection\tHeading");
}

void loop(void) {
  radio.startListening() ;
  Serial.println("Starting loop. Radio on.") ;
  char receivedMessage[2] = {0} ;
  if (radio.available()) {
    radio.read(receivedMessage, sizeof(receivedMessage));
    Serial.println(receivedMessage) ;
    Serial.println("Turning off the radio.") ;
    radio.stopListening() ;
    String stringMessage(receivedMessage) ;
    Serial.println(stringMessage);

    if (stringMessage == "G1") {
      Serial.println("Looks like they want a string!") ;
      Serial.println(stringMessage) ;
      String datastring = "xxxxxxxx";
      const char data[10] = "xxxxxxxx";
      const char text[2][5] = {
        "99.9",
        "180"
      };
      float windDirection;
      float wndSpeed;
      windDirection = getvanedir();
      wndSpeed = getwindSpeed();
      //Print voltage and windspeed to serial
      Serial.print("Wind speed: ");
      Serial.println(wndSpeed);
      //Serial.print("\t");
      Serial.print("Wind dir: ");
      Serial.println(windDirection);
      //Serial.print("\t");
      dtostrf(wndSpeed, 4, 1, text[0]);
      dtostrf(windDirection, 3, 0, text[1]);
      datastring = text[0];
      datastring += ",";
      datastring += text[1];
      //delay(sensorDelay);
      digitalWrite(anchor, HIGH);
      Serial.println(datastring);
      datastring.toCharArray(data, 10);
      radio.write(data, sizeof(data)) ;
      Serial.println("We sent our message.") ;
    }
    else if (stringMessage == "G0") {
      Serial.println("Looks like they want a string!") ;
      Serial.println(stringMessage) ;
      String datastring = "xxxxxxxx";
      const char data[10] = "xxxxxxxx";
      const char text[2][5] = {
        "99.9",
        "180"
      };
      float windDirection;
      float wndSpeed;
      windDirection = getvanedir();
      wndSpeed = getwindSpeed();

      //Print voltage and windspeed to serial
      Serial.print("Wind speed: ");
      Serial.println(wndSpeed);
      //Serial.print("\t");
      Serial.print("Wind dir: ");
      Serial.println(windDirection);
      //Serial.print("\t");
      dtostrf(wndSpeed, 4, 1, text[0]);
      dtostrf(windDirection, 3, 0, text[1]);
      datastring = text[0];
      datastring += ",";
      datastring += text[1];
      //delay(sensorDelay);
      digitalWrite(anchor, LOW);
      Serial.println(datastring);
      datastring.toCharArray(data,sizeof(data));
      radio.write(data, 10) ;
      Serial.println("We sent our message.") ;

    }
    else {
      Serial.println("unknown string!") ;
      Serial.println(stringMessage) ;
      //const char text[] = "xxxxx" ;
      //      float CalDirection;
      //      float windSpeed;
      //      CalDirection = getvanedir();
      //      windSpeed = getwindSpeed();

      //digitalWrite(anchor, LOW);
      //float tmp;
      //tmp = getwindSpeed();
      //Print voltage and windspeed to serial
      //      Serial.print("Wind speed: ");
      //      Serial.println(windSpeed);
      //      Serial.print("Wind dir: ");
      //      Serial.println(CalDirection);
      //      Serial.print("\t");
      //      dtostrf(windSpeed, 5, 1, text);
      //delay(sensorDelay);
      //     digitalWrite(anchor, HIGH);
      //      Serial.println(text);
      //      radio.write(text, sizeof(text)) ;
      //      Serial.println("We sent our message.") ;
    }
  }

  delay(100);

}





