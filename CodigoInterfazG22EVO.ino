/*-------------CÓDIGO DE PANTALLA Y LUCES LED G22EVO-------------

Autores: Raúl Arcos, Francisco Sánchez, Pablo Aguilar, Mathias Bause
Descripción: 
*/


/*-------------LIBRERÍAS-------------*/

#include <Ticker.h>
#include <Wire.h>
#include <SPI.h>
#include <stdlib.h>
#include <mcp_can.h>
#include <ArduinoSort.h>
#include <FastLED.h>


/*-------------PINES DE DATOS-------------*/

#define LED_PIN 6    //Se define el pin 6 del Arduino como aquel por el que se transmitirán serialmente los datos necesarios a la tira LED.


/*-------------DEFINICIÓN DE LONGITUD DE TIRA LED-------------*/

#define NUM_LEDS 20  //Se especifica el número de LEDS que tiene la tira que estamos programando


/*-------------DEFINICIÓN DE IDs DE VARIABLES PARA PASO DE MENSAJES A LA PANTALLA-------------*/

/*Las siguientes definiciones de bytes viene definida por la programación de la pantalla HMI T5L0 ASIC del salpicadero.
Cada constante corresponde a un parámetro recibido por CAN BUS de parte de la ECU, y se usan para especificar qué parámetro se envía a la pantalla en cada trama de datos.
NOTA: En la Wiki se puede encontrar la explicación detallada de la programación de la pantalla en: G22 EVO -> Salpicadero.
*/
#define RPM_ID 0x51 //RPM
#define ECT_ID 0x52 //Temperatura del agua
#define GEAR_ID 0x53 //Marcha
#define TPS_ID 0x54 
#define BPS_ID 0x55
#define BVOLT_ID 0x56 //Voltaje de batería


/*-------------DEFINICIÓN DE VARIABLES Y OBJETOS-------------*/

CRGB leds[NUM_LEDS]; //Se define un array de datos de tipo CRGB (un vector de leds en el que a cada uno se le asocian valores RGB o de otra escala de colores), que representa la tira LED.

const int SPI_CS_PIN = 7; //Constante que hace referencia a que el pin del arduino conectado a la señal CS(Chip Select) del MCP2515 es el pin 7.
MCP_CAN CAN(SPI_CS_PIN); //Con esa constante que determina el pin de CS se construye una instancia de la librería MCP_CAN. 

bool ledStripInitialized = false; // Se crea una variable de tipo booleana que sirve como indicador de estado de la inicialización de la tira LED. Por ello, la variable se inicializa como falsa aún.


/*-------------DEFINICIÓN DE FUNCIONES-------------*/

//Se utiliza para atenuar la intensidad de todos los LEDs de la tira.
void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(175); //Atenúa a un 175/255 de su valor actual la intensidad del LED "i". Para ello, multiplica por el mismo factor los parámetros RGB de dicho LED.
  }
}

//Secuencia de inicio que ejecutan los LEDs del volante en el momento en que reciben alimentación.
//Se produce un efecto de barrido de izquierda a derecha y luego en sentido contrario, alternando los colores azul y naranja para cada uno. Esto se repite 2 veces.
//Luego se efectúa un rápido parpadeo de los LEDs en azul y naranja.
void ledsBegin(){
  //Se realizan los barridos arriba mencionados.
  for (int j = 0; j <= 1; j++) {                //La secuencia de barrido se repite 2 veces.
    //El siguiente bucle for produce un efecto en el que parece que un LED azul va recorriendo la tira, dejando una estela.
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(154, 255, 255);            //El LED "i" se coloca en azul (cxpresado en formato HSV).
      FastLED.show();                           //Actualizamos los cambios.
      fadeall();                                //Se reduce la intensidad de todos los LEDs.
      delay(25);                                //Se introduce un retardo de 25ms para que el efecto se aprecie mejor.
    }
    //Se repite el bucle anterior, pero el barrido con la estela se produce en sentido inverso y en color naranja.
    for (int i = (NUM_LEDS)-1; i >= 0; i--) {   
      leds[i] = CRGB(253, 50, 0);
      FastLED.show();
      fadeall();
      delay(25);
    }
  }
  //Se realiza la última parte de la secuencia, en la que, sucesivamente y a intervalos de 100ms, se apagan todos los LEDs, se ponen en azul, se vuelven a apagar, se ponen en naranja, ys e vue lven a apagar.
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(100);
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(100);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(100);
  fill_solid(leds, NUM_LEDS, CRGB(253, 50, 0));
  FastLED.show();
  delay(100);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
  delay(100);
}

