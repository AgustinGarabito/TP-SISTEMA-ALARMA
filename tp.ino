// ----------------LIBRERIA KEYPAD----------------
#include <Keypad.h>

// DECLARACION KEYPAD
const byte COLS = 4;
const byte FILAS = 4;

// MAPEO KEYPAD
char teclado[FILAS][COLS] =
{
	'1', '2', '3', 'A',
	'4', '5', '6', 'B',
	'7', '8', '9', 'C',
  	'*', '0', '#', 'D'
};

byte pinesFilas[FILAS] = {12,8,7,6};
byte pinesCols[COLS] = {5,4,3,2};

// CONSTRUCTOR KEYPAD
Keypad kp = Keypad(makeKeymap(teclado),pinesFilas, pinesCols, FILAS, COLS);

// ----------------LIBRERIA DISPLAY----------------
#include <LiquidCrystal_I2C.h>

// DECLARACION DISPLAY
LiquidCrystal_I2C lcd(0x20,16,2);


// ----------------ESTADOS MENU----------------
enum Estado {
  MENU_PRINCIPAL,
  MENU_CONFIGURACION,
  INGRESAR_PIN,
  CONFIGURAR_PIN_NUEVO,
  CONFIGURAR_SENSORES,
  CONFIGURAR_SENSOR,
  CONFIGURAR_MODO_ALARMA,
  ALARMA,
  EVENTOS
};

// ESTADO ACTUAL Y TECLA
Estado estadoActual = MENU_PRINCIPAL;
char tecla;


// -------------------VARIABLES Y LIBRERIA EEPROM-----------------
#include <EEPROM.h>

// ESTADO ALARMA DEFAULT
int estadoAlarma = 1; // ACTIVADA (1) O DESACTIVADA (0)

boolean flagAlarma; 
// Si esta en true, se ingresa pin desde Menu alarma.
// Si esta en false, se ingresa pin desde Configurar pin.

boolean pinValidado = false;
// Si esta en true, la alarma puede ser desactivada.
// Si esta en false, a alarma no puede ser desactivada.

// MODO ALARMA DEFAULT
int modoAlarma = 1; // SONORO (1) O SILENCIOSO (0)

// ID SENSOR
int idSensor = -1; 
// Puede ser 1-2-3, segun que sensor que se este configurando

// ESTADO SENSORES DEFAULT
int estadoCocina = 1; // ACTIVADO (1) O DESACTIVADO (0)
int estadoHabitacion = 1; // ACTIVADA (1) O DESACTIVADA (0)
int estadoEntrada = 1; // ACTIVADA (1) O DESACTIVADA (0)

// INGRESAR Y VALIDAR PIN
int contador; // En que pos del pin se encuentra [0, 1, 2, 3, 4]
char pinEntrada[5] = ""; // Ej: 1 2 3 4 #
int pin[4] = {1, 2, 3, 4};

// CONTADORES EVENTOS
int contadorEvento = 0;
int contadorLectura = 0;
const int CANT_EVENTOS = 10; // CANTIDAD MAXIMA DE EVENTOS
const int TAM_EVENTO = 5; // (1 - idAmbiente, 2/3/4/5 - TiempoMin)

// -------------------DECLARACION DE LEDS-------------------
const int ledHabitacion = 9;
const int ledEntrada = 10;
const int ledCocina = 11;

const int brillo50 = 127; // VALOR 50%
const int brillo100 = 255; // VALOR 100%

// -------------------DECLARACION DE SENSORES-------------------
unsigned long duracionCocina; // DURACION DEL PULSO
unsigned long duracionHabitacion; // DURACION DEL PULSO
unsigned long duracionEntrada; // DURACION DEL PULSO

float distanciaCocina; // DISTANCIA EN CM
float distanciaHabitacion; // DISTANCIA EN CM
float distanciaEntrada; // DISTANCIA EN CM
const float distanciaDeteccion = 170; // UMBRAL DE DISTANCIA

// -------------------DECLARACION DE BUZZER-------------------
const int pinBuzzer = 13;
int frecuencia = 293;

// -------------------SETUP-------------------
void setup()
{
  // SERIAL
  Serial.begin(9600);
  
  // DEFINICION DE LEDS SALIDA
  pinMode(ledHabitacion, OUTPUT);
  pinMode(ledEntrada, OUTPUT);
  pinMode(ledCocina, OUTPUT);
  
  // PRENDER DISPLAY
  lcd.init();
  lcd.backlight();
  mostrarMenuPrincipal();
  
  // CARGA EEPROM DEFAULT
  cargarEepromDefault(pin, estadoAlarma, modoAlarma, 
        estadoCocina, estadoHabitacion, estadoEntrada);
}

