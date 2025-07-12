/*
 * Smart Door Lock System - Arduino Uno
 * Features: Fingerprint Scanner + 4x4 Keypad + LCD + Buzzer + Relay
 * Pin Configuration:
 * - Keypad: Pins 2-9 (4x4 matrix)
 * - Fingerprint: RX=10, TX=11, TOUCH_OUT=12
 * - Relay: Pin 13
 * - Buzzer: Pin A0
 * - LCD: SDA=A4, SCL=A5 (I2C)
 */

#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>

// ==================== DEBUG SETTINGS ====================
#define DEBUG_ENABLED 1  // Set to 0 to disable debug messages
#if DEBUG_ENABLED
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT_F(x) Serial.print(F(x))
#define DEBUG_PRINTLN_F(x) Serial.println(F(x))
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINT_F(x)
#define DEBUG_PRINTLN_F(x)
#endif

// ==================== LANGUAGE SELECTION ====================
#define LANG_ENGLISH 0
#define LANG_VIETNAMESE 1
byte currentLanguage = LANG_ENGLISH;  // Default language

// ==================== PROGMEM STRINGS - ENGLISH ====================
const char str_init_en[] PROGMEM = "Initializing...";
const char str_ready_en[] PROGMEM = "System Ready";
const char str_enter_pass_en[] PROGMEM = "Enter Password";
const char str_wrong_pass_en[] PROGMEM = "Wrong Password";
const char str_access_granted_en[] PROGMEM = "Access Granted";
const char str_access_denied_en[] PROGMEM = "Access Denied";
const char str_door_unlocked_en[] PROGMEM = "Door Unlocked";
const char str_door_locked_en[] PROGMEM = "Door Locked";
const char str_welcome_en[] PROGMEM = "Welcome!";
const char str_setup_mode_en[] PROGMEM = "Setup Mode";
const char str_checking_en[] PROGMEM = "Checking...";
const char str_please_wait_en[] PROGMEM = "Please wait";
const char str_cancelled_en[] PROGMEM = "Cancelled";
const char str_timeout_en[] PROGMEM = "Timeout";
const char str_try_again_en[] PROGMEM = "Try Again";
const char str_door_lock_system_en[] PROGMEM = "Door Lock System";
const char str_setup_menu_en[] PROGMEM = "Setup Menu";
const char str_change_password_en[] PROGMEM = "1:Change Password";
const char str_add_fingerprint_en[] PROGMEM = "2:Add Fingerprint";
const char str_delete_all_en[] PROGMEM = "3:Delete All Fgr";
const char str_security_en[] PROGMEM = "4:Security";
const char str_language_en[] PROGMEM = "5:Language";
const char str_exit_setup_en[] PROGMEM = "6:Exit Setup";
const char str_setup_cancelled_en[] PROGMEM = "Setup Cancelled";
const char str_setup_complete_en[] PROGMEM = "Setup Complete";
const char str_password_cleared_en[] PROGMEM = "Password Cleared";
const char str_password_error_en[] PROGMEM = "Password Error";
const char str_need_4_digits_en[] PROGMEM = "Need 4 digits";
const char str_enter_admin_pass_en[] PROGMEM = "Enter Admin Pass";
const char str_change_password_title_en[] PROGMEM = "Change Password";
const char str_enter_new_4dig_en[] PROGMEM = "Enter New (4dig)";
const char str_confirm_password_en[] PROGMEM = "Confirm Password";
const char str_enter_again_en[] PROGMEM = "Enter Again";
const char str_password_changed_en[] PROGMEM = "Password Changed";
const char str_successfully_en[] PROGMEM = "Successfully!";
const char str_mismatch_en[] PROGMEM = "Mismatch!";
const char str_add_fingerprint_title_en[] PROGMEM = "Add Fingerprint";
const char str_place_finger_en[] PROGMEM = "Place finger";
const char str_added_success_en[] PROGMEM = "Added Success!";
const char str_memory_full_en[] PROGMEM = "Memory Full!";
const char str_add_failed_en[] PROGMEM = "Add Failed!";
const char str_delete_all_confirm_en[] PROGMEM = "Delete All Fgr?";
const char str_yes_no_en[] PROGMEM = "1:Yes 2:No";
const char str_all_fingerprints_en[] PROGMEM = "All Fingerprints";
const char str_deleted_en[] PROGMEM = "Deleted!";
const char str_delete_failed_en[] PROGMEM = "Delete Failed";
const char str_sensor_error_en[] PROGMEM = "Sensor Error";
const char str_checksum_error_en[] PROGMEM = "Checksum Error";
const char str_no_match_found_en[] PROGMEM = "No match found";
const char str_finger_removed_en[] PROGMEM = "Finger removed";
const char str_invalid_id_en[] PROGMEM = "Invalid ID";
const char str_password_ok_en[] PROGMEM = "Password OK";
const char str_operation_en[] PROGMEM = "Operation";
const char str_auto_lock_en[] PROGMEM = "Auto Lock";
const char str_30s_timeout_en[] PROGMEM = "30s Timeout";
const char str_auto_lock_countdown_en[] PROGMEM = "Auto lock: ";
const char str_step_1_3_en[] PROGMEM = "Step 1/3";
const char str_step_2_3_en[] PROGMEM = "Step 2/3";
const char str_step_3_3_en[] PROGMEM = "Step 3/3";
const char str_step_1_ok_en[] PROGMEM = "Step 1 OK";
const char str_step_2_ok_en[] PROGMEM = "Step 2 OK";
const char str_remove_finger_en[] PROGMEM = "Remove finger";
const char str_place_again_en[] PROGMEM = "Place again";
const char str_processing_en[] PROGMEM = "Processing...";
const char str_warning_en[] PROGMEM = "Warning:";
const char str_no_fingerprint_en[] PROGMEM = "No Fingerprint";
const char str_select_language_en[] PROGMEM = "Select Language";
const char str_english_en[] PROGMEM = "English";
const char str_vietnamese_en[] PROGMEM = "Vietnamese";
const char str_language_changed_en[] PROGMEM = "Language Changed";
const char str_security_settings_en[] PROGMEM = "Security Settings";
const char str_wrong_attempts_en[] PROGMEM = "Wrong Attempts";
const char str_0_disabled_en[] PROGMEM = "0:Disabled";
const char str_1_attempt_en[] PROGMEM = "1:1 Attempt";
const char str_2_attempts_en[] PROGMEM = "2:2 Attempts";
const char str_3_attempts_en[] PROGMEM = "3:3 Attempts";
const char str_security_warning_en[] PROGMEM = "SECURITY WARNING!";
const char str_too_many_attempts_en[] PROGMEM = "Too many attempts";
const char str_wait_seconds_en[] PROGMEM = "Wait seconds";
const char str_countdown_en[] PROGMEM = "Countdown: ";
const char str_sec_attempts_en[] PROGMEM = "A:Attempts";
const char str_sec_timeout_en[] PROGMEM = "B:Timeout";
const char str_timeout_settings_en[] PROGMEM = "Timeout Settings";
const char str_5_seconds_en[] PROGMEM = "1:5 seconds";
const char str_10_seconds_en[] PROGMEM = "2:10 seconds";
const char str_20_seconds_en[] PROGMEM = "3:20 seconds";
const char str_30_seconds_en[] PROGMEM = "4:30 seconds";
const char str_40_seconds_en[] PROGMEM = "5:40 seconds";
const char str_50_seconds_en[] PROGMEM = "6:50 seconds";
const char str_60_seconds_en[] PROGMEM = "7:60 seconds";

