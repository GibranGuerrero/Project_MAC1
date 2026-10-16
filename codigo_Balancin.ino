#include <Arduino.h>
#include <Wire.h>

// ============================================================================
// ASIGNACIÓN DE PINES (ESP32-S3 + TB6612FNG + GY-521)
// ============================================================================
// I2C
#define PIN_I2C_SDA          11
#define PIN_I2C_SCL          12
#define I2C_CLOCK_SPEED      400000

// MOTOR A (IZQUIERDO)
#define PWMA                 15
#define AIN1                 17
#define AIN2                 16
const int ENC_A_A          = 41;  // Interrupción
const int ENC_A_B          = 42;

// MOTOR B (DERECHO)
#define PWMB                 14
#define BIN1                 18
#define BIN2                 8
const int ENC_B_A          = 40;  // Interrupción
const int ENC_B_B          = 39;

// TB6612FNG STANDBY
#define STBY                 21
#define PWM_FREQ_HZ          20000 // 20 kHz

// ============================================================================
// PARÁMETROS MECÁNICOS Y DE ENCODERS (JGA25-370 + LLANTA 66 mm)
// ============================================================================
const float PULSES_PER_REV     = 450.0f; // 2x resolución usando CHANGE
const float WHEEL_DIAMETER_CM  = 6.6f;   // 66 mm = 6.6 cm
const float WHEEL_PERIMETER_CM = PI * WHEEL_DIAMETER_CM; // ~20.7345 cm por vuelta
const float CM_PER_PULSE       = WHEEL_PERIMETER_CM / PULSES_PER_REV; // ~0.046 cm/pulso

// Si al girar una rueda hacia adelante su RPM sale negativa, cambia su 1 por -1
const int DIR_ENC_A            = 1;
const int DIR_ENC_B            = -1;

volatile long encoderTicksA    = 0;
volatile long encoderTicksB    = 0;

// ============================================================================
// REGISTROS MPU-6050
// ============================================================================
#define RAD2GRAD             57.2957795f
#define MPU6050_I2C_ADDR     0x68
#define REG_SMPLRT_DIV       0x19
#define REG_CONFIG           0x1A
#define REG_GYRO_CONFIG      0x1B
#define REG_ACCEL_CONFIG     0x1C
#define REG_ACCEL_XOUT_H     0x3B
#define REG_PWR_MGMT_1       0x6B
#define REG_WHO_AM_I         0x75

typedef union {
  struct {
    uint8_t x_accel_h, x_accel_l;
    uint8_t y_accel_h, y_accel_l;
    uint8_t z_accel_h, z_accel_l;
    uint8_t t_h, t_l;
    uint8_t x_gyro_h, x_gyro_l;
    uint8_t y_gyro_h, y_gyro_l;
    uint8_t z_gyro_h, z_gyro_l;
  } reg;
  struct {
    int16_t x_accel, y_accel, z_accel;
    int16_t temp;
    int16_t x_gyro, y_gyro, z_gyro;
  } value;
} MPU_Data;

MPU_Data mpu;
float x_gyro_offset          = 0.0f;
float angle_fused            = 0.0f;
float gyro_rate              = 0.0f;

// ============================================================================
// PARÁMETROS DE CONTROL Y SINTONIZACIÓN
// ============================================================================
// 1. Lazo interno (Ángulo) - PD (200 Hz)
volatile float Kp_angle      = 24.0f;  // Proporcional ángulo (Comando Serial: P)
volatile float Kd_angle      = 0.15f;  // Derivativo ángulo   (Comando Serial: D)

// 2. Lazo externo (Velocidad RPM + Posición en CM) - PI (20 Hz)
volatile float Kp_speed      = -0.05f;   // Proporcional RPM [grados / RPM] (Comando Serial: S)
volatile float Ki_speed      = 0.0f;   // Integral = Posición [grados / cm] (Comando Serial: I)

// 3. Lazo diferencial de dirección (Yaw)
volatile float Kp_yaw        = 0.0f;   // Sincronización A/B  (Comando Serial: Y)

// Parámetros físicos
volatile float target_angle_offset = -2.0f; // Centro de masa  (Comando Serial: O)
volatile int   deadband_pwm        = 10;   // Zona muerta PWM (Comando Serial: Z)
const float    MAX_FALL_ANGLE      = 35.0f;// Corte de emergencia (grados)
const float    MAX_POS_ERROR_CM    = 40.0f;// Límite anti-windup de distancia (±40 cm)

