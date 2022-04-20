#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

const int localPort = 8847;
byte localMac[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress localIP(192, 168, 112, 130);

int broadcastPort = 8847;
IPAddress broadcastIP(192, 168, 112, 131);

int potentValue = 0;


// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  // buffer to hold incoming packet,
String reply;

EthernetUDP Udp;

void setup() {
    Serial.println(F("Embeded arduino server init..."));
    Ethernet.begin(localMac, localIP);

    Serial.begin(9600);
    while (!Serial) {
      ; // wait for serial port to connect. Needed for native USB port only
    }
  
    Udp.begin(localPort);

    // send "HELLO"
    sendUDPPacket(broadcastIP, broadcastPort, "HELLO");
}

void sendUDPPacket(IPAddress rhost, int rport, String message){
    Udp.beginPacket(rhost, rport);
    Udp.write(message.c_str());
    Udp.endPacket();
}

void serveClient(){
    // check if something has come up
    int packetSize = Udp.parsePacket();
    if (packetSize) {
      // read value from potentiometr
      sensorValue = analogRead(A0);
      reply = String(sensorValue);
      // send a reply, to the IP address and port that sent us the packet we received
      sendUDPPacket(Udp.remoteIP(), Udp.remotePort(), reply);
    }
}

void loop() {
    serveClient();
}