// ==================== PROGMEM STRINGS - VIETNAMESE ====================
const char str_init_vi[] PROGMEM = "Dang khoi tao...";
const char str_ready_vi[] PROGMEM = "He thong san sang";
const char str_enter_pass_vi[] PROGMEM = "Nhap mat khau";
const char str_wrong_pass_vi[] PROGMEM = "Sai mat khau";
const char str_access_granted_vi[] PROGMEM = "Cho phep vao";
const char str_access_denied_vi[] PROGMEM = "Tu choi truy cap";
const char str_door_unlocked_vi[] PROGMEM = "Cua da mo";
const char str_door_locked_vi[] PROGMEM = "Cua da khoa";
const char str_welcome_vi[] PROGMEM = "Chao mung!";
const char str_setup_mode_vi[] PROGMEM = "Che do cai dat";
const char str_checking_vi[] PROGMEM = "Dang kiem tra...";
const char str_please_wait_vi[] PROGMEM = "Vui long cho";
const char str_cancelled_vi[] PROGMEM = "Da huy";
const char str_timeout_vi[] PROGMEM = "Het thoi gian";
const char str_try_again_vi[] PROGMEM = "Thu lai";
const char str_door_lock_system_vi[] PROGMEM = "He thong khoa cua";
const char str_setup_menu_vi[] PROGMEM = "Menu cai dat";
const char str_change_password_vi[] PROGMEM = "1:Doi mat khau";
const char str_add_fingerprint_vi[] PROGMEM = "2:Them van tay";
const char str_delete_all_vi[] PROGMEM = "3:Xoa tat ca VT";
const char str_security_vi[] PROGMEM = "4:Bao mat";
const char str_language_vi[] PROGMEM = "5:Ngon ngu";
const char str_exit_setup_vi[] PROGMEM = "6:Thoat cai dat";
const char str_setup_cancelled_vi[] PROGMEM = "Huy cai dat";
const char str_setup_complete_vi[] PROGMEM = "Hoan thanh";
const char str_password_cleared_vi[] PROGMEM = "Xoa mat khau";
const char str_password_error_vi[] PROGMEM = "Loi mat khau";
const char str_need_4_digits_vi[] PROGMEM = "Can 4 chu so";
const char str_enter_admin_pass_vi[] PROGMEM = "Nhap MK quan tri";
const char str_change_password_title_vi[] PROGMEM = "Doi mat khau";
const char str_enter_new_4dig_vi[] PROGMEM = "Nhap MK moi (4so)";
const char str_confirm_password_vi[] PROGMEM = "Xac nhan MK";
const char str_enter_again_vi[] PROGMEM = "Nhap lai";
const char str_password_changed_vi[] PROGMEM = "Da doi mat khau";
const char str_successfully_vi[] PROGMEM = "Thanh cong!";
const char str_mismatch_vi[] PROGMEM = "Khong khop!";
const char str_add_fingerprint_title_vi[] PROGMEM = "Them van tay";
const char str_place_finger_vi[] PROGMEM = "Dat ngon tay";
const char str_added_success_vi[] PROGMEM = "Them thanh cong!";
const char str_memory_full_vi[] PROGMEM = "Het bo nho!";
const char str_add_failed_vi[] PROGMEM = "Them that bai!";
const char str_delete_all_confirm_vi[] PROGMEM = "Xoa tat ca VT?";
const char str_yes_no_vi[] PROGMEM = "1:Co 2:Khong";
const char str_all_fingerprints_vi[] PROGMEM = "Tat ca van tay";
const char str_deleted_vi[] PROGMEM = "Da xoa!";
const char str_delete_failed_vi[] PROGMEM = "Xoa that bai";
const char str_sensor_error_vi[] PROGMEM = "Loi cam bien";
const char str_checksum_error_vi[] PROGMEM = "Loi checksum";
const char str_no_match_found_vi[] PROGMEM = "Khong tim thay";
const char str_finger_removed_vi[] PROGMEM = "Da rut tay";
const char str_invalid_id_vi[] PROGMEM = "ID khong hop le";
const char str_password_ok_vi[] PROGMEM = "Mat khau dung";
const char str_operation_vi[] PROGMEM = "Thao tac";
const char str_auto_lock_vi[] PROGMEM = "Tu dong khoa";
const char str_30s_timeout_vi[] PROGMEM = "Het 30 giay";
const char str_auto_lock_countdown_vi[] PROGMEM = "Tu khoa sau: ";
const char str_step_1_3_vi[] PROGMEM = "Buoc 1/3";
const char str_step_2_3_vi[] PROGMEM = "Buoc 2/3";
const char str_step_3_3_vi[] PROGMEM = "Buoc 3/3";
const char str_step_1_ok_vi[] PROGMEM = "Buoc 1 OK";
const char str_step_2_ok_vi[] PROGMEM = "Buoc 2 OK";
const char str_remove_finger_vi[] PROGMEM = "Rut ngon tay";
const char str_place_again_vi[] PROGMEM = "Dat lai";
const char str_processing_vi[] PROGMEM = "Dang xu ly...";
const char str_warning_vi[] PROGMEM = "Canh bao:";
const char str_no_fingerprint_vi[] PROGMEM = "Khong co VT";
const char str_select_language_vi[] PROGMEM = "Chon ngon ngu";
const char str_english_vi[] PROGMEM = "Tieng Anh";
const char str_vietnamese_vi[] PROGMEM = "Tieng Viet";
const char str_language_changed_vi[] PROGMEM = "Doi ngon ngu";
const char str_security_settings_vi[] PROGMEM = "Cai dat bao mat";
const char str_wrong_attempts_vi[] PROGMEM = "Lan sai";
const char str_0_disabled_vi[] PROGMEM = "0:Tat";
const char str_1_attempt_vi[] PROGMEM = "1:1 Lan";
const char str_2_attempts_vi[] PROGMEM = "2:2 Lan";
const char str_3_attempts_vi[] PROGMEM = "3:3 Lan";
const char str_security_warning_vi[] PROGMEM = "CANH BAO BAO MAT!";
const char str_too_many_attempts_vi[] PROGMEM = "Qua nhieu lan sai";
const char str_wait_seconds_vi[] PROGMEM = "Cho giay";
const char str_countdown_vi[] PROGMEM = "Dem nguoc: ";
const char str_sec_attempts_vi[] PROGMEM = "A:Lan sai";
const char str_sec_timeout_vi[] PROGMEM = "B:Thoi gian";
const char str_timeout_settings_vi[] PROGMEM = "Cai dat thoi gian";
const char str_5_seconds_vi[] PROGMEM = "1:5 giay";
const char str_10_seconds_vi[] PROGMEM = "2:10 giay";
const char str_20_seconds_vi[] PROGMEM = "3:20 giay";
const char str_30_seconds_vi[] PROGMEM = "4:30 giay";
const char str_40_seconds_vi[] PROGMEM = "5:40 giay";
const char str_50_seconds_vi[] PROGMEM = "6:50 giay";
const char str_60_seconds_vi[] PROGMEM = "7:60 giay";

// ==================== PIN DEFINITIONS ====================
#define FINGER_RX_PIN 10
#define FINGER_TX_PIN 11
#define TOUCH_OUT_PIN 12
#define RELAY_PIN A0
#define BUZZER_PIN 13

// ==================== KEYPAD SETUP ====================
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 2, 3, 4, 5 };
byte colPins[COLS] = { 6, 7, 8, 9 };
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ==================== LCD SETUP ====================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==================== FINGERPRINT SETUP ====================
SoftwareSerial fingerSerial(FINGER_RX_PIN, FINGER_TX_PIN);

// Fingerprint commands and constants
#define ACK_SUCCESS 0x00
#define ACK_FAIL 0x01
#define ACK_FULL 0x04
#define ACK_NO_USER 0x05
#define ACK_TIMEOUT 0x08
#define ACK_GO_OUT 0x0F
#define USER_MAX_CNT 1000

#define CMD_HEAD 0xF5
#define CMD_TAIL 0xF5
#define CMD_ADD_1 0x01
#define CMD_ADD_2 0x02
#define CMD_ADD_3 0x03
#define CMD_MATCH 0x0C
#define CMD_DEL 0x04
#define CMD_DEL_ALL 0x05
#define CMD_USER_CNT 0x09
#define CMD_COM_LEV 0x28

uint8_t g_rx_buf[10];
bool isInMatchingMode = false;
bool isInAddingMode = false;

// ==================== EEPROM ADDRESSES ====================
#define EEPROM_PASSWORD_ADDR 0
#define EEPROM_INIT_FLAG_ADDR 10
#define EEPROM_SECURITY_SETTING_ADDR 11
#define EEPROM_TIMEOUT_SETTING_ADDR 12