// Variables de estado del lazo en cascada y telemetría
volatile float target_angle        = 0.0f;
volatile float target_angle_smooth = 0.0f;
volatile float current_pos_cm      = 0.0f; // Posición actual en cm respecto al origen (0 cm)
volatile float current_rpmA        = 0.0f; // RPM Motor A (Izquierdo)
volatile float current_rpmB        = 0.0f; // RPM Motor B (Derecho)
volatile float current_speed_rpm   = 0.0f; // RPM Promedio
volatile int   current_pwm         = 0;    // PWM promedio enviado
volatile bool  reset_origin_flag   = false;// Comando Serial 'R' para poner 0 cm actual

portMUX_TYPE controlMux = portMUX_INITIALIZER_UNLOCKED;

// ============================================================================
// ISR ENCODERS (Cuadratura 2x con CHANGE en ESP32-S3)
// ============================================================================
void IRAM_ATTR handleEncoderA() {
  if (digitalRead(ENC_A_A) == digitalRead(ENC_A_B)) encoderTicksA += DIR_ENC_A;
  else                                              encoderTicksA -= DIR_ENC_A;
}

void IRAM_ATTR handleEncoderB() {
  if (digitalRead(ENC_B_A) == digitalRead(ENC_B_B)) encoderTicksB += DIR_ENC_B;
  else                                              encoderTicksB -= DIR_ENC_B;
}

// ============================================================================
// I2C BAJO NIVEL
// ============================================================================
int i2c_write(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU6050_I2C_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission();
}

int i2c_read(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(MPU6050_I2C_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return -1;
  if (Wire.requestFrom((uint8_t)MPU6050_I2C_ADDR, len, (uint8_t)true) != len) return -2;
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
  return 0;
}

bool mpu_read() {
  if (i2c_read(REG_ACCEL_XOUT_H, (uint8_t *)&mpu, sizeof(mpu)) != 0) return false;
  #define SWAP_B(a, b) { uint8_t t = a; a = b; b = t; }
  SWAP_B(mpu.reg.x_accel_h, mpu.reg.x_accel_l);
  SWAP_B(mpu.reg.y_accel_h, mpu.reg.y_accel_l);
  SWAP_B(mpu.reg.z_accel_h, mpu.reg.z_accel_l);
  SWAP_B(mpu.reg.x_gyro_h,  mpu.reg.x_gyro_l);
  #undef SWAP_B
  return true;
}

// ============================================================================
// ACCIONAMIENTO DE MOTORES CON ZONA MUERTA SIN TRUNCAMIENTO
// ============================================================================
int applyDeadbandAndClamp(float u) {
  int db = deadband_pwm;
  int pwm_out = 0;
  if (u > 0.05f) {
    pwm_out = (int)roundf(u) + db;
  } else if (u < -0.05f) {
    pwm_out = (int)roundf(u) - db;
  }
  return constrain(pwm_out, -255, 255);
}

void driveSingleMotor(uint8_t pinPWM, uint8_t pinIN1, uint8_t pinIN2, int pwm) {
  if (pwm > 0) {
    digitalWrite(pinIN1, HIGH);
    digitalWrite(pinIN2, LOW);
    analogWrite(pinPWM, pwm);
  } else if (pwm < 0) {
    digitalWrite(pinIN1, LOW);
    digitalWrite(pinIN2, HIGH);
    analogWrite(pinPWM, -pwm);
  } else {
    digitalWrite(pinIN1, LOW);
    digitalWrite(pinIN2, LOW);
    analogWrite(pinPWM, 0);
  }
}

void stopMotors() {
  current_pwm = 0;
  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);
  digitalWrite(AIN1, LOW); digitalWrite(AIN2, LOW);
  digitalWrite(BIN1, LOW); digitalWrite(BIN2, LOW);
}

void setMotorsControl(float u_balance, float u_yaw) {
  int pwmA = applyDeadbandAndClamp(u_balance - u_yaw);
  int pwmB = applyDeadbandAndClamp(u_balance + u_yaw);

  current_pwm = (pwmA + pwmB) / 2;

  driveSingleMotor(PWMA, AIN1, AIN2, pwmA);
  driveSingleMotor(PWMB, BIN1, BIN2, pwmB);
}

