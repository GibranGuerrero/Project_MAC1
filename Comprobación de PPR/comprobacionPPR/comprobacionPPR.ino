// Cuenta los pulsos del CANAL A usando RISING.
// NO hay cálculo de RPM todavía.
// ============================================================

// ============================================================
// ENCODER MOTOR A
// ============================================================

const int ENC_A_A = 18;
const int ENC_A_B = 19;

// ============================================================
// ENCODER MOTOR B
// ============================================================

const int ENC_B_A = 22;
const int ENC_B_B = 23;


// ============================================================
// CONTADORES
// ============================================================

volatile long pulsosA = 0;
volatile long pulsosB = 0;


// ============================================================
// INTERRUPCIÓN MOTOR A
// ============================================================

void IRAM_ATTR encoderA_ISR()
{
  pulsosA++;
}


// ============================================================
// INTERRUPCIÓN MOTOR B
// ============================================================

void IRAM_ATTR encoderB_ISR()
{
  pulsosB++;
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  // Encoder A
  pinMode(ENC_A_A, INPUT_PULLUP);
  pinMode(ENC_A_B, INPUT_PULLUP);

  // Encoder B
  pinMode(ENC_B_A, INPUT_PULLUP);
  pinMode(ENC_B_B, INPUT_PULLUP);

  // Interrupciones
  attachInterrupt(
    digitalPinToInterrupt(ENC_A_A),
    encoderA_ISR,
    RISING
  );

  attachInterrupt(
    digitalPinToInterrupt(ENC_B_A),
    encoderB_ISR,
    RISING
  );

  Serial.println();
  Serial.println("======================================");
  Serial.println("      PRUEBA DE ENCODERS");
  Serial.println("======================================");
  Serial.println();

  Serial.println("Motor A -> GPIO 18");
  Serial.println("Motor B -> GPIO 22");
  Serial.println();

  Serial.println("Contadores iniciados.");
  Serial.println();
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
  static unsigned long ultimoTiempo = 0;

  if (millis() - ultimoTiempo >= 1000)
  {
    ultimoTiempo = millis();

    // Copiar los contadores de forma segura
    noInterrupts();

    long cuentaA = pulsosA;
    long cuentaB = pulsosB;

    interrupts();


    Serial.print("Pulsos A: ");
    Serial.print(cuentaA);

    Serial.print("\tPulsos B: ");
    Serial.println(cuentaB);
  }
}