// ==================== GLOBAL VARIABLES ====================
char currentPassword[5] = "1234";
char enteredPassword[5] = "";
byte enteredLen = 0;
byte systemState = 0;  // 0=normal, 1=setup, 2=adding, 3=deleting, 4=security
bool doorUnlocked = false;
unsigned long doorUnlockTime = 0;
unsigned long lastActivityTime = 0;
const unsigned long AUTO_LOCK_DURATION = 30000;  // 30 seconds auto lock
const unsigned long QUIET_PERIOD = 5000;         // 5 seconds quiet before showing countdown
bool lastFingerState = false;
unsigned long lastCountdownUpdate = 0;
bool isShowingCountdown = false;

// ==================== SECURITY VARIABLES ====================
byte wrongAttemptCount = 0;        // CHỈ DÙNG 1 COUNTER CHO CẢ PASSWORD VÀ FINGERPRINT
byte maxWrongAttempts = 3;         // Default: 3 attempts
byte securityTimeoutSeconds = 10;  // Default: 10 seconds
bool isSecurityLocked = false;
unsigned long securityLockTime = 0;

// Timeout options: 5, 10, 20, 30, 40, 50, 60 seconds
const byte TIMEOUT_OPTIONS[] = { 5, 10, 20, 30, 40, 50, 60 };
const byte NUM_TIMEOUT_OPTIONS = 7;

// ==================== HELPER FUNCTIONS ====================
void getTextFromProgmem(const char* text_en, const char* text_vi, char* buffer, byte bufferSize) {
  if (currentLanguage == LANG_ENGLISH) {
    strncpy_P(buffer, text_en, bufferSize - 1);
  } else {
    strncpy_P(buffer, text_vi, bufferSize - 1);
  }
  buffer[bufferSize - 1] = '\0';
}

void displayLang(const char* line1_en, const char* line1_vi, const char* line2_en = nullptr, const char* line2_vi = nullptr) {
  char buffer[17];
  lcd.clear();

  // Line 1
  lcd.setCursor(0, 0);
  getTextFromProgmem(line1_en, line1_vi, buffer, 17);
  lcd.print(buffer);

  // Line 2 (optional)
  if (line2_en && line2_vi) {
    lcd.setCursor(0, 1);
    getTextFromProgmem(line2_en, line2_vi, buffer, 17);
    lcd.print(buffer);
  }
}