// -------------------LOOP-------------------
void loop()
{
  	// RECIBO
  	char tecla = kp.getKey();
  
  	// MANEJO
  	if (tecla != NO_KEY) {
    	manejarEntrada(tecla);
  	}
  
  	delay(100);
  
  	// TRAIGO DESDE LA EEPROM EL ESTADO DE LA ALARMA
  	int estadoAlarmaE = EEPROM.read(4);
  
  	if (estadoAlarmaE == 1) {
      	// CALCULAR DISTANCIA
  		distanciaCocina = calcularDistanciaCocina(); // DISTANCIA EN CM
  		distanciaHabitacion = calcularDistanciaHabitacion(); // DISTANCIA EN CM
  		distanciaEntrada = calcularDistanciaEntrada(); // DISTANCIA EN CM
  	
  		// SISTEMA ALARMA
  		sistemaAlarma(distanciaCocina, distanciaHabitacion, distanciaEntrada);
    } else {
    	// LUZ 0%
     	analogWrite(ledHabitacion, 0);
  		analogWrite(ledEntrada, 0);
  		analogWrite(ledCocina, 0);
      	// NO SUENA
      	noTone(pinBuzzer); 
    }
  	
  	
}

// ----------------CARGAR EEPROM----------------
void cargarEepromDefault(int pin[4], int estadoAlarma, int modoAlarma, 
     int estadoCocina, int estadoHabitacion, int estadoEntrada){
  	// PIN 1234
	EEPROM.write(0,pin[0]);
 	EEPROM.write(1,pin[1]);
  	EEPROM.write(2,pin[2]);
  	EEPROM.write(3,pin[3]);
  
  	// ESTADO ALARMA ACTIVADA
  	EEPROM.write(4,estadoAlarma);
  
  	// MODO ALARMA SONORO
  	EEPROM.write(5,modoAlarma);
  
  	// ESTADO COCINA ACTIVADO
  	EEPROM.write(6,estadoCocina);
  
  	// ESTADO HABITACION ACTIVADO
  	EEPROM.write(7,estadoHabitacion);
  
  	// ESTADO ENTRADA ACTIVADO
  	EEPROM.write(8,estadoEntrada);
}


// ----------------FUNCIONES MENU PRINCIPAL------------------
// MOSTRAR MENU PRINCIPAL
// Configiguracion - Alarma - Eventos
void mostrarMenuPrincipal(){
  		lcd.clear();
    	lcd.print("1-Config 2-Alarma");
    	lcd.setCursor(3, 1);
    	lcd.print("3-Eventos");
}

// MANEJAR TECLA
// Segun el estado en el que se encuentra, 
// llama a una función determinada.
// Al iniciar el sistema se encuentra en estado MENU_PRINCIPAL.
void manejarEntrada(char tecla) {
  switch (estadoActual) {
    case MENU_PRINCIPAL:
      manejarMenuPrincipal(tecla);
      break;
    case MENU_CONFIGURACION:
      manejarMenuConfiguracion(tecla);
      break;
    case ALARMA:
      manejarMenuAlarma(tecla);
      break;
    case EVENTOS:
      manejarMenuEvento(tecla);
      break;
    case INGRESAR_PIN:
      manejarIngresarPin(tecla);
      break;
    case CONFIGURAR_PIN_NUEVO:
      manejarConfigurarPinNuevo(tecla);
      break;
    case CONFIGURAR_SENSORES:
      manejarConfigurarSensores(tecla);
      break;
    case CONFIGURAR_SENSOR:
      manejarConfigurarSensor(tecla, idSensor);
      break;
    case CONFIGURAR_MODO_ALARMA:
      manejarModoAlarma(tecla);
      break;
  }
}

// MANEJAR MENU PRINCIPAL
// Segun la tecla que recibe, 
// llama a una función determinada.
void manejarMenuPrincipal(char tecla) {
  if (tecla == '1') {
    // CAMBIA EL ESTADO Y MUESTRA EL MENU CONFIG
    estadoActual = MENU_CONFIGURACION;
    mostrarMenuConfiguracion();
  } else if (tecla == '2') {
    // CAMBIA EL ESTADO Y MUESTRA EL MENU ALARMA
    // Levanta la flag que se utilizará en ingresarPin().
    flagAlarma = true;
    estadoActual = ALARMA;
    mostrarAlarma();
  } else if (tecla == '3') {
    // CAMBIA EL ESTADO Y MUESTRA EL MENU EVENTOS
    estadoActual = EVENTOS;
    mostrarEventos();
  }
}