//Lógica de control de la tira LED mientras el coche está encendido
void ledsVolante(unsigned int rev) {
  if (!ledStripInitialized) {
    return;  //La función termina su ejecución sin realizar nada si la iniciación de la tira LED falla.
  }
  int minRev = 5000;                                      // Valor mínimo de revoluciones
  int maxRev = 10500;                                     // Valor máximo de revoluciones
  int numLedsOn = map(rev, minRev, maxRev, 0, NUM_LEDS);  // El valor de RPM dentro del rango establecido por las 2 variables anteriores se interpola al número de LEDs de la tira a encender.

  if (rev == 0) {                                //Lógica opcional en caso de que se desee que los LEDs realicen algo cuando el coche tiene contacto pero el motor no está arrancado.
  
  } else {                                       //NOTA: Al final de este código se explica en detalle la lógica de la función fill_solid(), que a partir de este punto se usa mucho.
    
    if (rev < minRev) {                          // --- El valor de las RPM está por debajo de minRev ---
      fill_solid(leds, NUM_LEDS, CRGB::Black);   // Apaga todos los LEDs.
    
    } else if (numLedsOn <= NUM_LEDS / 3) {      // --- El valor de las RPM está en el 1er tercio del rango minRev - maxRev (La condición se establece con el número de LEDs a encender, pero recordar que eso está previamente interpolado)---
      fill_solid(leds, NUM_LEDS, CRGB::Black);   // Apaga todos los LEDs para refrescar la tira del estado anterior.
      fill_solid(leds, numLedsOn, CRGB::Green);  // Enciende tantos LEDs como numLedsOn indique, en color VERDE.
    
    } else if (numLedsOn <= 2 * NUM_LEDS / 3) {                                       // --- El valor de las RPM está en el 2er tercio del rango minRev - maxRev ---
      fill_solid(leds, NUM_LEDS, CRGB::Black);                                        // Apaga todos los LEDs para refrescar la tira del estado anterior.
      fill_solid(leds, NUM_LEDS / 3, CRGB::Green);                                    // Enciende los LEDs de todo el 1er tercio de la tira, en color VERDE.
      fill_solid(leds + NUM_LEDS / 3, numLedsOn - NUM_LEDS / 3, CRGB::Red);           // Enciende a partir del 1er LED del 2do tercio de la tira, tantos LEDs como falten para llegar al valor que numLedsOn indique, en color ROJO.
    
    } else if (rev <= maxRev) {                                                       // --- El valor de las RPM está en el 3er tercio del rango minRev - maxRev ---
      fill_solid(leds, NUM_LEDS, CRGB::Black);                                        // Apaga todos los LEDs para refrescar la tira del estado anterior.
      fill_solid(leds, 2 * NUM_LEDS / 3, CRGB::Green);                                // Enciende los LEDs de todo el 1er tercio de la tira, en color VERDE.
      fill_solid(leds + NUM_LEDS / 3, numLedsOn - NUM_LEDS / 3, CRGB::Red);           // Enciende los LEDs de todo el 2do tercio de la tira, en color ROJO.
      fill_solid(leds + 2 * NUM_LEDS / 3, numLedsOn - 2 * NUM_LEDS / 3, CRGB::Blue);  // Enciende a partir del 1er LED del 3er tercio de la tira, tantos LEDs como falten para llegar al valor que numLedsOn indique, en color AZUL.
    
    } else {                                                                          // --- El valor de las RPM está por encima de maxRev ---
      if (millis() % 100 < 50) {                                                      // Se crea un patrón alternado en el que dependiendo de si los milisegundos de ejecución del program al dividirlos entre 100 dan un resultado mayor o menor a 50...
        fill_solid(leds, NUM_LEDS, CRGB::Blue);                                       // ...los LEDs se encienden en color AZUL.
      } else {
        fill_solid(leds, NUM_LEDS, CRGB::Black);                                      // ...los LEDs se apagan.               
      }                                                                               // De esta forma se crea un parpadeo en azul de los LEDS cuando las RPM superan maxRev
    }
    FastLED.show(); //Se actualiza la tira LED para poder visualizar los cambios que se le han hecho desde la última vez que este mismo comando se ejecutó.
    delay(10);
  }
}

