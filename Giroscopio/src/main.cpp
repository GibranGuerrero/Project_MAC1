#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <math.h>

// ============================================================
//                 PINES SPI - ESP32-S3
// ============================================================

#define SCK_PIN   12
#define MISO_PIN  13
#define MOSI_PIN  11
#define CS_PIN    10

// ============================================================
//                 CONFIGURACION BMI160
// ============================================================

#define ACCEL_SENSITIVITY 16384.0f
#define GYRO_SENSITIVITY  65.6f

#define REG_CHIP_ID    0x00
#define REG_CMD        0x7E
#define REG_ACC_CONF   0x40
#define REG_GYR_CONF   0x42
#define REG_GYR_RANGE  0x43
#define REG_GYR_DATA   0x0C

// ============================================================
//                    EEPROM
// ============================================================

#define EEPROM_SIZE        64
#define EEPROM_MAGIC_ADDR  0
#define EEPROM_DATA_ADDR   1

#define EEPROM_MAGIC_VALUE 0xB9

// ============================================================
//                VARIABLES DE CALIBRACION
// ============================================================

long accOffsetX = 0;
long accOffsetY = 0;
long accOffsetZ = 0;

float gyroOffsetX = 0;
float gyroOffsetY = 0;
float gyroOffsetZ = 0;

// ============================================================
//                 FILTRO COMPLEMENTARIO
// ============================================================

const float ALPHA = 0.98f;

float angle = 0.0f;

// ============================================================
//                 CONTROL DE TIEMPO
// ============================================================

const unsigned long CONTROL_PERIOD_US = 2500;

unsigned long lastControl = 0;

unsigned long lastPrint = 0;
const unsigned long PRINT_INTERVAL_MS = 100;

// ============================================================
//                     VARIABLES RAW
// ============================================================

int16_t gx, gy, gz;
int16_t ax, ay, az;

float gyroX_dps = 0.0f;

float ax_g = 0.0f;
float ay_g = 0.0f;
float az_g = 0.0f;

// ============================================================
//                     SPI
// ============================================================

SPISettings bmiSPISettings(
  1000000,
  MSBFIRST,
  SPI_MODE0
);

// ============================================================
//                  PROTOTIPOS
// ============================================================

uint8_t readRegister(uint8_t reg);

void readRegisters(
  uint8_t reg,
  uint8_t *buffer,
  uint8_t len
);

void writeRegister(
  uint8_t reg,
  uint8_t value
);

void setPreciseMode();
void setFastMode();

void calibrate();
void printOffsets();

bool isCalibrationSaved();

void saveCalibrationToEEPROM();
void loadCalibrationFromEEPROM();

bool readAllRaw(
  int16_t &gx,
  int16_t &gy,
  int16_t &gz,
  int16_t &ax,
  int16_t &ay,
  int16_t &az
);

