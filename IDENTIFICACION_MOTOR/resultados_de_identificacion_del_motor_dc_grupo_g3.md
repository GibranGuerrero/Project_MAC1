# Laboratorio 1: Identificación del Motor DC — Grupo G3

**Control Inteligente II (168418) · Universidad de Pamplona**

**Fecha:** 2026-11-10

**Versión de parámetros:** 2

## 1. Fotografía del montaje experimental

El robot se acondicionó sobre un soporte elevado manteniendo ambas ruedas motrices completamente en el aire y libres de rozamiento con la superficie de trabajo durante todos los ensayos.

## 2. Tabla resumen de parámetros identificados

* **Voltaje de batería medido:** $11.75\text{ V}$

* **Resolución del encoder:** $898.0\text{ cuentas/vuelta}$ ($CPV$)

| Parámetro | Rueda Izquierda (L) | Rueda Derecha (R) | Unidades | 
| ----- | ----- | ----- | ----- | 
| $K$ adelante | $1.129$ | $1.151$ | $\text{RPM/PWM}$ | 
| $B$ adelante | $6.00$ | $6.82$ | $\text{RPM}$ | 
| Zona muerta adelante | $15$ | $20$ | $\text{PWM}$ | 
| $K$ atrás | $1.148$ | $1.127$ | $\text{RPM/PWM}$ | 
| $B$ atrás | $6.62$ | $6.46$ | $\text{RPM}$ | 
| Zona muerta atrás | $15$ | $15$ | $\text{PWM}$ | 
| $K$ promediada (referencia) | $1.138$ | $1.139$ | $\text{RPM/PWM}$ | 
| $B$ promediada (referencia) | $6.31$ | $6.64$ | $\text{RPM}$ | 
| $\tau$ aceleración / cambio | $21.3$ | $22.0$ | $\text{ms}$ | 
| Retardo puro $L$ | $0.2$ | $0.0$ | $\text{ms}$ | 
| $\tau$ freno en corto ($\text{PWM} \rightarrow 0$) | $13.3$ | $13.9$ | $\text{ms}$ | 
| Retardo de freno $L_{\text{freno}}$ | $3.1$ | $2.7$ | $\text{ms}$ | 
| $K$ incremental girando adelante ($60 \rightarrow 120$) | $1.130$ | $1.149$ | $\text{RPM/PWM}$ | 
| $K$ incremental girando atrás ($-60 \rightarrow -120$) | $1.161$ | $1.101$ | $\text{RPM/PWM}$ | 
| $\text{RPM}_{\text{máx}}$ estimada ($\text{PWM} = 255$) | $282.0$ | $286.7$ | $\text{RPM}$ | 

## 3. Preguntas de análisis (Sección 7)

### 1. En un escalón desde parado ($0 \rightarrow 120$), la $K$ incremental sale menor que en un escalón con la rueda girando ($60 \rightarrow 120$). ¿Por qué? ¿Cuál de las dos describe mejor el motor para el lazo de RPM?

* **Valores medidos:**

  * Rueda L: $K_{\text{inc}}(0 \rightarrow 120) = 1.078\text{ RPM/PWM}$ vs. $K_{\text{inc}}(60 \rightarrow 120) = 1.140\text{ RPM/PWM}$.

  * Rueda R: $K_{\text{inc}}(0 \rightarrow 120) = 1.094\text{ RPM/PWM}$ vs. $K_{\text{inc}}(60 \rightarrow 120) = 1.156\text{ RPM/PWM}$.

* **Causa:** La curva estática no pasa por el origen debido al rozamiento seco y zona muerta ($\text{RPM} = K \cdot \text{PWM} - B$, con $B \approx 6.0\text{ a } 6.8\text{ RPM}$). Desde parado se disipa tensión venciendo el umbral inicial, mientras que en $60 \rightarrow 120$ el motor ya opera en la zona lineal y todo incremento de PWM se traduce en velocidad.

* **Modelo para el lazo de RPM:** Describe mejor el escalón $60 \rightarrow 120$, dado que el controlador de velocidad operará sobre consignas en régimen dinámico activo y no alrededor de cero.

### 2. ¿Por qué la $\tau$ al llevar el PWM a 0 sale menor que al acelerar? ¿Qué pasaría si en lugar de PWM 0 se pusiera el driver en standby (STBY en bajo)?

* **Valores medidos:**

  * En aceleración: $\tau_L \approx 21.3\text{ ms}$, $\tau_R \approx 22.0\text{ ms}$.

  * En frenado a PWM 0: $\tau_{L,\text{freno}} \approx 13.3\text{ ms}$, $\tau_{R,\text{freno}} \approx 13.9\text{ ms}$.

* **Causa:** Con PWM 0 y driver activo, el puente H TB6612 conecta ambos terminales del motor a tierra en cortocircuito. La fuerza contraelectromotriz produce una corriente inversa que frena dinámicamente el rotor de forma activa.

* **Efecto de STBY en bajo:** El puente H pasaría a alta impedancia (*rueda libre*). El motor se detendría exclusivamente por fricción mecánica natural, elevando drásticamente el tiempo de parada ($\tau$ significativamente mayor).

