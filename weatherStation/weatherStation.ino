#include <ConnectinoAjax.h> // Library for Communication with the Connectino
#include <SoftwareSerial.h> // Necessary for Connectino
#include <Wire.h> //I2C needed for sensors
#include "SparkFunMPL3115A2.h" //Pressure sensor
#include "SparkFunHTU21D.h" //Humidity sensor
 
#define ZONE "+1"
#define PLACE "Hannover"
#define T_MAX 40  // Temperature maximum, Default: 85
#define T_MIN -20  // Temperature minimum, Default: -40
#define T_UNIT "°C"  // Temperature unit, Default: "C"
#define P_MAX 1060  // Pressure maximum, Default: 1100
#define P_MIN 966  // Pressure minimum, Default: 500
 
 
MPL3115A2 pressure; //Create an instance of the pressure sensor
HTU21D humidity; //Create an instance of the humidity sensor
 
 
// Variables to save the values in
float hum = 0; // Humidity[%]
float tempf = 0; // [temperature °F]
float tempc = 0; // [temperature °C]
float press = 0; // Pressure[Pa]
uint16_t cmdLen = 0; // the length of the command (not used)
uint8_t cmdReadingHead = 0;
char ajaxCmd[32] = "";
String ajaxResponse = "";
 
ConnectinoAjax ajax = ConnectinoAjax(StxSerial);
SoftwareSerial debug = SoftwareSerial(4,5);
 
void setup() {
  //Configure the pressure sensor
  pressure.begin();               // Get sensor online
  pressure.setModeBarometer();    // Measure pressure in Pascals from 20 to 110 kPa
  pressure.setOversampleRate(7);  // Set Oversample to the recommended 128 (=>7)
  pressure.enableEventFlags();    // Enable all three pressure and temp event flags 
 
  StxSerial.begin(115200);
  StxSerial.add(ajax);
 
  debug.begin(9600);
 
  //Configure the humidity sensor
  humidity.begin();
}
 
void loop() {
  //Get command from Connectino
  if (ajax.dataAvailable())
  {
    ajax.getData(ajaxCmd, cmdLen);    
    cmdReadingHead = 0;
    while ( cmdReadingHead < cmdLen ) {
      switch (ajaxCmd[cmdReadingHead]){
 
        case 'c':
          //Return the config for the temperature scale
          ajaxResponse.concat("c=" + String(T_MIN) + "," + String(T_MAX) + "," + T_UNIT);
        break;
         case 'd':
          //Return the config for the pressure scale
          ajaxResponse.concat("d=" + String(P_MIN) + "," + String(P_MAX));
        break;
        case 't':
          //Measure temp in fahrenheit from pressure sensor
          tempf = pressure.readTempF();
          //Convert from Fahrenheit to Celsius
          tempc = (tempf - 32) / 1.8 ;
          ajaxResponse.concat("t=" + String(tempc));
          break;
 
        case 'p':
          //Calc pressure & Convert from pascal to hektoPascal
          press = pressure.readPressure() / 100;
          ajaxResponse.concat("p=" + String(press));
          break;
 
        case 'h':
          //Calc humidity
          hum = humidity.readHumidity();
          ajaxResponse.concat("h=" + String(hum));
          break;
 
        case 'z':
          //respond time zone
          ajaxResponse.concat("z="ZONE);
          break;
 
        case 'l':
          //respond place/location
          ajaxResponse.concat("l="PLACE);
          break;
 
        default:
          ajaxResponse.concat(";");
          break;
      }
      cmdReadingHead++;
    }
    ajax.respond(ajaxResponse.c_str()); // send response
    ajaxResponse = ""; // "clear" response
  }
}
