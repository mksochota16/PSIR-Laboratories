#include <Arduino.h>
#include <ZsutEthernet.h>
#include <ZsutEthernetUdp.h>
#include <ZsutFeatures.h>

#define buffer_size 5
#define period 2620
#define local_port 8847

#define MSG_HELLO '\x40'
#define MSG_REQUEST '\x80'
#define MSG_RESPONSE '\xc0'

#define MASK_TYPE '\xc0'
#define MASK_CNT '\x3c'
#define MASK_VAL_0 '\x03'
#define MASK_VAL_1 '\xff'

ZsutIPAddress remote_ip;
ZsutEthernetUDP Udp;

byte request_buffer[1];
byte response_buffer[buffer_size] = {'\0'};
byte temp_buffer[buffer_size] = {'\0'};

byte mac[] = {
        0x00, 0xaa, 0xbb, 0xcc, 0xde, 0xf2
};

unsigned int remote_port;
int counter = 0;

void setup() {
    //Zwyczajowe przywitanie z userem (niech wie ze system sie uruchomil poprawnie)
    Serial.begin(115200);
    Serial.print(F("Zsut eth udp server init... ["));
    Serial.print(F(__FILE__));
    Serial.print(F(", "));
    Serial.print(F(__DATE__));
    Serial.print(F(", "));
    Serial.print(F(__TIME__));
    Serial.println(F("]"));

    //inicjaja karty sieciowe - proforma dla ebsim'a
    ZsutEthernet.begin(mac);

    //potwierdzenie na jakim IP dzialamy - proforma dla ebsim'a
    Serial.print(F("My IP address: "));
    for (byte thisByte = 0; thisByte < 4; thisByte++) {
        Serial.print(ZsutEthernet.localIP()[thisByte], DEC);
        Serial.print(F("."));
    }
    Serial.println();

    Udp.begin(local_port);
    //waiting for a hello message
    for (;;) {
        int packetSize = Udp.parsePacket();
        if (packetSize > 0) {
            int len = Udp.read(response_buffer, buffer_size);
            response_buffer[len] = '\0';

            Serial.print("Recieved: ");
            Serial.println((char *) response_buffer);

            //check if first 2 bits are equal '01' - hello message
            //if not, back to listening.
            if ((response_buffer[0] & MASK_TYPE) != (byte) MSG_HELLO)
                continue;

            //print server ip and port, break and go to loop()
            remote_ip = Udp.remoteIP();
            remote_port = Udp.remotePort();
            Serial.print("Server noticed:\nIP: ");
            for (int i = 0; i < 4; i++) {
                Serial.print(remote_ip[i], DEC);
                if (i < 3) { Serial.print("."); }
            }
            Serial.print("\nPort: ");
            Serial.println(remote_port);
            break;
        }
    }
}



void loop() {
    //send request message
    Udp.beginPacket(remote_ip, remote_port);

    request_buffer[0] = MSG_REQUEST | ((counter << 2) & MASK_CNT);
    Udp.write(request_buffer, sizeof request_buffer);
    Serial.print("Wyslalem prosbe o odczyt, czekam na pomiar\n");
    counter++;

    Udp.endPacket();

    //waiting for a response
    for (;;) {
        int packetSize = Udp.parsePacket();
        if (packetSize > 0) {
            int len = Udp.read(response_buffer, buffer_size - 1);
            response_buffer[len] = '\0';

            //check if first 2 bits are equal '11' - response message
            //if not, back to listening.
            if ((response_buffer[0] & MASK_TYPE) != (byte) MSG_RESPONSE)
                continue;

            //extract and print received counter value
            temp_buffer[0] = response_buffer[0] & MASK_CNT;
            Serial.print("Odebralem pomiar nr: ");
            Serial.println(temp_buffer[0] >> 2, HEX);

            //extract and print received measurement value
            temp_buffer[0] = response_buffer[0] & MASK_VAL_0;
            temp_buffer[1] = response_buffer[1] & MASK_VAL_1;
            Serial.print("Wartosc: ");
            Serial.print(temp_buffer[0], HEX);
            if(temp_buffer[1]<16)
                Serial.print("0");        
            Serial.println(temp_buffer[1], HEX);

            //break to main loop, wait, and send request message again.
            break;
        }
    }
    //wait approx. [period] ms
    unsigned time = ZsutMillis();
    while (ZsutMillis() < time + period) {
    }

}