//Función en la que se inicializan un objeto CAN y la tira LED
void start() {
  //Se intenta inicializar un objeto CAN hasta conseguirlo.
  while (CAN_OK != CAN.begin(CAN_250KBPS)) {      
    Serial.println("CAN BUS Shield Init fail");
    Serial.println("Init CAN BUS Shield again");
    delay(100);
  }
  Serial.println("CAN BUS Shield init ok!");

  // Se inicializa la tira LED y se revisa que no haya errores.
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS); //Se configura la tira LED como una WS2812, conectada en LED_PIN (definido al inicio del código), y se crea una matriz de tipo CRGB llamada leds, con un número de LEDs igual a NUM_LEDS.
  FastLED.show();              // Se actualizan los cambios para luego comprobar la inicialización.
  ledStripInitialized = true;  // Se asume previamente que la inicialización fue correcta.
  if (leds[0]) {
    Serial.println("Error initializing LED strip!");
    ledStripInitialized = false;  // La inicialización se considera falsa si el valor de leds[0], no es 0, como debería ser al recién estar creada la tira.
  }
  //Si la tira se inicializa correctamente, se fija su brillo, y se ejecuta la secuencia de inicio con ledsBegin()
  if (ledStripInitialized) {
    FastLED.setBrightness(50);
    ledsBegin();
  }
}

//Función para generar un arreglo de bytes, con todos los valores necesarios, para poder enviar un dato de hasta 2 bytes de longitud a la pantalla HMI.
void send_serial(byte type, unsigned int value) {                     //Como parámetros se pasan el ID (type), que es el ID establecido al inicio del código para el dato que se quiera enviar. Ej: RPM_ID -> 0x51; y se envía el valor de dicho dato.
  byte dato[8] = { 0x5A, 0xA5, 0x05, 0x82, 0x00, 0x00, 0x00, 0x00 };  //Se establece un arreglo de bytes con los primeros datos necesarios para que la pantalla lo interprete como mensaje (En la Wiki hay tutoriales que lo explican a fondo), como ser la longitud y el tipo de mensaje.
  dato[4] = type;                                                     //Se configura en el mensaje el ID correspondiente al dato a enviar.
  dato[6] = (value >> 8) & 0xFF;                                      //Se configura el dato en los últimos 2 bytes.
  dato[7] = value & 0xFF;

  Serial.write(dato, 8);                                              //Se envía serialmente el mensaje, indicando su longituden bytes para ello.
}

