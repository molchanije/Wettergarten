//Bibliotheken einbinden
#include <FastLED.h>
#include <ArduinoJson.h>                  
#include <math.h>                         
#include <WiFiManager.h>  

//Konstanten deklarieren, benutzte Pins definieren                
#define BRIGHTNESS 64
#define DATA_PIN D3 
#define NUM_LEDS 8   

CRGB leds[NUM_LEDS];

WiFiManager wifiManager;
WiFiClient client;

// ========================  hier deinen API-Key eintragen!!!  ============================================================================================================
const String city = "Oldenburg";
const String api_key = "4c4d7d7572e47f1c20830e57c092a434";    // dein Open-Weather-Map-API-Schluessel, kostenlos beziehbar ueber https://openweathermap.org/
// ========================================================================================================================================================================

//Variablen deklarieren und initialisieren, die die z.B. Wetter-ID und die Temperatur speichern sollen 
int weatherID = 0;
String weatherforecast_shortened = " ";
int temperature_Celsius_Int = 0;
unsigned long lastcheck = 0;
int LEDnumber; 
                  

//============================ setup =================================================
void setup() {
  Serial.begin(9600);   

 //Einstellungen für die Fast.LED
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 400);
  FastLED.clear();
  FastLED.show();

  leds[1] = CRGB::Blue;
  leds[4] = CRGB::Green;
  leds[7] = CRGB::Red;
  FastLED.show();

  //Verbindung zum Access Point
  wifiManager.autoConnect("deineWetterLampe");

  //LEDs ausschalten
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB( 0, 0, 0);
  }
  FastLED.show();
  delay(2000);
  getCurrentWeatherConditions();
}

//======================= loop ===============================================================
void loop() {
  
  //das aktuelle Wetter wird alle 10 Sekunden abgefragt
  if (millis() - lastcheck >= 10000) {         
    getCurrentWeatherConditions();
    lastcheck = millis();                         
  }
        
  //Lichtanimationen bei bestimmten Wetterzuständen abspielen
    if ((weatherID == 800) || (weatherID == 801)){
      LED_animation_clearSky();
    } else if ((weatherID == 211) || (weatherID == 212) || (weatherID == 221) || (weatherID == 781)) {
      LED_animation_thunderstorm();
    } else if ((weatherID == 201) || (weatherID == 200) || (weatherID == 202) || (weatherID == 230) || (weatherID == 231) || (weatherID == 232)) {
      LED_animation_thunderstorm_with_rain();
    } else if (weatherID/100 == 3) {
      LED_animation_drizzle();
    } else if ((weatherID == 500) || (weatherID == 501) || (weatherID == 520) || (weatherID == 521)) {
      LED_animation_rain();
    } else if ((weatherID == 502) || (weatherID == 503) || (weatherID == 504) || (weatherID == 511) || (weatherID == 522) || (weatherID == 531)) {
      LED_animation_heavy_rain();
    } else if (weatherID/100 == 6) {
      LED_animation_snow();
    } else if ((weatherID == 701) || (weatherID == 711) || (weatherID == 721) || (weatherID == 731) || (weatherID == 741) || (weatherID == 761) || (weatherID == 762)) {
      LED_animation_dust();
    } else if ((weatherID == 802) || (weatherID == 803) || (weatherID == 804)) {
      LED_animation_cloudy();
    } else if ((weatherID == 771) || (weatherID == 210)) {
      LED_animation_squall();
    } else {
      LED_error_animation();  //falls die Wetter-ID fehlerhaft ist oder nicht mit den angegebenen übereinstimmt wird die Error-Lichtanimation eingespielt 
  }
}

