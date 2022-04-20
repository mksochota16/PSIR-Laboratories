#include <Arduino.h>
#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>
#include <ZsutFeatures.h>

int localPort = 8888;
int serverPort = 12397;
byte mac[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
ZsutIPAddress ip(192, 168, 112, 130);
ZsutEthernetUDP Udp;

void setup() {
    Serial.begin(115200);
    Serial.println(F("Zsut client init..."));
    ZsutEthernet.begin(mac);
    Udp.begin(localPort);
}

void loop() {
    Udp.beginPacket(ip, serverPort);
    int sensorValue = (int) ZsutAnalog0Read();
    String str = "A0: " + String(sensorValue) + "\n";
    Udp.write(str.c_str(), str.length());
    Udp.endPacket();
    delay(1000);
}

