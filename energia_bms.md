# Documentacion del Subsistema de Energia: Baterias 18650 y BMS 3S

Este documento detalla la arquitectura, el cableado y el funcionamiento del sistema de alimentacion de energia diseñado para el robot balancin seguidor de linea. Se implemento un paquete de baterias de iones de litio (Li-Ion) gestionado por un Sistema de Gestion de Baterias (BMS).

---

## 1. Referencias Visuales

A continuacion, se presentan los componentes principales del subsistema de energia:

![Baterias Li-Ion 18650](https://m.media-amazon.com/images/I/61Nl8a02aEL.jpg)
*Figura 1: Celdas cilindricas de Iones de Litio (Li-Ion) tamaño 18650 (3.7V - 2600mAh).*

![Modulo BMS 3S 20A](https://m.media-amazon.com/images/I/61y8B34g4xL.jpg)
*Figura 2: Modulo BMS 3S 20A utilizado para la proteccion y balanceo de carga de las celdas en serie.*

---

## 2. Funciones Completas del Modulo BMS 3S

El BMS (Battery Management System) es el controlador de seguridad del paquete de baterias. Su implementacion es estrictamente obligatoria al trabajar con celdas de litio en serie. Sus cuatro funciones principales son:

1.  **Proteccion contra Sobredescarga (Over-discharge):** Evita la "muerte" de las baterias. Corta automaticamente el suministro de energia al robot si el voltaje de alguna celda cae por debajo de los 2.5V - 3.0V, previniendo daños quimicos irreversibles.
2.  **Proteccion contra Sobrecarga (Overcharge):** Durante el proceso de recarga, el BMS detiene la entrada de corriente en el momento exacto en que las celdas alcanzan su capacidad maxima (4.2V por celda / 12.6V en total), evitando el riesgo de inflacion, fuego o explosion.
3.  **Proteccion contra Cortocircuitos y Sobrecorriente:** Los motores JGA25 generan picos altos de corriente (hasta 3.0A) al invertir el giro para equilibrar el robot. El BMS de 20A soporta estos picos de trabajo, pero si detecta un consumo anomalo mayor a su limite o un cortocircuito en la placa (PCB), bloquea la salida de energia en milisegundos para no derretir los cables ni dañar la electronica.
4.  **Balanceo de Celdas (Cell Balancing):** Asegura que las tres baterias en la serie se carguen y descarguen al mismo ritmo. Sin esta funcion, una bateria podria sobrecargarse mientras otra sigue descargada, acortando drasticamente la vida util del paquete.

---

## 3. Especificaciones Tecnicas del Paquete

*   **Configuracion:** 3S1P (3 celdas en serie, 1 en paralelo).
*   **Voltaje Nominal:** 11.1V.
*   **Voltaje de Carga Maxima:** 12.6V.
*   **Capacidad Total:** 2600 mAh.
*   **Corriente de Descarga del BMS:** 20A continuos.
*   **Autonomia Estimada del Robot:** 45 a 60 minutos continuos de operacion dinamica.

---

## 4. Diagrama y Guia de Conexiones

Para garantizar el correcto funcionamiento del BMS, las soldaduras desde el portapilas de 3 celdas hacia el modulo deben realizarse en el siguiente orden secuencial:

### Fase 1: Conexiones de Balanceo (Desde el Portapilas al BMS)

1.  **Pin B- (o 0V):** Conectar mediante soldadura al polo NEGATIVO de la Celda 1 (Extremo negativo principal de la serie).
2.  **Pin B1 (o 4.2V):** Conectar a la pletina metalica que une el POSITIVO de la Celda 1 con el NEGATIVO de la Celda 2.
3.  **Pin B2 (o 8.4V):** Conectar a la pletina metalica que une el POSITIVO de la Celda 2 con el NEGATIVO de la Celda 3.
4.  **Pin B+ (o 12.6V):** Conectar mediante soldadura al polo POSITIVO de la Celda 3 (Extremo positivo principal de la serie).

### Fase 2: Puerto Comun (Carga y Descarga)

El modulo utilizado es de "puerto comun", lo que significa que la salida de energia hacia el robot y la entrada para el cargador utilizan los mismos terminales:

*   **Pin P+ / P- (A veces marcados como + / - en los bordes):** Estos son los terminales de salida de potencia principal.
    *   **Descarga:** De aqui se derivan los cables hacia el interruptor principal (ON/OFF) del chasis, alimentando posteriormente el conversor buck XL4005 y el driver de los motores.
    *   **Carga:** En paralelo a la salida del interruptor, se instala un Jack DC hembra fijado al chasis para conectar el cargador de pared de 12.6V sin necesidad de retirar las baterias.

> **Nota de Inicializacion:** Al terminar las soldaduras, es posible que los pines P+ y P- entreguen 0V. Para "despertar" el BMS y activar la salida, se debe conectar el cargador de pared de 12.6V a estos pines durante unos segundos.