// MENU CONFIGURACION
// Cambiar pin - Act/Desact sensores - Cambiar modo alarma
void mostrarMenuConfiguracion(){
  	lcd.clear();
    lcd.print("1-Pin 2-Sensores");
    lcd.setCursor(2, 1);
    lcd.print("3-Modo alarma");
}

// MANEJAR MENU CONFIGURACION
// Segun la tecla que recibe, 
// llama a una función determinada.
void manejarMenuConfiguracion(char tecla) {
  if (tecla == '1') {
    // CAMBIA EL ESTADO Y MUESTRA EL MENU CONFIG PIN
    // Baja la flag que se utilizará en ingresarPin().
    flagAlarma = false;
    estadoActual = INGRESAR_PIN;
    mostrarConfigurarPin();
  } else if (tecla == '2') {
    // CAMBIA EL ESTADO Y MUESTRA EL MENU CONFIG SENSORES
    estadoActual = CONFIGURAR_SENSORES;
    mostrarConfigurarSensores();
  } else if (tecla == '3') {
    // CAMBIA EL ESTADO Y MUESTRA EL MENU MODO ALARMA
    estadoActual = CONFIGURAR_MODO_ALARMA;
    mostrarModoAlarma();
  } else if (tecla == 'A') {
    // VUELVE AL MENU ANTERIOR
    // En este caso también es el principal.
    estadoActual = MENU_PRINCIPAL;
    mostrarMenuPrincipal();
  } else if (tecla == 'C') {
    // VUELVE AL MENU PRINCIPAL
    estadoActual = MENU_PRINCIPAL;
    mostrarMenuPrincipal();
  }
}

// MOSTRAR MENU ALARMA
// Muestra si la alarma esta Activada o Desactivada
// Presionar 1 para Activar / Desactivar
void mostrarAlarma(){
  	lcd.clear();
  	// LEO EL ESTADO DESDE LA EEPROM
  	int estadoAlarmaE = EEPROM.read(4);
  	if(estadoAlarmaE == 1){
      	lcd.setCursor(4, 0);
    	lcd.print("ACTIVADA");
      	lcd.setCursor(4, 1);
    	lcd.print("1-Desact");
    } else {
      	lcd.setCursor(3, 0);
    	lcd.print("DESACTIVADA");
      	lcd.setCursor(5, 1);
    	lcd.print("1-Act");
    }	    
}

// MANEJAR MENU ALARMA
// Si el pin no esta validado (pinValidado), pide ingresarlo
// Sino cambia el estado de la alarma (Act / Desact)
void manejarMenuAlarma(char tecla) {
  if (tecla == '1') {
    // SI ESTA VALIDADO CAMBIA EL ESTADO
    int estadoAlarmaE = EEPROM.read(4);
    if(pinValidado){
      	if(estadoAlarmaE == 1){
          	// CARGO EN LA EEPROM EL NUEVO ESTADO
          	EEPROM.update(4,0);
        } else {
          	// CARGO EN LA EEPROM EL NUEVO ESTADO
          	EEPROM.update(4,1);
        }
      	
      	// MUESTRA EL ESTADO DESPUES DEL CAMBIO
      	pinValidado = false;
      	estadoActual = ALARMA;
      	mostrarAlarma();
    } else {
      	// SINO PIDE INGRESAR EL PIN
      	// Con la bandera levantada por manejarMenuPrincipal().
    	mostrarConfigurarPin();
    	estadoActual = INGRESAR_PIN;
    }
  } else if (tecla == 'A') {
    // VUELVE AL MENU ANTERIOR
    // En este caso también es el principal.
    estadoActual = MENU_PRINCIPAL;
    mostrarMenuPrincipal();
  } else if (tecla == 'C') {
    // VUELVE AL MENU PRINCIPAL
    estadoActual = MENU_PRINCIPAL;
    mostrarMenuPrincipal();
  }
}

