# Prueba y Caracterización de RPM - Motor A

## Descripción

El objetivo principal es determinar experimentalmente la relación entre el **PWM aplicado al motor** y la **velocidad de giro expresada en RPM (revoluciones por minuto)**.

Para realizar la medición se utiliza el **canal A del encoder**, conectado a una interrupción del ESP32. Cada flanco ascendente (`RISING`) detectado genera un pulso que es contabilizado por el programa.

La velocidad del motor se calcula a partir de la cantidad de pulsos detectados durante una ventana de medición de **100 ms**.

Como resultado de una prueba experimental previa se determinó que el encoder genera aproximadamente:

**225 pulsos por revolución (PPR).**

---

## Objetivos

### Objetivo general

Caracterizar experimentalmente la velocidad del Motor A en función del PWM aplicado, utilizando el encoder incorporado para obtener la velocidad en RPM.

### Objetivos específicos

- Verificar el funcionamiento del encoder del Motor A.
- Determinar experimentalmente los pulsos por revolución (PPR).
- Controlar la velocidad del motor mediante PWM.
- Contabilizar los pulsos generados por el encoder.
- Calcular las RPM utilizando una ventana de medición de 100 ms.
- Obtener diferentes puntos experimentales de la relación PWM-RPM.
- Establecer una base experimental para futuras etapas de modelado y control.

---

# Hardware utilizado

- ESP32
- Motor DC **25GA370 12 V - 300 RPM**
- Encoder integrado en el motor
- Driver **TB6612FNG**
- Fuente de alimentación adecuada para el motor
- Cables de conexión

---

# Conexiones

## Motor A

| Función | GPIO ESP32 |
|---|---:|
| PWMA | GPIO 27 |
| AIN1 | GPIO 25 |
| AIN2 | GPIO 26 |
| STBY | GPIO 33 |
| Encoder Canal A | GPIO 18 |
| Encoder Canal B | GPIO 19 |

### Encoder

El encoder del Motor A cuenta con dos canales:

- **Canal A → GPIO 18**
- **Canal B → GPIO 19**

En esta versión del programa solamente se utiliza el **canal A** para realizar el conteo de pulsos.

El canal B se encuentra configurado como entrada, pero todavía no se utiliza para determinar el sentido de giro.

---

# Configuración del motor

El driver TB6612FNG se habilita mediante el pin `STBY`:

```cpp
digitalWrite(STBY, HIGH);
```

El sentido de giro utilizado durante la prueba es:

```cpp
digitalWrite(AIN1, HIGH);
digitalWrite(AIN2, LOW);
```

Por lo tanto, el motor trabaja en un único sentido de giro durante esta caracterización.

El PWM se aplica mediante:

```cpp
analogWrite(PWMA, pwmActual);
```

El rango utilizado por el programa es:

```text
0 ≤ PWM ≤ 255
```

---

# Determinación experimental del PPR

Antes de realizar la medición de RPM se realizó una prueba para determinar la cantidad de pulsos generados por el encoder durante una revolución.

La prueba experimental obtenida fue:

```text
10 vueltas = 2250 pulsos
```

Por lo tanto:

```text
PPR = 2250 / 10
```

y:

```text
PPR = 225 pulsos/vuelta
```

Por esta razón, el valor utilizado en el programa es:

```cpp
const float PULSES_PER_REVOLUTION = 225.0;
```

Por lo tanto, para esta caracterización se utiliza:

```text
PPR = 225 pulsos/revolución
```

Este valor fue obtenido experimentalmente a partir de la prueba del encoder.

---

# Funcionamiento del encoder

El conteo de pulsos se realiza mediante una interrupción.

La variable encargada de almacenar el conteo es:

```cpp
volatile long encoderValueA = 0;
```

La rutina de interrupción es:

```cpp
void IRAM_ATTR handleEncoderA()
{
  encoderValueA++;
}
```

Esta función se ejecuta cada vez que se detecta un flanco ascendente en el canal A del encoder.

La interrupción se configura mediante:

```cpp
attachInterrupt(
  digitalPinToInterrupt(ENC_A_A),
  handleEncoderA,
  RISING
);
```

El modo utilizado es:

```text
RISING
```

Por lo tanto, solamente se contabilizan los flancos ascendentes de la señal del canal A.

---

# Ventana de medición

Para calcular la velocidad se utiliza una ventana de medición de:

```text
100 ms
```

Este valor se establece en el código mediante:

```cpp
const unsigned long interval = 100;
```

Cada 100 ms el programa obtiene la cantidad de pulsos generados durante ese intervalo.

Para ello se calcula la diferencia entre el contador actual y el contador de la medición anterior:

```cpp
long pulsos =
  encoderValueA - lastEncoderValueA;
```

