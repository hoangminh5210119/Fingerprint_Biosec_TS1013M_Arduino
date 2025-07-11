/*
 * Universal Fingerprint Reader with TOUCH_OUT
 * Compatible with: Arduino Uno, ESP8266, ESP32
 * Uses SoftwareSerial for Arduino Uno, HardwareSerial for ESP32
 */

// Platform detection and includes
#if defined(ESP32)
  #include <WiFi.h>
  #define PLATFORM_NAME "ESP32"
  #define USE_HARDWARE_SERIAL
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <SoftwareSerial.h>
  #define PLATFORM_NAME "ESP8266"
  #define USE_SOFTWARE_SERIAL
#else
  #include <SoftwareSerial.h>
  #define PLATFORM_NAME "Arduino"
  #define USE_SOFTWARE_SERIAL
#endif

// Pin definitions based on platform
#if defined(ESP32)
  #define FINGER_RX_PIN    16    // GPIO16
  #define FINGER_TX_PIN    17    // GPIO17
  #define TOUCH_OUT_PIN    18    // GPIO18
  #define SERIAL_BAUD      115200
  #define FINGER_BAUD      115200
#elif defined(ESP8266)
  #define FINGER_RX_PIN    D5    // GPIO14
  #define FINGER_TX_PIN    D6    // GPIO12
  #define TOUCH_OUT_PIN    D7    // GPIO13
  #define SERIAL_BAUD      115200
  #define FINGER_BAUD      115200
#else // Arduino Uno
  #define FINGER_RX_PIN    2     // Digital pin 2
  #define FINGER_TX_PIN    3     // Digital pin 3
  #define TOUCH_OUT_PIN    4     // Digital pin 4
  #define SERIAL_BAUD      115200   // Arduino Uno serial limit
  #define FINGER_BAUD      115200  // Lower baud for Arduino
#endif

// Serial initialization based on platform
#ifdef USE_HARDWARE_SERIAL
  HardwareSerial fingerSerial(2); // ESP32 uses UART2
#else
  SoftwareSerial fingerSerial(FINGER_RX_PIN, FINGER_TX_PIN);
#endif

// Constants
#define ACK_SUCCESS      0x00
#define ACK_FAIL         0x01
#define ACK_FULL         0x04
#define ACK_NO_USER      0x05
#define ACK_TIMEOUT      0x08
#define ACK_GO_OUT       0x0F

#define USER_MAX_CNT     1000

// Commands
#define CMD_HEAD         0xF5
#define CMD_TAIL         0xF5
#define CMD_ADD_1        0x01
#define CMD_ADD_2        0x02
#define CMD_ADD_3        0x03
#define CMD_MATCH        0x0C
#define CMD_DEL          0x04
#define CMD_DEL_ALL      0x05
#define CMD_USER_CNT     0x09
#define CMD_COM_LEV      0x28

// Global variables
uint8_t g_rx_buf[10];
uint8_t fingerSleepFlag = 0;
bool isInMatchingMode = false;
bool isInAddingMode = false;

void setup() {
  Serial.begin(SERIAL_BAUD);
  
  // Platform-specific serial initialization
#ifdef USE_HARDWARE_SERIAL
  fingerSerial.begin(FINGER_BAUD, SERIAL_8N1, FINGER_RX_PIN, FINGER_TX_PIN);
#else
  fingerSerial.begin(FINGER_BAUD);
#endif
  
  pinMode(TOUCH_OUT_PIN, INPUT);
  
  // WiFi off for ESP platforms
#if defined(ESP32) || defined(ESP8266)
  WiFi.mode(WIFI_OFF);
#endif
  
  // Platform identification
  Serial.println("=== Universal Fingerprint Reader ===");
  Serial.print("Platform: ");
  Serial.println(PLATFORM_NAME);
  Serial.print("Serial baud: ");
  Serial.println(SERIAL_BAUD);
  Serial.print("Finger baud: ");
  Serial.println(FINGER_BAUD);
  
  initFingerModule();
  printMenu();
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    
    if (command == "ESC" || command == "CANCEL" || command == "EXIT") {
      if (isInMatchingMode || isInAddingMode) {
        Serial.println("Operation cancelled by user.");
        isInMatchingMode = false;
        isInAddingMode = false;
        printMenu();
        return;
      }
    }
    
    if (!isInMatchingMode && !isInAddingMode) {
      analysisPCCommand(command);
    } else {
      Serial.println("System is busy. Send 'ESC' to exit.");
    }
  }
  
  delay(100);
}

