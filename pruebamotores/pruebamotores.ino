// ============================================================
// ESP32 + TB6612FNG
// 2 MOTORES JGA25-370 + 2 ENCODERS
// ============================================================


// ============================================================
// MOTOR A - IZQUIERDO
// ============================================================

#define PWMA 27
#define AIN1 25
#define AIN2 26

// Encoder Motor A
const int ENC_A_A = 18;   // Fase A
const int ENC_A_B = 19;   // Fase B


// ============================================================
// MOTOR B - DERECHO
// ============================================================

#define PWMB 4
#define BIN1 16
#define BIN2 17

// Encoder Motor B
const int ENC_B_A = 22;   // Fase A
const int ENC_B_B = 23;   // Fase B


// ============================================================
// STANDBY TB6612FNG
// ============================================================

#define STBY 33


// ============================================================
// ENCODERS
// ============================================================

volatile long encoderValueA = 0;
volatile long encoderValueB = 0;


// ============================================================
// CONFIGURACIÓN RPM
// ============================================================

// Tiempo entre mediciones
const unsigned long interval = 100;  // 100 ms


// Pulsos por revolución del eje de salida
// Valor provisional
const float PULSES_PER_REVOLUTION = 330.0;


// ============================================================
// VARIABLES RPM MOTOR A
// ============================================================

unsigned long previousMillis = 0;

long lastEncoderValueA = 0;

float rpmA = 0;


// ============================================================
// VARIABLES RPM MOTOR B
// ============================================================

long lastEncoderValueB = 0;

float rpmB = 0;


// ============================================================
// INTERRUPCIÓN ENCODER MOTOR A
// ============================================================

void IRAM_ATTR handleEncoderA()
{
  int estadoB = digitalRead(ENC_A_B);

  if (estadoB == HIGH)
  {
    encoderValueA++;
  }
  else
  {
    encoderValueA--;
  }
}


// ============================================================
// INTERRUPCIÓN ENCODER MOTOR B
// ============================================================

void IRAM_ATTR handleEncoderB()
{
  int estadoB = digitalRead(ENC_B_B);

  if (estadoB == HIGH)
  {
    encoderValueB++;
  }
  else
  {
    encoderValueB--;
  }
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);


  // ----------------------------------------------------------
  // MOTOR A
  // ----------------------------------------------------------

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);


  // ----------------------------------------------------------
  // MOTOR B
  // ----------------------------------------------------------

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);


  // ----------------------------------------------------------
  // STBY
  // ----------------------------------------------------------

  pinMode(STBY, OUTPUT);


  // ----------------------------------------------------------
  // ENCODER A
  // ----------------------------------------------------------

  pinMode(ENC_A_A, INPUT_PULLUP);
  pinMode(ENC_A_B, INPUT_PULLUP);


  // ----------------------------------------------------------
  // ENCODER B
  // ----------------------------------------------------------

  pinMode(ENC_B_A, INPUT_PULLUP);
  pinMode(ENC_B_B, INPUT_PULLUP);


  // ----------------------------------------------------------
  // INTERRUPCIONES
  // ----------------------------------------------------------

  attachInterrupt(
    digitalPinToInterrupt(ENC_A_A),
    handleEncoderA,
    RISING
  );

  attachInterrupt(
    digitalPinToInterrupt(ENC_B_A),
    handleEncoderB,
    RISING
  );


  // ----------------------------------------------------------
  // ACTIVAR TB6612FNG
  // ----------------------------------------------------------

  digitalWrite(STBY, HIGH);


  // ----------------------------------------------------------
  // MOTOR A - SENTIDO 1
  // ----------------------------------------------------------

  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);


  // ----------------------------------------------------------
  // MOTOR B - SENTIDO 1
  // ----------------------------------------------------------

  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);


  // ----------------------------------------------------------
  // PWM INICIAL
  // ----------------------------------------------------------

  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);


  Serial.println();
  Serial.println("========================================");
  Serial.println("   PRUEBA 2 MOTORES + 2 ENCODERS");
  Serial.println("========================================");
  Serial.println();
}


// ============================================================
// LOOP
// ============================================================

void loop()
{

  // ==========================================================
  // SENTIDO 1
  // ==========================================================

  Serial.println("SENTIDO 1");

  // Motor A
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  // Motor B
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);

  // PWM
  analogWrite(PWMA, 250);
  analogWrite(PWMB, 250);


  unsigned long startMillis = millis();


  while (millis() - startMillis < 3000)
  {
    calcularRPM();
  }


  // ==========================================================
  // PARAR
  // ==========================================================

  Serial.println("PARANDO");

  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);


  startMillis = millis();


  while (millis() - startMillis < 2000)
  {
    calcularRPM();
  }


  // ==========================================================
  // SENTIDO 2
  // ==========================================================

  Serial.println("SENTIDO 2");


  // Motor A
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);


  // Motor B
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);


  // PWM
  analogWrite(PWMA, 250);
  analogWrite(PWMB, 250);


  startMillis = millis();


  while (millis() - startMillis < 3000)
  {
    calcularRPM();
  }


  // ==========================================================
  // PARAR
  // ==========================================================

  Serial.println("PARANDO");

  analogWrite(PWMA, 0);
  analogWrite(PWMB, 0);


  startMillis = millis();


  while (millis() - startMillis < 2000)
  {
    calcularRPM();
  }
}


// ============================================================
// CALCULAR RPM DE LOS DOS MOTORES
// ============================================================

void calcularRPM()
{

  unsigned long currentMillis = millis();


  if (currentMillis - previousMillis >= interval)
  {

    previousMillis = currentMillis;


    // ========================================================
    // MOTOR A
    // ========================================================

    long pulsosA =
      encoderValueA - lastEncoderValueA;

    lastEncoderValueA = encoderValueA;


    rpmA =
      ((float)abs(pulsosA) / PULSES_PER_REVOLUTION)
      *
      (60000.0 / interval);


    // ========================================================
    // MOTOR B
    // ========================================================

    long pulsosB =
      encoderValueB - lastEncoderValueB;

    lastEncoderValueB = encoderValueB;


    rpmB =
      ((float)abs(pulsosB) / PULSES_PER_REVOLUTION)
      *
      (60000.0 / interval);


    // ========================================================
    // MOSTRAR RESULTADOS
    // ========================================================

    Serial.print("Pulsos A: ");
    Serial.print(encoderValueA);

    Serial.print("\tRPM A: ");
    Serial.print(rpmA);


    Serial.print("\tPulsos B: ");
    Serial.print(encoderValueB);

    Serial.print("\tRPM B: ");
    Serial.println(rpmB);
  }
}