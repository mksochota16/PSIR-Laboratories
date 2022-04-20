#include <Arduino.h>
#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>
#include <ZsutFeatures.h>

#define buffer_size 100

int local_port = 8847;
int remote_port;

ZsutIPAddress remote_ip;
ZsutEthernetUDP Udp;
byte buffer[buffer_size] = {'\0'};


byte mac[] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};



void setup() {
    //Zwyczajowe przywitanie z userem (niech wie ze system sie uruchomil poprawnie)
    Serial.begin(115200);
    Serial.print(F("Zsut eth udp server init... ["));Serial.print(F(__FILE__));
    Serial.print(F(", "));Serial.print(F(__DATE__));Serial.print(F(", "));Serial.print(F(__TIME__));Serial.println(F("]"));

    //inicjaja karty sieciowe - proforma dla ebsim'a
    ZsutEthernet.begin(mac);

    //potwierdzenie na jakim IP dzialamy - proforma dla ebsim'a
    Serial.print(F("My IP address: "));
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
        Serial.print(ZsutEthernet.localIP()[thisByte], DEC);Serial.print(F("."));
    }
    Serial.println();

    Udp.begin(local_port);
    for(;;){
        int packetSize = Udp.parsePacket();
        if (packetSize > 0) {
            int len = Udp.read(buffer, buffer_size);
            buffer[len] = '\0';

            Serial.print("Recieved: ");
            Serial.println((char *) buffer);

            remote_ip = Udp.remoteIP();
            remote_port = Udp.remotePort();


            Serial.print("Server noticed:\nIP: ");
            for (int i=0; i < 4; i++) {
                Serial.print(remote_ip[i], DEC);
                if (i < 3) {
                    Serial.print(".");
                }
            }
            Serial.print("\nPort: ");
            Serial.print(remote_port);
            Serial.print("\n");

            break;
        }
    }
}


void loop() {
    Udp.beginPacket(remote_ip, remote_port);
    Udp.write("hce otczyd\n", sizeof("hce otczyd\n"));
    Serial.print("Wyslalem 'hce otczyd'\n");
    Udp.endPacket();

    Serial.print("Czekam na pomiar z fizycznego urzadzenia\n");
    for(;;) {
        int packetSize=Udp.parsePacket();
        if(packetSize>0){
            int len = Udp.read(buffer, buffer_size);
            buffer[len] = '\0';

            Serial.print("Odebralem pomiar: ");
            Serial.println((char *) buffer);
            break;
        }
    }
    delay(1000);
}

