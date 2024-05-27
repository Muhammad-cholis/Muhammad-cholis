#include <WiFiManager.h>
#include <FirebaseESP32.h>
#include <ArduinoJson.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define NUMFLAKES     10 // Number of snowflakes in the animation example


WiFiManager wm;

#define FIREBASE_HOST "agritech-c8ba0-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "myR9BGY3zPHNnYCd2lDJHtIgAdtzIx1cbwuXjm0D"

FirebaseData firebaseData;
FirebaseData firebaseData1;
FirebaseData firebaseData2;
FirebaseData firebaseData3;
FirebaseData firebaseData4;
FirebaseData firebaseData5;
FirebaseData firebaseData6;
FirebaseData firebaseData7;
FirebaseData firebaseData8;

FirebaseAuth auth;
FirebaseConfig config;
const int numNodes = 20;
String nodePaths[numNodes] = {"/Fertigasi/J_Flow_Air_1", "/Fertigasi/J_Flow_Air_2", "/Fertigasi/J_Flow_Pupuk_1", "/Fertigasi/J_Flow_Pupuk_2", "/Fertigasi/J_Jam_1",  "/Fertigasi/J_Jam_2", "/Fertigasi/J_Jam_3", "/Fertigasi/J_Jam_4", "/Fertigasi/J_Mnt_1", "/Fertigasi/J_Mnt_2",  "/Fertigasi/J_Mnt_3", "/Fertigasi/J_Mnt_4", "/Fertigasi/J_Penyiraman_1", "/Fertigasi/J_Penyiraman_2", "/Fertigasi/J_pemupukan_1",  "/Fertigasi/J_pemupukan_2", "/Fertigasi/K_air", "/Fertigasi/K_flow_air", "/Fertigasi/K_flow_pupuk", "/Fertigasi/K_pupuk"};


#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

//sensor fc-28
const int soilMoisturePin = 34;
int soilMoistureValue = 0;
int kel;

//Sensor flow
#define FlowSensorPin  32
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 5.75;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

//TDS Sensor
#define TdsSensorPin 35
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, temperature = 25;
int tdsValue = 0;

#define pump_air 27
#define pump_nutrisi 26


#define SD_CS 5

String K_pupuk = "" ;
String K_Air = "" ;
String K_FlowAir = "" ;
String K_FlowPupuk = "" ;

String Pemupukan1 = "" ;
String Pemupukan2 = "" ;
String Penyiraman1 = "" ;
String Penyiraman2 = "" ;

String JFlowPupuk1 = "" ;
String JFlowPupuk2 = "" ;
String JFlowAir1 = "" ;
String JFlowAir2 = "" ;

String J_Jam_1 = "" ;
String J_Jam_2 = "" ;
String J_Jam_3 = "" ;
String J_Jam_4 = "" ;

String J_Mnt_1 = "" ;
String J_Mnt_2 = "" ;
String J_Mnt_3 = "" ;
String J_Mnt_4 = "" ;
String Waktu = "" ;

char penyiraman_1;
char penyiraman_2;
char pemupukan_1;
char pemupukan_2;

String J_1_Pemupukan = "" ;
String J_2_Pemupukan = "" ;
String J_1_Penyiraman = "" ;
String J_2_Penyiraman = "" ;

String S_VolPupuk1 =  "" ;
String S_VolPupuk2 =  "" ;
String S_VolAir1 =  "" ;
String S_VolAir2 =  "" ;
String DataWaktu = "" ;

int PompaAir ;
int PompaPupuk ;

unsigned long previousMillisDS = 0;
unsigned long previousMillisFirebase = 0;
unsigned long previousMillisOled = 0;
String S_Penyiraman1 = "";
String S_Penyiraman2 = "";
String S_Pemupukan1 = "";
String S_Pemupukan2 = "";


bool pumpNutrisi = false; // Status pompa Nutrisi mati
bool pumpAir = false; //Status pompa air mati

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

