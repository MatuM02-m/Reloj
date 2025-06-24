# Trabajo Práctico de Laboratorio N° 7

- Milenkovitch, Matías
- Electrónica IV
- Laboratorio 7

Se deberá crear un repositorio git con acceso público que tenga el código fuente de una biblioteca desarrollada en C utilizando TDD (Test-Driven Development) que gestione la hora actual y la alarma de un reloj de un reloj despertador. La biblioteca deberá cumplir los siguientes requisitos: 
- La librería deberá mantener la hora actual, con precisión de segundos, a partir de la llamada a una función que se produce una cierta cantidad de veces por segundo. 
- La librería deberá proporcionar una función para ajustar la hora actual.
- La librería deberá permitir la configuración, una vez al inicio, de la cantidad de veces por segundo que se llamará a la función para mantener la hora actualizada.
- La librería deberá informar que la hora actual es inválida hasta que se llama a la función para ajustar la hora por primera vez.
- La librería deberá proporcionar una función para fijar la hora de la alarma.
- La librería deberá proporcionar una función para consultar la hora fijada para la alarma.
- La librería deberá proporcionar una función para habilitar y deshabilitar la alarma.
- La librería deberá proporcionar una función para consultar si la alarma está, o no, habilitada.
- La librería deberá generar un evento cuando la alarma esté habilitada y además hora actual coincida con la hora de la alarma.
- La librería deberá proporcionar una función para posponer la alarma una cantidad arbitraria de minutos.
- La librería deberá manejar todas las horas como un arreglo de bytes en formato BCD sin compactar, con la decena de horas en la primera posición y la unidad de los segundos en la última posición del vector.