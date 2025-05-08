#include "hid_foot_pedal/foot_pedal.h"

int main() {
  if (!UsbFootPedal::DetermineExpectedPedalValue(0x1a86, 0xe026)) return 1;
  return 0;
}