void displayText(const char* line1, const char* line2 = nullptr) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);

  if (line2) {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

void beep(byte times, int duration) {
  for (byte i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
    if (i < times - 1) delay(duration);
  }
}

void platformYield() {
  delayMicroseconds(10);
}

bool isRealFingerDetected() {
  return digitalRead(TOUCH_OUT_PIN) == HIGH;
}

void recordActivity() {
  lastActivityTime = millis();
  isShowingCountdown = false;

  if (doorUnlocked && systemState == 0) {
    displayCurrentDoorState();
  }

  DEBUG_PRINTLN_F("Activity recorded - countdown hidden");
}

// ==================== SECURITY FUNCTIONS ====================
void loadSecuritySettings() {
  maxWrongAttempts = EEPROM.read(EEPROM_SECURITY_SETTING_ADDR);
  if (maxWrongAttempts > 3) {  // Invalid value, reset to default
    maxWrongAttempts = 3;
    saveSecuritySettings();
  }

  securityTimeoutSeconds = EEPROM.read(EEPROM_TIMEOUT_SETTING_ADDR);
  if (securityTimeoutSeconds == 255 || securityTimeoutSeconds == 0) {  // Invalid value, reset to default
    securityTimeoutSeconds = 10;
    saveTimeoutSettings();
  }
}

void saveSecuritySettings() {
  EEPROM.write(EEPROM_SECURITY_SETTING_ADDR, maxWrongAttempts);
}

void saveTimeoutSettings() {
  EEPROM.write(EEPROM_TIMEOUT_SETTING_ADDR, securityTimeoutSeconds);
}

void handleSecurityWarning() {
  if (maxWrongAttempts == 0) return;  // Security disabled

  wrongAttemptCount++;  // SỬA: Dùng chung counter cho cả password và fingerprint
  DEBUG_PRINT_F("Wrong attempt count: ");
  DEBUG_PRINTLN(wrongAttemptCount);

  if (wrongAttemptCount >= maxWrongAttempts) {
    // Trigger security warning
    isSecurityLocked = true;
    securityLockTime = millis();
    wrongAttemptCount = 0;  // Reset counter

    // Security warning display and sound
    displayLang(str_security_warning_en, str_security_warning_vi, str_too_many_attempts_en, str_too_many_attempts_vi);

    // Long warning beeps
    for (int i = 0; i < 5; i++) {
      beep(3, 300);
      delay(200);
    }

    delay(2000);
    char buffer[17];
    lcd.clear();
    lcd.setCursor(0, 0);
    getTextFromProgmem(str_wait_seconds_en, str_wait_seconds_vi, buffer, 17);
    lcd.print(buffer);
    lcd.setCursor(0, 1);
    lcd.print(securityTimeoutSeconds);
    lcd.print("s");
    delay(1000);

    // Start countdown
    startSecurityCountdown();
  }
}

void startSecurityCountdown() {
  for (int i = securityTimeoutSeconds; i > 0; i--) {
    char buffer[17];
    lcd.clear();
    lcd.setCursor(0, 0);
    getTextFromProgmem(str_countdown_en, str_countdown_vi, buffer, 17);
    lcd.print(buffer);
    lcd.print(i);
    lcd.print("s");

    // Warning beep every second
    beep(1, 100);
    delay(1000);
  }

  isSecurityLocked = false;
  displayCurrentDoorState();
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(9600);
  DEBUG_PRINTLN_F("Smart Door Lock Starting");

  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TOUCH_OUT_PIN, INPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  displayLang(str_door_lock_system_en, str_door_lock_system_vi, str_init_en, str_init_vi);

  // Initialize fingerprint sensor
  fingerSerial.begin(115200);
  delay(500);

  // Load settings from EEPROM
  loadPasswordFromEEPROM();
  loadSecuritySettings();

  // Initialize fingerprint module
  initFingerModule();

  // Welcome message
  displayCurrentDoorState();
  beep(2, 100);

  DEBUG_PRINTLN_F("System Ready");
}

// ==================== MAIN LOOP ====================
void loop() {
  // Check security lock
  if (isSecurityLocked) {
    // Don't process anything during security lock
    delay(100);
    return;
  }

  // Check auto lock
  if (doorUnlocked && millis() - doorUnlockTime > AUTO_LOCK_DURATION) {
    autoLockDoor();
  }

  // Handle countdown display logic when door is unlocked
  if (doorUnlocked && systemState == 0) {
    unsigned long timeSinceActivity = millis() - lastActivityTime;
    unsigned long timeSinceUnlock = millis() - doorUnlockTime;

    if (timeSinceActivity > QUIET_PERIOD && timeSinceUnlock < AUTO_LOCK_DURATION) {
      if (!isShowingCountdown) {
        isShowingCountdown = true;
        lastCountdownUpdate = millis();
        DEBUG_PRINTLN_F("Starting countdown display");
      }

      if (millis() - lastCountdownUpdate > 1000) {
        updateCountdownDisplay();
        lastCountdownUpdate = millis();
      }
    }
  }

  // Handle keypad input
  char key = keypad.getKey();
  if (key) {
    recordActivity();
    handleKeypadInput(key);
  }

  // Check fingerprint
  if (systemState == 0) {
    bool currentFingerState = isRealFingerDetected();

    if (currentFingerState && !lastFingerState) {
      recordActivity();
      checkFingerprint();
    }

    lastFingerState = currentFingerState;
  }

  delay(100);
}

// ==================== KEYPAD HANDLING ====================
void handleKeypadInput(char key) {
  if (systemState == 1) {  // Setup mode
    handleSetupMode(key);
    return;
  }

  if (systemState >= 2) {  // Adding/deleting fingerprint or security
    if (key == '*') {
      cancelOperation();
    }
    return;
  }

  // Normal operation mode
  if (key == '*') {
    enteredLen = 0;
    enteredPassword[0] = '\0';
    displayLang(str_password_cleared_en, str_password_cleared_vi, str_enter_pass_en, str_enter_pass_vi);
    DEBUG_PRINTLN_F("Password cleared");

  } else if (key == '#') {
    enterSetupMode();

  } else if (key == 'D') {
    if (enteredLen == 4) {
      checkPassword();
    } else {
      displayLang(str_password_error_en, str_password_error_vi, str_need_4_digits_en, str_need_4_digits_vi);
      beep(3, 200);
      delay(1500);
      displayCurrentDoorState();
    }

  } else if (key >= '0' && key <= '9') {
    if (enteredLen < 4) {
      enteredPassword[enteredLen++] = key;
      enteredPassword[enteredLen] = '\0';

      char display[5];
      for (byte i = 0; i < enteredLen; i++) {
        display[i] = '*';
      }
      display[enteredLen] = '\0';

      char buffer[17];
      getTextFromProgmem(str_enter_pass_en, str_enter_pass_vi, buffer, 17);
      displayText(buffer, display);
    }
  }
}

void enterSetupMode() {
  DEBUG_PRINTLN_F("Entering setup mode");
  systemState = 1;
  enteredLen = 0;
  enteredPassword[0] = '\0';
  displayLang(str_setup_mode_en, str_setup_mode_vi, str_enter_admin_pass_en, str_enter_admin_pass_vi);
}

void handleSetupMode(char key) {
  if (key == '*') {
    systemState = 0;
    enteredLen = 0;
    displayLang(str_setup_cancelled_en, str_setup_cancelled_vi, str_ready_en, str_ready_vi);
    delay(1000);
    displayCurrentDoorState();

  } else if (key == 'D') {
    enteredPassword[enteredLen] = '\0';
    if (strcmp(enteredPassword, currentPassword) == 0) {
      showSetupMenu();
    } else {
      displayLang(str_wrong_pass_en, str_wrong_pass_vi, str_access_denied_en, str_access_denied_vi);
      beep(3, 200);
      delay(1500);
      systemState = 0;
      enteredLen = 0;
      displayCurrentDoorState();
    }

  } else if (key >= '0' && key <= '9') {
    if (enteredLen < 4) {
      enteredPassword[enteredLen++] = key;

      char display[5];
      for (byte i = 0; i < enteredLen; i++) {
        display[i] = '*';
      }
      display[enteredLen] = '\0';

      char buffer[17];
      getTextFromProgmem(str_enter_admin_pass_en, str_enter_admin_pass_vi, buffer, 17);
      displayText(buffer, display);
    }
  }
}

void showSetupMenu() {
  showScrollMenu();
}

void showScrollMenu() {
  byte currentOption = 0;  // 0=Password, 1=Add, 2=Delete, 3=Security, 4=Language, 5=Exit
  const int totalOptions = 6;
  bool needRedraw = true;

  while (true) {
    if (needRedraw) {
      char buffer1[17], buffer2[17];

      lcd.clear();
      lcd.setCursor(0, 0);
      getTextFromProgmem(str_setup_menu_en, str_setup_menu_vi, buffer1, 17);
      lcd.print(buffer1);

      lcd.setCursor(0, 1);
      switch (currentOption) {
        case 0:
          getTextFromProgmem(str_change_password_en, str_change_password_vi, buffer2, 17);
          break;
        case 1:
          getTextFromProgmem(str_add_fingerprint_en, str_add_fingerprint_vi, buffer2, 17);
          break;
        case 2:
          getTextFromProgmem(str_delete_all_en, str_delete_all_vi, buffer2, 17);
          break;
        case 3:
          getTextFromProgmem(str_security_en, str_security_vi, buffer2, 17);
          break;
        case 4:
          getTextFromProgmem(str_language_en, str_language_vi, buffer2, 17);
          break;
        case 5:
          getTextFromProgmem(str_exit_setup_en, str_exit_setup_vi, buffer2, 17);
          break;
      }
      lcd.print(buffer2);

      needRedraw = false;
    }

    char key = keypad.getKey();
    if (key) {
      if (key == '#') {  // Next option
        currentOption++;
        if (currentOption >= totalOptions) {
          currentOption = 0;  // Wrap to first
        }
        beep(1, 50);
        needRedraw = true;

      } else if (key == '*') {  // Previous option hoặc cancel
        if (currentOption > 0) {
          currentOption--;
          beep(1, 50);
          needRedraw = true;
        } else {
          // Cancel setup nếu đang ở option đầu
          systemState = 0;
          enteredLen = 0;
          displayLang(str_setup_cancelled_en, str_setup_cancelled_vi, str_ready_en, str_ready_vi);
          delay(1000);
          displayCurrentDoorState();
          return;
        }

      } else if (key == '1') {  // Change Password
        if (currentOption == 0) {
          changePassword();
          return;
        }

      } else if (key == '2') {  // Add Fingerprint
        if (currentOption == 1) {
          addFingerprint();
          return;
        }

      } else if (key == '3') {  // Delete All
        if (currentOption == 2) {
          deleteFingerprint();
          return;
        }

      } else if (key == '4') {  // Security Settings
        if (currentOption == 3) {
          securitySettings();
          needRedraw = true;  // Quay lại menu
          continue;
        } else {
          // Execute current option
          beep(2, 100);
          if (currentOption == 0) {
            changePassword();
            return;
          } else if (currentOption == 1) {
            addFingerprint();
            return;
          } else if (currentOption == 2) {
            deleteFingerprint();
            return;
          }
        }

      } else if (key == '5') {  // Language selection
        if (currentOption == 4) {
          changeLanguage();
          needRedraw = true;
          continue;
        }

      } else if (key == '6') {  // Exit Setup
        if (currentOption == 5) {
          systemState = 0;
          enteredLen = 0;
          displayLang(str_setup_complete_en, str_setup_complete_vi, str_ready_en, str_ready_vi);
          delay(1000);
          displayCurrentDoorState();
          return;
        }
      }
    }
    delay(50);
  }
}

void changeLanguage() {
  byte currentLangOption = 0;  // 0=English, 1=Vietnamese
  bool needRedraw = true;

  while (true) {
    if (needRedraw) {
      char buffer1[17], buffer2[17];

      lcd.clear();
      lcd.setCursor(0, 0);
      getTextFromProgmem(str_select_language_en, str_select_language_vi, buffer1, 17);
      lcd.print(buffer1);

      lcd.setCursor(0, 1);
      if (currentLangOption == 0) {
        getTextFromProgmem(str_english_en, str_english_vi, buffer2, 17);
      } else {
        getTextFromProgmem(str_vietnamese_en, str_vietnamese_vi, buffer2, 17);
      }
      lcd.print("1:");
      lcd.print(buffer2);

      needRedraw = false;
    }

    char key = keypad.getKey();
    if (key) {
      if (key == '#') {                             // Next language
        currentLangOption = 1 - currentLangOption;  // Toggle between 0 and 1
        beep(1, 50);
        needRedraw = true;

      } else if (key == '*') {  // Previous language hoặc cancel
        if (currentLangOption == 0) {
          return;  // Cancel language change
        } else {
          currentLangOption = 0;
          beep(1, 50);
          needRedraw = true;
        }

      } else if (key == '1') {  // Select current language
        if (currentLangOption == 0) {
          currentLanguage = LANG_ENGLISH;
        } else {
          currentLanguage = LANG_VIETNAMESE;
        }

        displayLang(str_language_changed_en, str_language_changed_vi, nullptr, nullptr);
        delay(1500);
        return;
      }
    }
    delay(50);
  }
}

void securitySettings() {
  byte currentSecOption = 0;  // 0=Attempts, 1=Timeout
  bool needRedraw = true;

  while (true) {
    if (needRedraw) {
      char buffer1[17], buffer2[17];

      lcd.clear();
      lcd.setCursor(0, 0);
      getTextFromProgmem(str_security_settings_en, str_security_settings_vi, buffer1, 17);
      lcd.print(buffer1);

      lcd.setCursor(0, 1);
      if (currentSecOption == 0) {
        getTextFromProgmem(str_sec_attempts_en, str_sec_attempts_vi, buffer2, 17);
      } else {
        getTextFromProgmem(str_sec_timeout_en, str_sec_timeout_vi, buffer2, 17);
      }
      lcd.print(buffer2);

      needRedraw = false;
    }

    char key = keypad.getKey();
    if (key) {
      if (key == '#') {  // Next option
        currentSecOption = 1 - currentSecOption;  // Toggle between 0 and 1
        beep(1, 50);
        needRedraw = true;

      } else if (key == '*') {  // Cancel - back to main setup menu
        return;

      } else if (key == 'A') {  // Select Attempts setting
        attemptSettings();
        needRedraw = true;  // Redraw menu when back from submenu

      } else if (key == 'B') {  // Select Timeout setting
        timeoutSettings();
        needRedraw = true;  // Redraw menu when back from submenu
        
      } else if (key == '1') {  // Direct select current option
        if (currentSecOption == 0) {
          attemptSettings();
          needRedraw = true;
        } else {
          timeoutSettings();
          needRedraw = true;
        }
      }
    }
    delay(50);
  }
}

void attemptSettings() {
  byte currentAttemptOption = maxWrongAttempts;  // Current setting
  bool needRedraw = true;

  while (true) {
    if (needRedraw) {
      char buffer1[17], buffer2[17];

      lcd.clear();
      lcd.setCursor(0, 0);
      getTextFromProgmem(str_wrong_attempts_en, str_wrong_attempts_vi, buffer1, 17);
      lcd.print(buffer1);

      lcd.setCursor(0, 1);
      switch (currentAttemptOption) {
        case 0:
          getTextFromProgmem(str_0_disabled_en, str_0_disabled_vi, buffer2, 17);
          break;
        case 1:
          getTextFromProgmem(str_1_attempt_en, str_1_attempt_vi, buffer2, 17);
          break;
        case 2:
          getTextFromProgmem(str_2_attempts_en, str_2_attempts_vi, buffer2, 17);
          break;
        case 3:
          getTextFromProgmem(str_3_attempts_en, str_3_attempts_vi, buffer2, 17);
          break;
      }
      lcd.print(buffer2);

      needRedraw = false;
    }

    char key = keypad.getKey();
    if (key) {
      if (key == '#') {  // Next option
        currentAttemptOption++;
        if (currentAttemptOption > 3) {
          currentAttemptOption = 0;
        }
        beep(1, 50);
        needRedraw = true;

      } else if (key == '*') {  // Previous option hoặc cancel
        if (currentAttemptOption > 0) {
          currentAttemptOption--;
          beep(1, 50);
          needRedraw = true;
        } else {
          return;  // Cancel without saving
        }

      } else if (key == '0' && currentAttemptOption == 0) {  // Select disabled
        maxWrongAttempts = 0;
        saveSecuritySettings();
        displayLang(str_security_settings_en, str_security_settings_vi, str_successfully_en, str_successfully_vi);
        delay(1500);
        return;

      } else if (key == '1' && currentAttemptOption == 1) {  // Select 1 attempt
        maxWrongAttempts = 1;
        saveSecuritySettings();
        displayLang(str_security_settings_en, str_security_settings_vi, str_successfully_en, str_successfully_vi);
        delay(1500);
        return;

      } else if (key == '2' && currentAttemptOption == 2) {  // Select 2 attempts
        maxWrongAttempts = 2;
        saveSecuritySettings();
        displayLang(str_security_settings_en, str_security_settings_vi, str_successfully_en, str_successfully_vi);
        delay(1500);
        return;

      } else if (key == '3' && currentAttemptOption == 3) {  // Select 3 attempts
        maxWrongAttempts = 3;
        saveSecuritySettings();
        displayLang(str_security_settings_en, str_security_settings_vi, str_successfully_en, str_successfully_vi);
        delay(1500);
        return;
      }
    }
    delay(50);
  }
}

void timeoutSettings() {
  byte currentTimeoutIndex = 1;  // Default to 10 seconds (index 1)

  // Find current timeout index
  for (byte i = 0; i < NUM_TIMEOUT_OPTIONS; i++) {
    if (TIMEOUT_OPTIONS[i] == securityTimeoutSeconds) {
      currentTimeoutIndex = i;
      break;
    }
  }

  bool needRedraw = true;

  while (true) {
    if (needRedraw) {
      char buffer1[17], buffer2[17];

      lcd.clear();
      lcd.setCursor(0, 0);
      getTextFromProgmem(str_timeout_settings_en, str_timeout_settings_vi, buffer1, 17);
      lcd.print(buffer1);

      lcd.setCursor(0, 1);
      switch (currentTimeoutIndex) {
        case 0:
          getTextFromProgmem(str_5_seconds_en, str_5_seconds_vi, buffer2, 17);
          break;
        case 1:
          getTextFromProgmem(str_10_seconds_en, str_10_seconds_vi, buffer2, 17);
          break;
        case 2:
          getTextFromProgmem(str_20_seconds_en, str_20_seconds_vi, buffer2, 17);
          break;
        case 3:
          getTextFromProgmem(str_30_seconds_en, str_30_seconds_vi, buffer2, 17);
          break;
        case 4:
          getTextFromProgmem(str_40_seconds_en, str_40_seconds_vi, buffer2, 17);
          break;
        case 5:
          getTextFromProgmem(str_50_seconds_en, str_50_seconds_vi, buffer2, 17);
          break;
        case 6:
          getTextFromProgmem(str_60_seconds_en, str_60_seconds_vi, buffer2, 17);
          break;
      }
      lcd.print(buffer2);

      needRedraw = false;
    }

    char key = keypad.getKey();
    if (key) {
      if (key == '#') {  // Next option
        currentTimeoutIndex++;
        if (currentTimeoutIndex >= NUM_TIMEOUT_OPTIONS) {
          currentTimeoutIndex = 0;
        }
        beep(1, 50);
        needRedraw = true;

      } else if (key == '*') {  // Previous option hoặc cancel
        if (currentTimeoutIndex > 0) {
          currentTimeoutIndex--;
          beep(1, 50);
          needRedraw = true;
        } else {
          return;  // Cancel without saving
        }

      } else if (key >= '1' && key <= '7') {  // Direct selection
        byte selectedIndex = key - '1';
        if (selectedIndex < NUM_TIMEOUT_OPTIONS && selectedIndex == currentTimeoutIndex) {
          securityTimeoutSeconds = TIMEOUT_OPTIONS[selectedIndex];
          saveTimeoutSettings();
          displayLang(str_timeout_settings_en, str_timeout_settings_vi, str_successfully_en, str_successfully_vi);
          delay(1500);
          return;
        }
      }
    }
    delay(50);
  }
}

// ==================== PASSWORD MANAGEMENT ====================
void changePassword() {
  displayLang(str_change_password_title_en, str_change_password_title_vi, str_enter_new_4dig_en, str_enter_new_4dig_vi);

  char newPassword[5] = "";
  byte newLen = 0;

  while (newLen < 4) {
    char key = keypad.getKey();
    if (key) {
      if (key == '*') {
        displayLang(str_change_password_title_en, str_change_password_title_vi, str_cancelled_en, str_cancelled_vi);
        delay(1000);
        return;
      } else if (key >= '0' && key <= '9') {
        newPassword[newLen++] = key;

        char display[5];
        for (byte i = 0; i < newLen; i++) {
          display[i] = '*';
        }
        display[newLen] = '\0';

        char buffer[17];
        getTextFromProgmem(str_enter_new_4dig_en, str_enter_new_4dig_vi, buffer, 17);
        displayText(buffer, display);
      }
    }
    delay(50);
  }
  newPassword[4] = '\0';

  // Confirm password
  displayLang(str_confirm_password_en, str_confirm_password_vi, str_enter_again_en, str_enter_again_vi);
  char confirmPassword[5] = "";
  byte confirmLen = 0;

  while (confirmLen < 4) {
    char key = keypad.getKey();
    if (key) {
      if (key == '*') {
        displayLang(str_change_password_title_en, str_change_password_title_vi, str_cancelled_en, str_cancelled_vi);
        delay(1000);
        return;
      } else if (key >= '0' && key <= '9') {
        confirmPassword[confirmLen++] = key;

        char display[5];
        for (byte i = 0; i < confirmLen; i++) {
          display[i] = '*';
        }
        display[confirmLen] = '\0';

        char buffer[17];
        getTextFromProgmem(str_confirm_password_en, str_confirm_password_vi, buffer, 17);
        displayText(buffer, display);
      }
    }
    delay(50);
  }
  confirmPassword[4] = '\0';

  if (strcmp(newPassword, confirmPassword) == 0) {
    strcpy(currentPassword, newPassword);
    savePasswordToEEPROM();
    displayLang(str_password_changed_en, str_password_changed_vi, str_successfully_en, str_successfully_vi);
    beep(2, 100);
  } else {
    displayLang(str_password_error_en, str_password_error_vi, str_mismatch_en, str_mismatch_vi);
    beep(3, 200);
  }

  delay(2000);
  systemState = 0;
  enteredLen = 0;
  displayCurrentDoorState();
}

void loadPasswordFromEEPROM() {
  if (EEPROM.read(EEPROM_INIT_FLAG_ADDR) != 0xAA) {
    savePasswordToEEPROM();
    return;
  }

  for (byte i = 0; i < 4; i++) {
    currentPassword[i] = EEPROM.read(EEPROM_PASSWORD_ADDR + i);
  }
  currentPassword[4] = '\0';
}

void savePasswordToEEPROM() {
  for (byte i = 0; i < 4; i++) {
    EEPROM.write(EEPROM_PASSWORD_ADDR + i, currentPassword[i]);
  }
  EEPROM.write(EEPROM_INIT_FLAG_ADDR, 0xAA);
}

// ==================== FINGERPRINT MANAGEMENT ====================
void addFingerprint() {
  systemState = 2;
  isInAddingMode = true;
  displayLang(str_add_fingerprint_title_en, str_add_fingerprint_title_vi, str_place_finger_en, str_place_finger_vi);

  DEBUG_PRINTLN_F("Starting addFingerprint");

  uint8_t result = addUser();

  DEBUG_PRINT_F("addUser result: ");
  DEBUG_PRINTLN(result);

  if (result == ACK_SUCCESS) {
    displayLang(str_add_fingerprint_title_en, str_add_fingerprint_title_vi, str_added_success_en, str_added_success_vi);
    beep(2, 100);
    DEBUG_PRINTLN_F("Fingerprint added successfully");
  } else if (result == ACK_FULL) {
    displayLang(str_add_fingerprint_title_en, str_add_fingerprint_title_vi, str_memory_full_en, str_memory_full_vi);
    beep(3, 200);
    DEBUG_PRINTLN_F("Fingerprint memory full");
  } else if (result == ACK_TIMEOUT) {
    displayLang(str_add_fingerprint_title_en, str_add_fingerprint_title_vi, str_timeout_en, str_timeout_vi);
    beep(3, 200);
    DEBUG_PRINTLN_F("Fingerprint add timeout");
  } else {
    displayLang(str_add_fingerprint_title_en, str_add_fingerprint_title_vi, str_add_failed_en, str_add_failed_vi);
    beep(3, 200);
    DEBUG_PRINT_F("Fingerprint add failed with code: ");
    DEBUG_PRINTLN(result);
  }

  delay(2000);
  systemState = 0;
  isInAddingMode = false;
  enteredLen = 0;
  displayCurrentDoorState();
}

void deleteFingerprint() {
  systemState = 3;
  displayLang(str_delete_all_confirm_en, str_delete_all_confirm_vi, str_yes_no_en, str_yes_no_vi);

  while (true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '1') {
        uint8_t result = clearAllUser();
        if (result == ACK_SUCCESS) {
          displayLang(str_all_fingerprints_en, str_all_fingerprints_vi, str_deleted_en, str_deleted_vi);
          beep(2, 100);
        } else {
          displayLang(str_delete_failed_en, str_delete_failed_vi, str_try_again_en, str_try_again_vi);
          beep(3, 200);
        }
        delay(2000);
        break;
      } else if (key == '2' || key == '*') {
        displayLang(str_operation_en, str_operation_vi, str_cancelled_en, str_cancelled_vi);
        delay(1000);
        break;
      }
    }
    delay(50);
  }

  systemState = 0;
  enteredLen = 0;
  displayCurrentDoorState();
}