//================= Methode ruft aktuelle Wetterlage ab ==============================================
void getCurrentWeatherConditions() {
  int WeatherData;

  //Verbindung zu api.openweathermap.org herstellen
  Serial.print("connecting to "); Serial.println("api.openweathermap.org");
  if (client.connect("api.openweathermap.org", 80)) {
    //Wetter für eine engegebene Stadt mithilfe des API-Keays abfragen
    client.println("GET /data/2.5/weather?q=" + city + ",DE&units=metric&lang=de&APPID=" + api_key);
    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed");
  }
  //JSON Dokument wird dekodiert
  const size_t capacity = JSON_ARRAY_SIZE(2) + 2 * JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(14) + 360;
  DynamicJsonDocument doc(capacity);
  deserializeJson(doc, client);
  client.stop();

  //aktuelle Wetter-ID und Temperatur in Variablen speichern
  weatherID = doc["weather"][0]["id"];
   int temperature_Celsius = doc["main"]["temp"];
   temperature_Celsius_Int = (int)temperature_Celsius;

  //Kurzbezeichnungen für Wetterzustände definieren
  if ((weatherID == 800) ||(weatherID == 801)) {
    weatherforecast_shortened = "klar"; 
  } else if ((weatherID == 211) || (weatherID == 212) || (weatherID == 221) || (weatherID == 781)) {
    weatherforecast_shortened = "Gewitter";
  } else if ((weatherID == 201) || (weatherID == 200) || (weatherID == 202) || (weatherID == 230) || (weatherID == 231) || (weatherID == 232)) {
    weatherforecast_shortened = "Gewitter mit Regen";
  } else if (weatherID/100 == 3) {
    weatherforecast_shortened = "Nieselregen";
  } else if ((weatherID == 500) || (weatherID == 501) || (weatherID == 520) || (weatherID == 521)) {
    weatherforecast_shortened = "Regen";
  } else if ((weatherID == 502) || (weatherID == 503) || (weatherID == 504) || (weatherID == 511) || (weatherID == 522) || (weatherID == 531)) {
    weatherforecast_shortened = "Starker Regen";
  } else if (weatherID/100 == 6) {
    weatherforecast_shortened = "Schnee";
  } else if ((weatherID == 701) || (weatherID == 711) || (weatherID == 721) || (weatherID == 731) || (weatherID == 741) || (weatherID == 761) || (weatherID == 762)) {
    weatherforecast_shortened = "Nebel/Dunst";
  } else if ((weatherID == 802) || (weatherID == 803) || (weatherID == 804)) {
    weatherforecast_shortened = "Wolken";
  } else if ((weatherID == 771) || (weatherID == 210)) {
    weatherforecast_shortened = "Böe";
  } else {
    weatherforecast_shortened = "Error";
  }

  //Bestimmen, welche LED leuchten soll - in Abhängigkeit von der Temperatur
  if (temperature_Celsius_Int <= 5) {  //bei Temperaturen von unter oder gleich 5°C leuchtet die LED mit dem Index 1, also die 'Eisblume'
    LEDnumber = 1;
    leds[4] = CRGB::Black;
    leds[7] = CRGB::Black;
  } else if (temperature_Celsius_Int > 5 && temperature_Celsius_Int <= 15) {  //bei temperaturen zwischen 6 und 15°C leuchtet die LED Nummer 4, also die 'Heckenrose'
    LEDnumber = 4;
    leds[1] = CRGB::Black;
    leds[7] = CRGB::Black;
  } else if (temperature_Celsius_Int > 15) {  //bei Temperaturen von über 15°C leuchtet die LED Nummer 7, also die 'Sonnenblume'  
    LEDnumber = 7;
    leds[1] = CRGB::Black;
    leds[4] = CRGB::Black;
  } else {
    LED_error_animation();
  }
  //Ausgabe im Seriellen Monitor zu den aktuellen Zuständen (WetterID, Wetter-Kurzbezeichnung, Temperatur, Nummer der leuchtenden LED) 
  Serial.println("Wetter: " + weatherforecast_shortened + "; Wetter-ID: " + weatherID + "; Temperatur: " + temperature_Celsius + "°C; LED-Nummer: " + LEDnumber);
}


//==================== Lichtanimationen ================================================

//klarer Himmel --------------------------------------------------------------------------------------
//das Licht der LED wechselt in einem sanften Übergang von Blau zu Gelb und zurück
void LED_animation_clearSky() { 
  FastLED.setBrightness(255);
  //von blau zu gelb
  for( int i = 0; i < 256; i++ ) {
      int r = i;  
      int b = 255 - i;  
      int g = i;              
      leds[LEDnumber] = CRGB(r,g,b);
      FastLED.show();
      delay(20); 
  }
  //von gelb zu blau
  for( int i = 0; i < 256; i++ ) {
      int r = 255 - i;  
      int b = i;  
      int g = 255 - i;              
      leds[LEDnumber] = CRGB(r,g,b);
      FastLED.show();
      delay(20); 
  }
}

