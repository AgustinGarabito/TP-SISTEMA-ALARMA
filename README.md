<div align="center">
<h1> SISTEMAS EMBEBIDOS</h1>
</div>
<div align="center">
TRABAJO PRÁCTICO 2024: SISTEMA DE ALARMA
</div>

### OBJETIVO
Desarrollar un sistema de alarma en un hogar, protegiendo los ambientes de: entrada, comedor, habitación.

<hr>

### CIRCUITO
El circuito deberá contar con los siguientes elementos: 
<div align="center">
  <table style="width: 100%; text-align: center;">
    <tr>
      <td style="width: 33%;">Nombre</td>
      <td style="width: 33%;">Cantidad</td>
      <td style="width: 33%;">Componente</td>
    </tr>
    <tr>
      <td colspan="1" style="width: 100%; padding-top: 50px;">Input de usuario</td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">1</td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">Teclado 4x4 (Keypad)</td>
    </tr>
    <tr>
      <td colspan="1" style="width: 100%; padding-top: 50px;">
        Entrada <br>
        Habitación <br>
        Comedor	
      </td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">3</td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">Sensor de distancia ultrasónico de 3 pines</td>
    </tr>
    <tr>
      <td colspan="1" style="width: 100%; padding-top: 50px;">Interfaz con el usuario</td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">1</td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">LCD 16 X 2 (I2C)</td>
    </tr>
    <tr>
      <td colspan="1" style="width: 100%; padding-top: 50px;">
        Led Sensor Entrada <br>
        Led Sensor Habitación <br>
        Led Sensor Cocina	
      </td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">3</td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">LED Rojo</td>
    </tr>
    <tr>
      <td colspan="1" style="width: 100%; padding-top: 50px;">Placa programable</td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">1</td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">Arduino Uno R3</td>
    </tr>
    <tr>
      <td colspan="1" style="width: 100%; padding-top: 50px;">Alarma sonora</td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">1</td>
      <td colspan="1" style="width: 100%; padding-top: 50px;">Piezo (Buzzer)</td>
    </tr>
  </table>
</div>

<br>

 <div align="center">
   
   <img>  ![image](https://github.com/AgustinGarabito/TP-SISTEMA-ALARMA/assets/131818760/2bb7817a-2ad2-46c2-9c88-e04424e842dc) </img>

</div>

<br>
<hr>

## CARACTERÍSTICAS A DESARROLLAR
### CONFIGURACIÓN
Al iniciar el sistema, el sistema leerá la configuración guardada, en caso de que no haya configuración, se le pedirá al
usuario que configure:
1.	<b> PIN de Activación/Desactivación de alarma: <b> la alarma se podrá activar únicamente introduciendo ese pin. Lo mismo para desactivarla.
2.	<b> Activación/Desactivación sensores: <b> cada sensor podrá activarse o desactivarse manualmente. Por ejemplo,
se podría tener activado solamente el sensor de la entrada y tener desactivados los de la habitación y la cocina.
Cuando un sensor está desactivado, su correspondiente led estará apagado, y cuando esté activado, su
correspondiente led brillará con una intensidad del 50%.
3.	<b> Alarma silenciosa: <b> se podrá configurar el modo de la alarma para que sea silenciosa.

NOTA: debido a que en Tinkercad, la "EEPROM" se reinicia cuando se inicia la simulación, programar un método para introducir una configuración por defecto de los puntos mencionados para probar el caso donde ya haya una configuración guardada.

<br>

### ALARMA
El buzzer actuará como emisor de sonido para la alarma, teniendo el siguiente comportamiento:
1. <b> Modo sonoro: <b> cuando alguno de los sensores que esté activado detecte presencia en el rango de movimiento, el buzzer empezará a sonar.
2. <b> Modo silencioso: <b> en el caso de estar configurado este modo, el buzzer no emitirá ningún sonido.
3. <b> Leds: <b> el led correspondiente al sensor que esté activado donde se esté detectando una presencia, brillará con una intensidad del 100%.
4. <b> Registro de eventos: <b> se registrará en la EEPROM el ambiente donde se detectó la presencia y el tiempo en minutos de permanencia en dicho ambiente. Se almacenarán los últimos 10 eventos solamente.
5. <b> Alarma activada: <b> los puntos anteriores solamente aplicarán si la alarma fue activada, independientemente si los sensores están activados.
6. <b> Alarma desactivada: <b> mientras la alarma esté en este estado, se ignoran los puntos del 1 al 4.

<br>

### INTERFAZ CON EL USUARIO
Mediante el display LCD, el usuario tendrá acceso al sistema, disponiendo de menús con opciones para cada operación, en las cuáles siempre podrá volver al menú anterior si así lo desea, validando que se ingrese una opción válida en cada caso:
1.	<b> Menú Configuración: <b>
a.	<b> PIN: <b> podrá configurar el pin de activación/desactivación numérico de longitud 4 fija. En caso de querer modificar el pin actual, se le pedirá primero que introduzca el pin actual y luego el nuevo pin. Debe validar que:
•	El pin actual sea el correcto.
•	El pin nuevo sea de longitud 4.
b.	<b> Activación/Desactivación de sensores: <b> mostrará los sensores disponibles para que el usuario elija el que quiere. Una vez elegido, mostrará el estado en qué está ese sensor y dará la opción de activarlo o desactivarlo según sea el caso.
c.	<b> Modo de alarma: <b> muestra en qué modo está la alarma y dará la opción de cambiar el modo
2.	<b> Alarma: <b>
a.	<b> Activar/Desactivar: <b> según sea el caso, mostrará la opción correspondiente. Una vez elegida, pedirá el PIN para cambiar el estado de la alarma. En caso de elegir desactivar y que la alarma esté ejecutándose, detendrá la alarma, y los leds brillarán en la intensidad inicial si su correspondiente sensor está activado. 
3.	<b> Eventos: <b> mostrará los eventos sucedidos, uno por vez, empezando por el más reciente. Dando la posibilidad de mostrar el siguiente o salir.

<br>

### INPUT DE USUARIO 
Con el keypad, se podrá operar el sistema, teniendo las siguientes teclas, su función especial: 
1.	<b> Números del 0 al 9: <b> se usan para la selección de las opciones/menús que presentará el display LCD y para la introducción del PIN. 
2.	<b> Tecla de confirmación numeral #: <b> se usará para confirmar cada acción. Por ejemplo, cuando se quiere cambiar el PIN, se pide el PIN actual, una vez introducido, se confirma con # y el sistema pedirá el PIN nuevo, el cual también se confirma con #. Lo mismo para Activar/Desactivar la alarma. 
3.	<b> Tecla borrado D: <b> para el caso de introducción de PIN, con esta tecla se podrá borrar lo introducido hasta el momento para volver a tipear el PIN. 
4.	<b> Tecla Anterior A: <b> con esta tecla, el usuario volverá al menú anterior. Por ejemplo, si entró a un sensor para cambiarle el estado, con la tecla A podrá volver a la selección de sensores y si presiona otra vez A, volverá al menú de Configuración, si presiona de nuevo, volverá al menú principal. 
5.	<b> Tecla Menú principal C: <b> esta tecla sirve como atajo para volver al menú principal en un solo paso. 

<br>

### DETECCIÓN DE PRESENCIA 
La detección de presencia se simulará estableciendo un rango de distancia para los sensores dentro del cual se interpretará que hay una intrusión en el ambiente. Se deja establecer este rango a criterio del programador.

<br>