// ============================================================
//                         SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(1000);

  EEPROM.begin(EEPROM_SIZE);

  // ----------------------------------------------------------
  // SPI
  // ----------------------------------------------------------

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  SPI.begin(
    SCK_PIN,
    MISO_PIN,
    MOSI_PIN,
    CS_PIN
  );

  delay(100);

  // ----------------------------------------------------------
  // Primera lectura para poner BMI160 en SPI
  // ----------------------------------------------------------

  readRegister(0x7F);

  delay(10);

  // ----------------------------------------------------------
  // Acelerometro NORMAL
  // ----------------------------------------------------------

  writeRegister(REG_CMD, 0x11);

  delay(100);

  // ----------------------------------------------------------
  // Giroscopio NORMAL
  // ----------------------------------------------------------

  writeRegister(REG_CMD, 0x15);

  delay(100);

  // ----------------------------------------------------------
  // Rango gyro = ±500 °/s
  // ----------------------------------------------------------

  writeRegister(REG_GYR_RANGE, 0x02);

  delay(10);

  // ----------------------------------------------------------
  // Verificar CHIP ID
  // ----------------------------------------------------------

  uint8_t chipId = readRegister(REG_CHIP_ID);

  Serial.print("Chip ID: 0x");
  Serial.println(chipId, HEX);

  if (chipId != 0xD1) {

    Serial.println(
      "ERROR: Chip ID incorrecto."
    );

    Serial.println(
      "Revisa SCK, MOSI, MISO y CS."
    );
  }

  // ----------------------------------------------------------
  // Modo rapido
  // ----------------------------------------------------------

  setFastMode();

  Serial.println(
    "Esperando estabilizacion termica..."
  );

  delay(1500);

  // ----------------------------------------------------------
  // CALIBRACION
  // ----------------------------------------------------------

  if (isCalibrationSaved()) {

    loadCalibrationFromEEPROM();

    Serial.println(
      "Calibracion cargada desde EEPROM."
    );

  } else {

    Serial.println(
      "No hay calibracion guardada."
    );

    Serial.println(
      "Deja el robot completamente quieto."
    );

    delay(1000);

    setPreciseMode();

    calibrate();

    setFastMode();

    saveCalibrationToEEPROM();

    Serial.println(
      "Calibracion guardada."
    );
  }

  printOffsets();

  // ----------------------------------------------------------
  // Inicializar angulo con acelerometro
  // ----------------------------------------------------------

  if (readAllRaw(gx, gy, gz, ax, ay, az)) {

    ax_g =
      ((float)ax - accOffsetX) /
      ACCEL_SENSITIVITY;

    ay_g =
      ((float)ay - accOffsetY) /
      ACCEL_SENSITIVITY;

    az_g =
      ((float)az - accOffsetZ) /
      ACCEL_SENSITIVITY;

    angle =
      atan2(ay_g, az_g)
      * 180.0f / PI;
  }

  // ----------------------------------------------------------
  // Tiempo inicial
  // ----------------------------------------------------------

  lastControl = micros();

  Serial.println();
  Serial.println("=============================");
  Serial.println(" BMI160 LISTO");
  Serial.println("=============================");
  Serial.print("Angulo inicial: ");
  Serial.println(angle, 2);
  Serial.println();
}

// ============================================================
//                         LOOP
// ============================================================

void loop() {

  // ==========================================================
  // COMANDO DE RECALIBRACION
  // ==========================================================

  if (Serial.available()) {

    char c = Serial.read();

    if (c == 'c' || c == 'C') {

      Serial.println();
      Serial.println(
        "RECALIBRANDO..."
      );

      Serial.println(
        "NO MUEVAS EL ROBOT."
      );

      setPreciseMode();

      calibrate();

      setFastMode();

      saveCalibrationToEEPROM();

      printOffsets();

      // Reiniciar angulo
      if (readAllRaw(gx, gy, gz, ax, ay, az)) {

        ax_g =
          ((float)ax - accOffsetX) /
          ACCEL_SENSITIVITY;

        ay_g =
          ((float)ay - accOffsetY) /
          ACCEL_SENSITIVITY;

        az_g =
          ((float)az - accOffsetZ) /
          ACCEL_SENSITIVITY;

        angle =
          atan2(ay_g, az_g)
          * 180.0f / PI;
      }

      Serial.println(
        "Recalibracion terminada."
      );
    }
  }

  // ==========================================================
  // LAZO DE CONTROL A 400 Hz
  // ==========================================================

  unsigned long now = micros();

  if (
    (unsigned long)(now - lastControl)
    >= CONTROL_PERIOD_US
  ) {

    lastControl += CONTROL_PERIOD_US;

    const float dt =
      CONTROL_PERIOD_US / 1000000.0f;

    // --------------------------------------------------------
    // Leer BMI160
    // --------------------------------------------------------

    if (!readAllRaw(
          gx, gy, gz,
          ax, ay, az
        )) {

      return;
    }

    // --------------------------------------------------------
    // Giroscopio X
    // --------------------------------------------------------

    gyroX_dps =
      ((float)gx / GYRO_SENSITIVITY)
      - gyroOffsetX;

    // --------------------------------------------------------
    // Acelerometro
    // --------------------------------------------------------

    ax_g =
      ((float)ax - accOffsetX) /
      ACCEL_SENSITIVITY;

    ay_g =
      ((float)ay - accOffsetY) /
      ACCEL_SENSITIVITY;

    az_g =
      ((float)az - accOffsetZ) /
      ACCEL_SENSITIVITY;

    // --------------------------------------------------------
    // ANGULO DEL ACELEROMETRO
    // --------------------------------------------------------

    float angleAcc =
      atan2(ay_g, az_g)
      * 180.0f / PI;

    // --------------------------------------------------------
    // FILTRO COMPLEMENTARIO
    // --------------------------------------------------------

    angle =
      ALPHA *
      (angle + gyroX_dps * dt)
      +
      (1.0f - ALPHA) *
      angleAcc;

    angle = constrain(angle, -45.0f, 45.0f);
  }

  // ==========================================================
  // MOSTRAR SOLO EL ANGULO
  // ==========================================================

  if (
    millis() - lastPrint
    >= PRINT_INTERVAL_MS
  ) {

    lastPrint = millis();

    Serial.print("Angulo: ");
    Serial.print(angle, 2);
    Serial.println(" grados");
  }
}