void initFingerModule() {
  Serial.println("Initializing fingerprint module...");
  delay(500);
  
  int attempts = 0;
  while (setCompareLevel(5) != 5 && attempts < 10) {
    Serial.println("***ERROR***: Please check module connection.");
    delay(1000);
    attempts++;
  }
  
  if (attempts >= 10) {
    Serial.println("Failed to initialize module!");
    return;
  }
  
  Serial.println("Module initialized successfully!");
  Serial.println("TOUCH_OUT detection enabled");
}

// Platform-independent yield function
void platformYield() {
#if defined(ESP32) || defined(ESP8266)
  yield();
#else
  // Arduino Uno doesn't need yield
  delayMicroseconds(10);
#endif
}

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
  
  // Correct checksum calculation
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
    if (Serial.available()) {
      String command = Serial.readStringUntil('\n');
      command.trim();
      command.toUpperCase();
      if (command == "ESC" || command == "CANCEL" || command == "EXIT") {
        Serial.println("Operation cancelled by user.");
        return ACK_FAIL;
      }
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
  
  // Correct checksum calculation
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
  uint8_t commandBuf[5] = {CMD_COM_LEV, 0, level, 0, 0};
  uint8_t r = txAndRxCmd(commandBuf, 8, 0.5); // Longer timeout for Arduino
  
  if (r == ACK_TIMEOUT) return ACK_TIMEOUT;
  if (r == ACK_SUCCESS && g_rx_buf[4] == ACK_SUCCESS) return g_rx_buf[3];
  return 0xFF;
}

uint8_t getUserCount() {
  uint8_t commandBuf[5] = {CMD_USER_CNT, 0, 0, 0, 0};
  uint8_t r = txAndRxCmd(commandBuf, 8, 0.5);
  
  if (r == ACK_TIMEOUT) return ACK_TIMEOUT;
  if (r == ACK_SUCCESS && g_rx_buf[4] == ACK_SUCCESS) return g_rx_buf[3];
  return 0xFF;
}

bool isRealFingerDetected() {
  return digitalRead(TOUCH_OUT_PIN) == HIGH;
}

bool waitForRealFinger(unsigned long timeoutMs) {
  unsigned long startTime = millis();
  Serial.println("Waiting for real finger...");
  
  while ((millis() - startTime) < timeoutMs) {
    if (Serial.available()) {
      String command = Serial.readStringUntil('\n');
      command.trim();
      command.toUpperCase();
      if (command == "ESC" || command == "CANCEL" || command == "EXIT") {
        Serial.println("Operation cancelled.");
        return false;
      }
    }
    
    if (isRealFingerDetected()) {
      Serial.println("Real finger detected!");
      return true;
    }
    
    delay(100);
    platformYield();
  }
  
  Serial.println("Timeout waiting for finger.");
  return false;
}

void waitForFingerRemoval() {
  Serial.println("Please remove finger...");
  while (isRealFingerDetected()) {
    delay(100);
    platformYield();
  }
  Serial.println("Finger removed.");
  delay(500);
}