### 3. Comparen la zona muerta hacia adelante y hacia atrás, y la de las dos ruedas. Cuando el balancín está casi quieto, las ruedas cambian de sentido todo el tiempo. ¿Qué efecto tiene la zona muerta en ese caso?

* **Comparación:** La rueda L es simétrica ($15\text{ PWM}$ en ambos sentidos), mientras que la rueda R presenta asimetría ($20\text{ PWM}$ adelante y $15\text{ PWM}$ atrás).

* **Efecto en el equilibrio:** Al estar el robot en balanceo, la señal de control cruza por cero constantemente. La zona muerta genera un intervalo ciego sin par motor, introduciendo histéresis y retraso de fase que desencadenan oscilaciones continuas (*chattering*) y degradación de la estabilidad si no se implementa compensación de zona muerta en el firmware.

### 4. Con su $\tau$ y la resolución de su encoder, ¿qué limita más la rapidez del lazo de RPM, el motor o la medición de la RPM? Calculen cuántas RPM vale una cuenta en una ventana de $10\text{ ms}$.

* **Cálculo de resolución en** $10\text{ ms}$**:**
  

  $$
  \Delta \text{RPM} = \frac{1\text{ cuenta}}{898\text{ cuentas/vuelta}} \times \frac{60\text{ s/min}}{0.010\text{ s}} \approx 6.68\text{ RPM/cuenta}
  $$

* **Factor limitante:** La **medición de la RPM**. El motor responde rápidamente ($\tau \approx 21\text{--}22\text{ ms}$), pero la velocidad calculada cambia en saltos discretos de $6.68\text{ RPM}$ y aporta un retardo medio de $5\text{ ms}$ ($\text{ventana}/2$), introduciendo ruido de alta frecuencia que saturaría la acción derivativa/proporcional si se exige un lazo cerrado demasiado rápido.

### 5. Si la batería baja de $8.4\text{ V}$ a $7.4\text{ V}$, ¿cuánto cambia $K$ según la sección 4.2? ¿Qué parte del lazo de RPM tiene que corregir eso?

* **Variación porcentual de** $K$**:**
  

  $$
  \Delta K = \frac{7.4\text{ V} - 8.4\text{ V}}{8.4\text{ V}} \times 100\% \approx -11.90\%
  $$

  
  La ganancia de la planta disminuye un $11.9\%$.

* **Corrección:** La **acción integral** del controlador PI, acumulando el error en el tiempo para incrementar el ciclo útil del PWM y alcanzar la velocidad deseada a pesar de la disminución de voltaje.

### 6. Con la RPM máxima de su motor (PWM 255), calculen la velocidad máxima del robot en m/s. El lazo de equilibrio necesita reserva de velocidad para recuperarse cuando el robot se inclina. ¿Qué velocidad de avance les parece razonable para la pista, y por qué?

* **Cálculo de velocidad lineal máxima:**

  * $\text{RPM}_{\text{máx}}$ promedio: $284.4\text{ RPM}$ ($282.0\text{ RPM}$ en L y $286.7\text{ RPM}$ en R).

  * Diámetro de rueda estándar: $65\text{ mm}$ ($r = 0.0325\text{ m}$).
    

    $$
    v_{\text{máx}} = \left(284.4 \times \frac{2\pi}{60}\right) \times 0.0325\text{ m} \approx 29.78\text{ rad/s} \times 0.0325\text{ m} \approx 0.97\text{ m/s}
    $$

* **Velocidad de avance recomendada:** $0.25\text{ a } 0.30\text{ m/s}$ ($\approx 25\text{--}30\%$ de la velocidad máxima).

* **Justificación:** Es imprescindible reservar entre el $70\%$ y el $75\%$ del par y la velocidad máxima para que el lazo de balanceo e inclinación pueda acelerar o frenar repentinamente ante perturbaciones sin que los actuadores entren en saturación.

## 4. Tarea propuesta: Comparación de métodos de identificación

Para el escalón en marcha de la rueda derecha ($60 \rightarrow 120$ PWM, Ensayo 13, transición $62.2 \rightarrow 131.6\text{ RPM}$):

| Método | $\tau$ \[ms\] | $L$ \[ms\] | $\tau + L$ \[ms\] | 
| ----- | ----- | ----- | ----- | 
| **Mínimos Cuadrados (Notebook)** | $23.1$ | $0.0$ | $23.1$ | 
| **Smith (dos puntos:** $28.3\%$ **y** $63.2\%$**)** | $23.6$ | $0.2$ | $23.8$ | 
| **Ziegler-Nichols (recta tangente)** | $21.8$ | $1.9$ | $23.7$ | 

* **¿Cuál método clásico se acerca más al notebook?:** El **método de Smith**, dado que al basarse en dos puntos distribuidos a lo largo de la respuesta amortigua mejor las variaciones locales que el trazado de una tangente en un único punto.

* **Discrepancia entre métodos:** Proviene de la cuantización del encoder en intervalos pequeños de tiempo. La señal de RPM presenta saltos discretos que deforman la pendiente máxima tangible para Ziegler-Nichols.

* **Método adoptado:** **Mínimos cuadrados sobre las cuentas acumuladas**. Al integrar la respuesta en el dominio de posición angular, se elimina el ruido derivativo de cuantización y se obtienen parámetros globales físicamente consistentes.