// ============================================================
//                    MODO PRECISO
// ============================================================

void setPreciseMode() {

  writeRegister(
    REG_ACC_CONF,
    0x28
  );

  writeRegister(
    REG_GYR_CONF,
    0x28
  );

  delay(20);
}

// ============================================================
//                    MODO RAPIDO
// ============================================================

void setFastMode() {

  writeRegister(
    REG_ACC_CONF,
    0x2C
  );

  writeRegister(
    REG_GYR_CONF,
    0x2C
  );

  delay(20);
}

// ============================================================
//                     CALIBRACION
// ============================================================

void calibrate() {

  const int N = 500;

  long sumAx = 0;
  long sumAy = 0;
  long sumAz = 0;

  long sumGx = 0;
  long sumGy = 0;
  long sumGz = 0;

  int16_t minAx = 32767;
  int16_t maxAx = -32768;

  int16_t minAy = 32767;
  int16_t maxAy = -32768;

  int16_t minAz = 32767;
  int16_t maxAz = -32768;

  int valid = 0;

  Serial.println(
    "Tomando muestras..."
  );

  for (int i = 0; i < N; i++) {

    int16_t rgx, rgy, rgz;
    int16_t rax, ray, raz;

    if (
      readAllRaw(
        rgx, rgy, rgz,
        rax, ray, raz
      )
    ) {

      sumAx += rax;
      sumAy += ray;
      sumAz += raz;

      sumGx += rgx;
      sumGy += rgy;
      sumGz += rgz;

      if (rax < minAx) minAx = rax;
      if (rax > maxAx) maxAx = rax;

      if (ray < minAy) minAy = ray;
      if (ray > maxAy) maxAy = ray;

      if (raz < minAz) minAz = raz;
      if (raz > maxAz) maxAz = raz;

      valid++;
    }

    delay(2);
  }

  if (valid == 0) {

    Serial.println(
      "ERROR: no se recibieron datos."
    );

    return;
  }

  // ----------------------------------------------------------
  // Offset acelerometro
  // ----------------------------------------------------------

  accOffsetX =
    sumAx / valid;

  accOffsetY =
    sumAy / valid;

  accOffsetZ =
    (sumAz / valid)
    - (long)ACCEL_SENSITIVITY;

  // ----------------------------------------------------------
  // Bias giroscopio
  // ----------------------------------------------------------

  gyroOffsetX =
    (sumGx / (float)valid)
    / GYRO_SENSITIVITY;

  gyroOffsetY =
    (sumGy / (float)valid)
    / GYRO_SENSITIVITY;

  gyroOffsetZ =
    (sumGz / (float)valid)
    / GYRO_SENSITIVITY;

  // ----------------------------------------------------------
  // Detectar movimiento
  // ----------------------------------------------------------

  int rangeAx =
    maxAx - minAx;

  int rangeAy =
    maxAy - minAy;

  int rangeAz =
    maxAz - minAz;

  Serial.println();

  Serial.print("Rango Ax: ");
  Serial.println(rangeAx);

  Serial.print("Rango Ay: ");
  Serial.println(rangeAy);

  Serial.print("Rango Az: ");
  Serial.println(rangeAz);

  if (
    rangeAx > 1000 ||
    rangeAy > 1000 ||
    rangeAz > 1000
  ) {

    Serial.println(
      "ADVERTENCIA: hubo movimiento durante la calibracion."
    );
  }

  Serial.println(
    "Calibracion terminada."
  );
}

// ============================================================
//                    MOSTRAR OFFSETS
// ============================================================

