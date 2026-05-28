#include "serialout.h"
#include "encoder.h"

static int printTick = 0;

void plotterPrint() {
  Serial.print(ASpeed);
  Serial.print(",");
  Serial.print(BSpeed);
  Serial.print(",");
  Serial.print(CSpeed);
  Serial.print(",");
  Serial.println(DSpeed);
}

void debugPrint(int cmd) {
  printTick++;
  if (printTick < 10) return;
  printTick = 0;

  const char* modeStr = (cmd == 'x') ? "[停]" :
                        (cmd == 'w') ? "[正]" : "[反]";

  Serial2.print(modeStr);
  Serial2.print(" A RPM=");
  Serial2.print(ASpeed);
  Serial2.print("  B RPM=");
  Serial2.print(BSpeed);
  Serial2.print("  C RPM=");
  Serial2.print(CSpeed);
  Serial2.print("  D RPM=");
  Serial2.print(DSpeed);
  Serial2.print("  A dist=");
  Serial2.print(Adistance_m);
  Serial2.print("  B dist=");
  Serial2.print(Bdistance_m);
  Serial2.print("  C dist=");
  Serial2.print(Cdistance_m);
  Serial2.print("  D dist=");
  Serial2.println(Ddistance_m);
}
