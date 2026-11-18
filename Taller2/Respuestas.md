
1. ¿Cuánto vale el polo inestable de su robot con el par como entrada (paso 16) y con α como entrada? ¿Por qué el segundo es menor? ¿Qué cambia eso en el periodo de muestreo?

R/: Con el par como entrada el polo inestable vale **17.26 rad/s** (tiempo característico 57.9 ms); con α como entrada vale **7.37 rad/s** (135.8 ms). El segundo es menor porque, con el lazo de ruedas cerrado, la rueda se opone al movimiento relativo y el cuerpo cae más cerca de una rotación alrededor del punto de contacto. Por eso 5 ms (200 Hz) dejan holgura suficiente y ya no se necesita muestrear cerca de 17 rad/s.

2. Con su geometría, ¿cuál es el mínimo de kθ? Páselo a unidades del firmware (RPM/s por grado) y compárelo con la kθ que diseñaron.

R/: Con nuestra geometría, el mínimo es **kθ = (a+kx)/b = 192.5** en las unidades de diseño; en firmware corresponde a **32.08 RPM/s por grado**. Diseñamos **kθ = 485.051**, es decir, **80.84 RPM/s por grado**, por lo que queda por encima del mínimo y tiene margen de estabilidad frente a la gravedad.

3. ¿Por qué kv y kx salen positivas? Explíquelo con lo que tiene que hacer el robot para frenar.

R/: **kv = 3.207** y **kx = 2.156** salen positivas porque la acción integral y de velocidad están orientadas a corregir la posición del eje. Para frenar un movimiento, el robot debe inclinarse hacia atrás y después hacer avanzar las ruedas respecto al cuerpo; las dos ganancias positivas producen esa acción de corrección. Además, con esta convención el par de control se escribe con α positivo y los cuatro términos tienen signo positivo.

4. En la parte A, ¿qué tanto se parecieron la medida y el modelo en subida, sobrepico y error? Si hubo diferencia, ¿a qué la atribuyen? ¿Qué aportaron la prealimentación (ensayo B8 de la guía), el modelo de referencia (B7) y el λ de 30 ms (B6)?

R/: En los ensayos con lazo cerrado el modelo converge a la consigna con error cercano a cero (entre -0.04 y 0.04 RPM), mientras que en la medición no fue posible calcular varias subidas porque quedaron como `nan` y el sobrepico medido fue 0 %. El modelo mostró subidas de 10 a 115 ms y sobrepicos pequeños, según el ensayo. Esto debe atribuirse principalmente a que el aviso del notebook indicó que el `lazo_config.h` no correspondía al modelo G3: el Kp medido estaba aproximadamente en 1.3–1.5, frente al diseño del JSON de 0.32–0.34. Por ello la comparación no es totalmente representativa. La prealimentación entrega desde el inicio el PWM correspondiente a la consigna y evita el arranque lento; el modelo de referencia suaviza la consigna para reducir excursiones y cambios bruscos; y λ = 30 ms hace la respuesta teórica más rápida que λ = 50 ms, aunque puede exigir mayor PWM.

5. ¿Qué pasó con el criterio promediado en el escalón hacia atrás y en el que cruza el cero? ¿Por qué?

R/: Con el criterio promediado, hacia atrás y al cruzar cero el error empeora respecto al criterio por sentido, porque el modelo usa una misma recta RPM = K·PWM − B para ambos sentidos. En G3 las rectas reales sí son diferentes: en izquierda, K adelante = 1.12925 y K atrás = 1.14761; en derecha, 1.15105 y 1.12705. Por eso los ensayos hacia atrás y con cruce por cero quedan mal representados al promediar K y B.

6. De la tabla de B.5, ¿qué juego de polos escogen y por qué? ¿Qué retardo total creen que tiene su robot (IMU, filtro y periodo)?

R/: Escogeríamos los polos **[-6.0, -6.5, -1.5, -2.0]**: son suficientemente lentos para ser robustos a la zona muerta y al retardo, y en la tabla dieron 0.22° con 5 ms de IMU. Polos más rápidos, como [-10,-11,-2,-2.5], ya pierden estabilidad a partir de retardos grandes. Estimamos un retardo total cercano a **10 ms**: 5 ms de filtro IMU + 5 ms de periodo de control.

7. En la simulación de B.4, ¿llega la RPM deseada al tope? ¿Qué inclinación inicial aguanta su diseño antes de saturar?

R/: Sí; con inclinación inicial de 3° la RPM deseada alcanza aproximadamente **55 RPM**, que es el tope configurado. En el barrido, 10° ya saturó en 55 RPM y mantuvo una respuesta estable; desde 15° el diseño no recuperó el equilibrio. Por tanto, el límite práctico de este diseño queda alrededor de **10°** antes de que aparezca saturación crítica.

Ganancias diseñadas en unidades del firmware: **Kθ = 80.84 RPM/s por grado; Kω = 10.92 RPM/s por grado/s; Kv = 3.21 1/s; Kx = 129.38 RPM/s por vuelta**.