uint8_t addUser() {
  isInAddingMode = true;
  
  uint8_t userCount = getUserCount();
  Serial.print("Current count: ");
  Serial.println(userCount);
  
  if (userCount >= USER_MAX_CNT) {
    isInAddingMode = false;
    return ACK_FULL;
  }
  
  uint8_t newUserId = userCount + 1;
  Serial.print("Adding ID: ");
  Serial.println(newUserId);
  
  // Step 1
  Serial.println("Step 1: Place finger");
  
  if (!waitForRealFinger(30000)) {
    isInAddingMode = false;
    return ACK_FAIL;
  }
  
  Serial.println("Keep steady...");
  delay(2000);
  
  if (!isRealFingerDetected()) {
    Serial.println("Finger removed too early");
    isInAddingMode = false;
    return ACK_FAIL;
  }
  
  uint8_t commandBuf[5] = {CMD_ADD_1, 0, newUserId, 3, 0};
  uint8_t r = txAndRxCmdWithCancel(commandBuf, 8, 15);
  
  if (r == ACK_FAIL) {
    isInAddingMode = false;
    return ACK_FAIL;
  }
  
  Serial.print("ADD_1: ");
  for (int i = 0; i < 8; i++) {
    Serial.print("0x");
    if (g_rx_buf[i] < 16) Serial.print("0");
    Serial.print(g_rx_buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  if (r != ACK_SUCCESS || g_rx_buf[4] != ACK_SUCCESS) {
    Serial.print("First scan failed: 0x");
    Serial.println(g_rx_buf[4], HEX);
    isInAddingMode = false;
    return ACK_FAIL;
  }
  
  Serial.println("First scan OK!");
  
  waitForFingerRemoval();
  delay(1000);
  
  // Step 2
  Serial.println("Step 2: Same finger again");
  
  if (!waitForRealFinger(30000)) {
    isInAddingMode = false;
    return ACK_FAIL;
  }
  
  Serial.println("Keep steady...");
  delay(2000);
  
  if (!isRealFingerDetected()) {
    Serial.println("Finger removed too early");
    isInAddingMode = false;
    return ACK_FAIL;
  }
  
  commandBuf[0] = CMD_ADD_2;
  r = txAndRxCmdWithCancel(commandBuf, 8, 15);
  
  if (r == ACK_FAIL) {
    isInAddingMode = false;
    return ACK_FAIL;
  }
  
  Serial.print("ADD_2: ");
  for (int i = 0; i < 8; i++) {
    Serial.print("0x");
    if (g_rx_buf[i] < 16) Serial.print("0");
    Serial.print(g_rx_buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  if (r != ACK_SUCCESS || g_rx_buf[4] != ACK_SUCCESS) {
    Serial.print("Second scan failed: 0x");
    Serial.println(g_rx_buf[4], HEX);
    isInAddingMode = false;
    return ACK_FAIL;
  }
  
  Serial.println("Second scan OK!");
  delay(2000);
  
  // Step 3
  Serial.println("Step 3: Creating template...");
  commandBuf[0] = CMD_ADD_3;
  r = txAndRxCmdWithCancel(commandBuf, 8, 15);
  
  if (r == ACK_FAIL) {
    isInAddingMode = false;
    return ACK_FAIL;
  }
  
  Serial.print("ADD_3: ");
  for (int i = 0; i < 8; i++) {
    Serial.print("0x");
    if (g_rx_buf[i] < 16) Serial.print("0");
    Serial.print(g_rx_buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  if (r == ACK_SUCCESS && g_rx_buf[4] == ACK_SUCCESS) {
    uint8_t savedID = g_rx_buf[3];
    Serial.print("SUCCESS! Saved to ID: ");
    Serial.println(savedID);
    
    uint8_t newCount = getUserCount();
    Serial.print("Count: ");
    Serial.print(userCount);
    Serial.print(" -> ");
    Serial.println(newCount);
    
    isInAddingMode = false;
    return ACK_SUCCESS;
  } else {
    Serial.print("Template failed: 0x");
    Serial.println(g_rx_buf[4], HEX);
    isInAddingMode = false;
    return ACK_FAIL;
  }
}

uint8_t clearAllUser() {
  uint8_t commandBuf[5] = {CMD_DEL_ALL, 0, 0, 0, 0};
  uint8_t r = txAndRxCmd(commandBuf, 8, 5);
  
  if (r == ACK_TIMEOUT) return ACK_TIMEOUT;
  if (r == ACK_SUCCESS && g_rx_buf[4] == ACK_SUCCESS) return ACK_SUCCESS;
  return ACK_FAIL;
}

uint8_t verifyUserContinuous() {
  isInMatchingMode = true;
  Serial.println("Continuous matching mode");
  Serial.println("Send 'ESC' to exit");
  
  while (isInMatchingMode) {
    if (Serial.available()) {
      String command = Serial.readStringUntil('\n');
      command.trim();
      command.toUpperCase();
      if (command == "ESC" || command == "CANCEL" || command == "EXIT") {
        Serial.println("Exiting matching mode");
        isInMatchingMode = false;
        return ACK_FAIL;
      }
    }
    
    if (!isRealFingerDetected()) {
      delay(200);
      continue;
    }
    
    Serial.println("Processing...");
    
    fingerSerial.flush();
    delay(100);
    while (fingerSerial.available()) {
      fingerSerial.read();
      delay(1);
    }
    
    delay(500);
    
    if (!isRealFingerDetected()) {
      Serial.println("Finger removed");
      continue;
    }
    
    uint8_t commandBuf[5] = {CMD_MATCH, 0, 0, 0, 0};
    uint8_t r = txAndRxCmd(commandBuf, 8, 3);
    
    if (r != ACK_SUCCESS) {
      Serial.print("Comm error: 0x");
      Serial.println(r, HEX);
      delay(500);
      continue;
    }
    
    // Verify checksum
    uint8_t checkSum = 0;
    for (int i = 1; i < 6; i++) {
      checkSum ^= g_rx_buf[i];
    }
    
    if (checkSum != g_rx_buf[6]) {
      Serial.println("Checksum error");
      delay(1000);
      continue;
    }
    
    uint8_t userID = g_rx_buf[3];
    uint8_t statusCode = g_rx_buf[4];
    
    Serial.print("Response: ");
    for (int i = 0; i < 8; i++) {
      Serial.print("0x");
      if (g_rx_buf[i] < 16) Serial.print("0");
      Serial.print(g_rx_buf[i], HEX);
      Serial.print(" ");
    }
    Serial.print(" | ID: ");
    Serial.print(userID);
    Serial.print(" | Status: 0x");
    Serial.print(statusCode, HEX);
    Serial.print(" | Touch: ");
    Serial.println(isRealFingerDetected() ? "REAL" : "NONE");
    
    if (userID == 0) {
      Serial.println("No match found");
    } else if (userID > 0 && userID <= USER_MAX_CNT) {
      if (isRealFingerDetected()) {
        Serial.print("*** MATCHED User ID: ");
        Serial.print(userID);
        Serial.println(" (REAL) ***");
      } else {
        Serial.print("Match but finger removed: ");
        Serial.println(userID);
      }
      
      while (isRealFingerDetected()) {
        delay(100);
        platformYield();
      }
      Serial.println("Ready for next scan");
      
    } else {
      Serial.print("Invalid User ID: ");
      Serial.println(userID);
    }
    
    delay(500);
    platformYield();
  }
  
  return ACK_SUCCESS;
}

void analysisPCCommand(String command) {
  if (command == "CMD1" && fingerSleepFlag != 1) {
    Serial.print("Fingerprints: ");
    Serial.println(getUserCount());
    
  } else if (command == "CMD2" && fingerSleepFlag != 1) {
    if (isInMatchingMode || isInAddingMode) {
      Serial.println("System busy");
      return;
    }
    Serial.println("Adding fingerprint...");
    uint8_t r = addUser();
    if (r == ACK_SUCCESS) {
      Serial.println("Added successfully!");
    } else if (r == ACK_FAIL) {
      Serial.println("Failed to add!");
    } else if (r == ACK_FULL) {
      Serial.println("Library full!");
    }
    
  } else if (command == "CMD3" && fingerSleepFlag != 1) {
    if (isInMatchingMode || isInAddingMode) {
      Serial.println("System busy");
      return;
    }
    Serial.print("Current count: ");
    Serial.println(getUserCount());
    verifyUserContinuous();
    Serial.println("Exited matching");
    printMenu();
    
  } else if (command == "CMD4" && fingerSleepFlag != 1) {
    Serial.print("Before: ");
    Serial.println(getUserCount());
    uint8_t r = clearAllUser();
    if (r == ACK_SUCCESS) {
      Serial.println("All cleared!");
      Serial.print("After: ");
      Serial.println(getUserCount());
    } else {
      Serial.print("Clear failed: 0x");
      Serial.println(r, HEX);
    }
    
  } else if (command == "CMD5" && fingerSleepFlag != 1) {
    fingerSleepFlag = 1;
    Serial.println("Sleep mode");
    
  } else if (command == "CMD6") {
    fingerSleepFlag = 0;
    Serial.println("Awake!");
    
  } else if (command == "INFO") {
    Serial.print("Platform: ");
    Serial.println(PLATFORM_NAME);
    Serial.print("RX Pin: ");
    Serial.println(FINGER_RX_PIN);
    Serial.print("TX Pin: ");
    Serial.println(FINGER_TX_PIN);
    Serial.print("Touch Pin: ");
    Serial.println(TOUCH_OUT_PIN);
    Serial.print("Finger count: ");
    Serial.println(getUserCount());
    
  } else {
    Serial.println("Invalid command!");
  }
}

void printMenu() {
  Serial.println("=== Universal Fingerprint Reader ===");
  Serial.print("Platform: ");
  Serial.println(PLATFORM_NAME);
  Serial.print("Fingerprints: ");
  Serial.println(getUserCount());
  Serial.println("Commands:");
  Serial.println("  CMD1 : Query count");
  Serial.println("  CMD2 : Add fingerprint");
  Serial.println("  CMD3 : Continuous matching");
  Serial.println("  CMD4 : Clear all");
  Serial.println("  CMD5 : Sleep mode");
  Serial.println("  CMD6 : Wake up");
  Serial.println("  INFO : Platform info");
  Serial.println("  ESC  : Cancel");
  Serial.println("====================================");
  Serial.print("Input command: ");
}