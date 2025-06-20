#include "FastRotaryEncoder.h"

// FSM
static const int8_t kFsmTable[4][4] = {
        /*            curr: 0     1     2     3   */
        /* prev=0 */ {0,  +1, -1, 0},
        /* prev=1 */
                     {-1, 0,  0,  +1},
        /* prev=2 */
                     {+1, 0,  0,  -1},
        /* prev=3 */
                     {0,  -1, +1, 0}
};

FastRotaryEncoder::FastRotaryEncoder(int pinA,
                             int pinB,
                             int pinBtn,
                             long minInclude,
                             long maxExclude,
                             int step,
                             int usePullUp) :
        pinA_(pinA),
        pinB_(pinB),
        pinBtn_(pinBtn),
        minInclude_(minInclude),
        maxExclude_(maxExclude),
        step_(step),
        usePullUp_(usePullUp) {
}


void IRAM_ATTR encoderIsrRouter(void *arg) {
    auto *encoder = static_cast<FastRotaryEncoder *>(arg);
    encoder->encoderIsrHandler(encoder);
}

QueueHandle_t FastRotaryEncoder::createQueue() {
    return xQueueCreate(10, sizeof(EncoderEvent));
}

void FastRotaryEncoder::setup() {
    encoderPosition_ = 0;
    encoderQueue_ = createQueue();

    pinMode(pinA_, (usePullUp_ & PULL_UP_PIN_A) ? INPUT_PULLUP : INPUT_PULLDOWN);
    pinMode(pinB_, (usePullUp_ & PULL_UP_PIN_B) ? INPUT_PULLUP : INPUT_PULLDOWN);
    attachInterruptArg(digitalPinToInterrupt(pinA_), encoderIsrRouter, this, CHANGE);
    attachInterruptArg(digitalPinToInterrupt(pinB_), encoderIsrRouter, this, CHANGE);

    if (pinBtn_ >= 0) {
        bool btnPullUp = usePullUp_ & PULL_UP_PIN_BTN;
        ptrButton_ = new OneButton(pinBtn_, !btnPullUp, btnPullUp);
    }

    initMappedPosition(minInclude_);

    xTaskCreate([](void *arg) {
        auto ptrEncoder = static_cast<FastRotaryEncoder *>(arg);
        ptrEncoder->encoderProcessTask(ptrEncoder);
    }, "EncoderTask", 4096, this, 2, nullptr);
}

void IRAM_ATTR FastRotaryEncoder::encoderIsrHandler(FastRotaryEncoder *ptrRotaryEncoder) {
auto *this_ = static_cast<FastRotaryEncoder *>(ptrRotaryEncoder);
static uint8_t prev_state = 0;
static int8_t step_accumulator = 0;

int a = digitalRead(this_->pinA_);
int b = digitalRead(this_->pinB_);
uint8_t curr_state = (a << 1) | b;

int8_t delta = kFsmTable[prev_state][curr_state];
if (delta != 0) {
step_accumulator += delta;
if (step_accumulator == 4 || step_accumulator == -4) {
EncoderEvent evt = {
        .direction = (step_accumulator > 0) ? +1 : -1,
        .timestamp_us = esp_timer_get_time()
};
xQueueSendFromISR(this_->encoderQueue_, &evt, nullptr);
step_accumulator = 0;
}
}
prev_state = curr_state;
}

bool FastRotaryEncoder::updateMappedPosition(float speed) {
    const int maxStep = step_ * 10;
    int step = step_ * pow(1.3, int(abs(speed) / 10));
    if (step > maxStep) {
        step = maxStep;
    }
    step = (speed >= 0 ? step : -step);
    int position = mappedPosition_;
    position += step;

    if (position >= maxExclude_) {
        position = maxExclude_ - 1;
    } else if (position < minInclude_) {
        position = minInclude_;
    }

    if (position == mappedPosition_) {
        return false;
    } else {
        mappedPosition_ = position;
    }

#ifdef LOG_DEBUG
    Serial.print("step: ");
    Serial.print(step);
    Serial.print(", speed: ");
    Serial.print(speed);
    Serial.print(", mappedPosition:");
    Serial.print(mappedPosition_);
    Serial.println();
#endif

    return true;
}

void FastRotaryEncoder::encoderProcessTask(FastRotaryEncoder *ptrRotaryEncoder) {
    auto *this_ = static_cast<FastRotaryEncoder *>(ptrRotaryEncoder);
    EncoderEvent last_event{};
    while (1) {
        EncoderEvent evt;

        if (xQueueReceive(this_->encoderQueue_, &evt, portMAX_DELAY)) {
            this_->encoderPosition_ += evt.direction;

            float speed = 0.1f;
            if (last_event.timestamp_us > 0) {
                int64_t dt = evt.timestamp_us - last_event.timestamp_us;
                if (dt > 0) {
                    speed = 1e6f / static_cast<float>(dt);
                }
            }
            if (evt.direction < 0) {
                speed = 0 - speed;
            }

            if (updateMappedPosition(speed)) {
                for (const auto &cb: this_->callbackList) {
                    cb(this_->mappedPosition_);
                }
            }

            last_event = evt;
        }
    }
    vTaskDelete(NULL);
}

void FastRotaryEncoder::registerEncoderChangedCb(EncoderChangedCb_t cb) {
    callbackList.push_back(std::move(cb));
}

OneButton *FastRotaryEncoder::getButton() {
    return ptrButton_;
}

bool FastRotaryEncoder::initMappedPosition(long initPosition) {
    if (initPosition < minInclude_) {
        return false;
    }
    if (initPosition >= maxExclude_) {
        return false;
    }
    mappedPosition_ = initPosition;
    return true;
}

long FastRotaryEncoder::getMappedPosition() {
    return mappedPosition_;
}