void checkFingerprint() {
  displayLang(str_checking_en, str_checking_vi, str_please_wait_en, str_please_wait_vi);

  DEBUG_PRINTLN_F("Processing fingerprint...");

  fingerSerial.flush();
  delay(100);
  while (fingerSerial.available()) {
    fingerSerial.read();
    delay(1);
  }

  delay(500);

  if (!isRealFingerDetected()) {
    DEBUG_PRINTLN_F("Finger removed");
    displayCurrentDoorState();
    return;
  }

  uint8_t commandBuf[5] = { CMD_MATCH, 0, 0, 0, 0 };
  uint8_t r = txAndRxCmd(commandBuf, 8, 3);

  if (r != ACK_SUCCESS) {
    DEBUG_PRINT_F("Comm error: 0x");
    DEBUG_PRINTLN(r);
    displayLang(str_sensor_error_en, str_sensor_error_vi, str_try_again_en, str_try_again_vi);
    delay(1500);
    displayCurrentDoorState();
    return;
  }

  // Verify checksum
  uint8_t checkSum = 0;
  for (int i = 1; i < 6; i++) {
    checkSum ^= g_rx_buf[i];
  }

  if (checkSum != g_rx_buf[6]) {
    DEBUG_PRINTLN_F("Checksum error");
    displayLang(str_checksum_error_en, str_checksum_error_vi, str_try_again_en, str_try_again_vi);
    delay(1500);
    displayCurrentDoorState();
    return;
  }

  uint8_t userID = g_rx_buf[3];
  uint8_t statusCode = g_rx_buf[4];

// Debug: Show what we received
#if DEBUG_ENABLED
  Serial.print(F("Response: "));
  for (int i = 0; i < 8; i++) {
    Serial.print(F("0x"));
    if (g_rx_buf[i] < 16) Serial.print(F("0"));
    Serial.print(g_rx_buf[i], HEX);
    Serial.print(F(" "));
  }
  Serial.print(F(" | ID: "));
  Serial.print(userID);
  Serial.print(F(" | Status: 0x"));
  Serial.print(statusCode, HEX);
  Serial.print(F(" | Touch: "));
  Serial.println(isRealFingerDetected() ? F("REAL") : F("NONE"));
#endif

  if (userID == 0) {
    displayLang(str_access_denied_en, str_access_denied_vi, str_no_match_found_en, str_no_match_found_vi);
    beep(3, 200);
    DEBUG_PRINTLN_F("No match found");

    // SỬA: Gọi security warning cho fingerprint fail
    handleSecurityWarning();

    delay(1500);
    displayCurrentDoorState();
  } else if (userID > 0 && userID <= USER_MAX_CNT) {
    if (isRealFingerDetected()) {
      // Successful fingerprint match - reset wrong attempt counter
      wrongAttemptCount = 0;  // SỬA: Reset counter khi thành công
      resetDoorTimer();
      toggleDoorState();
      DEBUG_PRINT_F("*** MATCHED User ID: ");
      DEBUG_PRINT(userID);
      DEBUG_PRINTLN_F(" (REAL) ***");
    } else {
      displayLang(str_access_denied_en, str_access_denied_vi, str_finger_removed_en, str_finger_removed_vi);
      beep(3, 200);
      DEBUG_PRINT_F("Match but finger removed: ");
      DEBUG_PRINTLN(userID);
      delay(1500);
      displayCurrentDoorState();
    }
  } else {
    displayLang(str_access_denied_en, str_access_denied_vi, str_invalid_id_en, str_invalid_id_vi);
    beep(3, 200);
    DEBUG_PRINT_F("Invalid User ID: ");
    DEBUG_PRINTLN(userID);
    delay(1500);
    displayCurrentDoorState();
  }
}

