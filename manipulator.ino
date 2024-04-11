#include <Ethernet.h>
#include <EthernetUdp.h>
#include <EEPROM.h>
#include <Regexp.h>

#define UDP_PORT 8888
bool done = false;
IPAddress MyIP(192, 168, 42, 20);
IPAddress MANIPULATOR(192, 168, 42, 13);

EthernetUDP udp;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
byte mac[6];

byte detailZ = 50;
int manipulatorCmd = 0;
int manipulatorStatus = 1;

byte counter = 0;

byte coordinates[10][4] = {
  { 120, 60, 20, 0 },
  { 120, -60, 20, 0 },
  { 240, 60, 20, 0 },
  { 240, -60, 20, 0 },
  { 240, 60, 140, 0 },
  { 240, -60, 140, 0 },
  { 120, 60, 140, 0 },
  { 120, -60, 140, 0 },
  { 180, 0, 80, 1 },
  { 180, 0, 200, 0 }
};

// void processUdp() {
//   int packetSize = udp.parsePacket();
//   if (packetSize) {
//     udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
//     if (packetBuffer[0] == 'M') {
//       Serial.print("MANIPULATOR ");
//       byte counter = 0;
//       byte ind = 0;
//       for (int i = 0; i < 7; i++) {
//         if (packetBuffer[i] == ':') {
//           counter++;
//           continue;
//         }
//         if(packetBuffer[i] != ':' && counter < 2) {
//           continue;
//         } else {ind = i; break;}
//         if(counter > 2) break;
//       }
//       Serial.println(packetBuffer[ind] == '0' ? "STOPPED" : "MOVING");
//       manipulatorStatus = packetBuffer[ind] == '0' ? 0 : 1;
//       return;
//     }
//   }
// }

void processUdp() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    if (packetBuffer[0] == 'M') {
      Serial.print("M ");
      MatchState ms;
      ms.Target(packetBuffer);
      char result = ms.Match("M:%d+:(%d+):(%d+):", 0);
      if (result == REGEXP_MATCHED) {
        char m[10];
        ms.GetMatch(m);
        if (ms.level == 2) {
          String mv = ms.GetCapture(m, 0), cmd = ms.GetCapture(m, 1);
          manipulatorStatus = mv.toInt();
          manipulatorCmd = cmd.toInt();
          Serial.print("MV: ");
          Serial.print(manipulatorStatus);
          Serial.print(" CMD: ");
          Serial.print(manipulatorCmd);
        }
        Serial.println();
        // for (int j = 0; j < ms.level; j++)
        //   Serial.println(ms.GetCapture(m, j));
      } else if (result == REGEXP_NOMATCH) {
        Serial.println("No luck");
      } else {
        Serial.println("error");
      }
    }
  }
}

void moveManipulator(bool type = false, int x = 180, int y = 0, int z = 150, int g = 0) {
  String o = "g:" + (String)x + ":" + (String)y + ":0:" + (String)z + ":" + (String)g + "#";
  if (type == true) {  //если тип манипулятора - паллетайзер, меняем формат сообщения
    o = "p:" + (String)x + ":" + (String)y + ":" + (String)z + ":" + (String)g + "#";
  }
  int cmdMemo = manipulatorCmd;
  do {
    processUdp();
    if (manipulatorStatus == 0) break;
  } while (manipulatorStatus == 1);
  sendUdp(MANIPULATOR, o);
  manipulatorStatus = 1;
  do {
    processUdp();
    if (manipulatorStatus == 0 && manipulatorCmd > cmdMemo) break;
  } while (manipulatorStatus == 1 || manipulatorCmd <= cmdMemo);
}

void sendUdp(IPAddress adr, String msg) {
  char out[msg.length() + 1];
  msg.toCharArray(out, msg.length() + 1);
  udp.beginPacket(adr, UDP_PORT);
  udp.write(out);
  udp.endPacket();
  Serial.println(out);
}

void setup() {
  Serial.begin(115200);
  mac[0] = MyIP[0];
  mac[1] = MyIP[1];
  mac[2] = MyIP[2];
  mac[3] = MyIP[3];
  mac[4] = MyIP[2];
  mac[5] = MyIP[3];
  Serial.begin(115200);
  Ethernet.begin(mac, MyIP);
  udp.begin(UDP_PORT);
}
void loop() {
   int counter;
 // int cord[2][8]={{25,270},{}};
  int x = 104; int y = -324;
  sendUdp(MANIPULATOR, "r#");
 for (int i = 0; i < 2; i++){
   for (int k = 0; k < 4; k++){
    moveManipulator(true, 50, 310, 100, 0);
    moveManipulator(true, x-k*38, y+i*38, 100, 1);
    delay(1000);
    moveManipulator(true, x-k*38, y+i*38, 55, 1);
    moveManipulator(true, x-k*38, y+i*38, 55, 1);
    moveManipulator(true, x-k*38, y+i*38, 100, 1);
    moveManipulator(true, 50 + i * 38, 310 - i*38, 100, 0);
    moveManipulator(true, 50, 310, 60, 0);
    moveManipulator(true, 50, 310, 100, 0);
    //counter++;
    }   
  }
}