//Función para leer los valores que envía la ECU por CAN BUS, y transmitirlos a la pantalla HMI, llamando a la función send_serial().
void readCanBus() {
  //Se crean las variables necesarias para almacenar los datos recibidos por CAN BUS
  unsigned char len = 0;                                              //Variable para almacenar la longitud de la trama de CAN BUS.
  unsigned char buf[8];                                               //Búfer para almacenar la trama recibida.
  unsigned int rpm, ect, gear, bvolt;                                 //Variables para almacenar los distintos datos que componen la trama. 

  if (CAN_MSGAVAIL == CAN.checkReceive()) {                           //Se verifica si existe alguna trama recibida para leer. Una vez sucede esto...
    CAN.readMsgBuf(&len, buf);                                        //...se calcula la longitud de la trama y se almacena en el búfer.
    if (buf[0] == 1) {                                                //Se revisa el primer byte del búfer, que corresponde a la ID, para ver si esta es 1, como debería ser en los mensajes dirigidos a la placa. 
      
      //Si la ID del CAN es la correcta, entonces se pueden leer, separar y retransimitir los datos de la trama.
      rpm = buf[1] * 256 + buf[2];                                    //Se extrae el dato de las rpm de los bytes correspondientes del búfer.                                    
      ledsVolante(rpm);                                               //Se pasa el dato de las rpm como parámetro a ledsVolante para que la tira LED funcione según las rpm.
      Serial.print("RPM: ");                                          //En el puerto serie se imprime para verificar el valor de rpm.
      Serial.println(rpm);
      send_serial(RPM_ID, rpm);                                       //Se llama a la función send_serial() para enviar el dato de las rpm con su respectiva ID a la pantalla HMI.
      //Con la salvedad de que solo con las rpm se realiza algo con la tira LED, para los demás parámetros recibidos por CAN se repite el proceso anterior: Extraerlos del búfer, imprimirlos para comprobar en el puerto serie, y enviarlos a la pantalla HMI.
      //NOTA: De qué byte(s) del búfer se extre cada parámetro, tiene que ver con cómo se configuró la trama para su envío del lado de la ECU. El video de Raúl al respecto lo explica muy bien.
      ect = buf[3] * 256 + buf[4];
      Serial.print("ECT: ");
      Serial.println(ect);
      send_serial(ECT_ID, ect);
      gear = buf[5];
      Serial.print("GEAR: ");
      Serial.println(gear);
      send_serial(GEAR_ID, gear);
      bvolt = buf[6] * 256 + buf[7];
      Serial.print("BVOLT: ");
      Serial.println(bvolt);
      send_serial(BVOLT_ID, bvolt);
    }
  }
}


/*-------------FUNCIONES setup() Y loop()-------------*/

//Gracias al extensivo uso de funciones anidadas entre sí, y arriba explicadas, el setup() y el loop() del programa son muy sencillos:

//En el setup solo se inicializa el puerto serie en 115200 baudios, que es a el baud rate del CAN programado desde la ECU y se llama a la función start() para inicializar un objeto CAN, la tira LED, y ejecutar la secuencia de inicio de los LED.
void setup() {
  Serial.begin(115200);
  start();
}

//En el loop, habiéndose comprobado que todo se inicializó correctamente, se llama a la función readCanBus() para que verifique si hay mensajes de CAN, y cuando los haya, los procese, y reenvíe los datos a la pantalla HMI.
void loop() {
  readCanBus();
}


/*------------EXPLICACIÓN fill_solid()------------*/

//La función fill_solid() tiene 3 parámetros como argumento. Entender cada uno ayuda a entender mejor la maraña de funicones fill_solid() que hay en partes del código. Su definición es: 
//fill_solid(CRGB *leds, int numToFill, const CRGB& color);

//1.- "CRGB *leds" es un puntero que apunta a la matriz que representa la tira LED y apunta al primer LED que se va a configurar por la función que estamos escribiendo. 
//Ej: Si nuestra tira LED es un objeto llamado "leds[]", y nosotros ponemos "leds" como 1er parámetro en la función fill_solid(), el puntero apunta al primer elemento de "leds[]".
//Si queremos cambiar el color de los LEDs a partir de uno que no sea el primero, se suma a "leds" la posición de dicho LED en la matriz.