//Gewitter -------------------------------------------------------------------------------------------
//die LED leuchtet 'blitzartig' in Weiß und Gelb auf, unterbrpchen von längeren Schwarz-Phasen
void LED_animation_thunderstorm() {
  FastLED.setBrightness(255);
  leds[LEDnumber] = CRGB::White;
  FastLED.show();
  delay(400);
  fade_brightness(2, 30);  
  delay(10);
  leds[LEDnumber] = CRGB::Yellow;
  FastLED.show();
  delay(200);
  fade_brightness(2, 30);  
  delay(10);
  leds[LEDnumber] = CRGB::Black;
  FastLED.show();
  delay(1000);
  leds[LEDnumber] = CRGB::Yellow;
  FastLED.show();
  delay(500);
  leds[LEDnumber] = CRGB::White;
  FastLED.show();
  delay(100);
  fade_brightness(2, 30);
  delay(10);
  leds[LEDnumber] = CRGB::Black;
  FastLED.show();
  delay(1000);
  leds[LEDnumber] = CRGB::Yellow;
  FastLED.show();
  delay(400);
  fade_brightness(2, 30);
  delay(10);
}

//Gewitter mit Regen ------------------------------------------------------------------------------------
//wie bei der Gewitter-Lichtanimation mit dem Unterschied, dass Schwarz-Phasen durch ein Blinken in Blau ersetzt wurde
void LED_animation_thunderstorm_with_rain() {
  FastLED.setBrightness(255);
  blink_blue_black(65, 5);
  leds[LEDnumber] = CRGB::White;
  FastLED.show();
  delay(400);
  fade_brightness(2, 30);
  delay(10);
  leds[LEDnumber] = CRGB::Yellow;
  FastLED.show();
  delay(200);
  fade_brightness(2, 30);
  delay(10);
  FastLED.setBrightness(100);
  blink_blue_black(65, 5);
  leds[LEDnumber] = CRGB::Yellow;
  FastLED.show();
  delay(500);
  leds[LEDnumber] = CRGB::White;
  FastLED.show();
  delay(100);
  fade_brightness(2, 30);
  delay(10);
  FastLED.setBrightness(100);
  blink_blue_black(65, 5);
  leds[LEDnumber] = CRGB::Yellow;
  FastLED.show();
  delay(400);
  fade_brightness(2, 30);
  delay(10);
}

//Nieselregen -----------------------------------------------------------------------------------
//Die LED blinkt schnell in Blau und Grau auf, dabei ändert sich die Frequenz des Blinkens 
void LED_animation_drizzle() {
  FastLED.setBrightness(100);
  blink_blue_grey(30, 10);
  blink_blue_grey(60, 10);
}

//Regen --------------------------------------------------------------------------------------------
//Wie beim Nieselregen, nur langsamer
void LED_animation_rain() {
  FastLED.setBrightness(100);
  blink_blue_grey(150, 5);
  blink_blue_grey(200, 5);
}

//starker Regen ------------------------------------------------------------------------------------
//Die LED blinkt in Blau-Schwarz, Blau-Weiß und Blau-Grau auf, wobei die frequenz des Blinkens variiert
void LED_animation_heavy_rain() {
  FastLED.setBrightness(100);
  blink_blue_black(40, 10);
  blink_blue_white(50, 3);
  blink_blue_grey(100, 8);
}

//Schnee -------------------------------------------------------------------------------------------
//Die LED blinkt langsam in Weiß und Grau auf, die sich mit Schwarzen Episoden abwechseln
void LED_animation_snow() {
  leds[LEDnumber] = CRGB::White;  
  increase_brightness(2, 70);
  FastLED.show();
  delay(50);
  fade_brightness(2, 70);
  leds[LEDnumber] = CRGB::Black;
  FastLED.show();
  delay(10);
  leds[LEDnumber] = CRGB::Grey;
  increase_brightness(2, 70);
  FastLED.show();
  delay(50);
  fade_brightness(2, 70);
  leds[LEDnumber] = CRGB::Black;
  FastLED.show();
  delay(10);
}

//Nebel, Dunst und Staub --------------------------------------------------------------------------------
//Die LED blinkt in Weiß auf, wobei die Helligkeit variiert, aber nie vollständig erreicht oder abgeschaltet wird
void LED_animation_dust() {
  leds[LEDnumber] = CRGB::White;
  //Helligkeit nimmt zu
  for( int i = 0; i < 111; i++) {
      FastLED.setBrightness(150 - i);            
      FastLED.show();
      delay(30); 
  }
  //Helligkeit nimmt ab
  for( int i = 0; i < 111; i++) {
      leds[LEDnumber] = CRGB::White;
      FastLED.setBrightness(40 + i);            
      FastLED.show();
      delay(30); 
    }
}