Posteriormente se actualiza el valor anterior:

```cpp
lastEncoderValueA = encoderValueA;
```

De esta forma, `pulsos` representa únicamente los pulsos registrados durante los últimos 100 ms.

---

# Cálculo de RPM

La velocidad del motor se obtiene mediante la siguiente ecuación:

```text
RPM = (N / PPR) × (60000 / T)
```

Donde:

| Variable | Descripción |
|---|---|
| `N` | Pulsos detectados durante la ventana de medición |
| `PPR` | Pulsos por revolución |
| `T` | Tiempo de medición en milisegundos |
| `60000` | Milisegundos correspondientes a un minuto |

Para esta prueba:

```text
PPR = 225 pulsos/revolución
```

y:

```text
T = 100 ms
```

Por lo tanto:

```text
RPM = (N / 225) × (60000 / 100)
```

Simplificando:

```text
RPM = (N / 225) × 600
```

Esta es la ecuación utilizada por el programa.

En el código se implementa mediante:

```cpp
rpmA =
  ((float)pulsos / PULSES_PER_REVOLUTION)
  *
  (60000.0 / interval);
```

---

# Ejemplo de cálculo

Para un registro de:

```text
112 pulsos / 100 ms
```

se obtiene:

```text
RPM = (112 / 225) × 600
```

Por lo tanto:

```text
RPM = 298.67
```

Así:

```text
112 pulsos/100 ms → 298.67 RPM
```

Este resultado coincide con la medición obtenida experimentalmente para un PWM de aproximadamente 250.

---

# Procedimiento experimental

Para realizar la caracterización se utilizó el **Monitor Serial del ESP32**.

El programa permite ingresar un valor de PWM entre `0` y `255`.

Durante la prueba se introdujeron los siguientes valores:

```text
50
100
150
200
250
255
```

Para cada valor de PWM se observaron los pulsos registrados durante una ventana de 100 ms y las RPM calculadas por el programa.

Los resultados fueron registrados en una tabla para posteriormente analizar la relación entre el PWM aplicado y la velocidad del motor.

---

# Resultados experimentales

Los resultados obtenidos durante la prueba fueron:

| PWM | Pulsos / 100 ms | RPM |
|---:|---:|---:|
| 50  | 20 | 53.33 |
| 100 | 43 | 114.67 |
| 150 | 66 | 176.00 |
| 200 | 88–89 | 234.67–237.33 |
| 250 | 112 | 296.00–298.67 |
| 255 | 114–115 | 304.00–306.67 |

Los rangos presentes en algunos valores se deben a que el número de pulsos registrado durante cada ventana de 100 ms puede variar entre mediciones.

Por ejemplo:

```text
88 pulsos → 234.67 RPM
89 pulsos → 237.33 RPM
```

De igual manera:

```text
114 pulsos → 304.00 RPM
115 pulsos → 306.67 RPM
```

---

# Caracterización PWM vs RPM

A partir de los resultados experimentales se obtiene la siguiente relación aproximada:

| PWM | RPM aproximadas |
|---:|---:|
| 50 | 53.33 |
| 100 | 114.67 |
| 150 | 176.00 |
| 200 | 234.67–237.33 |
| 250 | 296.00–298.67 |
| 255 | 304.00–306.67 |

Los resultados muestran que, bajo las condiciones de la prueba, la velocidad del motor aumenta a medida que aumenta el PWM.

Los datos obtenidos permiten establecer una primera caracterización experimental de la relación:

```text
PWM → RPM
```

Esta caracterización puede utilizarse posteriormente para realizar el modelado del motor y desarrollar estrategias de control de velocidad.

---

# Gráfica experimental

Los resultados obtenidos permiten representar la relación entre el PWM aplicado y la velocidad del motor.

La tendencia observada es creciente: al aumentar el PWM, aumenta la velocidad de giro del Motor A.

Los puntos experimentales utilizados son aproximadamente:

```text
PWM = 50  → 53.33 RPM
PWM = 100 → 114.67 RPM
PWM = 150 → 176.00 RPM
PWM = 200 → 234.67–237.33 RPM
PWM = 250 → 296.00–298.67 RPM
PWM = 255 → 304.00–306.67 RPM
```

<img width="641" height="325" alt="image" src="https://github.com/user-attachments/assets/7454843d-34ef-4366-a09e-91bb31c8a077" />

Esta gráfica constituye una primera referencia experimental del comportamiento del motor.

---

# Comunicación serial

La comunicación con el ESP32 se realiza a:

```cpp
Serial.begin(115200);
```

Al iniciar el programa se muestra:

```text
========================================
       PRUEBA DE RPM - MOTOR A
========================================

PPR utilizado: 225

Escribe un PWM entre 0 y 255.
Ejemplo: 100
```

