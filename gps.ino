
#define uS_TO_S_FACTOR      1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP       30          /* Time ESP32 will go to sleep (in seconds) */

#define UART_BAUD           115200

#define MODEM_TX            27
#define MODEM_RX            26
#define MODEM_PWRKEY        4
#define MODEM_DTR           32
#define MODEM_RI            33
#define MODEM_FLIGHT        25
#define MODEM_STATUS        34

#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13

#define LED_PIN             12
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024 // Set RX buffer to 1Kb
#define SerialAT Serial1

#define DUMP_AT_COMMANDS

#define GSM_PIN ""

#include <TinyGsmClient.h>

#ifdef USING_TINYGPS_LIBRARY
// Use TinyGPS NMEA math analysis library
#include <TinyGPS++.h>
TinyGPSPlus gps;
#endif


#ifdef DUMP_AT_COMMANDS  // if enabled it requires the streamDebugger lib
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

char receivedChar;

unsigned long cur_time_led, old_time_led;
unsigned long cur_time, old_time;

void setup()
{
    Serial.begin(115200); // Set console baud rate
    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
    /*
    The indicator light of the board can be controlled
    */
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    /*
    MODEM_PWRKEY IO:4 The power-on signal of the modulator must be given to it,
    otherwise the modulator will not reply when the command is sent
    */
    pinMode(MODEM_PWRKEY, OUTPUT);
    digitalWrite(MODEM_PWRKEY, HIGH);
    delay(300); //Need delay
    digitalWrite(MODEM_PWRKEY, LOW);

    /*
    MODEM_FLIGHT IO:25 Modulator flight mode control,
    need to enable modulator, this pin must be set to high
    */
    pinMode(MODEM_FLIGHT, OUTPUT);
    digitalWrite(MODEM_FLIGHT, HIGH);

    Serial.println("Start modem...");


    for (int i = 0; i < 3; ++i) {
        while (!modem.testAT(500)) {
            Serial.println("Try to start modem...");
            pinMode(MODEM_PWRKEY, OUTPUT);
            digitalWrite(MODEM_PWRKEY, HIGH);
            delay(100); //Need delay
            digitalWrite(MODEM_PWRKEY, LOW);
        }
    }

    Serial.println("Modem Response Started.");

    // Stop GPS Server
    //send_at("AT+CGPS=0");
    //modem.sendAT("+CGPS=0");
    //modem.waitResponse(30000);

    // Enable GPS
    send_at("AT+CGPS=1");
    //modem.sendAT("+CGPS=1");
    modem.waitResponse(3000);

    send_at("AT+HTTPINIT");
    modem.waitResponse(3000);

    send_at("AT+HTTPPARA=\"URL\",\"https://vcbkalbh84.execute-api.us-east-1.amazonaws.com/ab/\"");
    modem.waitResponse(3000);

    send_at("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
    modem.waitResponse(3000);
    //Disable NMEA OUTPUT
    // modem.sendAT("+CGPSINFOCFG=0,31");
    // modem.waitResponse(30000);
}

void loop()
{
  float lat      = 0;
	float lon      = 0;
	float speed    = 0;
	float alt      = 0;
	int   vsat     = 0;
	int   usat     = 0;
	float accuracy = 0;
	int   year     = 0;
	int   month    = 0;
	int   day      = 0;
	int   hour     = 0;
	int   min      = 0;
	int   sec      = 0;
  modem.getGPS(&lat, &lon, &speed, &alt, &vsat, &usat, &accuracy,&year, &month, &day, &hour, &min, &sec);
  Serial.println("Latitude: " + String(lat, 8) + "\tLongitude: " + String(lon, 8));
  sent_data(lat , lon);
  //delay(10000);  
    
}
void send_at(char *_command) {
  SerialAT.println(_command);
  while (SerialAT.available() > 0) {
      Serial.print(SerialAT.readString());
    }
}
void sent_data(float lat , float lon){
  if(lat == 0  && lon == 0){
    
    send_at("AT+CGPS=1");
    //modem.sendAT("+CGPS=1");
    modem.waitResponse(3000);
    //delay(20000);
  }
    
  String data = "{\"key\":\"location\",\"lat\":" + String(lat, 8) + ",\"lon\":" + String(lon, 8) + "}";
  String length = String(data.length());
  String httpData = "AT+HTTPDATA=" + length + ",10000";
  char command[httpData.length() + 1];
  httpData.toCharArray(command, httpData.length() + 1);

  
  send_at(command);
  modem.waitResponse(3000);
    
  char dataBuffer[data.length() + 1]; // 为字符串长度加上一个额外的空间以存放字符串终止符'\0'
  data.toCharArray(dataBuffer, data.length() + 1); // 将String转换为char*类型
  send_at(dataBuffer);
  modem.waitResponse(3000);

  send_at("AT+HTTPACTION=1");
  modem.waitResponse(3000);
  modem.waitResponse(3000);
}


