#include "FastRotaryEncoder.h"

FastRotaryEncoder encoder(32, 25, 4, 0, 100);

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

    // https://github.com/mathertel/OneButton
    encoder.getButton()->attachDoubleClick([]() {
        Serial.println("double click");
    });
    encoder.getButton()->attachClick([]() {
        Serial.println("click");
    });
}

void loop() {
    encoder.getButton()->tick();
}