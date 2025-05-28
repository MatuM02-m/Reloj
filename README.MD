# Trabajo Práctico de Laboratorio N° 6

- Milenkovitch, Matías
- Electrónica IV
- Laboratorio 6

Se deberá crear un repositorio git con acceso público que tenga el código fuente de un proyecto en C que cumpla las siguientes condiciones: 
- Modifique la HAL creada en el TP4 para darle soporte a los botones del poncho y al display de 7 segmentos, con el objetivo de utilizarlo en los próximos prácticos para la implementación del sistema de reloj despertador.
- La BSP debe dar soporte para 6 entradas digitales para las teclas, una salida digital para el zumbador y para la pantalla de 7 segmentos de 4 dígitos.
- El controlador de la pantalla de 7 segmentos deberá ser portable a cualquier otra plataforma, por lo que no deberá utilizar funciones del fabricante para acceder al hardware.
- Toda la gestión de las entradas y salidas digitales se deberá encapsular en un tipo abstracto de datos que deberá estar implementado en un archivo fuente separado del programa principal.
- Toda la configuración de la placa de soporte del proyecto, como también la creación de los recursos de entradas y salidas digitales, deberá estar encapsulada en una abstracción que también deberá ser implementada en un archivo fuente separado.
- La abstracción para la gestión de la placa será a medida del proyecto, asignando nombres funcionales a los recursos creados para facilitar la programación de la aplicación.
- El programa principal solo deberá utilizar las abstracciones creadas evitando interactuar directamente con las funciones del fabricante.