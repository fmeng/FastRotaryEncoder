#pragma once

#include <Arduino.h>
#include <vector>
#include <memory>
#include <OneButton.h>


#define PULL_UP_PIN_A (1<<0)
#define PULL_UP_PIN_B (1<<1)
#define PULL_UP_PIN_BTN (1<<2)

// 编码器事件结构体
struct EncoderEvent {
    int direction;         // +1 or -1
    int64_t timestamp_us;  // 微秒时间戳
};

using EncoderChangedCb_t = std::function<void(long mappedPosition)>;

class FastRotaryEncoder {

protected:

    int pinA_;
    int pinB_;
    int pinBtn_;
    int usePullUp_;

    int step_;
    int mappedPosition_;
    long minInclude_;
    long maxExclude_;

protected:
    OneButton *ptrButton_;

    QueueHandle_t encoderQueue_;

    std::vector<EncoderChangedCb_t> callbackList;

    virtual void encoderIsrHandler(FastRotaryEncoder *ptrRotaryEncoder);

    virtual void encoderProcessTask(FastRotaryEncoder *ptrRotaryEncoder);

    QueueHandle_t createQueue();

private:
    long encoderPosition_;

    bool updateMappedPosition(float speed);

    friend void IRAM_ATTR encoderIsrRouter(void *ptrRotaryEncoder);

public:

    FastRotaryEncoder(int pinA,
                  int pinB,
                  int pinBtn,
                  long minInclude,
                  long maxExclude,
                  int step = 1,
                  int usePullUp = (PULL_UP_PIN_A | PULL_UP_PIN_B | PULL_UP_PIN_BTN));

    virtual OneButton *getButton();

    virtual bool initMappedPosition(long initPosition);

    virtual long getMappedPosition();

    virtual void setup();

    virtual void registerEncoderChangedCb(EncoderChangedCb_t encoderChangedCb);
};