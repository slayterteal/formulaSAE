#include <CAN.h>
#include <math.h>
//J1939 message ID constants NEED TO BE DETERMINED
const char SPEED = 0;
const char GEAR = 1;
const char RPM = 2;
const char ENGINE_TEMP = 3;

//Global variables to be filled by CAN messages and then displayed
long raw_speed = -1; 
long raw_gear = -1; 
long raw_rpm = -1; 
long raw_engine_temp = -1; 

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("CAN Receiver Callback");

  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    //make sure to display this failure onscreen
    Serial.println("Starting CAN failed!");
    while (1);
  }

  // register the receive callback
  CAN.onReceive(onReceive);
}

void loop() {
  // do nothing
}

long hexToLong(char *c, int end, int start = 0){
    long decimal = 0;
    for (int i = start; i < end; i++) {
        int temp = (c[i] >= 'A') ? (c[i] >= 'a') ? (c[i] - 'a' + 10) : (c[i] - 'A' + 10) : (c[i] - '0');
        decimal += temp*pow(16,end-i-1);
    }
    return decimal;
}

void onReceive(int packet_size) {
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
    Serial.println(packet_size);

//if packetSize is only data size then this will work
    char message[packet_size];

    // only print packet data for non-RTR packets
    int i = 0;
    while (CAN.available()) {
      char message_data = (char)CAN.read();
      Serial.print(message_data);
      message[i++] = message_data;
    }
    /*The data will need to be interpreted
    For now the raw decimal value is assigned to the global variable
    */
    switch(ID){
        //speed
        case SPEED:
            raw_speed = hexToLong(message, packet_size);
            break;
        //gear
        case GEAR:
            raw_gear = hexToLong(message, packet_size);
            break;
        //rpm
        case RPM:
            raw_rpm = hexToLong(message, packet_size);
            break;
        //engine temperature
        case ENGINE_TEMP:
            raw_engine_temp = hexToLong(message, packet_size);
            break;
        default:
            Serial.print(" ID not identified");
            break;
      }
    Serial.println();
  }

  Serial.println();
}