Después de introducir un PWM válido, el programa informa el valor establecido:

```text
PWM establecido: 100
```

Posteriormente se muestran continuamente:

```text
Pulsos: XX    RPM: XX    PWM: XX
```

Por ejemplo:

```text
Pulsos: 112    RPM: 298.67    PWM: 250
```

---

# Código principal

El programa utiliza la siguiente configuración:

```cpp
// Motor A
#define PWMA 27
#define AIN1 25
#define AIN2 26

// Encoder
const int ENC_A_A = 18;
const int ENC_A_B = 19;

// TB6612FNG
#define STBY 33
```

Los parámetros principales de la medición son:

```cpp
// PPR obtenido experimentalmente
const float PULSES_PER_REVOLUTION = 225.0;

// Tiempo de medición
const unsigned long interval = 100;
```

---

# Protección de la lectura del contador

Debido a que `encoderValueA` es modificada dentro de una rutina de interrupción, antes de copiar su valor se desactivan temporalmente las interrupciones:

```cpp
noInterrupts();

long pulsos =
  encoderValueA - lastEncoderValueA;

lastEncoderValueA = encoderValueA;

interrupts();
```

Esto permite realizar la lectura de los contadores de manera segura antes de continuar con el cálculo de RPM.

---

# Interpretación de los resultados

La prueba permitió verificar experimentalmente que el encoder puede utilizarse para determinar la velocidad del motor.

Con un valor experimental de:

```text
225 PPR
```

y una ventana de medición de:

```text
100 ms
```

es posible convertir directamente los pulsos registrados en RPM.

Por ejemplo:

```text
20 pulsos → 53.33 RPM
43 pulsos → 114.67 RPM
66 pulsos → 176.00 RPM
88 pulsos → 234.67 RPM
89 pulsos → 237.33 RPM
112 pulsos → 298.67 RPM
```

Los resultados muestran una tendencia creciente de la velocidad respecto al PWM aplicado.

Además, para valores altos de PWM se obtiene una velocidad cercana a las **300 RPM**, valor coherente con la velocidad nominal aproximada indicada para el motor utilizado.

---

# Limitaciones de la prueba

Esta prueba corresponde a una primera caracterización experimental del Motor A. Por lo tanto, existen algunas condiciones y limitaciones que deben tenerse en cuenta:

- Se utiliza únicamente el canal A del encoder.
- El canal B no se utiliza para determinar el sentido de giro.
- El motor trabaja en un único sentido durante la prueba.
- La ventana de medición utilizada es de 100 ms.
- La cantidad de pulsos registrada es un número entero, por lo que existe una resolución limitada en la medición.
- No se realiza compensación por cambios de carga.
- No se consideran explícitamente variaciones de tensión de alimentación.
- Los resultados corresponden a las condiciones experimentales en las que se realizaron las mediciones.
- El valor de 225 PPR corresponde al valor obtenido experimentalmente para este encoder mediante la prueba de 10 vueltas.

---

# Conclusiones

La prueba permitió realizar una caracterización inicial del **Motor A** utilizando un ESP32, un driver TB6612FNG y el encoder integrado.

A partir de una prueba experimental de 10 vueltas se determinó:

```text
10 vueltas = 2250 pulsos
```

por lo que:

```text
PPR = 225 pulsos/revolución
```

Este valor se utilizó para calcular las RPM del motor mediante una ventana de medición de 100 ms.

La ecuación utilizada fue:

```text
RPM = (N / 225) × 600
```

Los resultados experimentales mostraron que la velocidad aumenta conforme se incrementa el PWM aplicado al motor.

Para los valores máximos probados se obtuvieron aproximadamente:

```text
PWM = 250 → 296.00–298.67 RPM
PWM = 255 → 304.00–306.67 RPM
```

Estos resultados permiten establecer una primera relación experimental entre el PWM y la velocidad del motor.

La información obtenida puede utilizarse como base para las siguientes etapas del proyecto, especialmente para el **diseño de un sistema de control de velocidad**.


## Resumen de parámetros

| Parámetro | Valor |
|---|---:|
| Microcontrolador | ESP32 |
| Driver | TB6612FNG |
| Motor | 25GA370 |
| Tensión nominal del motor | 12 V |
| Velocidad nominal aproximada | 300 RPM |
| Encoder utilizado | Canal A |
| PPR experimental | 225 |
| Ventana de medición | 100 ms |
| Frecuencia de actualización | 10 Hz |
| PWM mínimo | 0 |
| PWM máximo | 255 |
| Pin PWM | GPIO 27 |
| Pin Encoder A | GPIO 18 |
| Pin Encoder B | GPIO 19 |
| Pin STBY | GPIO 33 |
| Baud rate | 115200 |