//Bewölkter Himmel -------------------------------------------------------------------------
//Das Licht geht sanft von Blau zu Weiß und andersrum, wobei die Helligkeit zwischenzeitlich heruntergesetzt wird
void LED_animation_cloudy() {
  FastLED.setBrightness(255);
  //von blau zu weiß, Helligkeit nimmt ab
  for( int i = 0; i < 256; i++ ) {
      int r = 0 + i;  
      int b = 255;  
      int g = 0 + i;              
      leds[LEDnumber] = CRGB(r,g,b);
      FastLED.setBrightness(255 - i);
      FastLED.show();
      delay(10); 
  }
  delay(10);
  //von weiß zu blau, Helligkeit nimmt zu
  for( int i = 0; i < 256; i++ ) {
      int r = 255 - i;  
      int b = 255;  
      int g = 255 - i;              
      leds[LEDnumber] = CRGB(r,g,b);
      FastLED.setBrightness(0 + i);
      FastLED.show();
      delay(10); 
  }
  //von blau zu weiß, Helligkeit konstant
  for( int i = 0; i < 256; i++ ) {
      int r = 0 + i;  
      int b = 255;  
      int g = 0 + i;              
      leds[LEDnumber] = CRGB(r,g,b);
      FastLED.show();
      delay(10); 
  }
  delay(1000);
  //von weiß zu blau, Helligkeit konstant
  for( int i = 0; i < 256; i++ ) {
      int r = 255 - i;  
      int b = 255;  
      int g = 255 - i;              
      leds[LEDnumber] = CRGB(r,g,b);
      FastLED.show();
      delay(10); 
  }
}

//Böen -------------------------------------------------------------------------------------
//Blau wechselt sich zügig mit Violett ab
void LED_animation_squall() {
  FastLED.setBrightness(255);
  //von blau zu lila
  for( int i = 0; i < 121; i++ ) {
      int r = i;  
      int b = 255 - i;  
      int g = 0;              
      leds[LEDnumber] = CRGB(r,g,b);
      FastLED.show();
      delay(5); 
  }
  //von lila zu blau
  for( int i = 0; i < 121; i++ ) {
      int r = 170 - i;  
      int b = 135 + i;  
      int g = 0;              
      leds[LEDnumber] = CRGB(r,g,b);
      FastLED.show();
      delay(5); 
  }
}

//========================== Hilfsmethoden ===================================================
//Helligkeit nimmt ab, ermöglicht sanfte Übergänge
void fade_brightness(int factor, int del) {
for (int i = 255; i > 0; i = (int)i/factor) {
    FastLED.setBrightness(i);
    FastLED.show();
    delay(del);
  }
}

//Helligkeit nimmt zu, ermöglicht sanfte Übergänge
void increase_brightness(int factor, int del) {
for (int i = 1; i < 255; i = (int)i*factor) {
    FastLED.setBrightness(i);
    FastLED.show();
    delay(del);
  }
}

// LED blinkt blau-weiß
void blink_blue_white(int frequency, int rows) {
  FastLED.setBrightness(255);
  for (int i = 0; i < rows; i++) {
    leds[LEDnumber] = CRGB::Blue;
    FastLED.show();
    delay(frequency);
    leds[LEDnumber] = CRGB::White;
    FastLED.show();
    delay(frequency);
  }
}

// LED blinkt blau-grau
void blink_blue_grey(int frequency, int rows) {
  for (int i = 0; i < rows; i++) {
    leds[LEDnumber] = CRGB::Blue;
    FastLED.show();
    delay(frequency);
    leds[LEDnumber] = CRGB::Grey;
    FastLED.show();
    delay(frequency);
  }
}

// LED blinkt blau-schwarz 
void blink_blue_black(int frequency, int rows) {
  for (int i = 0; i < rows; i++) {
    leds[LEDnumber] = CRGB::Blue;
    FastLED.show();
    delay(frequency);
    leds[LEDnumber] = CRGB::Black;
    FastLED.show();
    delay(frequency);
  }
}

// Lichtanimation bei Fehlermeldung
void LED_error_animation() {
  FastLED.setBrightness(255);
  leds[7] = CRGB::Red;
  leds[4] = CRGB::Red;
  leds[1] = CRGB::Red;
  FastLED.show();
  delay(500);
  leds[7] = CRGB::Black;
  leds[4] = CRGB::Black;
  leds[1] = CRGB::Black;
  FastLED.show();
  delay(500);
}
