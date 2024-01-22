#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define NIVELCARGA 1
#define NBAJA      2
#define NMEDIA     3
#define NALTA      4
#define INICIO     5
#define PAUSA      6

/*Variables maquina estados*/

//Botones BN=boton nivel, BI=boton inicio, BP=boton pausa
int bn=0;
int bi=0;
int bp=0;

int estado = NIVELCARGA;

/*Inicio subrutinas atención interrupciones*/

//Contador de vueltas necesarias para generar 1 segundo con la Preescala establecida
int count_overflow = 0;
// Registra segundos transcurridos
int sec=0;

//Interrupcion para Timer0, cada vez que transcurra 1 segundo genera una Interrupcion
ISR (TIMER0_OVF_vect){
  if (count_overflow==63){
    count_overflow=0;
    ++sec;
  }
  else  count_overflow++;
}

//Atiende boton pausa
ISR (PCINT_A_vect){
  bp=1;
}

//Atiende boton selección nivel agua
ISR (PCINT_B_vect){
  bn++;
  if(bn>3){
    bn=0;
  }
  estado = NIVELCARGA;
}

//Atiende boton inicio
ISR (PCINT_D_vect){
  estado = INICIO;
  bi = ~(bi);
}

/*Fin subrutinas atención interrupciones*/

int main(void){

  // Puerto A, pin A2 entrada / boton pausa
  DDRA = 0x02;

  // Puerto B [0:3] codificador de BCD-7 Segmentos, pines como salidas
  // Puerto B [4:6] leds de selección de nivel de agua, pines como salidas
  // Puerto B, pin B7 entrada / boton selección nivel agua
  DDRB = 0x7F;

  // Puerto D, pin 6; selector del display de 7 segmentos a encender
  // Puerto D [2:5] leds indicador estado lavado (suministro, lavar, enjuagar, centrifugar)
  // Puerto D, pin D1 activa motor
  // Puerto D, pin D0 entrada / boton inicio
  DDRD = 0x40; //0100 0000

  unsigned char reg    = 0; // Registro empleado para conversion bcd 7 segmentos
  unsigned char cont   = 0; // contador empleado etapa bcd 7 segmentos
  unsigned char bcd0   = 0; // digito 0 display 7 segmentos
  unsigned char bcd1   = 0; // digito 1 display 7 segmentos
  unsigned char bcdNum = 0; // Registro empleado para conversion bcd 7 segmentos
  unsigned char temp   = 0; // Registro empleado para conversion bcd 7 segmentos

  // Se emplea Timer0 para crear el temporizador
  //TCCR0A – Timer/Counter Control Register A se configura en normal mode
  TCCR0A=0x00;
  //TCCR0B – Timer/Counter Control Register B se configura en normal mode
  // WGM01,WGM00 se encuentran en TCCR0A, WGM02 se encuentra en TCCR0B
  TCCR0B=0x00;
  // Preescala en 1024 el reloj
  TCCR0B |= (1<<CS00)|(1<<CS02);
  //Habilita interrupciones
  sei();
  //TCNT0 – Timer/Counter Register
  TCNT0=0;
  //Habilita las interrupciones por timer
  TIMSK|=(1<<TOIE0);
  //GIMSK – General Interrupt Mask Register;
  //Se habilita interrupciones externas por cambio de pines Puertos A,B y D
  GIMSK  |= (1<<PCIE0) | (1<<PCIE2) | (1<<PCIE1);
  //Habilitan interrupciones pin A2 (PCINT10), D0 (PCINT11), B7 (PCINT7)
  PCMSK2 |= (1<<PCINT11);
  PCMSK1 |= (1<<PCINT10);
  PCMSK |= (1<<PCINT7);

  while(1){

  while(cont<8){

    /*Desplaza 1 bit a la izquierda BCD*/
    bcdNum = bcdNum << 1;

    /*Agregar primer bit mas a la izquierda*/
    temp    = (reg >> (7-cont)) & 0x01;
    bcdNum |= temp;

    if(cont==7){
      cont++;
      break;
    }
    bcd0 = bcdNum & 0x0f;

    if(bcd0>=5){
      bcd0 += 3;
    }

    bcd1 = (bcdNum & 0xf0) >> 4;

    if(bcd1>=5){
      bcd1 += 3;
      bcd1 = bcd1 << 4;
    }
    else{
      bcd1 = bcd1 << 4;
    }

    bcdNum = bcd0 | bcd1;

    cont++;
  } //Fin while BCD

    PORTD = 0x00;
    PORTB = bcdNum & 0x0f;
    _delay_ms(1000);

    PORTD = 0x40;
    PORTB = bcdNum >> 4;
    _delay_ms(1000);

  }//Fin while main

} //Fin main