// MENU EVENTOS
// Lista de eventos desde la EEPROM
void mostrarEventos(){
  	// SI NO HAY EVENTOS
  	if(contadorLectura == 0){
      	lcd.clear();
    	lcd.setCursor(1,0);
  		lcd.print("No hay eventos");
    // SI HAY POR LO MENOS UN EVENTO
    } else {
    	lcd.clear();
      	// SE DIRIGE A LA POS DEL EVENTO
      	// contadorLectura es la cantidad de eventos que hay
      	// TAM_EVENTO = 5 (0 idAmbiente, 1-4 Tiempo en minutos)
    	int pos = 4 + contadorLectura * TAM_EVENTO;
  		lcd.setCursor(0,0);
      	// EVENTO CON NRO DE EVENTO
  		lcd.print("Evento ");
  		int nroEvento = contadorLectura;
  		lcd.setCursor(7,0);
  		lcd.print(nroEvento);
  		lcd.setCursor(9,0);
  		lcd.print("Sig(1)");
  	
  		int ambienteLeido;
  		int tiempoMinLeido[4];
  		// LEER DESDE EEPROM
  		for(int i = 0; i < TAM_EVENTO; i++){
      		if(i == 0){
              	// AMBIENTE I -> 0
      			ambienteLeido = EEPROM.read(pos + i);
        	} else {
              	// TIEMPO I -> 1-2-3-4
        		tiempoMinLeido[i-1] = EEPROM.read(pos + i);
        	}
    	}
  		
      	// SEGUN EL ID SE ASIGNA UN STRING AMBIENTE
  		String ambiente = ambienteLeido == 9 ? "Habitacion" : ambienteLeido == 10 ? "Entrada" : "Cocina";
  		
      	// SE IMPRIME EL AMBIENTE DEL EVENTO
    	lcd.setCursor(0,1);
  		lcd.print(ambiente);
  		lcd.setCursor(ambiente.length() + 1, 1);
      	// SE IMPRIME EL TIEMPO DEL EVENTO EN MIN
  		lcd.print(tiempoMinLeido[0]);
  		lcd.print(tiempoMinLeido[1]);
  		lcd.print(":");
  		lcd.print(tiempoMinLeido[2]);
  		lcd.print(tiempoMinLeido[3]); 
    }
}

// MANEJAR MENU EVENTOS
// Al solo mostrar eventos, solo tiene un boton sig
void manejarMenuEvento(char tecla) {
  	// BOTON SIGUIENTE EVENTO
  	if(tecla == '1'){
      // DECREMENTA PARA MOSTRAR DEL MAS RECIENTE AL MAS VIEJO
      if(contadorLectura > 0){
      	contadorLectura--;
      }
  		mostrarEventos();
  	// VUELVE AL MENU ANTERIOR
  	// En este caso también es el principal.
  	} else if (tecla == 'A') {
    	estadoActual = MENU_PRINCIPAL;
    	mostrarMenuPrincipal();
      	contadorLectura = contadorEvento;
  	// VUELVE AL MENU PRINCIPAL
  	} else if (tecla == 'C') {
    	estadoActual = MENU_PRINCIPAL;
    	mostrarMenuPrincipal();
      	contadorLectura = contadorEvento;
  	}
}


// ----------------FUNCIONES SUBMENU CONFIGURACION----------------

// MENU CONFIGURAR PIN
// pIDE INGRESAR UN PIN
void mostrarConfigurarPin() {
  	lcd.clear();
  	lcd.setCursor(0, 0);
  	lcd.print("Ingrese PIN act:");
}

