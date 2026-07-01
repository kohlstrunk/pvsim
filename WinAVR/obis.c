/*
https://chatgpt.com/s/t_69d2afd6cfd081919f6056d3bb99afb1

*/

#include <SoftwareSerial.h>

SoftwareSerial sml(2, 3); // RX, TX

byte bu[300];
int idx = 0;

void setup() {
  Serial.begin(115200);
  sml.begin(9600); // ggf. 19200 testen
}

void loop() {
  while (sml.available()) {
    byte b = sml.read();
    // Ringbuffer
    if (idx<sizeof(bu)){ bu[idx++]=b;} else {idx = 0;}
    // Nach OBIS 16.7.0 suchen   
	// 0x01,0x00,0x10,0x07,0x00 also 1.0.16.7.0.xxxx
    for (int i=0; i<idx-10; i++) { // weil unten i+11
//      if (bu[i]==0x01 && bu[i+1] == 0x00 && bu[i+2] == 0x10 && bu[i+3] == 0x07 && buffer[i+4] == 0x00) {
        if (bu[i]==1)&(bu[i+1]==0)&(bu[i+2]==16)&(bu[i+3]==7)&(bu[i+4]==0)) {
        long power = 0;
        power=(bu[i+8]<<24)|bu[i+9]<<16)|(bu[i+10]<<8)|(bu[i+11]);
        Serial.print("Leistung: ");Serial.print(power);Serial.println(" W");
        idx = 0; // Reset für nächsten Frame
        break;
      }
    }
  }
}