bool wifiConfigured = false;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(TdsSensorPin, INPUT);
  pinMode(soilMoisturePin, INPUT);
  pinMode(FlowSensorPin, INPUT_PULLUP);
  pinMode (pump_air, OUTPUT);
  pinMode (pump_nutrisi, OUTPUT);

  digitalWrite(pump_air, HIGH);
  digitalWrite(pump_nutrisi, HIGH);
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(FlowSensorPin), pulseCounter, FALLING);
  //  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Layar OLED tidak terdeteksi"));
    for (;;);
  }
  display.clearDisplay();
  display.display();
  delay(100);

  rtc.begin();
  if (rtc.lostPower()) {
    // this will adjust to the date and time at compilation
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  while (!Serial);
  SD.begin(SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("Gagal Memuat Kartu SD");
    return;
  }
  else {
    Serial.println("Sd Card Ready");
  }

  WiFi.mode(WIFI_STA);
  //Konfigurasi Wi-fi
  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(60);
  if (wm.autoConnect("Fertigasi")) {
    Serial.println("connected...yeey :)");
    wifiConfigured = true;
  }
  else {
    Serial.println("Configportal running");
    wifiConfigured = false;
  }

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  // Inisialisasi Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!Firebase.beginStream(firebaseData, "/Fertigasi")) {
    Serial.printf("Stream begin error, %s\n\n", firebaseData.errorReason().c_str());
  }
  // Memulai streaming data dari Firebase
  if (!Firebase.beginStream(firebaseData1, "/Fertigasi/K_air")) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData1.errorReason());
  }
  if (!Firebase.beginStream(firebaseData2, "/Fertigasi/K_flow_air")) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData2.errorReason());
  }
  if (!Firebase.beginStream(firebaseData3, "/Fertigasi/K_pupuk")) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData3.errorReason());
  }
  if (!Firebase.beginStream(firebaseData4, "/Fertigasi/K_flow_pupuk")) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData4.errorReason());
  }
  if (!Firebase.beginStream(firebaseData5, "/Fertigasi/J_pemupukan_1")) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData4.errorReason());
  }
  if (!Firebase.beginStream(firebaseData6, "/Fertigasi/J_pemupukan_2")) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData4.errorReason());
  }
  if (!Firebase.beginStream(firebaseData7, "/Fertigasi/J_Penyiraman_1")) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData4.errorReason());
  }
  if (!Firebase.beginStream(firebaseData8, "/Fertigasi/J_Penyiraman_2")) {
    Serial.println("Could not begin stream");
    Serial.println("REASON: " + firebaseData4.errorReason());
  }
}

void loop() {
  wm.process();
  WaktuRtc();
  TDSSensor();
  SoilMoisture();
  
  
  if (!wifiConfigured) {
    Serial.println("Tidak terhubung");
    penjadwalan();
  } else {
    Serial.println("Terhubung");
    display.println("Terhubung");
    kontrolFrbs();
  }
}

void WaktuRtc() {
  DateTime now = rtc.now();
  int jam     = now.hour();
  int menit   = now.minute();
  int detik   = now.second();
  DataWaktu = String(jam) + ":" + String (menit) + ":" + String (detik);

  Waktu = (jam * 60) + menit;
  String PumpaAir = pumpAir ? "true" : "false";
  String PumpaNutrisi = pumpNutrisi ? "true" : "false";
  if (now.minute() % 5 == 0 && now.second() == 0){
  String DataSimpan = String (DataWaktu) + String (";") + String (tdsValue) + String ("ppm")+ String (";") + String (kel) + String ("%") + String (";") + String (PumpaAir) + String (";") + String (PumpaNutrisi);
   appendFile(SD, "/DataSimpan.txt", DataSimpan);
  }
  Serial.print ("Waktu :");;
  Serial.println (DataWaktu);
  Serial.println (Waktu);
  Serial.println ();
}

/*-----Sensor Flow, TDS Meter, Soil Moisture-------------*/

void TDSSensor() {
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U)  //every 40 milliseconds,read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U)
  {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4095.0;
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
    float compensationVolatge = averageVoltage / compensationCoefficient;
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.4;
    Serial.print("TDS Value:");
    Serial.print(tdsValue);
    Serial.println("ppm");
  }
}

int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}

unsigned long previousMillisSoil = 0;
void SoilMoisture() {
  unsigned long currentMillisSoil = millis();
  if (currentMillisSoil - previousMillisSoil >= 1000) {
    previousMillisSoil = currentMillisSoil;

    float soilMoistureValue = analogRead(soilMoisturePin);
    kel = map(soilMoistureValue, 1400, 3000, 100, 0);

    Serial.print("Voltage :" );
    Serial.println (soilMoistureValue);

    Serial.print("Kelembapan :" );
    Serial.println (kel);
    delay(1000);
  }
}

void flowMeter() {
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    //    while (millis() - currentMillis < 1000) {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    //    }
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;

    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.print("L");
    Serial.print("\t");       // Print tab space

  }
}

void UpdateFlow() {
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(FlowSensorPin), pulseCounter, FALLING);
}

/*---------------------Firebase-----------------------*/