// MANEJAR INGRESAR PIN
// Posee dos lógicas ya que es utilizado en dos secciones del menu
// distintas, si viene desde Menu Alarma, solamente pide ingresar
// pin actual, si viene desde Configurar Pin, pide ingresar pin
// actual y si es correcto, un nuevo pin para cambiarlo
void manejarIngresarPin(char tecla) {
  	// SI LLEGA UN NUMERO Y TODAVIA NO ESTA COMPLETO EL PIN (4 NUMEROS)
    if(tecla >= '0' && tecla <= '9' && contador < 4){
        // SE INTRODUCE UN NUMERO
      	pinEntrada[contador] = tecla;
      	// SE IMPRIME
      	lcd.setCursor(contador, 1);
        lcd.print(tecla);
      	// Y SE AUMENTA LA CANTIDAD DE NUMEROS INGRESADOS
        contador++;
  	
    // SI LLEGA 'D' Y HAY POR LO MENOS UNA LETRA
    } else if (tecla == 'D' && contador > 0){
      	// DISMINUYE LA CANTIDAD DE NUMEROS INGRESADOS
      	contador--;
      	// BORRO EL NUMERO
      	lcd.setCursor(contador, 1);
        lcd.print(' ');  
     	
    // SI LLEGA '#' Y HAY 4 NUMEROS INGRESADOS (PIN COMPLETO)
    } else if (tecla == '#' && contador == 4){
        // TRANSFORMO EL PIN QUE SE INGRESO (ARRAY DE CHARS) EN UN INT
      	int pinIngresado =  (pinEntrada[0] - '0') * 1000 + 
                   			(pinEntrada[1] - '0') * 100 + 
                   			(pinEntrada[2] - '0') * 10 + 
                   			(pinEntrada[3] - '0');
      	// TRAIGO EL PIN DESDE LA EEPROM
      	// Ej: [1 ,2 ,3 ,4]
      	// 1 * 1000 = 1000 +
      	// 2 * 100 = 200 +
      	// 3 * 10 = 30 +
      	// 4 = 1234
      	int pinEeprom =  EEPROM.read(0) * 1000 + EEPROM.read(1) * 100 + EEPROM.read(2) * 10 + EEPROM.read(3);
      	// SI SON IGUALES
        if(pinIngresado == pinEeprom){
            // RESETEO CONTADOR PARA FUTURO INTENTO
          	contador = 0;
            lcd.clear();
            lcd.setCursor(0, 0);
          
          	// SI VENGO DESDE MENU ALARMA (LOGICA MENU ALARMA)
          	// SI VENGO DESDE CONFIGURAR PIN (LOGICA CONFIG PIN)
          	if(!flagAlarma){
              	// PIDO INGRESAR EL PIN NUEVO
           		lcd.print("Ingrese PIN nuevo:");
            	estadoActual = CONFIGURAR_PIN_NUEVO;
            } else {
              	// VALIDO EL PIN Y VUELVO A PANTALLA ALARMA
              	pinValidado = true;
            	estadoActual = ALARMA;
              	manejarMenuAlarma('1');
            }
          
        // SI SON DISTINTOS
        } else {
          	// PIN INCORRECTO
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("PIN incorrecto!");
            delay(2000);
          	// RESETEO CONTADOR PARA PROXIMO INTENTO
          	contador = 0;
          	// VUELVO A PEDIR EL PIN
            mostrarConfigurarPin();
        }
      
    // VUELVE AL MENU ANTERIOR
    // Según de donde venga (Menu alarma o Configurar pin).
    } else if (tecla == 'A') {
      	contador = 0;
      	// SI VIENE DE ALARMA AL MENU PRINCIPAL
      	if(flagAlarma){
          	estadoActual = MENU_PRINCIPAL;
    		mostrarMenuPrincipal();
        // SI VIENE DE CONFIGURAR PIN AL MENU CONFIGURACION
        } else {
        	estadoActual = MENU_CONFIGURACION;
        	mostrarMenuConfiguracion();
        }
    // VUELVE AL MENU PRINCIPAL  
  	} else if (tecla == 'C') {
      	contador = 0;
    	estadoActual = MENU_PRINCIPAL;
    	mostrarMenuPrincipal();
  	}
}

// MANEJAR CONFIGURAR PIN NUEVO
// Logica similar a ingresar pin
void manejarConfigurarPinNuevo(char tecla) {
  	// SI LLEGA UN NUMERO Y TODAVIA NO ESTA COMPLETO EL PIN (4 NUMEROS)
    if (tecla >= '0' && tecla <= '9' && contador < 4) {
        pinEntrada[contador] = tecla;
        
      	lcd.setCursor(contador, 1);
        lcd.print(tecla);
        contador++;
    // SI LLEGA 'D' Y HAY POR LO MENOS UNA LETRA
    } else if (tecla == 'D' && contador > 0) {
        contador--;
        lcd.setCursor(contador, 1);
        lcd.print(' ');
    // SI LLEGA '#' Y HAY 4 NUMEROS INGRESADOS (PIN COMPLETO) 
    } else if (tecla == '#' && contador == 4) {
        if (contador == 4) {
            int pinIngresado =  (pinEntrada[0] - '0') * 1000 + 
                   				(pinEntrada[1] - '0') * 100 + 
                   				(pinEntrada[2] - '0') * 10 + 
                				(pinEntrada[3] - '0');
            
          	// MODIFICA EL PIN EN LA EEPROM
          	EEPROM.write(0, pinIngresado / 1000);
            EEPROM.write(1, (pinIngresado / 100) % 10);
            EEPROM.write(2, (pinIngresado / 10) % 10);
            EEPROM.write(3, pinIngresado % 10);
          
          	// NOTIFICA QUE SE CAMBIO EL PIN
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("PIN cambiado!");
          	lcd.setCursor(0, 1);
          	// Y CUAL ES EL NUEVO PIN
            lcd.print("Su PIN es: ");
          	lcd.setCursor(12, 1);
            lcd.print(pinIngresado);
            delay(2000);
          	// RESETA EL CONTADOR PARA UN FUTURO INGRESO
          	contador = 0;
          	// VUELVE AL MENU ANTERIOR
            estadoActual = MENU_CONFIGURACION;
            mostrarMenuConfiguracion();
        }
    // VULEVE AL MENU CONFIGURACION
    } else if (tecla == 'A') {
      	// RESETA EL CONTADOR PARA UN FUTURO INGRESO
      	contador = 0;
        estadoActual = MENU_CONFIGURACION;
        mostrarMenuConfiguracion();
    // VUELVE AL MENU PRINCIPAL
  	} else if (tecla == 'C') {
      	// RESETA EL CONTADOR PARA UN FUTURO INGRESO
      	contador = 0;
    	estadoActual = MENU_PRINCIPAL;
    	mostrarMenuPrincipal();
  	}
}