// ==================== DOOR CONTROL ====================
void resetDoorTimer() {
  doorUnlockTime = millis();
  lastActivityTime = millis();
  isShowingCountdown = false;
  DEBUG_PRINTLN_F("Door timer reset - successful authentication");
}

void toggleDoorState() {
  if (doorUnlocked) {
    lockDoor();
  } else {
    unlockDoor();
  }
}

void unlockDoor() {
  doorUnlocked = true;
  resetDoorTimer();
  digitalWrite(RELAY_PIN, HIGH);
  displayLang(str_door_unlocked_en, str_door_unlocked_vi, str_welcome_en, str_welcome_vi);
  beep(1, 500);
  delay(1000);
  displayCurrentDoorState();
  DEBUG_PRINTLN_F("Door unlocked");
}

void lockDoor() {
  doorUnlocked = false;
  digitalWrite(RELAY_PIN, LOW);
  enteredLen = 0;
  enteredPassword[0] = '\0';
  isShowingCountdown = false;
  displayCurrentDoorState();
  beep(2, 200);
  DEBUG_PRINTLN_F("Door locked");
}

void autoLockDoor() {
  if (doorUnlocked) {
    doorUnlocked = false;
    digitalWrite(RELAY_PIN, LOW);
    enteredLen = 0;
    enteredPassword[0] = '\0';
    isShowingCountdown = false;
    displayLang(str_auto_lock_en, str_auto_lock_vi, str_30s_timeout_en, str_30s_timeout_vi);
    beep(3, 300);
    delay(2000);
    displayCurrentDoorState();
    DEBUG_PRINTLN_F("Door auto-locked after timeout");
  }
}