void kontrolFrbs() {
  unsigned long currentMillisFrb = millis();
  if (Firebase.ready() && (currentMillisFrb - previousMillisFirebase >= 100 || previousMillisFirebase == 0)) {
    // Simpan waktu sekarang sebagai waktu sebelumnya
    previousMillisFirebase = currentMillisFrb;

    Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_ph_tanah"), kel) ? "ok" : firebaseData.errorReason().c_str());
    Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_TDS_Sensor"), tdsValue) ? "ok" : firebaseData.errorReason().c_str());
    Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Flow_sensor"), totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
    Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Pupuk"), PompaPupuk) ? "ok" : firebaseData.errorReason().c_str());
    Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), PompaAir) ? "ok" : firebaseData.errorReason().c_str());
    ReadFrdb();
  }
  unsigned long currentMillisDS = millis();
  if (currentMillisDS - previousMillisDS >= 100) {
    //   Simpan waktu sekarang sebagai waktu sebelumnya
    previousMillisDS = currentMillisDS;
    int KFlowAir = K_FlowAir.toInt();
    int KFlowPupuk = K_FlowPupuk.toInt();

    if ( K_Air == "1" && KFlowAir >= totalMilliLitres ) {
      PumpAirNyala();
      Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), PompaAir) ? "ok" : firebaseData.errorReason().c_str());
      while (true) {
        flowMeter();
        ReadFrdb();
        WaktuRtc();
        oledConn();
        Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Flow_sensor"), totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
        if (KFlowAir <= totalMilliLitres || K_Air == "0") {
          PumpAirMati();
          Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), PompaAir) ? "ok" : firebaseData.errorReason().c_str());
          break;
        }
      }
    }

    if ( K_pupuk == "1" && totalMilliLitres <= KFlowPupuk) {
      PumpPupukNyala();
      Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Pupuk"), PompaPupuk) ? "ok" : firebaseData.errorReason().c_str());
      while (true) {
        flowMeter();
        ReadFrdb();
        WaktuRtc();
        oledConn();
        Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Flow_sensor"), totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
        if (totalMilliLitres >= KFlowPupuk || K_pupuk == "0") {
          PumpPupukMati();
          Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Pupuk"), PompaPupuk) ? "ok" : firebaseData.errorReason().c_str());
          break;
        }
      }
    }

    int WaktuInt = Waktu.toInt();

    int jam1 = J_Jam_1.toInt();
    int mnt1 = J_Mnt_1.toInt();
    int S_pupuk1 = (jam1 * 60) + mnt1;
    int S_1_pupuk1 = S_pupuk1 + 60;
    int vol_pupuk1 = JFlowPupuk1.toInt();

    S_Pemupukan1 = String(S_pupuk1);

    if (Pemupukan1 == "1" && WaktuInt >= S_pupuk1 && WaktuInt < S_1_pupuk1 && vol_pupuk1  >= totalMilliLitres ) {
      PumpPupukNyala();
      Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Pupuk"), PompaPupuk) ? "ok" : firebaseData.errorReason().c_str());
      while (true) {
        flowMeter();
        ReadFrdb();
        WaktuRtc();
        oledConn();
        Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Flow_sensor"), totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
        if (totalMilliLitres >= vol_pupuk1 || Pemupukan1 == "0") {
          PumpPupukMati();
          Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Pupuk"), PompaPupuk) ? "ok" : firebaseData.errorReason().c_str());
          break;
        }
      }
    }


    int jam2 = J_Jam_2.toInt();
    int mnt2 = J_Mnt_2.toInt();
    int S_pupuk2 = (jam1 * 60) + mnt1;
    int S_2_pupuk1 = S_pupuk2 + 60;
    int vol_pupuk2 = JFlowPupuk2.toInt();
    S_Pemupukan2 = String(S_pupuk2);

    if (Pemupukan2 == "1" && WaktuInt >= S_pupuk2 && WaktuInt < S_2_pupuk1 && vol_pupuk2  >= totalMilliLitres) {
      PumpPupukNyala();
      Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Pupuk"), PompaPupuk) ? "ok" : firebaseData.errorReason().c_str());
      while (true) {
        flowMeter();
        ReadFrdb();
        WaktuRtc();
        oledConn();
        Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Flow_sensor"), totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
        if (totalMilliLitres >= vol_pupuk2 || Pemupukan2 == "0") {
          PumpPupukMati();
          Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Pupuk"), PompaPupuk) ? "ok" : firebaseData.errorReason().c_str());
          break;
        }
      }
    }

    if ( (K_pupuk == "0" && (Pemupukan1 == "0" || S_pupuk1 > WaktuInt || WaktuInt > S_1_pupuk1)) && (K_pupuk == "0" && (Pemupukan2 == "0" || S_pupuk2 > WaktuInt || WaktuInt > S_2_pupuk1))) {
      PumpPupukMati();

    }

    int jam3 = J_Jam_3.toInt();
    int mnt3 = J_Mnt_3.toInt();
    int S_air1 = (jam3 * 60) + mnt3;
    int S_1_air1 = S_air1 + 60;
    int vol_Air1 = JFlowAir1.toInt();

    S_Penyiraman1 = String(S_air1);

    if (Penyiraman1 == "1" && WaktuInt >= S_air1 && WaktuInt < S_1_air1 && vol_Air1  >= totalMilliLitres ) {
      PumpAirNyala();
      Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), PompaAir) ? "ok" : firebaseData.errorReason().c_str());
      while (true) {
        flowMeter();
        ReadFrdb();
        WaktuRtc();
        oledConn();
        Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Flow_sensor"), totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
        if (totalMilliLitres >= vol_Air1 || Penyiraman1 == "0") {
          PumpAirMati();
          Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), PompaAir) ? "ok" : firebaseData.errorReason().c_str());
          break;
        }
      }
    }

    int jam4 = J_Jam_4.toInt();
    int mnt4 = J_Mnt_4.toInt();
    int S_air2 = (jam4 * 60) + mnt4;
    int S_2_air1 = S_air2 + 60;
    int vol_Air2 = JFlowAir2.toInt();

    S_Penyiraman2 = String(S_air2);

    if (Penyiraman2 == "1" && WaktuInt >= S_air2 && WaktuInt < S_2_air1 && vol_Air2  >= totalMilliLitres) {
      PumpAirNyala();
      Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), PompaAir) ? "ok" : firebaseData.errorReason().c_str());
      while (true) {
        flowMeter();
        ReadFrdb();
        WaktuRtc();
        oledConn();
        Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Flow_sensor"), totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
        if (totalMilliLitres >= vol_Air2) {
          PumpAirMati();
          Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), PompaAir) ? "ok" : firebaseData.errorReason().c_str());
          break;
        }
      }
    }

    if ((K_Air == "0" && (Penyiraman1 == "0" || S_air1 > WaktuInt || WaktuInt > S_1_air1)) && (K_Air == "0" && (Penyiraman1 == "0" || S_air2 > WaktuInt || WaktuInt > S_2_air1))) {
      PumpAirMati();
    }

    if (K_Air == "0" && !pumpAir && K_pupuk == "0" && !pumpNutrisi && ((Penyiraman1 == "1" && S_air1 > WaktuInt && WaktuInt > S_1_air1) || Penyiraman1 == "0" ) && ((Penyiraman2 == "1" && S_air2 > WaktuInt && WaktuInt > S_2_air1) || Penyiraman2 == "0" ) && ((Pemupukan1 == "1" && WaktuInt < S_pupuk1 && WaktuInt > S_1_pupuk1) || Pemupukan1 == "0" ) && ((Pemupukan2 == "1" && WaktuInt < S_pupuk2 && WaktuInt && WaktuInt > S_2_pupuk1) || Pemupukan2 == "0")) {
      UpdateFlow();
    }
    oledConn();

    if (Pemupukan1 == "1" ) {
      writeFile(SD, "/Pemupukan1.txt",  Pemupukan1);
      writeFile(SD, "/S_Pemupukan1.txt", S_Pemupukan1);
      writeFile(SD, "/S_VolPupuk1.txt", JFlowPupuk1);
    } else {
      Serial.println();
      writeFile(SD, "/Pemupukan1.txt",  Pemupukan1);
    }
    if (S_Pemupukan2 == "1") {

      writeFile(SD, "/S_Pemupukan2.txt", S_Pemupukan2);
      writeFile(SD, "/Pemupukan2.txt", Pemupukan2);
      writeFile(SD, "/S_VolPupuk2.txt", JFlowPupuk2);
    } else {
      Serial.println();
      writeFile(SD, "/S_Pemupukan2.txt", S_Pemupukan2);
    }
    if (S_Penyiraman1 == "1") {

      writeFile(SD, "/S_Penyiraman1.txt", S_Penyiraman1);
      writeFile(SD, "/Penyiraman1.txt", Penyiraman1);
      writeFile(SD, "/S_VolAir1.txt", JFlowAir1);
    } else {
      Serial.println();
      writeFile(SD, "/S_Penyiraman1.txt", S_Penyiraman1);
    }
    if (S_Penyiraman2 == "1") {

      writeFile(SD, "/S_Penyiraman2.txt", S_Penyiraman2);
      writeFile(SD, "/Penyiraman2.txt", Penyiraman2);
      writeFile(SD, "/S_VolAir2.txt", JFlowAir2);
    } else {
      Serial.println();
      writeFile(SD, "/S_Penyiraman2.txt", S_Penyiraman2);
    }

  }
}