// MENU CONFIG SENSORES
// Muestra los sensores: Cocina - Habitacion - Entrada
void mostrarConfigurarSensores(){
	lcd.clear();
    lcd.print("1-Cocina 2-Hab.");
    lcd.setCursor(3, 1);
    lcd.print("3-Entrada");
} 

// MANEJAR CONFIG SENSORES
// Muestra los distintos sensores con sus respectivos estados
void manejarConfigurarSensores(char tecla){
  	// MANDA A CONFIGURAR SENSOR SEGUN EL ID
  	estadoActual = CONFIGURAR_SENSOR;
  	// SENSOR COCINA
  	if (tecla == '1') {
      	// CAMBIA EL ID PARA COCINA
      	idSensor = 1;
      	// MUESTRA LA INFORMACION DEL SENSOR
    	lcd.clear();
      	lcd.setCursor(0, 0);
   		lcd.print("Cocina");
    	
      	int estadoCocinaE = EEPROM.read(6);	
      
      	if(estadoCocinaE == 1){
          	lcd.setCursor(8, 0);
      		lcd.print("ACT.");
        } else {
          	lcd.setCursor(8, 0);
        	lcd.print("DESACT.");
        }
      
      	lcd.setCursor(0, 1);
   		lcd.print("Alternar (1)");
    // SENSOR HABITACION
  	} else if (tecla == '2') {
      	// CAMBIA EL ID PARA HABITACION
      	idSensor = 2;
      	// MUESTRA LA INFORMACION DEL SENSOR
    	lcd.clear();
      	lcd.setCursor(0, 0);
   		lcd.print("Hab.");
    	
      	int estadoHabitacionE = EEPROM.read(7);	
      
      	if(estadoHabitacionE == 1){
          	lcd.setCursor(8, 0);
      		lcd.print("ACT.");
        } else {
          	lcd.setCursor(8, 0);
        	lcd.print("DESACT.");
        }
      
      	lcd.setCursor(0, 1);
   		lcd.print("Alternar (1)");
    // SENSOR ENTRADA
  	} else if (tecla == '3') {
      	// CAMBIA EL ID PARA ENTRADA
      	idSensor = 3;
      	// MUESTRA LA INFORMACION DEL SENSOR
    	lcd.clear();
      	lcd.setCursor(0, 0);
   		lcd.print("Entrada");
    	
      	int estadoEntradaE = EEPROM.read(8);	
      	if(estadoEntradaE == 1){
          	lcd.setCursor(8, 0);
      		lcd.print("ACT.");
        } else {
          	lcd.setCursor(8, 0);
        	lcd.print("DESACT.");
        }
      
      	lcd.setCursor(0, 1);
   		lcd.print("Alternar (1)");
	// VUELVE AL MENU CONFIGURACION
  	} else if (tecla == 'A') {
    	estadoActual = MENU_CONFIGURACION;
        mostrarMenuConfiguracion();
    // VUELVE AL MENU PRINCIPAL
  	} else if (tecla == 'C') {
    	estadoActual = MENU_PRINCIPAL;
    	mostrarMenuPrincipal();
  	}
}
  
