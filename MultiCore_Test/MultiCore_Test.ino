
TaskHandle_t Core0;
TaskHandle_t Core1;
bool syncFlag0 = false;
bool syncFlag1 = false;
uint16_t keys = 0;

void setup() {
  // put your setup code here, to run once:
  xTaskCreatePinnedToCore(
    Core0_Task,
    "Core0",
    10000,
    NULL,
    1,
    &Core0,
    0);
  xTaskCreatePinnedToCore(
    Core1_Task,
    "Core1",
    10000,
    NULL,
    1,
    &Core1,
    1);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void Core0_Task(void* pvParameters) {
  uint8_t triggeredKeys = 0b00000001;
  
}

void Core1_Task(void* pvParameters) {
  uint8_t triggeredKeys = 0b00000001;
}