// ============================================================================
// TAREA CONTROL EN CASCADA (CORE 1 - TIEMPO REAL 200 Hz)
// ============================================================================
void TaskBalanceControl(void *pvParameters) {
  const TickType_t xPeriod = pdMS_TO_TICKS(5); // 200 Hz = 5 ms
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const float dt = 0.005f;

  uint8_t loop_counter = 0;
  long last_enc_A = 0;
  long last_enc_B = 0;
  long origin_ticks_sum = 0; // Punto de referencia 0.0 cm
  long yaw_ref_diff = 0;

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xPeriod);

    if (mpu_read()) {
      // 1. FILTRO COMPLEMENTARIO (200 Hz)
      float accel_angle = atan2f((float)mpu.value.y_accel, (float)mpu.value.z_accel) * RAD2GRAD;
      gyro_rate = ((float)mpu.value.x_gyro - x_gyro_offset) / 65.5f;
      angle_fused = 0.995f * (angle_fused + gyro_rate * dt) + 0.005f * accel_angle;

      // Lectura atómica de encoders
      long cur_A, cur_B;
      portENTER_CRITICAL(&controlMux);
      cur_A = encoderTicksA;
      cur_B = encoderTicksB;
      portEXIT_CRITICAL(&controlMux);

      // Si se envió el comando 'R' por Serial, reiniciar el origen 0.0 cm aquí
      if (reset_origin_flag) {
        origin_ticks_sum = cur_A + cur_B;
        yaw_ref_diff     = cur_A - cur_B;
        reset_origin_flag = false;
      }

      // Calcular posición exacta en centímetros (promedio de ambas ruedas)
      float avg_ticks_from_origin = 0.5f * (float)((cur_A + cur_B) - origin_ticks_sum);
      current_pos_cm = avg_ticks_from_origin * CM_PER_PULSE;

      // 2. MEDICIÓN DE RPM Y LAZO EXTERNO PI (Cada 50 ms / 20 Hz)
      loop_counter++;
      if (loop_counter >= 10) {
        loop_counter = 0;

        long delta_A = cur_A - last_enc_A;
        long delta_B = cur_B - last_enc_B;
        last_enc_A = cur_A;
        last_enc_B = cur_B;

        // RPM individuales de cada rueda con filtro EMA
        float raw_rpmA = ((float)delta_A / PULSES_PER_REV) * (60.0f / 0.05f);
        float raw_rpmB = ((float)delta_B / PULSES_PER_REV) * (60.0f / 0.05f);

        current_rpmA = 0.70f * current_rpmA + 0.30f * raw_rpmA;
        current_rpmB = 0.70f * current_rpmB + 0.30f * raw_rpmB;
        current_speed_rpm = 0.5f * (current_rpmA + current_rpmB);

        // Actualizar control PI externo solo si el robot está en pie
        if (fabs(angle_fused) <= MAX_FALL_ANGLE) {
          // Anti-windup en centímetros: limita el esfuerzo de posición a ±MAX_POS_ERROR_CM
          float pos_cm_clamped = constrain(current_pos_cm, -MAX_POS_ERROR_CM, MAX_POS_ERROR_CM);

          // Kp_speed frena las RPM; Ki_speed regresa el robot a 0.0 cm
          float cmd_angle = (current_speed_rpm * Kp_speed) + (pos_cm_clamped * Ki_speed) + target_angle_offset;
          target_angle = constrain(cmd_angle, -6.0f, 6.0f);
        }
      }

      // CORTE DE EMERGENCIA SI EL ROBOT SE CAE (> 35°)
      if (fabs(angle_fused) > MAX_FALL_ANGLE) {
        stopMotors();
        target_angle        = target_angle_offset;
        target_angle_smooth = target_angle_offset;
        // Al estar caído, si aún quieres medir cuántos cm gira la rueda en la mesa,
        // puedes usar el comando 'R' para poner en 0.0 cm; al levantarlo, presionas 'R' o se centra.
        yaw_ref_diff        = cur_A - cur_B;
        continue;
      }

      // Suavizado exponencial de target_angle a 200 Hz
      target_angle_smooth = 0.90f * target_angle_smooth + 0.10f * target_angle;

      // 3. LAZO INTERNO: ÁNGULO PD (200 Hz)
      float error_angle = angle_fused - target_angle_smooth;
      float u_balance = (error_angle * Kp_angle) + (gyro_rate * Kd_angle);

      // 4. CORRECCIÓN DIFERENCIAL DE GIRO (YAW)
      long yaw_error = (cur_A - cur_B) - yaw_ref_diff;
      float u_yaw = (float)yaw_error * Kp_yaw;
      u_yaw = constrain(u_yaw, -30.0f, 30.0f);

      // 5. APLICAR SALIDA A LOS MOTORES
      setMotorsControl(u_balance, u_yaw);
    }
  }
}

