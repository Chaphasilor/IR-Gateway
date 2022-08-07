#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <IRremoteESP8266.h>
#include <IRsend.h> // Needed if you want to send IR commands.
#include <NetworkCredentials.h>
#include <Wire.h>

//#region SETTINGS

int transmitPin433 = 12; // GPIO12 is transmit-pin for 433 MHz transmitter
int transmitPin868 = 14; // GPIO14 is transmit-pin for 868 MHz transmitter, see below
int transmitPinIR = 13; // GPIO13 is transmit-pin for infrared transmitter

//#endregion SETTINGS

/*
  GPIO Pin Usage:
  ---------------
  GPIO1:  default
  GPIO3:  default
  GPIO4:  default
  GPIO5:  DHC11 Sensor
  GPIO6:  default
  GPIO7:  default
  GPIO8:  default
  GPIO9:  default
  GPIO10: default
  GPIO11: default
  GPIO12: transmitPin433, Sende-Pin für den 433 MHz Transmitter
  GPIO13: IR-LED
  GPIO14: transmitPin868, Sende-Pin für den 868 MHz Transmitter (Garden Gate)
  GPIO15: default
  GPIO16: UP Button (with 1k Pullup Resistor
*/

ESP8266WebServer server(80); // set up HTTP-Server
ESP8266HTTPUpdateServer httpUpdater;