// MANEJAR CONFIGURAR SENSOR
// Una vez seleccionado el sensor a configurar mediante el id,
// asignado en manejarConfigurarSensores() se activa o desactiva
void manejarConfigurarSensor(char tecla, int idSensor){
  	// ALTERNAR ESTADO
  	if (tecla == '1') {
      // PARA SENSOR COCINA
      if(idSensor == 1){
        int estadoCocinaE = EEPROM.read(6);	
        if (estadoCocinaE == 1){
        	EEPROM.update(6, 0);
        } else {
        	EEPROM.update(6, 1);
        }
        // LLEVA A LA VISTA PARA VER LOS CAMBIOS
        manejarConfigurarSensores('1');
      // PARA SENSOR HABITACION
      } else if(idSensor == 2){
        int estadoHabitacionE = EEPROM.read(7);	
        if (estadoHabitacionE == 1){
        	EEPROM.update(7, 0);
        } else {
        	EEPROM.update(7, 1);
        }
        // LLEVA A LA VISTA PARA VER LOS CAMBIOS
        manejarConfigurarSensores('2');
	  // PARA EL SENSOR ENTRADA
      } else if(idSensor == 3){
        int estadoEntradaE = EEPROM.read(8);
        if (estadoEntradaE == 1){
        	EEPROM.update(8, 0);
        } else {
        	EEPROM.update(8, 1);
        }
        // LLEVA A LA VISTA PARA VER LOS CAMBIOS
        manejarConfigurarSensores('3');

      }
    // VUELVE AL MENU CONFIGURAR SENSORES
  	} else if (tecla == 'A') {
    	estadoActual = CONFIGURAR_SENSORES;
        mostrarConfigurarSensores();
    // VUELVE AL MENU PRINCIPAL
  	} else if (tecla == 'C') {
    	estadoActual = MENU_PRINCIPAL;
    	mostrarMenuPrincipal();
  	}   
}

// MENU MODO ALARMA
// Muestra el modo: Sonoro o Silencioso
void mostrarModoAlarma(){
	lcd.clear();
  	int modoAlarmaE = EEPROM.read(5);
  	if(modoAlarmaE == 1){
   		lcd.setCursor(3, 0);
      	lcd.print("Sonoro");
    } else {
        lcd.setCursor(2, 0);
        lcd.print("Silencioso.");
    }
  	
  	lcd.setCursor(0, 1);
   	lcd.print("Alternar (1)");
} 

// MANEJAR MODO ALARMA
// Permite cambiar el modo de la alarma
void manejarModoAlarma(char tecla){
  	// ALTERNAR
  	if (tecla == '1') {
      	int modoAlarmaE = EEPROM.read(5);
      	if (modoAlarmaE == 1){
        	EEPROM.update(5, 0);
        } else {
        	EEPROM.update(5, 1);
        }
      	// LEVA A LA VISTA PARA VER LOS CAMBIOS
		mostrarModoAlarma();
    // VUELVE AL MENU CONFIGURACION
  	} else if (tecla == 'A') {
    	estadoActual = MENU_CONFIGURACION;
        mostrarMenuConfiguracion();
    // VUELVE AL MENU PRINCIPAL
  	} else if (tecla == 'C') {
    	estadoActual = MENU_PRINCIPAL;
    	mostrarMenuPrincipal();
  	}
}

// ----------------CALCULOS DISTANCIA----------------
// DISTANCIA COCINA
float calcularDistanciaCocina(){
	// USO DE SENSOR 3 PINES (COCINA)
  	// DEFINO COMO SALIDA
  	pinMode(A1, OUTPUT);
  	digitalWrite(A1, LOW);
 	//delayMicroseconds(2);
  	digitalWrite(A1, HIGH);
  	//delayMicroseconds(10);
  	digitalWrite(A1, LOW);
  	// DEFINO COMO ENTRADA
  	pinMode(A1, INPUT);
  	// CALCULO DE DURACION
  	duracionCocina = pulseIn(A1, HIGH);
  	return duracionCocina/59;
}

// DISTANCIA HABITACION
float calcularDistanciaHabitacion(){
	// USO DE SENSOR 3 PINES (HABITACION)
  	// DEFINO COMO SALIDA
  	pinMode(A2, OUTPUT);
  	digitalWrite(A2, LOW);
  	//delayMicroseconds(2);
  	digitalWrite(A2, HIGH);
  	//delayMicroseconds(10);
  	digitalWrite(A2, LOW);
 	// DEFINO COMO ENTRADA
  	pinMode(A2, INPUT);
  	// CALCULO DE DURACION
  	duracionHabitacion = pulseIn(A2, HIGH);
  	return duracionHabitacion/59;
}

// DISTANCIA ENTRADA
float calcularDistanciaEntrada(){
  	// USO DE SENSOR 3 PINES (ENTRADA)
  	// DEFINO COMO SALIDA
  	pinMode(A3, OUTPUT);
  	digitalWrite(A3, LOW);
  	//delayMicroseconds(2);
  	digitalWrite(A3, HIGH);
  	//delayMicroseconds(10);
  	digitalWrite(A3, LOW);
  	// DEFINO COMO ENTRADA
  	pinMode(A3, INPUT);
  	// CALCULO DE DURACION
	duracionEntrada = pulseIn(A3, HIGH);
  	return duracionEntrada/59;
}