void printOffsets() {

  Serial.println();
  Serial.println(
    "===== CALIBRACION ====="
  );

  Serial.print("Acc X: ");
  Serial.println(accOffsetX);

  Serial.print("Acc Y: ");
  Serial.println(accOffsetY);

  Serial.print("Acc Z: ");
  Serial.println(accOffsetZ);

  Serial.print("Gyro X: ");
  Serial.println(
    gyroOffsetX,
    4
  );

  Serial.print("Gyro Y: ");
  Serial.println(
    gyroOffsetY,
    4
  );

  Serial.print("Gyro Z: ");
  Serial.println(
    gyroOffsetZ,
    4
  );

  Serial.println(
    "======================="
  );
}

// ============================================================
//                    EEPROM
// ============================================================

struct CalibData {

  long accOffsetX;
  long accOffsetY;
  long accOffsetZ;

  float gyroOffsetX;
  float gyroOffsetY;
  float gyroOffsetZ;
};

// ------------------------------------------------------------

bool isCalibrationSaved() {

  return
    EEPROM.read(
      EEPROM_MAGIC_ADDR
    )
    ==
    EEPROM_MAGIC_VALUE;
}

// ------------------------------------------------------------

void saveCalibrationToEEPROM() {

  CalibData d = {

    accOffsetX,
    accOffsetY,
    accOffsetZ,

    gyroOffsetX,
    gyroOffsetY,
    gyroOffsetZ
  };

  EEPROM.put(
    EEPROM_DATA_ADDR,
    d
  );

  EEPROM.write(
    EEPROM_MAGIC_ADDR,
    EEPROM_MAGIC_VALUE
  );

  EEPROM.commit();
}

// ------------------------------------------------------------

void loadCalibrationFromEEPROM() {

  CalibData d;

  EEPROM.get(
    EEPROM_DATA_ADDR,
    d
  );

  accOffsetX = d.accOffsetX;
  accOffsetY = d.accOffsetY;
  accOffsetZ = d.accOffsetZ;

  gyroOffsetX = d.gyroOffsetX;
  gyroOffsetY = d.gyroOffsetY;
  gyroOffsetZ = d.gyroOffsetZ;
}

// ============================================================
//                    LECTURA BMI160
// ============================================================

bool readAllRaw(
  int16_t &gx,
  int16_t &gy,
  int16_t &gz,
  int16_t &ax,
  int16_t &ay,
  int16_t &az
) {

  uint8_t buf[12];

  readRegisters(
    REG_GYR_DATA,
    buf,
    12
  );

  gx =
    (int16_t)(
      buf[0] |
      (buf[1] << 8)
    );

  gy =
    (int16_t)(
      buf[2] |
      (buf[3] << 8)
    );

  gz =
    (int16_t)(
      buf[4] |
      (buf[5] << 8)
    );

  ax =
    (int16_t)(
      buf[6] |
      (buf[7] << 8)
    );

  ay =
    (int16_t)(
      buf[8] |
      (buf[9] << 8)
    );

  az =
    (int16_t)(
      buf[10] |
      (buf[11] << 8)
    );

  return true;
}

// ============================================================
//                    LECTURA REGISTRO
// ============================================================

uint8_t readRegister(
  uint8_t reg
) {

  uint8_t value;

  readRegisters(
    reg,
    &value,
    1
  );

  return value;
}

// ============================================================
//                 LECTURA MULTIPLE SPI
// ============================================================

void readRegisters(
  uint8_t reg,
  uint8_t *buffer,
  uint8_t len
) {

  SPI.beginTransaction(
    bmiSPISettings
  );

  digitalWrite(
    CS_PIN,
    LOW
  );

  SPI.transfer(
    reg | 0x80
  );

  for (
    uint8_t i = 0;
    i < len;
    i++
  ) {

    buffer[i] =
      SPI.transfer(0x00);
  }

  digitalWrite(
    CS_PIN,
    HIGH
  );

  SPI.endTransaction();
}

// ============================================================
//                  ESCRITURA REGISTRO
// ============================================================

void writeRegister(
  uint8_t reg,
  uint8_t value
) {

  SPI.beginTransaction(
    bmiSPISettings
  );

  digitalWrite(
    CS_PIN,
    LOW
  );

  SPI.transfer(
    reg & 0x7F
  );

  SPI.transfer(value);

  digitalWrite(
    CS_PIN,
    HIGH
  );

  SPI.endTransaction();
}