//2.- "int numToFill" es simplemente el número de LEDs a los que se les quiere aplicar un color, contando siempre desde aquel al que apunte el puntero "*leds".

//3.- "const CRGB& color" es el color, descrito en RGB u otro formato, que se desea aplicar al rango de LEDs seleccionado.


/*------------FUNCIONES EN DESUSO/ALTERNATIVAS*------------/

//Las funciones mapByte y fadeDistance servían para generar una secuencia de inicio de la tira LED del volante distinta a la que se usa actualmente, con fines estéticos.
//La intención de esta secuencia era que los LEDs tengan un color predeterminado, como podía ser, la mitad azul y la mitad naranja, por ejemplo, y hacer un barrido de intensidad.
//En cada iteración, el programa se centraría en un LED "cursor" que iría avanzando desde el primero al último, para el cual, todos los demás tendrían un degradado de intensidad en función de la distancia a dicho LED. 

//Función que devuelve la interpolación de "value" dentro de un rango que va de 0 a max-1, a uno que va de 0 a 75. 
//Como se ve en la función fadeDistance, el contexto es que "value" representa la distancia entre 2 posiciones de LEDs en la tira, por eso su rango es de 0 a max-1 (max será igual a NUM_LEDS).  
/*int mapByte(int value, int max) {
  int a = map(value, 0, max - 1, 0, 75); //El valor entre 0 y 75 que retornará la función será el que determinará cuanto se atenúa el brillo de cada LED basado en su distancia a un LED fijo. 
  return a;
}*/

//Toma el resultado de la función mapByte como valor para atenuar individualmente cada LED en mayor o menor medida, según su distancia al LED cursor, o actual.
/*void fadeDistance(int ledActual) {
  for (int j = 0; j < NUM_LEDS; j++) {
    leds[j].nscale8(255 - mapByte(abs(j - ledActual), NUM_LEDS)); 
  }
}*/


//Las funciones send_data() y readCan() son variantes de send_serial() y readCanBus(), respectivamente.

//Función similar a send_serial(), en la que se opta por enviar un solo mensaje más largo, en el que se introduzcan todos los parámetros recibidos por CAN que se quieran enviar a la pantalla de una sola vez.
/*void send_data(unsigned int rpmVal, unsigned int ectVal, unsigned int gearVal, unsigned int bvoltVal){
  byte dato[18] = {0x5A, 0xA5, 0x0C, 0x82, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  dato[8] = (ectVal >> 8) & 0xFF;
  dato[9] = ectVal & 0xFF;

  dato[16] = (bvoltVal >> 8) & 0xFF;
  dato[17] = bvoltVal & 0xFF;  

  Serial.write(dato, 17);
}*/

//Función análoga a readCanBus() pero que llama a send_data() pasándole todos los datos, en vez de llamar varias veces a send_serial pasándole un dato cada vez.
//La razón por la cual esto funciona pasándole solo el primero de los ID de dato, y no necesitando pasar el ID de cada uno como en el caso de readCanBus() tiene que ver con como lee la pantalla los mensajes. Ver: G22 EVO -> Salpicadero. 
/*void readCan() {
  unsigned char len = 0;
  unsigned char buf[8];
  unsigned int rpm, ect, gear, bvolt;
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&len, buf);
    if (buf[0] == 1) {
      rpm = buf[1] * 256 + buf[2];
      ledsVolante(rpm);
      ect = buf[3] * 256 + buf[4];
      gear = buf[5];
      bvolt = buf[6] * 256 + buf[7];
      Serial.print("RPM: ");
      Serial.println(rpm);
      Serial.print("ECT: ");
      Serial.println(ect);
      Serial.print("GEAR: ");
      Serial.println(gear);
      Serial.print("BVOLT: ");
      Serial.println(bvolt);
      send_data(rpm, ect, gear, bvolt);
    }
  }
}*/