void displayCurrentDoorState() {
  if (doorUnlocked) {
    displayLang(str_door_unlocked_en, str_door_unlocked_vi, str_enter_pass_en, str_enter_pass_vi);
  } else {
    displayLang(str_ready_en, str_ready_vi, str_enter_pass_en, str_enter_pass_vi);
  }
}

void updateCountdownDisplay() {
  if (!doorUnlocked || systemState != 0 || !isShowingCountdown) return;

  unsigned long timeSinceUnlock = millis() - doorUnlockTime;
  unsigned long remainingTime = AUTO_LOCK_DURATION - timeSinceUnlock;

  if (remainingTime <= AUTO_LOCK_DURATION) {
    int secondsLeft = remainingTime / 1000;

    static int lastSecondsDisplayed = -1;
    if (secondsLeft != lastSecondsDisplayed) {
      char buffer[17];
      lcd.clear();

      // First line
      getTextFromProgmem(str_door_unlocked_en, str_door_unlocked_vi, buffer, 17);
      lcd.setCursor(0, 0);
      lcd.print(buffer);

      // Second line with countdown
      lcd.setCursor(0, 1);
      getTextFromProgmem(str_auto_lock_countdown_en, str_auto_lock_countdown_vi, buffer, 17);
      lcd.print(buffer);
      if (secondsLeft < 10) {
        lcd.print("0");
      }
      lcd.print(secondsLeft);
      lcd.print("s");

      lastSecondsDisplayed = secondsLeft;

      // Warning beep when less than 5 seconds
      if (secondsLeft <= 5 && secondsLeft > 0) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(50);
        digitalWrite(BUZZER_PIN, LOW);
      }
    }
  }
}

void checkPassword() {
  enteredPassword[enteredLen] = '\0';

  if (strcmp(enteredPassword, currentPassword) == 0) {
    // Correct password - reset wrong counter
    wrongAttemptCount = 0;  // SỬA: Reset counter khi thành công
    displayLang(str_access_granted_en, str_access_granted_vi, str_password_ok_en, str_password_ok_vi);
    delay(1000);
    resetDoorTimer();
    toggleDoorState();
  } else {
    // Wrong password - handle security
    displayLang(str_access_denied_en, str_access_denied_vi, str_wrong_pass_en, str_wrong_pass_vi);
    beep(3, 200);
    delay(1500);
    enteredLen = 0;
    enteredPassword[0] = '\0';

    // Handle security warning
    handleSecurityWarning();

    displayCurrentDoorState();
  }
}

void cancelOperation() {
  systemState = 0;
  isInAddingMode = false;
  enteredLen = 0;
  enteredPassword[0] = '\0';
  displayLang(str_operation_en, str_operation_vi, str_cancelled_en, str_cancelled_vi);
  delay(1000);
  displayCurrentDoorState();
}

// ==================== FINGERPRINT FUNCTIONS ====================
uint8_t txAndRxCmd(uint8_t* commandBuf, uint8_t rxBytesNeed, float timeout) {
  uint8_t checkSum = 0;
  uint8_t txBuf[10];
  uint8_t txLen = 0;

  txBuf[txLen++] = CMD_HEAD;
  for (int i = 0; i < 5; i++) {
    txBuf[txLen++] = commandBuf[i];
    checkSum ^= commandBuf[i];
  }
  txBuf[txLen++] = checkSum;
  txBuf[txLen++] = CMD_TAIL;

  fingerSerial.flush();
  for (int i = 0; i < txLen; i++) {
    fingerSerial.write(txBuf[i]);
  }

  memset(g_rx_buf, 0, sizeof(g_rx_buf));
  unsigned long startTime = millis();
  int rxIndex = 0;

  while ((millis() - startTime) < (timeout * 1000) && rxIndex < rxBytesNeed) {
    if (fingerSerial.available()) {
      g_rx_buf[rxIndex++] = fingerSerial.read();
    }
    platformYield();
  }

  if (rxIndex != rxBytesNeed) return ACK_TIMEOUT;
  if (g_rx_buf[0] != CMD_HEAD) return ACK_FAIL;
  if (g_rx_buf[rxBytesNeed - 1] != CMD_TAIL) return ACK_FAIL;
  if (g_rx_buf[1] != txBuf[1]) return ACK_FAIL;

  checkSum = 0;
  for (int i = 1; i < rxBytesNeed - 2; i++) {
    checkSum ^= g_rx_buf[i];
  }
  if (checkSum != g_rx_buf[rxBytesNeed - 2]) {
    return ACK_FAIL;
  }

  return ACK_SUCCESS;
}

uint8_t txAndRxCmdWithCancel(uint8_t* commandBuf, uint8_t rxBytesNeed, float timeout) {
  uint8_t checkSum = 0;
  uint8_t txBuf[10];
  uint8_t txLen = 0;

  txBuf[txLen++] = CMD_HEAD;
  for (int i = 0; i < 5; i++) {
    txBuf[txLen++] = commandBuf[i];
    checkSum ^= commandBuf[i];
  }
  txBuf[txLen++] = checkSum;
  txBuf[txLen++] = CMD_TAIL;

  fingerSerial.flush();
  for (int i = 0; i < txLen; i++) {
    fingerSerial.write(txBuf[i]);
  }

  memset(g_rx_buf, 0, sizeof(g_rx_buf));
  unsigned long startTime = millis();
  int rxIndex = 0;

  while ((millis() - startTime) < (timeout * 1000) && rxIndex < rxBytesNeed) {
    char key = keypad.getKey();
    if (key == '*') {
      DEBUG_PRINTLN_F("Operation cancelled by keypad");
      return ACK_FAIL;
    }

    if (fingerSerial.available()) {
      g_rx_buf[rxIndex++] = fingerSerial.read();
    }
    platformYield();
  }

  if (rxIndex != rxBytesNeed) return ACK_TIMEOUT;
  if (g_rx_buf[0] != CMD_HEAD) return ACK_FAIL;
  if (g_rx_buf[rxBytesNeed - 1] != CMD_TAIL) return ACK_FAIL;
  if (g_rx_buf[1] != txBuf[1]) return ACK_FAIL;

  checkSum = 0;
  for (int i = 1; i < rxBytesNeed - 2; i++) {
    checkSum ^= g_rx_buf[i];
  }

  if (checkSum != g_rx_buf[rxBytesNeed - 2]) {
    return ACK_FAIL;
  }

  return ACK_SUCCESS;
}

