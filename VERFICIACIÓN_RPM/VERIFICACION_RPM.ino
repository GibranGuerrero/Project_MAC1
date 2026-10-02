// ============================================================
// PRUEBA DE RPM
// ESP32 + TB6612FNG
// MOTOR A - 25GA370 12V 300RPM
// Encoder: 225 PPR experimental
// ============================================================


// ============================================================
// MOTOR A
// ============================================================

#define PWMA 27
#define AIN1 25
#define AIN2 26

// Encoder
const int ENC_A_A = 18;
const int ENC_A_B = 19;


// ============================================================
// TB6612FNG
// ============================================================

#define STBY 33


// ============================================================
// ENCODER
// ============================================================

volatile long encoderValueA = 0;

long lastEncoderValueA = 0;


// ============================================================
// CONFIGURACIÓN
// ============================================================

// PPR obtenido experimentalmente
const float PULSES_PER_REVOLUTION = 225.0;

// Tiempo de medición
const unsigned long interval = 100;


// ============================================================
// RPM
// ============================================================

float rpmA = 0;


// ============================================================
// INTERRUPCIÓN
// ============================================================

void IRAM_ATTR handleEncoderA()
{
  encoderValueA++;
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  // Motor
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  // STBY
  pinMode(STBY, OUTPUT);

  // Encoder
  pinMode(ENC_A_A, INPUT_PULLUP);
  pinMode(ENC_A_B, INPUT_PULLUP);

  // Interrupción
  attachInterrupt(
    digitalPinToInterrupt(ENC_A_A),
    handleEncoderA,
    RISING
  );

  // Activar driver
  digitalWrite(STBY, HIGH);

  // Sentido de giro
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  // PWM inicial
  analogWrite(PWMA, 0);

  Serial.println();
  Serial.println("========================================");
  Serial.println("       PRUEBA DE RPM - MOTOR A");
  Serial.println("========================================");
  Serial.println();

  Serial.println("PPR utilizado: 225");
  Serial.println();

  Serial.println("Escribe un PWM entre 0 y 255.");
  Serial.println("Ejemplo: 100");
  Serial.println();
}

int pwmActual = 0;

void loop()
{
  // ==========================================================
  // RECIBIR PWM POR SERIAL
  // ==========================================================

  if (Serial.available() > 0)
  {
    int nuevoPWM = Serial.parseInt();

    if (nuevoPWM >= 0 && nuevoPWM <= 255)
    {
      pwmActual = nuevoPWM;

      analogWrite(PWMA, pwmActual);

      Serial.print("PWM establecido: ");
      Serial.println(pwmActual);
    }

    while (Serial.available() > 0)
    {
      Serial.read();
    }
  }


  // ==========================================================
  // CALCULAR RPM CADA 100 ms
  // ==========================================================

  static unsigned long previousMillis = 0;

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;

    noInterrupts();

    long pulsos =
      encoderValueA - lastEncoderValueA;

    lastEncoderValueA = encoderValueA;

    interrupts();


    // --------------------------------------------------------
    // RPM
    // --------------------------------------------------------

    rpmA =
      ((float)pulsos / PULSES_PER_REVOLUTION)
      *
      (60000.0 / interval);


    // --------------------------------------------------------
    // MOSTRAR
    // --------------------------------------------------------

    Serial.print("Pulsos: ");
    Serial.print(pulsos);

    Serial.print("\tRPM: ");
    Serial.print(rpmA);

    Serial.print("\tPWM: ");
    Serial.println(pwmActual);
  }
}