void oledConn() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("WiFi:");
  display.print("Conn || ");
  display.println(DataWaktu);
  display.print("Kel Tanah : ");
  display.print(kel);
  display.println(" %");
  display.print("TDS       : ");
  display.print(tdsValue);
  display.println(" ppm");
  display.print("Pemupukan 1 : ");
  if (Pemupukan1 == "1" ) {
    display.print(J_Jam_1);
    display.print(":");
    display.println(J_Mnt_1);
  } else {
    display.println(" - ");
  }
  display.print("Pemupukan 2 : ");
  if (Pemupukan2 == "1" ) {
    display.print(J_Jam_2);
    display.print(":");
    display.println(J_Mnt_2);
  }  else {
    display.println(" - ");
  }
  display.print("Penyiraman 1 :");
  if (Penyiraman1 == "1") {
    display.print(J_Jam_3);
    display.print(":");
    display.println(J_Mnt_3);
  }  else {
    display.println(" - ");
  }
  display.print("Penyiraman 2 :");
  if (Penyiraman2 == "1") {
    display.print(J_Jam_4);
    display.print(":");
    display.println(J_Mnt_4);
  } else {
    display.println(" - ");
  }
  display.display();
}

unsigned long lastTime = 0;
void ReadFrdb() {
  unsigned long now = millis();
  if (now - lastTime > 100) {
    lastTime = now;
    // Cek perubahan data pada stream 1
    if (Firebase.readStream(firebaseData1)) {
      if (firebaseData1.streamAvailable()) {
        Serial.println("Stream1 data available:");
        if (firebaseData1.dataType() == "string") {
          K_Air = firebaseData1.stringData();
          Serial.println("K_Air: " + K_Air);
        }
      }
    }
    // Cek perubahan data pada stream 2
    if (Firebase.readStream(firebaseData2)) {
      if (firebaseData2.streamAvailable()) {
        Serial.println("Stream2 data available:");
        if (firebaseData2.dataType() == "string") {
          K_FlowAir = firebaseData2.stringData();
          Serial.println("K_FlowAir: " + K_FlowAir);
        }
      }
    }
    // Cek perubahan data pada stream 3
    if (Firebase.readStream(firebaseData3)) {
      if (firebaseData3.streamAvailable()) {
        Serial.println("Stream3 data available:");
        if (firebaseData3.dataType() == "string") {
          K_pupuk = firebaseData3.stringData();
          Serial.println("K_pupuk: " + K_pupuk);
        }
      }
    }

    // Cek perubahan data pada stream 4
    if (Firebase.readStream(firebaseData4)) {
      if (firebaseData4.streamAvailable()) {
        Serial.println("Stream4 data available:");
        if (firebaseData4.dataType() == "string") {
          K_FlowPupuk = firebaseData4.stringData();
          Serial.println("K_FlowPupuk: " + K_FlowPupuk);
        }
      }
    }

    // Cek perubahan data pada stream 5
    if (Firebase.readStream(firebaseData5)) {
      if (firebaseData5.streamAvailable()) {
        Serial.println("Stream5 data available:");
        if (firebaseData5.dataType() == "string") {
          Pemupukan1 = firebaseData5.stringData();
          Serial.println("Pemupukan1: " + Pemupukan1);
        }
      }
    }

    // Cek perubahan data pada stream 6
    if (Firebase.readStream(firebaseData6)) {
      if (firebaseData6.streamAvailable()) {
        Serial.println("Stream6 data available:");
        if (firebaseData6.dataType() == "string") {
          Pemupukan2 = firebaseData6.stringData();
          Serial.println("Pemupukan2: " + Pemupukan2);
        }
      }
    }

    // Cek perubahan data pada stream 7
    if (Firebase.readStream(firebaseData7)) {
      if (firebaseData7.streamAvailable()) {
        Serial.println("Stream7 data available:");
        if (firebaseData7.dataType() == "string") {
          Penyiraman1 = firebaseData7.stringData();
          Serial.println("Penyiraman1: " + Penyiraman1);
        }
      }
    }
    // Cek perubahan data pada stream 8
    if (Firebase.readStream(firebaseData8)) {
      if (firebaseData8.streamAvailable()) {
        Serial.println("Stream8 data available:");
        if (firebaseData8.dataType() == "string") {
          Penyiraman2 = firebaseData8.stringData();
          Serial.println("Penyiraman2: " + Penyiraman2);
        }
      }
    }
    delay(100);
    unsigned long GetString = millis();
    if (Pemupukan1 == "1" ) {
      for (int i = 0; i < 3; i++) {
        if ( i == 0) {
          if (Firebase.getString(firebaseData, nodePaths[2])) {
            JFlowPupuk1 = firebaseData.stringData();
            Serial.print("JFlowPupuk1 : ");
            Serial.println(JFlowPupuk1);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[2]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
        if (i == 1) {
          if (Firebase.getString(firebaseData, nodePaths[4])) {
            J_Jam_1 = firebaseData.stringData();
            Serial.print("J_Jam_1 : ");
            Serial.println(J_Jam_1);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[4]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
        if (i == 2) {
          if (Firebase.getString(firebaseData, nodePaths[8])) {
            J_Mnt_1 = firebaseData.stringData();
            Serial.print("J_Mnt_1 : ");
            Serial.println(J_Mnt_1);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[8]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
      }
    }

    if (Pemupukan2 == "1" ) {
      for (int i = 0; i < 3; i++) {
        if ( i == 0) {
          if (Firebase.getString(firebaseData, nodePaths[3])) {
            JFlowPupuk2 = firebaseData.stringData();
            Serial.print("JFlowPupuk2 : ");
            Serial.println(JFlowPupuk2);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[3]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
        if (i == 1) {
          if (Firebase.getString(firebaseData, nodePaths[5])) {
            J_Jam_2 = firebaseData.stringData();
            Serial.print("J_Jam_2 : ");
            Serial.println(J_Jam_2);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[5]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
        if (i == 2) {
          if (Firebase.getString(firebaseData, nodePaths[9])) {
            J_Mnt_2 = firebaseData.stringData();
            Serial.print("J_Mnt_2 : ");
            Serial.println(J_Mnt_2);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[9]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
      }
    }

    if (Penyiraman1 == "1" ) {
      for (int i = 0; i < 3; i++) {
        if ( i == 0) {
          if (Firebase.getString(firebaseData, nodePaths[0])) {
            JFlowAir1 = firebaseData.stringData();
            Serial.print("JFlowAir1 : ");
            Serial.println(JFlowAir1);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[0]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
        if (i == 1) {
          if (Firebase.getString(firebaseData, nodePaths[6])) {
            J_Jam_3 = firebaseData.stringData();
            Serial.print("J_Jam_3 : ");
            Serial.println(J_Jam_3);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[6]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
        if (i == 2) {
          if (Firebase.getString(firebaseData, nodePaths[10])) {
            J_Mnt_3 = firebaseData.stringData();
            Serial.print("J_Mnt_3 : ");
            Serial.println(J_Mnt_3);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[10]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
      }
    }

    if (Penyiraman2 == "1" ) {
      for (int i = 0; i < 3; i++) {
        if ( i == 0) {
          if (Firebase.getString(firebaseData, nodePaths[1])) {
            JFlowAir2 = firebaseData.stringData();
            Serial.print("JFlowAir2 : ");
            Serial.println(JFlowAir2);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[1]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
        if (i == 1) {
          if (Firebase.getString(firebaseData, nodePaths[7])) {
            J_Jam_4 = firebaseData.stringData();
            Serial.print("J_Jam_4 : ");
            Serial.println(J_Jam_4);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[7]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
        if (i == 2) {
          if (Firebase.getString(firebaseData, nodePaths[11])) {
            J_Mnt_4 = firebaseData.stringData();
            Serial.print("J_Mnt_4 : ");
            Serial.println(J_Mnt_4);
          } else {
            Serial.print("Failed to get value for ");
            Serial.println(nodePaths[11]);
            Serial.print("Reason: ");
            Serial.println(firebaseData.errorReason());
          }
        }
      }
    }

  }

}

/*------------------Penjadwalan----------------------*/

void OledNoConec() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("WiFi:");
  display.println("Disconn || ");
  display.println(DataWaktu);
  display.print("Kel Tanah : ");
  display.print(kel);
  display.println(" %");
  display.print("TDS       : ");
  display.print(tdsValue);
  display.println(" ppm");
  display.print("Pemupukan 1 : ");
  if (pemupukan_1 == '1' ) {
    display.print(J_Jam_1);
    display.print(":");
    display.println(J_Mnt_1);
  } else {
    display.println(" - ");
  }
  display.print("Pemupukan 2 : ");
  if (pemupukan_2 == '1') {
    display.print(J_Jam_2);
    display.print(" : ");
    display.println(J_Mnt_2);
  } else {
    display.println(" - ");
  }
  display.print("Penyiraman 1: ");
  if (penyiraman_1 == '1') {
    display.print(J_Jam_3);
    display.print(":");
    display.println(J_Mnt_3);
  } else {
    display.println(" - ");
  }
  display.print("Penyiraman 2: ");
  if (penyiraman_2 == '1') {
    display.print(J_Jam_4);
    display.print(":");
    display.println(J_Mnt_4);
  } else {
    display.println(" - ");
  }
  display.display();
}

void penjadwalan() {
  int WaktuInt = Waktu.toInt();
  K_pupuk = "0";
  K_Air = "0";

  readFilePemupukan_1(SD, "/Pemupukan1.txt");
  readFileS_Pemupukan1(SD, "/S_Pemupukan1.txt");
  readFileS_VolPupuk1(SD, "/S_VolPupuk1.txt");
  int vol_pupuk1 = S_VolPupuk1.toInt();
  int J_1_pupuk = J_1_Pemupukan.toInt();
  int J_1_pupuk1 = J_1_pupuk + 60;

  if (pemupukan_1 == '1' && WaktuInt >= J_1_pupuk && WaktuInt < J_1_pupuk1 && vol_pupuk1  >= totalMilliLitres) {
    PumpPupukNyala();
    while (true) {
      flowMeter();
      WaktuRtc();
      OledNoConec();
      Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Flow_sensor"), totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
      if (totalMilliLitres >= vol_pupuk1 || pemupukan_1 == '0') {
        PumpAirMati();
//        Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), PompaAir) ? "ok" : firebaseData.errorReason().c_str());
        break;
      }
    }
  }

  readFilePemupukan_2(SD, "/Pemupukan2.txt");
  readFileS_Pemupukan2(SD, "/S_Pemupukan2.txt");
  readFileS_VolPupuk2(SD, "/S_VolPupuk2.txt");
  int vol_pupuk2 = S_VolPupuk2.toInt();
  int J_2_pupuk = J_2_Pemupukan.toInt();
  int J_2_pupuk1 = J_2_pupuk + 60;

  if (pemupukan_2 == '1' && WaktuInt >= J_2_pupuk && WaktuInt < J_2_pupuk1 && vol_pupuk2  >= totalMilliLitres) {
    PumpPupukNyala();
    while (true) {
      flowMeter();
      WaktuRtc();
      OledNoConec();
      Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Flow_sensor"), totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
      if (totalMilliLitres >= vol_pupuk2 || pemupukan_2 == '0' ) {
        PumpAirMati();
//        Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), PompaAir) ? "ok" : firebaseData.errorReason().c_str());
        break;
      }
    }

  }

  if ((K_pupuk == "0" && (pemupukan_1 == '0' || J_1_pupuk > WaktuInt || WaktuInt > J_1_pupuk1)) && (K_pupuk == "0" && (pemupukan_2 == '0' || J_2_pupuk > WaktuInt || WaktuInt > J_2_pupuk1))) {
    PumpPupukMati();
  }

  readFilePenyiraman_1(SD, "/Penyiraman1.txt");
  readFileS_Penyiraman1(SD, "/S_Penyiraman1.txt");
  readFileS_VolAir1(SD, "/S_VolAir1.txt");
  int vol_Air1 = S_VolAir1.toInt();
  int J_1_Air = J_1_Penyiraman.toInt();
  int J_1_Air1 = J_1_Air + 60;
  if (penyiraman_1 == '1' && WaktuInt >= J_1_Air && WaktuInt < J_1_Air1 && vol_Air1  >= totalMilliLitres) {
    PumpAirNyala();
    while (true) {
      flowMeter();
      WaktuRtc();
      OledNoConec();
      Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Flow_sensor"), totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
      if (totalMilliLitres >= vol_Air1 || penyiraman_1 == '0' ) {
        PumpAirMati();
//        Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), PompaAir) ? "ok" : firebaseData.errorReason().c_str());
        break;
      }
    }

  }
  readFilePenyiraman_2(SD, "/Penyiraman2.txt");
  readFileS_Penyiraman2(SD, "/S_Penyiraman2.txt");
  readFileS_VolAir2(SD, "/S_VolAir2.txt");
  int vol_Air2 = S_VolAir2.toInt();
  int J_2_Air = J_2_Penyiraman.toInt();
  int J_2_Air1 = J_2_Air + 60;
  if (penyiraman_2 == '1' && WaktuInt >= J_2_Air && WaktuInt < J_2_Air1 && vol_Air2  >= totalMilliLitres) {
    PumpAirNyala();
    while (true) {
      flowMeter();
      WaktuRtc();
      OledNoConec();
      Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Flow_sensor"), totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
      if (totalMilliLitres >= vol_Air2 || penyiraman_2 == '0' ) {
        PumpAirMati();
//        Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), PompaAir) ? "ok" : firebaseData.errorReason().c_str());
        break;
      }
    }
  }
  if ((K_Air == "0" && (penyiraman_1 == '0' || J_1_Air > WaktuInt || WaktuInt > J_1_Air1)) && (K_Air == "0" && (penyiraman_2 == '0' || J_2_Air > WaktuInt || WaktuInt > J_2_Air1))) {
    PumpAirMati();
  }
  
  if ( (pemupukan_1 == '0' || (pemupukan_1 == '1' &&  J_1_pupuk > WaktuInt && WaktuInt > J_1_pupuk1)) && (pemupukan_2 == '0' || (pemupukan_2 == '1' &&  J_2_pupuk > WaktuInt && WaktuInt > J_2_pupuk1)) && (penyiraman_1 == '0' || (penyiraman_1 == '1' && J_1_Air > WaktuInt && WaktuInt> J_1_Air1)) && (penyiraman_2 == '0' || (penyiraman_2 == '1' && J_2_Air > WaktuInt && WaktuInt > J_2_Air1))) {
    UpdateFlow();
  }

  OledNoConec();
}

/*--------Pump Fertigasi--------------------------*/

void PumpAirMati() {
  pumpAir = false;
  digitalWrite(pump_air, HIGH);
  PompaAir = 0;
  //  Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), 0) ? "ok" : firebaseData.errorReason().c_str());
  //  Serial.printf("Set M_Pump_Air....", Firebase.setInt(firebaseData, "/Fertigasi/M_Pump_Air", 0) ? "ok" : firebaseData.errorReason().c_str());
  Serial.println();
}

void PumpAirNyala() {
  if (!pumpAir) {
    UpdateFlow();
    pumpAir = true;
  }
  //  flowMeter();
  digitalWrite(pump_air, LOW);
  PompaAir = 1;
  //  Serial.printf("Set M_Flow_sensor ...", Firebase.setInt(firebaseData, "/Fertigasi/M_Flow_sensor ", totalMilliLitres) ? "ok" : firebaseData.errorReason().c_str());
  //    Serial.printf("Set int... %s\n", Firebase.setInt(firebaseData, F("/Fertigasi/M_Pump_Air"), 1) ? "ok" : firebaseData.errorReason().c_str());
  //  Serial.printf("Set M_Pump_Air....", Firebase.setInt(firebaseData, "/Fertigasi/M_Pump_Air", 1) ? "ok" : firebaseData.errorReason().c_str());
  Serial.println();
}

void PumpPupukMati() {
  pumpNutrisi = false;
  digitalWrite(pump_nutrisi, HIGH);
  Serial.println();
  PompaPupuk = 0;
}

void PumpPupukNyala() {
  if (!pumpNutrisi) {
    UpdateFlow();
    pumpNutrisi = true;
  }
  //  flowMeter();
  digitalWrite(pump_nutrisi, LOW);
  Serial.println();
  PompaPupuk = 1;
}

/*-----Membaca dan menyimpan data di sd_card------------------------*/

void writeFile(fs::FS & fs, const char * path, String message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void readFilePemupukan_1 (fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.print("Read from file: ");
  while (file.available()) {
    pemupukan_1 = file.read(); // Baca satu karakter dari file
    Serial.println(pemupukan_1);
  }
  file.close();
  delay(300);
}

void readFilePemupukan_2 (fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.print("Read from file: ");
  while (file.available()) {
    pemupukan_2 = file.read(); // Baca satu karakter dari file
    Serial.println(pemupukan_2);
  }
  file.close();
  delay(300);
}

void readFilePenyiraman_1(fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.print("Read from file: ");
  while (file.available()) {
    penyiraman_1 = file.read(); // Baca satu karakter dari file
    Serial.println(penyiraman_1);
  }
  file.close();
  delay(300);
}

void readFilePenyiraman_2(fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.print("Read from file: ");
  while (file.available()) {
    penyiraman_2 = file.read(); // Baca satu karakter dari file
    Serial.println(penyiraman_2);
  }
  file.close();
  delay(300);
}

void readFileS_Pemupukan1(fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    J_1_Pemupukan = file.readStringUntil('\n'); // Baca satu karakter dari file
    Serial.println(J_1_Pemupukan);
  }
  file.close();
}

void readFileS_Pemupukan2(fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    J_2_Pemupukan = file.readStringUntil('\n'); // Baca satu karakter dari file
    Serial.println(J_2_Pemupukan);
  }
  file.close();
}

void readFileS_Penyiraman1(fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    J_1_Penyiraman = file.readStringUntil('\n'); // Baca satu karakter dari file
    Serial.println(J_1_Penyiraman);
  }
  file.close();
}

void readFileS_Penyiraman2(fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    J_2_Penyiraman = file.readStringUntil('\n'); // Baca satu karakter dari file
    Serial.println(J_2_Penyiraman);
  }
  file.close();
}

void readFileS_VolPupuk1(fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    S_VolPupuk1 = file.readStringUntil('\n'); // Baca satu karakter dari file
    Serial.println(S_VolPupuk1);
  }
  file.close();
}

void readFileS_VolPupuk2(fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    S_VolPupuk2 = file.readStringUntil('\n'); // Baca satu karakter dari file
    Serial.println(S_VolPupuk2);
  }
  file.close();
}

void readFileS_VolAir1   (fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    S_VolAir1 = file.readStringUntil('\n'); // Baca satu karakter dari file
    Serial.println(S_VolAir1);
  }
  file.close();
}

void readFileS_VolAir2   (fs::FS & fs, const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    S_VolAir2 = file.readStringUntil('\n'); // Baca satu karakter dari file
    Serial.println(S_VolAir2);
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, String message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
