#include <CAN.h>

//Pin Definitions Constants
#define rx 4 //GPIO4
#define tx 5 //GPIO5

TaskHandle_t CAN_Bus;

//Global variables to be filled by CAN messages and then displayed
//message: 0x01F0A000
double engine_speed = -1;               // 0-25,599.94 rpm
double throttle = -1;                   // 0-990998 %
double intake_air_temp = -1;            //-198.4-260.6 Deg F  #2c#
double coolant_temp = -1;               //-198.4-260.6 Deg F  #2c#

//message: 0x01F0A003
double afr_1 = -1;                      // 7.325-21.916 AFR
double afr_2 = -1;                      // 7.325-21.916 AFR
double vehicle_speed = -1;              // 0-255.996 mph
double gear = -1;                       // 0-255 unitless
double ign_timing = -1;                 //-17-72.65 Deg
double battery_voltage = -1;            // 0-16.089 volts

//message: 0x01F0A004
double manifold_absolute_pressure = -1; //-14.696-935.81 PSI(g)
double ve = -1;                         // 0-255 %
double fuel_pressure = -1;              // 0-147.939 PSI(g)
double oil_pressure = -1;               // 0-147.939 PSI(g)
double afr_target = -1;                 // 7.325-21.916 AFR

//Modifiers
//message: 0x01F0A000
double m_engine_speed = 0.39063;     // rpm/bit 
double m_throttle = 0.0015259;       // %/bit
double m_temp = 1.8;                 // Deg F/bit
double fahrenheit_offset = 32;       // Deg F

//message: 0x01F0A003
double m_afr = 0.057227;             // AFR/bit
double afr_offset = 7.325;           // AFR
double m_vehicle_speed = 0.00390625; // mph/bit
double m_gear = 1;                   // unitless
double m_ign_timing = 0.35156;       // Deg/bit
double ign_offset = -17;             // Deg
double m_battery_voltage = 0.0002455;// Volts/bit

//message: 0x01F0A004
double m_map = 0.014504;             // PSI/bit 
double map_offset = -14.6960;        // PSI(g)
double m_ve = 1;                     // %/bit
double m_pressure = 0.580151;        // PSIg/bit

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("CAN Receiver Callback");

  xTaskCreatePinnedToCore(CAN_Handler, "CAN_Bus", 10000, NULL, 0, &CAN_Bus, 0);
  //Set pins
  CAN.setPins(rx, tx);
  
  // start the CAN bus at 500 kbps
  if (!CAN.begin(500E3)) {
    //make sure to display this failure onscreen
    Serial.println("Starting CAN failed!");
    while (1);
  }
  Serial.println("Starting CAN success");
}

void loop() {
  // do nothing
}

int _2c8bit(int num){
  if (num > 0x7F) num -= 0x100;
  return num;
}

void CAN_Handler( void * parameter){
  for(;;) {
    int packet_size = CAN.parsePacket();
    if (packet_size) {
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
      long ID = CAN.packetId();
      Serial.print(ID, HEX);
    
      if (CAN.packetRtr()) {
        Serial.print(" and requested length ");
        Serial.println(CAN.packetDlc());
      } else {
        Serial.print(" and length ");
        Serial.println(packet_size);
    
        //Create array to hold message data
        int message[8];
    
        // only print packet data for non-RTR packets
        //assuming big endian
        int i = 0;
        while (CAN.available()) {
          int message_data = CAN.read();
          Serial.print(message_data);
          message[i++] = message_data;
        }
        //AEM Infinity Series 3 IDs
        switch(ID){
            //Engine Speed 0-1, Throttle 4-5, Intake Air Temp 6, Coolant Temp 7
            case 0x01F0A000:
                //template: variable = message * modifier + offset
                engine_speed = (message[0]*16 + message[1]) * m_engine_speed;
                throttle = (message[4]*16 + message[5]) * m_throttle;
                intake_air_temp = (_2c8bit(message[6]) * m_temp) + fahrenheit_offset;
                coolant_temp = (_2c8bit(message[7]) * m_temp) + fahrenheit_offset;
                break;
            //AFR #1 0, AFR #2 1, Vehicle Speed 2-3, Gear Calculated 4, Ign Timing 5, Battery Volts 6-7
            case 0x01F0A003:
                afr_1 = message[0] * m_afr + afr_offset;
                afr_2 = message[1] * m_afr + afr_offset;
                vehicle_speed = (message[2]*16 + message[3]) * m_vehicle_speed;
                gear = message[4] * m_gear;
                ign_timing = message[5] * m_ign_timing + ign_offset;
                battery_voltage = (message[6]*16 + message[7]) * m_battery_voltage;
                break;
            //MAP 0-1, VE 2, FuelPressure 3, OilPressure 4, AFRTarget 5, 6 and 7 are boolean variables that could be used as lamps
            case 0x01F0A004:
                manifold_absolute_pressure = (message[0]*16 + message[1]) * m_map + map_offset;
                ve = message[2] * m_ve;
                fuel_pressure = message[3] * m_pressure;
                oil_pressure = message[4] * m_pressure;
                afr_target = message[5] * m_afr + afr_offset;
                break;
            default:
                Serial.print(" message was not used");
                break;
          }
        Serial.println();
      }
    
      Serial.println();
    }
  }
}