// ----------------SITEMA ALARMA----------------
void sistemaAlarma(float distanciaC, float distanciaH, float distanciaE){
  	// ALARMA Y LED
  	int sensorC = EEPROM.read(6);
    int sensorH = EEPROM.read(7);
    int sensorE = EEPROM.read(8);
    
  	// SENSOR COCINA
  	manejarSensor(sensorC, distanciaC, ledCocina);

  	// SENSOR HABITACION
  	manejarSensor(sensorH, distanciaH, ledHabitacion);

  	// SENSOR ENTRADA
  	manejarSensor(sensorE, distanciaE, ledEntrada);

  	// SI NO HAY DETECCION NO SUENA
  	if (distanciaC >= distanciaDeteccion && distanciaH >= distanciaDeteccion && distanciaE >= distanciaDeteccion) {
    	noTone(pinBuzzer);
  	}
}

// MANEJAR SENSOR
void manejarSensor(int sensor, float distancia, int pinLed){
	// TRAIGO DESDE LA EEPROM EL MODO (SONORO O SILENCIOSO)
  	int modoAlarmaE = EEPROM.read(5);
  	// TIEMPO INICIO
	long unsigned tiempoInicio = millis();
  	// SI ESTA ACTIVADO
  	if (sensor == 1) {
      	// 50%
    	analogWrite(pinLed, brillo50);
      	// SI DETECTA
    	if (distancia < distanciaDeteccion) {
          	// 100%
      		analogWrite(pinLed, brillo100);
          	// SI ESTA EN SONORO
      		if (modoAlarmaE == 1) {
              	// SUENA
        		tone(pinBuzzer, frecuencia);
      		}
          	// MIDE EL TIEMPO QUE DURA LA DETECCION
          	while(distancia < distanciaDeteccion){
            	distancia = pinLed == ledCocina ? calcularDistanciaCocina() :
              				pinLed == ledHabitacion ? calcularDistanciaHabitacion() :
              				calcularDistanciaEntrada();
          	}
          	
          	// CALCULA EL TIEMPO DE DETECCION
          	long unsigned tiempo = millis() - tiempoInicio;
          	
          	// PASAJE DE MILISEGUNDOS A MINUTOS
          	int tiempoS = tiempo / 1000;
         	int tiempoM = tiempoS / 60;
          	int segR = tiempoS % 60;
          	
          	int tiempoMin[4];
        
          	tiempoMin[0] = tiempoM / 10;
    		tiempoMin[1] = tiempoM % 10;
    		tiempoMin[2] = segR / 10; 
    		tiempoMin[3] = segR % 10;
          	
          	// GUARDAR EVENTO EN EEPROM
          	guardarEvento(pinLed, tiempoMin); 	
        }
    // SI ESTA DESACTIVADO
  	} else {
      	// 0%
    	analogWrite(pinLed, 0);
  	}
}


// GUARDAR EVENTO 
// idAmbiente = 9 Habitacion // 10 Entrada // 11 Cocina
void guardarEvento(int idAmbiente, int tiempoMin[4]){
  	// SOLO SI HAY MAXIMO DE EVENTOS
  	if(contadorEvento >= CANT_EVENTOS){
      	for(int i = 0; i < CANT_EVENTOS - 1; i++){
          	// EVENTO Y SU SIGUIENTE
      		int posEvento = 9 + i * TAM_EVENTO;
        	int posSigEvento = 9 + (i + 1) * TAM_EVENTO;
          	
          	// SOBREESCRIBE EL EVENTO CON SU SIGUIENTE 
          	for(int j = 0; j < 5; j++){
            	byte valorLeido = EEPROM.read(posSigEvento + j);
              	EEPROM.write(posEvento + j, valorLeido);
            }
        }
      	
      	// BORRA EL ULTIMO EVENTO
      	int posUltEvento = 9 + (CANT_EVENTOS - 1) * 5;
      	for(int j = 0; j < CANT_EVENTOS; j++ ){
        	 EEPROM.write(posUltEvento + j, 0);
      	}
      	
      	// DRECREMENTO EL CONTADOR PORQUE YA ESTA EN EL MAXIMO
      	contadorEvento--;
 		contadorLectura = contadorEvento;
    } 
  		// AGREGO EL EVENTO NUEVO
    	int pos = 9 + contadorEvento * TAM_EVENTO;
  		// AMBIENTE
  		EEPROM.write(pos, idAmbiente);
  		pos = pos + 1;
  		// TIEMPO EN MIN
  		for(int i = 0; i < 4; i++){
  			EEPROM.write(pos + i, tiempoMin[i]); 	
  		}
  		
  		// AUMENTO EL NRO DE EVENTOS
  		contadorEvento++;
  		contadorLectura = contadorEvento;	
}