# Prueba de Encoders

## Descripción

Este programa permite realizar una prueba básica de los encoders instalados en dos motores, denominados **Motor A** y **Motor B**.

El código utiliza el **canal A de cada encoder** para detectar los pulsos mediante interrupciones externas configuradas con el evento `RISING`. Cada vez que se detecta un flanco ascendente en el canal correspondiente, se incrementa un contador.

Actualmente, el programa **únicamente realiza el conteo de pulsos**. No se realiza todavía el cálculo de velocidad, RPM, sentido de giro ni posición angular.

En este caso realicé prueba manual dandole 10 vueltas al eje del motor, el monitor serial me muestra los pulsos y los divido en el numero de vueltas que realicé... Esto se hizo para mejorar el margen de error humano. 

###Notaℹ️ El valor de PPR del motor es de 225
---

## Hardware utilizado

El programa está diseñado para trabajar con dos encoders:

| Motor   | Canal A | Canal B |
| ------- | ------: | ------: |
| Motor A | GPIO 18 | GPIO 19 |
| Motor B | GPIO 22 | GPIO 23 |

Los canales B están configurados como entradas, pero en esta versión del programa **no participan en el conteo**.

### Conexión de los encoders

**Encoder Motor A:**

* Canal A → GPIO 18
* Canal B → GPIO 19

**Encoder Motor B:**

* Canal A → GPIO 22
* Canal B → GPIO 23

Las entradas se configuran utilizando `INPUT_PULLUP`, por lo que se utiliza la resistencia de pull-up interna del microcontrolador.

---

## Funcionamiento

El programa utiliza interrupciones para detectar los pulsos generados por cada encoder.

### Motor A

Cuando se detecta un flanco ascendente (`RISING`) en el GPIO 18, se ejecuta:

```cpp
void IRAM_ATTR encoderA_ISR()
{
  pulsosA++;
}
```

Cada evento incrementa el contador:

```cpp
volatile long pulsosA = 0;
```

### Motor B

De forma similar, cuando se detecta un flanco ascendente en el GPIO 22, se ejecuta:

```cpp
void IRAM_ATTR encoderB_ISR()
{
  pulsosB++;
}
```

El contador correspondiente es:

```cpp
volatile long pulsosB = 0;
```

---

## Interrupciones

Las interrupciones se configuran de la siguiente manera:

```cpp
attachInterrupt(
  digitalPinToInterrupt(ENC_A_A),
  encoderA_ISR,
  RISING
);
```

y:

```cpp
attachInterrupt(
  digitalPinToInterrupt(ENC_B_A),
  encoderB_ISR,
  RISING
);
```

Se utiliza el modo `RISING`, por lo que únicamente se contabiliza el **flanco ascendente** de la señal del canal A.

De esta manera, cada pulso válido detectado genera una interrupción y aumenta el contador correspondiente.

---

## Visualización de los pulsos

El programa utiliza el puerto serial a una velocidad de:

```cpp
Serial.begin(115200);
```

Cada **1 segundo**, el programa muestra en el monitor serial la cantidad acumulada de pulsos detectados por cada motor.

Ejemplo:

```text
Pulsos A: 125    Pulsos B: 118
Pulsos A: 247    Pulsos B: 236
Pulsos A: 371    Pulsos B: 354
```

Los valores mostrados corresponden al **conteo acumulado desde que el programa fue iniciado**. Los contadores no se reinician cada segundo.

---

## Protección del acceso a los contadores

Los contadores se declaran como:

```cpp
volatile long pulsosA = 0;
volatile long pulsosB = 0;
```

La palabra clave `volatile` indica que estas variables pueden cambiar en cualquier momento debido a las rutinas de interrupción.

Antes de copiar los valores para enviarlos por el puerto serial, se desactivan temporalmente las interrupciones:

```cpp
noInterrupts();

long cuentaA = pulsosA;
long cuentaB = pulsosB;

interrupts();
```

Esto permite realizar una copia consistente de los contadores antes de utilizarlos en el programa principal.

---

## Estructura del programa

El programa está dividido en las siguientes partes:

1. **Definición de los pines de los encoders**
2. **Definición de los contadores**
3. **Rutina de interrupción del Motor A**
4. **Rutina de interrupción del Motor B**
5. **Configuración inicial (`setup`)**
6. **Conteo y visualización de pulsos (`loop`)**

---

## Limitaciones actuales

Esta versión corresponde a una primera etapa de prueba de los encoders. Actualmente:

* Se cuenta únicamente el **canal A**.
* Se detectan solamente los flancos **RISING**.
* El canal B está conectado y configurado, pero todavía no se utiliza.
* No se calcula la velocidad de los motores.
* No se calcula RPM.
* No se determina el sentido de giro.
* No se calcula la posición angular.
* Los pulsos se muestran como un contador acumulado.

---

## Próximas mejoras

Como continuación del desarrollo, se pueden implementar:

* Cálculo de **RPM** a partir de los pulsos registrados durante un intervalo de tiempo.
* Utilización del **canal B** para determinar el sentido de giro.
* Cálculo de la **posición angular** del eje.
* Determinación de la velocidad angular.
* Filtrado o procesamiento de la señal del encoder.
* Registro de datos para realizar análisis de la respuesta de los motores.

---

## Mensaje de inicio

Al iniciar el programa, el monitor serial muestra:

```text
======================================
      PRUEBA DE ENCODERS
======================================

Motor A -> GPIO 18
Motor B -> GPIO 22

Contadores iniciados.
```

Posteriormente, cada segundo se actualizan los valores de los pulsos acumulados de ambos motores.