String webPage = "<!DOCTYPE html> <html lang='en'> <head> <meta charset='UTF-8'> <meta name='viewport' content='width=device-width, initial-scale=1.0'> <title>IR Gateway</title> <style> table#t01 { font-size: 1.5em; border: none; width: 80%; margin-left: 10%; margin-right: 10%; } table #t02 { font - size : 1.5em; border: none; width: 80 % ; margin - left : 10 % ; margin - right : 10 % ; } table #t03 { font - size : 1.5em; border: none; width: 80 % ; margin - left : 10 % ; margin - right : 10 % ; } </ style></ head><body><h1 style = 'font-size: 2vw'> ESP8266 Web Server v16 by B.M.</ h1><table id = 't03'><caption style = 'font-size: 5vw'> Audio / Video Control</ caption><tr><th><a href = 'PanaTV_ONOFF'><button style = 'font-size: 2.0em; color:Red'> ON / OFF</ button></ a> &nbsp; </ a></ th><th><a href = 'SONYAV_free'></ a></ th><th><a href = 'PanaTV_INPUT'><button style = 'font-size: 2.0em; color:LightBlue'> INPUT</ button></ a></ th><th><a href = 'SONYAV_free'></ a></ th><th><a href = 'CREEK_MUTE'><button style = 'font-size: 2.0em; color:Blue'> MUTE</ button></ a></ th></ tr><tr><th><a href = 'PTV_free'></ a></ th><th><a href = 'PTV_free'></ a></ th><th><a href = 'PTV_free'><button style = 'font-size: 2.0em; color:White'></ button></ a></ th><th><a href = 'PTV_free'></ a></ th><th><a href = 'PTV_free'></ a></ th></ tr><tr><th><a href = 'CREEK_SEL_UP'><button style = 'font-size: 2.0em; color:Orange'> SEL + </ button></ a> &nbsp; </ a></ th><th><a href = 'PTV_free'></ a></ th><th><a href = 'CREEK_SEL_UP'><button style = 'font-size: 2.0em; background-color:Grey'> UP</ button></ a></ th><th><a href = 'PTV_free'></ a></ th><th><a href = 'CREEK_VOL_UP'><button style = 'font-size: 2.0em; color:Blue'> V + </ button></ a></ th></ tr><tr><th><a href = 'CREEK_MENU'><button style = 'font-size: 2.0em; background-color:LightGrey'> MENU</ button></ a> &nbsp; </ a></ th><th><a href = 'CREEK_LEFT'><button style = 'font-size: 2.0em; background-color:Grey'> LEFT</ button></ a></ th><th><a href = 'CREEK_MENU_T'><button style = 'font-size: 2.0em; color:Lime'> MENU</ button></ a></ th><th><a href = 'CREEK_RIGHT'><button style = 'font-size: 2.0em; background-color:Grey'> RIGHT</ button></ a></ th><th><a href = 'PTV_free'><button style = 'font-size: 2.0em; background-color:LightGrey'></ button></ a></ th></ tr><tr><th><a href = 'CREEK_SEL_DOWN'><button style = 'font-size: 2.0em; color:Orange'> SEL - </ button></ a> &nbsp; </ a></ th><th><a href = 'PTV_free'></ a></ th><th><a href = 'CREEK_SEL_DOWN'><button style = 'font-size: 2.0em; background-color:Grey'> DOWN</ button></ a></ th><th><a href = 'PTV_free'></ a></ th><th><a href = 'CREEK_VOL_DOWN'><button style = 'font-size: 2.0em; color:Blue'> V - </ button></ a></ th></ tr><tr><th><a href = 'PTV_free'></ a></ th><th><a href = 'PTV_free'></ a></ th><th><a href = 'PTV_free'><button style = 'font-size: 2.0em; color:White'></ button></ a></ th><th><a href = 'PTV_free'></ a></ th><th><a href = 'PTV_free'></ a></ th></ tr></ table><table id = 't01'><caption style = 'font-size: 5vw'> RGB LED Stripe / RGB Lamp</ caption><tr><th><a href = 'ledstripe_ON'><button style = 'font-size: 2.0em; color:Red'> --ON--</ button></ a> &nbsp</ th><th><a href = 'ledstripe_OFF'><button style = 'font-size: 2.0em; color:Red'> --OFF--</ button></ a> &nbsp</ th><th><a href = 'ledstripe_CYCLE'><button style = 'font-size: 2.5em; background-color:LightGrey'> CYCLE</ button></ a></ th></ tr><tr><th><a href = 'ledstripe_STORE'><button style = 'font-size: 2.5em; background-color:LightGrey'> MEMO</ button></ a> &nbsp</ th><th><a href = 'ledstripe_WARN'><button style = 'font-size: 2.5em; background-color:LightGrey'> WARN</ button></ a> &nbsp</ th><th><a href = 'ledstripe_MOVE'><button style = 'font-size: 2.5em; background-color:LightGrey'> MOVE</ button></ a></ th></ tr><tr><th><a href = 'ledstripe_B_DOWN'><button style = 'font-size: 2.5em; background-color:LightGrey'>[....B - ....]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_B_UP'><button style = 'font-size: 2.5em; background-color:LightGrey'>[....B + ....]</ button></ a> &nbsp</ th></ tr><tr><th><a href = 'ledstripe_B5_DOWN'><button style = 'font-size: 2.5em; background-color:LightGrey'>[... B5 - ...]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_B5_UP'><button style = 'font-size: 2.5em; background-color:LightGrey'>[... B5 + ...]</ button></ a> &nbsp</ th></ tr><tr><th><a href = 'ledstripe_RED'><button style = 'font-size: 2.5em; background-color:Red'>[... Red...]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_GREEN'><button style = 'font-size: 2.5em; background-color:Green'>[..Green..]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_BLUE'><button style = 'font-size: 2.5em; background-color:DarkBlue'>[....Blue....]</ button></ a></ th></ tr><tr><th><a href = 'ledstripe_YELLOW'><button style = 'font-size: 2.5em; background-color:Yellow'>[.Yellow.]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_MAGENTA'><button style = 'font-size: 2.5em; background-color:Magenta'>[Magenta]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_ORANGE'><button style = 'font-size: 2.5em; background-color:Orange'>[.Orange.]</ button></ a></ th></ tr><tr><th><a href = 'ledstripe_TURQUOISE'><button style = 'font-size: 2.5em; background-color:Turquoise'>[..Lime...]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_VIOLET'><button style = 'font-size: 2.5em; background-color:Violet'>[... Violet...]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_WHITE'><button style = 'font-size: 2.5em; background-color:WHITE'>[... White...]</ button></ a></ th></ tr><tr><th><a href = 'ledstripe_RED2'><button style = 'font-size: 2.5em; background-color:#FF4000'>[... Red2..]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_RED3'><button style = 'font-size: 2.5em; background-color:#FF0040'>[... Red3....]</ button></ a> &nbsp</ th></ tr><tr><th><a href = 'ledstripe_GREEN2'><button style = 'font-size: 2.5em; background-color:#9AFE2E  '>[.Green2]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_GREEN3'><button style = 'font-size: 2.5em; background-color:#40FF00'>[.Green3.]</ button></ a></ th></ tr><tr><th><a href = 'ledstripe_BLUE2'><button style = 'font-size: 2.5em; background-color:#819FF7'>[... Blue2...]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_BLUE3'><button style = 'font-size: 2.5em; background-color:#2E64FE'>[... Blue3...]</ button></ a> &nbsp</ th><th><a href = 'ledstripe_BLUE4'><button style = 'font-size: 2.5em; background-color:#013ADF'>[... Blue4...]</ button></ a></ th></ tr></ table><br><br></ body></ html> ";

