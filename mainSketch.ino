#include <CAN.h>
#include <math.h>
//J1939 message ID constants
const char SPEED = 0;
const char GEAR = 1;
const char RPM = 2;
const char ENGINE_TEMP = 3;

long speed = -1; 
long gear = -1; 
long rpm = -1; 
long engineTemp = -1; 

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("CAN Receiver Callback");

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }

  // register the receive callback
  CAN.onReceive(onReceive);
}

void loop() {
  // do nothing
}

long hex_to_long(char *c, int end, int start = 0){
    long decimal = 0;
    for (int i = start; i < end; i++) {
        int temp = (c[i] >= 'A') ? (c[i] >= 'a') ? (c[i] - 'a' + 10) : (c[i] - 'A' + 10) : (c[i] - '0');
        decimal += temp*pow(16,end-i-1);
    }
    return decimal;
}

void onReceive(int packetSize) {
  // received a packet
  Serial.print("Received ");

  if (CAN.packetExtended()) {
    Serial.print("extended ");
  }

  if (CAN.packetRtr()) {
    // Remote transmission request, packet contains no data
    Serial.print("RTR ");
  }

  Serial.print("packet with id 0x");
  char ID = CAN.packetId();
  Serial.print(ID, HEX);

  if (CAN.packetRtr()) {
    Serial.print(" and requested length ");
    Serial.println(CAN.packetDlc());
  } else {
    Serial.print(" and length ");
    Serial.println(packetSize);

//if packetSize is only data size then this will work
    char message[packetSize];

    // only print packet data for non-RTR packets
    int i = 0;
    while (CAN.available()) {
      char messageData = (char)CAN.read();
      Serial.print(messageData);
      message[i++] = messageData;
    }
    /*The data will need to be interpreted
    For now the raw decimal value is assigned to the global variable
    */
    switch(ID){
        //speed
        case SPEED:
            speed = hex_to_long(message, packetSize);
            break;
        //gear
        case GEAR:
            gear = hex_to_long(message, packetSize);
            break;
        //rpm
        case RPM:
            rpm = hex_to_long(message, packetSize);
            break;
        //engine temperature
        case ENGINE_TEMP:
            engineTemp = hex_to_long(message, packetSize);
            break;
        default:
            Serial.print(" ID not identified");
            break;
      }
    Serial.println();
  }

  Serial.println();
}