// ============================================================================
// SINTONIZACIÓN EN TIEMPO REAL POR PUERTO SERIE
// Comandos: P<val>, D<val>, S<val>, I<val>, O<val>, Z<val>, Y<val>, R (reset 0 cm)
// ============================================================================
void parseSerialCommands() {
  if (!Serial.available()) return;

  char cmd = toupper(Serial.read());
  if (cmd == 'R') {
    reset_origin_flag = true;
    while (Serial.available()) Serial.read();
    return;
  }

  float val = Serial.parseFloat();
  while (Serial.available()) Serial.read(); // Limpiar fin de línea

  switch (cmd) {
    case 'P': Kp_angle = val;            break;
    case 'D': Kd_angle = val;            break;
    case 'S': Kp_speed = val;            break;
    case 'I': Ki_speed = val;            break;
    case 'O': target_angle_offset = val; break;
    case 'Z': deadband_pwm = (int)val;   break;
    case 'Y': Kp_yaw = val;              break;
    default: return;
  }
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Configurar Pines Motor y Frecuencia PWM a 20 kHz
  pinMode(PWMA, OUTPUT); pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT); pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  analogWriteFrequency(PWMA, PWM_FREQ_HZ);
  analogWriteFrequency(PWMB, PWM_FREQ_HZ);

  #ifdef STBY
  pinMode(STBY, OUTPUT); digitalWrite(STBY, HIGH);
  #endif

  // Configurar Encoders en modo CHANGE (doble resolución)
  pinMode(ENC_A_A, INPUT_PULLUP); pinMode(ENC_A_B, INPUT_PULLUP);
  pinMode(ENC_B_A, INPUT_PULLUP); pinMode(ENC_B_B, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A_A), handleEncoderA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B_A), handleEncoderB, CHANGE);

  // Iniciar I2C
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_CLOCK_SPEED);

  // Configuración MPU-6050
  i2c_write(REG_PWR_MGMT_1, 0x80); delay(100);
  i2c_write(REG_PWR_MGMT_1, 0x01);
  i2c_write(REG_CONFIG, 0x03);       // DLPF ~44 Hz
  i2c_write(REG_GYRO_CONFIG, 0x08);  // +-500 dps
  i2c_write(REG_ACCEL_CONFIG, 0x00); // +-2g
  i2c_write(REG_SMPLRT_DIV, 4);      // 200 Hz

  // Calibración inicial en reposo
  Serial.println("Calibrando IMU... Manten el robot inmovil.");
  int32_t sum = 0;
  for (int i = 0; i < 200; i++) {
    mpu_read();
    sum += mpu.value.x_gyro;
    delay(10);
  }
  x_gyro_offset = (float)sum / 200.0f;
  mpu_read();
  angle_fused = atan2f((float)mpu.value.y_accel, (float)mpu.value.z_accel) * RAD2GRAD;

  // Arrancar Tarea en Core 1
  xTaskCreatePinnedToCore(
    TaskBalanceControl,
    "BalControl",
    4096,
    NULL,
    5,
    NULL,
    1
  );

  Serial.println("Control en Cascada activo.");
}

// ============================================================================
// LOOP (CORE 0 - TELEMETRÍA Y SINTONIZACIÓN)
// ============================================================================
void loop() {
  parseSerialCommands();

  // Imprime Ángulo, Ángulo Objetivo, Posición en cm (PosCM), RPMA, RPMB y PWM
  Serial.printf("Ang:%.2f,Tar:%.2f,PosCM:%.1f,RPMA:%.1f,RPMB:%.1f,PWM:%d\n",
                angle_fused, target_angle_smooth, current_pos_cm, current_rpmA, current_rpmB, current_pwm);

  vTaskDelay(pdMS_TO_TICKS(50));
}