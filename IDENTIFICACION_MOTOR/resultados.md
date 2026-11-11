7. Preguntas de Análisis · Identificación del Motor DC

Grupo: G3

Batería ensayada: $11.75\text{ V}$

Resolución del encoder: $898\text{ cuentas/vuelta}$

Pregunta 1

En un escalón desde parado ($0 \rightarrow 120$), la $K$ incremental sale menor que en un escalón con la rueda girando ($60 \rightarrow 120$). ¿Por qué? ¿Cuál de las dos describe mejor el motor para el lazo de RPM?

Datos medidos:

Rueda izquierda (L): $0 \rightarrow 120$ da $K = 1.08\text{ RPM/PWM}$; $60 \rightarrow 120$ da $K = 1.14\text{ RPM/PWM}$.

Rueda derecha (R): $0 \rightarrow 120$ da $K = 1.09\text{ RPM/PWM}$; $60 \rightarrow 120$ da $K = 1.16\text{ RPM/PWM}$.

Causa: La curva del motor no arranca en cero por la fricción estática y la zona muerta ($B \approx 6.0\text{ a } 6.8\text{ RPM}$). Al arrancar desde parado se pierde parte del PWM solo para poner en marcha el rotor, mientras que en $60 \rightarrow 120$ la rueda ya gira en la zona lineal y todo el cambio de PWM produce velocidad.

Modelo para el lazo de RPM: El escalón $60 \rightarrow 120$, debido a que el controlador de velocidad trabaja con consignas activas y no alrededor de cero.

Pregunta 2

¿Por qué la $\tau$ al llevar el PWM a 0 sale menor que al acelerar? ¿Qué pasaría si en lugar de PWM 0 se pusiera el driver en standby (STBY en bajo)?

Datos medidos:

Acelerando: $\tau_L \approx 21.3\text{ ms}$; $\tau_R \approx 22.0\text{ ms}$.

Freno a PWM 0: $\tau_{L,\text{freno}} \approx 13.3\text{ ms}$; $\tau_{R,\text{freno}} \approx 13.9\text{ ms}$.

Causa: Al enviar PWM 0 con el driver encendido, el chip TB6612 conecta los bornes del motor entre sí (freno en corto). El giro residual produce una corriente opuesta que frena la rueda activamente por fuerza contraelectromotriz.

Efecto de STBY en bajo: El driver desconecta las salidas y deja el motor suelto (rueda libre). La rueda frenaría únicamente por fricción mecánica natural, tardando mucho más tiempo en detenerse ($\tau$ significativamente mayor).

Pregunta 3

Comparen la zona muerta hacia adelante y hacia atrás, y la de las dos ruedas. Cuando el balancín está casi quieto, las ruedas cambian de sentido todo el tiempo. ¿Qué efecto tiene la zona muerta en ese caso?

Datos medidos:

Rueda izquierda (L): $15\text{ PWM}$ adelante y $15\text{ PWM}$ atrás.

Rueda derecha (R): $20\text{ PWM}$ adelante y $15\text{ PWM}$ atrás.

Efecto en el equilibrio: Al estar casi quieto, las ruedas alternan de giro constantemente. La zona muerta crea un intervalo ciego donde el motor no ejerce par, produciendo retraso de fase, oscilaciones (chattering) y pérdida de estabilidad en la posición vertical si no se compensa en el firmware.

Pregunta 4

Con su $\tau$ y la resolución de su encoder, ¿qué limita más la rapidez del lazo de RPM, el motor o la medición de la RPM? Calculen cuántas RPM vale una cuenta en una ventana de $10\text{ ms}$.

Cálculo de resolución en $10\text{ ms}$:


$$\Delta \text{RPM} = \frac{1\text{ cuenta}}{898\text{ cuentas/vuelta}} \times \frac{60\text{ s/min}}{0.010\text{ s}} \approx 6.68\text{ RPM/cuenta}$$

Limitante: La medición de RPM. El motor responde rápidamente ($\tau \approx 21\text{ a } 22\text{ ms}$), pero la lectura de velocidad cambia en saltos grandes de $6.68\text{ RPM}$ por cuenta y suma un retraso de $5\text{ ms}$ ($\text{ventana}/2$), lo que genera ruido y saturación si se sintoniza el lazo demasiado rápido.

Pregunta 5

Si la batería baja de $8.4\text{ V}$ a $7.4\text{ V}$, ¿cuánto cambia $K$ según la sección 4.2? ¿Qué parte del lazo de RPM tiene que corregir eso?

Variación de $K$:
Como el voltaje aplicado al motor es proporcional al voltaje de la batería ($V \approx \frac{\text{PWM}}{255} V_{\text{bat}}$):


$$\Delta K = \frac{7.4 - 8.4}{8.4} \times 100\% \approx -11.9\%$$


La ganancia cae un $11.9\%$.

Corrección: La acción integral del controlador PI, encargada de sumar el error acumulado y aumentar el PWM hasta alcanzar la velocidad pedida sin error de estado estacionario.

Pregunta 6

Con la RPM máxima de su motor (PWM 255), calculen la velocidad máxima del robot en m/s. El lazo de equilibrio necesita reserva de velocidad para recuperarse cuando el robot se inclina. ¿Qué velocidad de avance les parece razonable para la pista, y por qué?

Velocidad máxima teórica:

$\text{RPM}_{\text{máx}}$ promedio a PWM 255: $284.4\text{ RPM}$ ($282.0\text{ RPM}$ en L y $286.7\text{ RPM}$ en R).

Con ruedas estándar de $65\text{ mm}$ de diámetro ($r = 0.0325\text{ m}$):


$$v_{\text{máx}} = \left(284.4 \times \frac{2\pi}{60}\right) \times 0.0325\text{ m} \approx 0.97\text{ m/s}$$

Velocidad recomendada: $0.25\text{ a } 0.30\text{ m/s}$ (aproximadamente un $25\text{ a } 30\%$ del límite).

Justificación: El robot necesita reservar entre el $70\%$ y el $75\%$ de su velocidad y par disponibles para que el lazo de inclinación pueda acelerar o frenar bruscamente y corregir el equilibrio sin saturar los motores.
