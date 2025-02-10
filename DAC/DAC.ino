#define SINE_STEPS 20
#define PWM_PIN 9

uint8_t sineTable[SINE_STEPS];
constexpr const unsigned int stepDelayMicro = 1000000 / SINE_STEPS;

void setup() {
  setupPWM();

  pinMode(PWM_PIN, OUTPUT);

  for (int i = 0; i < SINE_STEPS; i++) {
    double angle = 2.0 * PI * i / SINE_STEPS;
    sineTable[i] = (uint8_t) (((sin(angle) + 1.0) / 2.0) * 255);
  }

}

void loop() {

  for (int i = 0; i < SINE_STEPS; i++) {
    analogWrite(PWM_PIN, sineTable[i]);
    delayMicroseconds(stepDelayMicro);
  }

}

void setupPWM() {
  TCCR1B = (TCCR1B & 0b11111000) | 0x01;
}