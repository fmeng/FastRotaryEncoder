#include "FastRotaryEncoder.h"

FastRotaryEncoder encoder(32, 25, -1, 0, 100);

void positionChanging(long position) {
    Serial.print("function, mappedPosition: ");
    Serial.println(position);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}
    encoder.setup();
    encoder.initMappedPosition(10); // reset init value

    // lambda, register callback when mappedPosition value changed
    encoder.registerEncoderChangedCb([](long mappedPosition) {
        Serial.print("lambda, mappedPosition: ");
        Serial.println(mappedPosition);
    });

    // function, register callback when mappedPosition value changed
    encoder.registerEncoderChangedCb(positionChanging);
}

void loop() {
    // do nothing
}