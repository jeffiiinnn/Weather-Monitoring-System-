#include <ESP8266WiFi.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThingSpeak.h>

// ----------------- WiFi and ThingSpeak -----------------
const char* ssid = "BSNL";
const char* password = "4792348408";

unsigned long myChannelNumber = 3120929; // ThingSpeak channel
const char* myWriteAPIKey = "AYBSMAFJDM44KFIC";

WiFiClient client;

// ----------------- DHT11 -----------------
#define DHTPIN D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ----------------- Sensors -----------------
#define LDR_PIN A0           // LDR analog pin
#define RAIN_PIN D4          // Rain sensor digital output

// ----------------- LCD -----------------
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16x2 LCD

void setup() {
  Serial.begin(115200);
  
  // LCD setup
  Wire.begin(D2, D1); // SDA, SCL
  lcd.init();
  lcd.backlight();

  // Sensors setup
  dht.begin();
  pinMode(RAIN_PIN, INPUT);
  
  // WiFi connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  ThingSpeak.begin(client);
}

void loop() {
  // ---- Read DHT11 ----
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // ---- Read LDR ----
  int ldrRaw = analogRead(LDR_PIN); // 0-1023
  String lightStatus = (ldrRaw < 800) ? "Bright" : "Dark"; // Adjust threshold if needed

  // ---- Read Rain Sensor ----
  int rainDigital = digitalRead(RAIN_PIN); // 0 = Rain, 1 = Dry
  String rainStatus = (rainDigital == 0) ? "Rainy" : "Dry";

  // ---- Display on LCD ----
  lcd.clear();
  if (isnan(temp) || isnan(hum)) {
    lcd.setCursor(0,0);
    lcd.print("DHT Error");
  } else {
    lcd.setCursor(0,0);
    lcd.print("T:"); lcd.print(temp,1); lcd.print("C ");
    lcd.print("H:"); lcd.print(hum,0); lcd.print("%");
  }
  
  lcd.setCursor(0,1);
  lcd.print("L:"); lcd.print(ldrRaw);
  lcd.print(" "); lcd.print(lightStatus);
  lcd.print(" R:"); lcd.print(rainStatus);

  // ---- Print to Serial ----
  Serial.print("Temp: "); Serial.print(temp); Serial.print(" °C ");
  Serial.print("Humidity: "); Serial.print(hum); Serial.println(" %");
  Serial.print("LDR: "); Serial.print(ldrRaw); Serial.print(" "); Serial.print(lightStatus);
  Serial.print(" Rain: "); Serial.println(rainStatus);

  // ---- Upload to ThingSpeak ----
  if (!isnan(temp) && !isnan(hum)) {
    ThingSpeak.setField(1, temp);
    ThingSpeak.setField(2, hum);
    ThingSpeak.setField(3, ldrRaw);
    ThingSpeak.setField(4, rainDigital);

    int response = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if(response == 200){
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(response));
    }
  }

  delay(20000); // 20 seconds delay (ThingSpeak free limit)
}