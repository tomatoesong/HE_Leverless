#include <Adafruit_TinyUSB.h>

const uint8_t pins[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
// number of pins
uint8_t pincount = sizeof(pins) / sizeof(pins[0]);
uint8_t hidcode[] = { HID_KEY_0, HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_6, HID_KEY_7, HID_KEY_8, HID_KEY_9, HID_KEY_A, HID_KEY_B, HID_KEY_C };
bool activeState = false;

TaskHandle_t Task1;
TaskHandle_t Task2;

uint16_t keys = 0;
uint8_t keys0 = 0;
uint8_t keys1 = 0;

static EventGroupHandle_t sync_event_group;

uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};
Adafruit_USBD_HID usb_hid(NULL, 0, HID_ITF_PROTOCOL_KEYBOARD, 1, false);


void setup() {
  // Serial.begin(115200);

  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  // usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  // usb_hid.setPollInterval(1);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  TinyUSBDevice.setProductDescriptor("HE_Leverless");

  usb_hid.begin();

  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  // for (uint8_t i = 0; i < 4; i++) {
  //   pinMode(17 + i, activeState ? INPUT_PULLDOWN : INPUT_PULLUP);
  // }

  sync_event_group = xEventGroupCreate();

  xTaskCreatePinnedToCore(
    Task1code, /* Task function. */
    "Task1",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task1,    /* Task handle to keep track of created task */
    0);        /* pin task to core 0 */

  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
    Task2code, /* Task function. */
    "Task2",   /* name of task. */
    10000,     /* Stack size of task */
    NULL,      /* parameter of the task */
    1,         /* priority of the task */
    &Task2,    /* Task handle to keep track of created task */
    1);        /* pin task to core 1 */

  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void Task1code(void* pvParameters) {
  TickType_t lastWake = xTaskGetTickCount();

  for (;;) {
    // Serial.println(xPortGetCoreID());
    for (uint8_t i = 0; i < 7; i++) {
      if (analogRead(pins[i]) < 1100) {
        keys0 |= 1 << i;
      } else {
        keys0 &= ~(1 << i);
      }
    }
    // Serial.printf("0: %d\n", keys0);

    xEventGroupSetBits(sync_event_group, BIT0);
    xEventGroupWaitBits(sync_event_group, BIT0 | BIT1, pdFALSE, pdTRUE, portMAX_DELAY);
    xEventGroupClearBits(sync_event_group, BIT0);

    keys = ((keys1 << 7) | keys0);
    // Serial.println(keys);

    // not enumerated()/mounted() yet: nothing to do
    // if (!TinyUSBDevice.mounted()) {
    //   continue;
    // }

    // poll gpio once each 2 ms
    // static uint32_t ms = 0;
    // if (millis() - ms > 1) {
    //   ms = millis();
    //   process_hid();
    // }
    process_hid();

    xEventGroupSetBits(sync_event_group, BIT0);

    // vTaskDelay(pdMS_TO_TICKS(1));
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(1));
  }
}

void Task2code(void* pvParameters) {

  for (;;) {
    // Serial.println(xPortGetCoreID());
    for (uint8_t i = 0; i < 6; i++) {
      if (analogRead(pins[i + 7]) < 1100) {
        keys1 |= 1 << i;
      } else {
        keys1 &= ~(1 << i);
      }
    }
    // Serial.printf("1: %d\n", keys1);

    xEventGroupSetBits(sync_event_group, BIT1);
    xEventGroupWaitBits(sync_event_group, BIT0 | BIT1, pdFALSE, pdTRUE, portMAX_DELAY);
    xEventGroupClearBits(sync_event_group, BIT1);

    xEventGroupWaitBits(sync_event_group, BIT0, pdTRUE, pdTRUE, portMAX_DELAY);

    // vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void process_hid() {
  // used to avoid send multiple consecutive zero report for keyboard
  static bool keyPressedPreviously = false;

  uint8_t count = 0;
  uint8_t keycode[6] = { 0 };

  // scan normal key and send report
  for (uint8_t i = 0; i < pincount; i++) {
    if (keys & (1 << i)) {
      // if pin is active (low), add its hid code to key report
      keycode[count++] = hidcode[i];

      // 6 is max keycode per report
      if (count == 6) break;
    }
  }

  if (TinyUSBDevice.suspended() && count) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  // skip if hid is not ready e.g still transferring previous report
  if (!usb_hid.ready()) return;

  if (count) {
    // Send report if there is key pressed
    uint8_t const report_id = 0;
    uint8_t const modifier = 0;

    keyPressedPreviously = true;
    usb_hid.keyboardReport(report_id, modifier, keycode);
  } else {
    // Send All-zero report to indicate there is no keys pressed
    // Most of the time, it is, though we don't need to send zero report
    // every loop(), only a key is pressed in previous loop()
    if (keyPressedPreviously) {
      keyPressedPreviously = false;
      usb_hid.keyboardRelease(0);
    }
  }
}