// IRsend irsend;
// IRsend irsend(14); //an IR led is connected to GPIO 14 (This GPIO pin on ESP8266-04 has PWM Support)
// IRsend irsend(3); //an IR led is connected to GPIO 3 (RX)
// IRsend irsend(0); //an IR led is connected to GPIO 0 ( it works ! )
IRsend irsend(transmitPinIR); // an IR led is connected to GPIO13

void (*resetFunc)(void) = 0; // declare reset function @ address 0

#define PanasonicAddress 0x4004 // Panasonic address (Pre data)

void setup(void)
{

  irsend.begin();
  Serial.begin(115200, SERIAL_8N1);

  Serial.println("");
  Serial.println("");
  Serial.println("**  IR-GW 2.0  **");
  Serial.println("***********************");

  pinMode(transmitPin433, OUTPUT); // buttonDownPin auch transmitPin433,  angeschlossene gelbe LED als Indikator für IR-Übertragung verwenden
  pinMode(transmitPin868, OUTPUT); // buttonOkPin auch transmitPin868,  angeschlossene blaue LED abschalten

  WiFi.begin(CREDENTIALS_SSID, CREDENTIALS_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print("*");
  }

  Serial.println("");
  Serial.println("WiFi connection Successful");
  Serial.print("IP Address: ");
  Serial.print(WiFi.localIP()); // Print the IP address

  httpUpdater.setup(&server);
  server.begin();

  delay(1000);

  Serial.println("Waiting for commands...");

  server.on("/", []() {
    server.send(200, "text/html", webPage);
  });

  server.on("/CREEK_VOL_UP", []() {
    irsend.sendRC5(0xC10, 12);
    delay(150);
    irsend.sendRC5(0xC10, 12);
    delay(150);
    irsend.sendRC5(0xC10, 12);
    delay(150);
    irsend.sendRC5(0xC10, 12);
    delay(150);
    irsend.sendRC5(0xC10, 12);
    delay(150);
    Serial.println("Sending Command: Creek Amplifier VOL+"); // Vol+
    server.send(200, "text/html", webPage);
  });
  server.on("/CREEK_VOL_DOWN", []() {
    irsend.sendRC5(0x411, 12);
    delay(150);
    irsend.sendRC5(0x411, 12);
    delay(150);
    irsend.sendRC5(0x411, 12);
    delay(150);      
    irsend.sendRC5(0x411, 12);
    delay(150);
    irsend.sendRC5(0x411, 12);
    delay(150);
    Serial.println("Sending Command: Creek Amplifier VOL-"); // Vol-
    server.send(200, "text/html", webPage);
  });
  server.on("/CREEK_MUTE", []() {
    irsend.sendRC5(0x40D, 12);
    delay(150);
    Serial.println("Sending Command: Creek Amplifier MUTE");
    server.send(200, "text/html", webPage);
  });
  server.on("/CREEK_SEL_UP", []() {
    irsend.sendRC5(0xC20, 12);
    delay(250);
    Serial.println("Sending Command: Creek Amplifier SEL+"); // SEL_UP/DOWN getauscht
    server.send(200, "text/html", webPage);
  });
  server.on("/CREEK_SEL_DOWN", []() {
    irsend.sendRC5(0x421, 12);
    delay(250);
    Serial.println("Sending Command: Creek Amplifier SEL-"); // SEL_UP/DOWN getauscht
    server.send(200, "text/html", webPage);
  });
  server.on("/CREEK_LEFT", []() {
    irsend.sendRC5(0xC1B, 12);
    delay(250);
    Serial.println("Sending Command: Creek Amplifier Balance RIGHT");
    server.send(200, "text/html", webPage);
  });
  server.on("/CREEK_RIGHT", []() {
    irsend.sendRC5(0x41A, 12);
    delay(250);
    Serial.println("Sending Command: Creek Amplifier BALANCE LEFT");
    server.send(200, "text/html", webPage);
  });
  server.on("/CREEK_TONE", []() {
    irsend.sendRC5(0xC15, 12);
    delay(250);
    Serial.println("Sending Command: Creek Amplifier TONE");
    server.send(200, "text/html", webPage);
  });
  server.on("/CREEK_MENU", []() {
    irsend.sendRC6(0x99DC5701, 32); delay(250);
    Serial.println("Sending Command: Creek Amplifier MENU"); // RC6 !?!
    server.send(200, "text/html", webPage);
  });
  server.on("/CREEK_MENU_T", []() {
    irsend.sendRC6(0x99DC5701, 32);
    delay(250); // manual toggle possible !?!
    irsend.sendRC6(0x867D1808, 32);
    delay(250);
    Serial.println("Sending Command: Creek Amplifier MENU"); //  RC6 !?!
    server.send(200, "text/html", webPage);

    // irsend.sendRC6(irsend.toggleRC6(0x99DC5701, RC6_36_BITS), RC6_36_BITS); //  code toggled, see https://github.com/markszabo/IRremoteESP8266/issues/43
  });
  server.on("/CREEK_1", []() {
    irsend.sendRC5(0x401, 12);
    Serial.println("Sending Command: Creek Amplifier 1");
    server.send(200, "text/html", webPage);
    delay(100);
  });
  server.on("/CREEK_2", []() {
    irsend.sendRC5(0x402, 12);
    Serial.println("Sending Command: Creek Amplifier 2");
    server.send(200, "text/html", webPage);
    delay(100);
  });
  server.on("/CREEK_3", []() {
    irsend.sendRC5(0x403, 12);
    Serial.println("Sending Command: Creek Amplifier 3");
    server.send(200, "text/html", webPage);
    delay(100);
  });
  server.on("/CREEK_4", []() {
    irsend.sendRC5(0x404, 12);
    Serial.println("Sending Command: Creek Amplifier 4");
    server.send(200, "text/html", webPage);
    delay(100);
  });

  server.on("/PanaTV_ONOFF", []() {
    int counter = 0;
    while ((counter != 20) ) {
      irsend.sendPanasonic(PanasonicAddress, 0x100BCBD);      
      delay(72); // send "Power" code for >2 seconds, Protocol needs 72 ms pause between packets
      counter += 1;
    }
    Serial.println("Sending Command: TV Power"); // TV on/off
    server.send(200, "text/html", webPage);
  });

  server.on("/PanaTV_INPUT", []() {
    int counter = 0;
    while ((counter != 3) ) {
      irsend.sendPanasonic(PanasonicAddress, 0x100A0A1);      
      delay(72); // send "Input" code, Protocol needs 72 ms pause between packets
      counter += 1;
    }
    Serial.println("Sending Command: TV INPUT"); // select Input
    server.send(200, "text/html", webPage);
  });

  server.on("/PanaTV_UP", []() {
    irsend.sendPanasonic(PanasonicAddress, 0x1005253);
    Serial.println("Sending Command: TV UP"); // UP
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/PanaTV_DOWN", []() {
    irsend.sendPanasonic(PanasonicAddress, 0x100D2D3);
    Serial.println("Sending Command: TV DOWN"); // DOWN
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/PanaTV_LEFT", []() {
    irsend.sendPanasonic(PanasonicAddress, 0x1007273);
    Serial.println("Sending Command: TV LEFT"); // LEFT (not working on Reflexion TV)
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/PanaTV_RIGHT", []() {
    irsend.sendPanasonic(PanasonicAddress, 0x100F2F3);
    Serial.println("Sending Command: TV RIGHT"); // RIGHT
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/PanaTV_OK", []() {
    irsend.sendPanasonic(PanasonicAddress, 0x1009293);
    Serial.println("Sending Command: TV OK"); // OK
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/PanaTV_MENU", []() {
    irsend.sendPanasonic(PanasonicAddress, 0x1004A4B);
    Serial.println("Sending Command: TV MENU"); // MENU
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/PanaTV_RETURN", []() {
    irsend.sendPanasonic(PanasonicAddress, 0x1002B2A);
    Serial.println("Sending Command: TV RETURN"); // RETURN
    server.send(200, "text/html", webPage);
    delay(250);
  });

  server.on("/ledstripe_ON", []() {
    //irsend.sendNEC(0xF7C03F, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 
    irsend.sendNEC(0xFFF807, 32, 1); // RGB LED Lamp
    //irsend.sendNEC(0xFFC837, 32, 1); // min. Brightness
    Serial.println("Sending Command: LED Stripe ON"); // LED Stripe On
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_OFF", []() {
    //irsend.sendNEC(0xF740BF, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFF7887, 32, 1); // RGB LED Lamp
    irsend.sendNEC(0xFF41BE, 32); // RGB LED Stripe 2
    Serial.println("Sending Command: LED Stripe OFF"); // Off
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_CYCLE", []() {
    irsend.sendNEC(0xFFA857, 32, 1); // RGB LED Lamp
    //irsend.sendNEC(0xF7C837, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFFC936, 32); // RGB LED Stripe 2 
    Serial.println("Sending Command: LED Stripe CYCLE"); // CYCLE
    server.send(200, "text/html", webPage);
    delay(250);
  });

  server.on("/ledstripe_STORE", []() {
    irsend.sendNEC(0xFF6897, 32, 1);
    Serial.println("Sending Command: RGB LED Stripe STORE"); // E27RGBLED STORE COLOR
    server.send(200, "text/html", webPage);
    delay(250);
  });

  server.on("/ledstripe_WARN", []() {
    Serial.println("Sending Command: RGB LED Stripe WARN"); // E27RGBLED Flash red/green
    server.send(200, "text/html", webPage);
    irsend.sendNEC(0xFFF807, 32, 1); // RGB Lamp on
    // irsend.sendNEC(0xF7C03F, 32); // RGB LED Stripe 1
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2
    delay(5);
    for (int i = 0; i < 10; i++)
    {
      irsend.sendNEC(0xFF18E7, 32, 1); // RGB Lamp red
      // irsend.sendNEC(0xF720DF, 32); // RGB LED Stripe 1
      irsend.sendNEC(0xFF21DE, 32); // RGB LED Stripe 2
      delay(5);
      irsend.sendNEC(0xFF9867, 32, 1); // RGB Lamp green
      // irsend.sendNEC(0xF7A05F, 32); // RGB LED Stripe 1
      irsend.sendNEC(0xFFA15E, 32); // RGB LED Stripe 2
      delay(5);
    }
    irsend.sendNEC(0xFF7887, 32, 1); // RGB Lamp off
    // irsend.sendNEC(0xF740BF, 32); // RGB LED Stripe 1
    irsend.sendNEC(0xFF41BE, 32); // RGB LED Stripe 2
  });

  server.on("/ledstripe_MOVE", []() {
    irsend.sendNEC(0xFFE817, 32, 1);
    Serial.println("Sending Command: RGB LED Stripe MOVE"); // E27RGBLED next color
    server.send(200, "text/html", webPage);
    delay(5);
  });

  server.on("/ledstripe_B5_DOWN", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Sending Command: LED Stripe Brightness 5 steps down"); // 
    for (int i = 0; i<5; i++) {
      irsend.sendNEC(0xFF817E, 32, 1);
      delay(10);
    }; // 
    delay(250);
  });
  server.on("/ledstripe_B5_UP", []() {
    server.send(200, "text/html", webPage);
    Serial.println("Sending Command: LED Stripe Brightness 5 steps up"); // 
    for (int i = 0; i<5; i++) {
      irsend.sendNEC(0xFF01FE, 32, 1);
      delay(10);
    };
    delay(250);
  });

  server.on("/ledstripe_B_UP", []() {
    irsend.sendNEC(0xFF01FE, 32);
    Serial.println("Sending Command: RGB LED Stripe Brightness +"); // LED Stripe Brightness +
    server.send(200, "text/html", webPage);
    delay(350);
  });
  server.on("/ledstripe_B_DOWN", []() {
    irsend.sendNEC(0xFF817E, 32);
    Serial.println("Sending Command: RGB LED Stripe Brightness -"); // Off LED Stripe Brightness -
    server.send(200, "text/html", webPage);
    delay(350);
  });
  server.on("/ledstripe_RED", []() {
    //irsend.sendNEC(0xF720DF, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFF21DE, 32); // RGB LED Stripe 2 
    //irsend.sendNEC(0xFF18E7, 32, 1); // RGB LED Lamp
    Serial.println("Sending Command: RGB LED Stripe Red"); // Red
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_GREEN", []() {
    irsend.sendNEC(0xF7A05F, 32); // RGB LED Stripe 1 
    //irsend.sendNEC(0xF7A05F, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFFA15E, 32); // RGB LED Stripe 2 
    //irsend.sendNEC(0xFF9867, 32, 1); // RGB LED Lamp
    Serial.println("Sending Command: RGB LED Stripe GREEN"); // Green
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_BLUE", []() {
    //irsend.sendNEC(0xF7609F, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFF619E, 32); // RGB LED Stripe 2 
    //irsend.sendNEC(0xFF58A7, 32, 1); // RGB LED Lamp
    Serial.println("Sending Command: RGB LED Stripe BLUE"); // Blue
    server.send(200, "text/html", webPage);    
    delay(250);
  });
  server.on("/ledstripe_YELLOW", []() {
    //irsend.sendNEC(0xF708F7, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFF31CE, 32); // RGB LED Stripe 2 
    //irsend.sendNEC(0xFF08F7, 32, 1); // RGB LED Lamp
    Serial.println("Sending Command: RGB LED Stripe YELLOW"); // Yellow
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_VIOLET", []() {
    //irsend.sendNEC(0xF748B7, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFF51AE, 32); // RGB LED Stripe 2 
    //irsend.sendNEC(0xFF50AF, 32, 1); // RGB LED Lamp
    Serial.println("Sending Command: RGB LED Stripe VIOLET"); // Violet
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_ORANGE", []() {
    //irsend.sendNEC(0xF710EF, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFF11EE, 32); // RGB LED Stripe 2 
    //irsend.sendNEC(0xFF10EF, 32, 1); // RGB LED Lamp
    Serial.println("Sending Command: RGB LED Stripe ORANGE"); // Orange
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_MAGENTA", []() {
    //irsend.sendNEC(0xF76897, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFF6996, 32); // RGB LED Stripe 2 
    //irsend.sendNEC(0xFF48B7, 32, 1); // RGB LED Lamp
    Serial.println("Sending Command: RGB LED Stripe MAGENTA"); // Magenta
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_TURQUOISE", []() {
    //irsend.sendNEC(0xF7A857, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFF916E, 32); // RGB LED Stripe 2 
    //irsend.sendNEC(0xFF906F, 32, 1); // RGB LED Lamp
    Serial.println("Sending Command: RGB LED Stripe LIME / TURQUOISE"); // Turquoise
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_WHITE", []() {
    irsend.sendNEC(0xFF28D7, 32, 1); // RGB LED Lamp
    //irsend.sendNEC(0xF7E01F, 32); // RGB LED Stripe 1 
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFFE11E, 32); // RGB LED Stripe 2 
    Serial.println("Sending Command: RGB LED Stripe WHITE"); // White
    server.send(200, "text/html", webPage);
    delay(250);
  });

  server.on("/ledstripe_RED2", []() {
    //irsend.sendNEC(0xFF28D7, 32, 1); // RGB LED Lamp
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFF718E, 32); // RGB LED Stripe 2 
    Serial.println("Sending Command: RGB ledstripe_RED2"); // ledstripe_RED2
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_RED3", []() {
    //irsend.sendNEC(0xFF28D7, 32, 1); // RGB LED Lamp
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFF49B6, 32); // RGB LED Stripe 2 
    Serial.println("Sending Command: RGB ledstripe_RED3"); // ledstripe_RED3
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_GREEN2", []() {
    //irsend.sendNEC(0xFF28D7, 32, 1); // RGB LED Lamp
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFF09F6, 32); // RGB LED Stripe 2 
    Serial.println("Sending Command: RGB ledstripe_GREEN2"); // ledstripe_GREEN2
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_GREEN3", []() {
    //irsend.sendNEC(0xFF28D7, 32, 1); // RGB LED Lamp
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFFB14E, 32); // RGB LED Stripe 2 
    Serial.println("Sending Command: RGB ledstripe_GREEN3"); // ledstripe_GREEN3
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_BLUE2", []() {
    //irsend.sendNEC(0xFF28D7, 32, 1); // RGB LED Lamp
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFFE11E, 32); // RGB LED Stripe 2 
    Serial.println("Sending Command: RGB ledstripe_BLUE2"); // ledstripe_BLUE2
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_BLUE3", []() {
    //irsend.sendNEC(0xFF28D7, 32, 1); // RGB LED Lamp
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFF8976, 32); // RGB LED Stripe 2 
    Serial.println("Sending Command: RGB ledstripe_BLUE3"); // ledstripe_BLUE3
    server.send(200, "text/html", webPage);
    delay(250);
  });
  server.on("/ledstripe_BLUE4", []() {
    //irsend.sendNEC(0xFF28D7, 32, 1); // RGB LED Lamp
    irsend.sendNEC(0xFFC13E, 32); // RGB LED Stripe 2 ON
    irsend.sendNEC(0xFFA956, 32); // RGB LED Stripe 2 
    Serial.println("Sending Command: RGB ledstripe_BLUE4"); // ledstripe_BLUE4
    server.send(200, "text/html", webPage);
    delay(250);
  });

  server.begin();
}

void loop(void)
{
  server.handleClient();
}