uint8_t setCompareLevel(uint8_t level) {
  uint8_t commandBuf[5] = { CMD_COM_LEV, 0, level, 0, 0 };
  uint8_t r = txAndRxCmd(commandBuf, 8, 0.5);

  if (r == ACK_TIMEOUT) return ACK_TIMEOUT;
  if (r == ACK_SUCCESS && g_rx_buf[4] == ACK_SUCCESS) return g_rx_buf[3];
  return 0xFF;
}

uint8_t getUserCount() {
  uint8_t commandBuf[5] = { CMD_USER_CNT, 0, 0, 0, 0 };
  uint8_t r = txAndRxCmd(commandBuf, 8, 0.5);

  if (r == ACK_TIMEOUT) return ACK_TIMEOUT;
  if (r == ACK_SUCCESS && g_rx_buf[4] == ACK_SUCCESS) return g_rx_buf[3];
  return 0xFF;
}

bool waitForRealFinger(unsigned long timeoutMs) {
  unsigned long startTime = millis();
  DEBUG_PRINTLN_F("Waiting for real finger...");

  while ((millis() - startTime) < timeoutMs) {
    char key = keypad.getKey();
    if (key == '*') {
      DEBUG_PRINTLN_F("Operation cancelled.");
      return false;
    }

    if (isRealFingerDetected()) {
      DEBUG_PRINTLN_F("Real finger detected!");
      return true;
    }

    delay(100);
    platformYield();
  }

  DEBUG_PRINTLN_F("Timeout waiting for finger.");
  return false;
}

void waitForFingerRemoval() {
  DEBUG_PRINTLN_F("Please remove finger...");
  while (isRealFingerDetected()) {
    delay(100);
    platformYield();
  }
  DEBUG_PRINTLN_F("Finger removed.");
  delay(500);
}

void initFingerModule() {
  DEBUG_PRINTLN_F("Initializing fingerprint module...");
  delay(500);

  int attempts = 0;
  while (setCompareLevel(5) != 5 && attempts < 10) {
    DEBUG_PRINTLN_F("***ERROR***: Please check module connection.");
    delay(1000);
    attempts++;
  }

  if (attempts >= 10) {
    DEBUG_PRINTLN_F("Failed to initialize module!");
    displayLang(str_warning_en, str_warning_vi, str_no_fingerprint_en, str_no_fingerprint_vi);
    delay(2000);
    return;
  }

  DEBUG_PRINTLN_F("Module initialized successfully!");
  DEBUG_PRINTLN_F("TOUCH_OUT detection enabled");
}

uint8_t addUser() {
  isInAddingMode = true;

  uint8_t userCount = getUserCount();
  DEBUG_PRINT_F("Current count: ");
  DEBUG_PRINTLN(userCount);

  if (userCount >= USER_MAX_CNT) {
    isInAddingMode = false;
    return ACK_FULL;
  }

  uint8_t newUserId = userCount + 1;
  DEBUG_PRINT_F("Adding ID: ");
  DEBUG_PRINTLN(newUserId);

  // Step 1
  displayLang(str_step_1_3_en, str_step_1_3_vi, str_place_finger_en, str_place_finger_vi);
  DEBUG_PRINTLN_F("Step 1: Place finger");

  if (!waitForRealFinger(30000)) {
    isInAddingMode = false;
    return ACK_FAIL;
  }

  DEBUG_PRINTLN_F("Keep steady...");
  delay(2000);

  if (!isRealFingerDetected()) {
    DEBUG_PRINTLN_F("Finger removed too early");
    isInAddingMode = false;
    return ACK_FAIL;
  }

  uint8_t commandBuf[5] = { CMD_ADD_1, 0, newUserId, 3, 0 };
  uint8_t r = txAndRxCmdWithCancel(commandBuf, 8, 15);

  if (r == ACK_FAIL) {
    isInAddingMode = false;
    return ACK_FAIL;
  }

  DEBUG_PRINT_F("ADD_1: ");
#if DEBUG_ENABLED
  for (int i = 0; i < 8; i++) {
    Serial.print(F("0x"));
    if (g_rx_buf[i] < 16) Serial.print(F("0"));
    Serial.print(g_rx_buf[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();
#endif

  if (r != ACK_SUCCESS || g_rx_buf[4] != ACK_SUCCESS) {
    DEBUG_PRINT_F("First scan failed: 0x");
    DEBUG_PRINTLN(g_rx_buf[4]);
    isInAddingMode = false;
    return ACK_FAIL;
  }

  DEBUG_PRINTLN_F("First scan OK!");
  displayLang(str_step_1_ok_en, str_step_1_ok_vi, str_remove_finger_en, str_remove_finger_vi);

  waitForFingerRemoval();
  delay(1000);

  // Step 2
  displayLang(str_step_2_3_en, str_step_2_3_vi, str_place_again_en, str_place_again_vi);
  DEBUG_PRINTLN_F("Step 2: Same finger again");

  if (!waitForRealFinger(30000)) {
    isInAddingMode = false;
    return ACK_FAIL;
  }

  DEBUG_PRINTLN_F("Keep steady...");
  delay(2000);

  if (!isRealFingerDetected()) {
    DEBUG_PRINTLN_F("Finger removed too early");
    isInAddingMode = false;
    return ACK_FAIL;
  }

  commandBuf[0] = CMD_ADD_2;
  r = txAndRxCmdWithCancel(commandBuf, 8, 15);

  if (r == ACK_FAIL) {
    isInAddingMode = false;
    return ACK_FAIL;
  }

  DEBUG_PRINT_F("ADD_2: ");
#if DEBUG_ENABLED
  for (int i = 0; i < 8; i++) {
    Serial.print(F("0x"));
    if (g_rx_buf[i] < 16) Serial.print(F("0"));
    Serial.print(g_rx_buf[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();
#endif

  if (r != ACK_SUCCESS || g_rx_buf[4] != ACK_SUCCESS) {
    DEBUG_PRINT_F("Second scan failed: 0x");
    DEBUG_PRINTLN(g_rx_buf[4]);
    isInAddingMode = false;
    return ACK_FAIL;
  }

  DEBUG_PRINTLN_F("Second scan OK!");
  displayLang(str_step_2_ok_en, str_step_2_ok_vi, str_remove_finger_en, str_remove_finger_vi);
  delay(2000);

  // Step 3
  displayLang(str_step_3_3_en, str_step_3_3_vi, str_processing_en, str_processing_vi);
  DEBUG_PRINTLN_F("Step 3: Creating template...");
  commandBuf[0] = CMD_ADD_3;
  r = txAndRxCmdWithCancel(commandBuf, 8, 15);

  if (r == ACK_FAIL) {
    isInAddingMode = false;
    return ACK_FAIL;
  }

  DEBUG_PRINT_F("ADD_3: ");
#if DEBUG_ENABLED
  for (int i = 0; i < 8; i++) {
    Serial.print(F("0x"));
    if (g_rx_buf[i] < 16) Serial.print(F("0"));
    Serial.print(g_rx_buf[i], HEX);
    Serial.print(F(" "));
  }
  Serial.println();
#endif

  if (r == ACK_SUCCESS && g_rx_buf[4] == ACK_SUCCESS) {
    uint8_t savedID = g_rx_buf[3];
    DEBUG_PRINT_F("SUCCESS! Saved to ID: ");
    DEBUG_PRINTLN(savedID);

    uint8_t newCount = getUserCount();
    DEBUG_PRINT_F("Count: ");
    DEBUG_PRINT(userCount);
    DEBUG_PRINT_F(" -> ");
    DEBUG_PRINTLN(newCount);

    isInAddingMode = false;
    return ACK_SUCCESS;
  } else {
    DEBUG_PRINT_F("Template failed: 0x");
    DEBUG_PRINTLN(g_rx_buf[4]);
    isInAddingMode = false;
    return ACK_FAIL;
  }
}

uint8_t clearAllUser() {
  uint8_t commandBuf[5] = { CMD_DEL_ALL, 0, 0, 0, 0 };
  uint8_t r = txAndRxCmd(commandBuf, 8, 5);

  if (r == ACK_TIMEOUT) return ACK_TIMEOUT;
  if (r == ACK_SUCCESS && g_rx_buf[4] == ACK_SUCCESS) return ACK_SUCCESS;
  return ACK_FAIL;
}