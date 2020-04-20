#include <EEPROM.h>

const unsigned char PasscodeLength = 6;
const char Master[] = "495876";

void setup() {
  // put your setup code here, to run once:
  EEPROM.put(0, PasscodeLength);
  for (unsigned char i = 0; i < PasscodeLength; i++) {
    EEPROM.put(i + 1, Master[i]);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
