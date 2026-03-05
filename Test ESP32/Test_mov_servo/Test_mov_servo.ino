#include <ESP32Servo.h>

Servo miMotor;
int pinMotor = 14; 

void setup() {
  miMotor.attach(pinMotor); 
  
  miMotor.write(90); 
  delay(1000); // Le damos 1 segundo para que llegue al centro y se estabilice
  
  // Bucle for arreglado con puntos y comas (;)
  for (int i = 0; i < 10; i++) {
    
    if (i == 0) {
      miMotor.write(0);
      delay(1000); // TIEMPO para que los engranajes se muevan a 0 grados
    }
    else if (i == 3) {
      miMotor.write(35);
      delay(1000); // TIEMPO para llegar a 35 grados
    }
    else if (i == 6) {
      miMotor.write(75);   
      delay(1000); // TIEMPO para llegar a 75 grados
    } 
    else if (i == 9) {
      miMotor.write(180); 
      delay(1000); // TIEMPO para llegar a 180 grados
    }
    
  } // Cierra el for
  
} // ¡Cierra el setup que faltaba!

void loop() {
  // El motor se queda quieto en su sitio
}