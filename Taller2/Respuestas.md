## Preguntas de análisis

Respóndanlas con los números de su robot, en frases cortas.

1. ¿Cuánto vale el polo inestable de su robot con el par como entrada (paso 16) y con α como entrada? ¿Por qué el segundo es menor? ¿Qué cambia eso en el periodo de muestreo?

R/: Con el par como entrada el polo inestable vale **7.54 rad/s** (tiempo característico 132.6 ms); con α como entrada vale **5.61 rad/s** (178.1 ms). El segundo es menor porque, con el lazo de RPM cerrado, la rueda se opone al movimiento relativo y el cuerpo cae más cerca de una rotación alrededor del punto de contacto. Por eso 5 ms (200 Hz) dejan holgura suficiente y ya no se necesita muestrear cerca de 7.54 rad/s.

2. Con su geometría, ¿cuál es el mínimo de kθ? Páselo a unidades del firmware (RPM/s por grado) y compárelo con la kθ que diseñaron.

R/: Con nuestra geometría, el mínimo es **kθ = (a+kx)/b = 202.8** en las unidades de diseño; en firmware corresponde a **202.8/6 = 33.80 RPM/s por grado**. Diseñamos **kθ = 696.280**, es decir, **116.05 RPM/s por grado**, por lo que queda bastante por encima del mínimo y tiene margen frente a la gravedad.

3. ¿Por qué kv y kx salen positivas? Explíquelo con lo que tiene que hacer el robot para frenar.

R/: **kv = 5.520** y **kx = 3.712** salen positivas porque la acción integral y de velocidad están orientadas a corregir la posición del eje. Para frenar un movimiento, el robot debe inclinarse hacia atrás y después hacer avanzar las ruedas respecto al cuerpo; las dos ganancias positivas producen esa acción de corrección. Además, con esta convención el par de control se escribe con α positivo y los cuatro términos tienen signo positivo.

4. En la parte A, ¿qué tanto se parecieron la medida y el modelo en subida, sobrepico y error? Si hubo diferencia, ¿a qué la atribuyen? ¿Qué aportaron la prealimentación (ensayo B8 de la guía), el modelo de referencia (B7) y el λ de 30 ms (B6)?

R/: En el escalón 10→30 RPM hubo bastante semejanza en varios ensayos: por ejemplo, en la rueda L la subida fue 25 ms tanto medida como modelo y el sobrepico fue 34 % en ambas; en L el error fue 0.00 RPM frente a -0.00 RPM. En R hubo mayor diferencia: 30 ms medido frente a 15 ms modelo y 17 % frente a 50 % de sobrepico, con error -0.07 RPM. En el cruce por cero la medida fue de 25 ms en L y 20 ms en R, frente a 25 ms y 35 ms del modelo. La prealimentación fue importante: sin ella la subida pasó a 105 ms en L y 135 ms en R, frente a unos 25–30 ms con prealimentación. El modelo de referencia retrasó el cambio: 45 ms en L y 25 ms medidos frente a 45 ms y 30 ms del modelo, pero redujo la respuesta brusca. El λ = 30 ms hizo la subida más rápida que el λ = 50 ms, 15–20 ms frente a 25–30 ms, aunque puede requerir mayor mando.

5. ¿Qué pasó con el criterio promediado en el escalón hacia atrás y en el que cruza el cero? ¿Por qué?

R/: Con el criterio promediado, hacia atrás y al cruzar cero aparecen diferencias mayores con la medida real. En el cruce 10→-10 RPM, el error medido fue 0.09 RPM en L y -0.11 RPM en R; en el escalón -10→-30 RPM fue 0.03 RPM en L y 0.00 RPM en R. Esto ocurre porque se utiliza una sola recta promedio para ambos sentidos, mientras que las rectas reales son distintas: en izquierda K es 1.12925 adelante y 1.14761 atrás; en derecha es 1.15105 adelante y 1.12705 atrás.

6. De la tabla de B.5, ¿qué juego de polos escogen y por qué? ¿Qué retardo total creen que tiene su robot (IMU, filtro y periodo)?

R/: Escogeríamos los polos **[-6.0, -6.5, -1.5, -2.0]**: con 5 ms de IMU dieron una oscilación de solo **0.11°**, y siguieron estables hasta **0.26°** con 30 ms de retardo. Polos más rápidos, como [-8,-9,-2,-2.5], llegan a 1.34° con 20 ms y 1.51° con 30 ms; por eso no son tan robustos. Estimamos un retardo total cercano a **10 ms**: 5 ms del filtro DLPF de la IMU + 5 ms del periodo de muestreo.

7. En la simulación de B.4, ¿llega la RPM deseada al tope? ¿Qué inclinación inicial aguanta su diseño antes de saturar?

R/: Con inclinación inicial de 3° la RPM deseada alcanza aproximadamente **21.2 RPM**, no el tope de 55 RPM. Desde alrededor de 8° la RPM deseada sí llega a **55 RPM** y el robot todavía se recupera; en 9° también se alcanza el tope, pero el robot cae. Por tanto, el límite práctico antes de caer queda aproximadamente en **8°**, aunque cerca de ese valor ya aparece saturación.

Ganancias diseñadas en unidades del firmware: **Kθ = 116.05 RPM/s por grado; Kω = 20.64 RPM/s por grado/s; Kv = 5.52 1/s; Kx = 222.71 RPM/s por vuelta**.
