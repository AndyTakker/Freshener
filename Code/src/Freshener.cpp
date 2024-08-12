/* Управление освежителем воздуха. (29-01-2024)
Если свет включен - измеряется время включенного состояния. 
При переходе из включенного в выключенное состояние проверяется измеренное время
Если во включенном состоянии находились более 5мин, делается три пшика.
После этого происходит переход в спящее состояние с пробуждением по включению света.
В состоянии, когда свет включен, можно пшикнуть кнопкой на морде.
*/
#include <Arduino.h>
#include <avr/sleep.h>

#define pinPress 0    // (нога 5) Выход нажатия на распылитель
#define pinRelease 1  // (нога 6) Выход возврата распылителя в исходное положение
#define pinLed 3      // (нога 2) Светодиод на корпусе
#define pinLight 2    // (нога 7) Вход датчика освещенности (13кОм на землю, датчик к +3.3в)
#define pinBtn 4      // (нога 3) Кнопка на корпусе

#define sittingTime 300000  // 5*60*1000 Время сидения, после которого пшикаем
#define shortSitTime 120000 // 2*60*1000 Короткое время (для жены)
// #define sittingTime 15000 // 15 сек. для отладки

#define ON 1
#define OFF 0

#define LED_ON()    digitalWrite(pinLed, LOW)   // Включить светодиод
#define LED_OFF()   digitalWrite(pinLed, HIGH)  // Погасить светодиод
#define bttnPressed digitalRead(pinBtn) == LOW  // Кнопка нажата

void pshik(uint8_t cnt);      // Пшикаем   
void ledBlink(uint8_t cnt);   // Мигнем светодиодом
void system_sleep(void);      // Переход в спячку

uint32_t lightOn = 0;         // Момент включения света

void setup() {
  
  // Настройка выводов контроллера
  pinMode(pinPress, OUTPUT);
  digitalWrite(pinPress, LOW);
  pinMode(pinRelease, OUTPUT);
  digitalWrite(pinRelease, LOW);

  pinMode(pinLed, OUTPUT);
  LED_OFF();

  pinMode(pinBtn, INPUT_PULLUP);
  pinMode(pinLight,INPUT_PULLUP);

  // При включении помигаем светодиодом
  ledBlink(5);
  system_sleep(); // Сразу в сон после включения
  
} // setup

void loop() {
static bool lightState = OFF;
bool prevState = lightState;

  lightState = digitalRead(pinLight);
  // Если состояние света изменилось (свет был выключен и включился)
  if(lightState != prevState) {
    prevState = lightState;
    if(lightState == ON) {  // Свет включили. Начнем измерять время включенного состояния
      lightOn = millis();   // Момент включения
      LED_ON();
    } else {                // Свет выключили
      if (lightOn !=0) {
        if(millis() - lightOn > sittingTime) {  // Свет горел дольше заданного значения, нужно пшикать
          ledBlink(3);
          pshik(3);
        } else {
          if(millis() - lightOn > shortSitTime) {  // Свет горел по меньшему порогу, нужно пшикать, но меньше
            ledBlink(1);
            pshik(1);
          }
        }
      }
      // И тут засыпаем, т.к. свет уже погас и пшики сделаны (если должны были)
      lightOn = 0;
      ledBlink(6);
      system_sleep();
    }
  }

  // При каждом нажатии кнопки один раз мигаем и пшикаем.
  // Кнопка обрабатывается при включенном свете, а при выключенном мы спим.
  if( bttnPressed) {
    ledBlink(1);
    pshik(1);
    LED_ON();
  }

} // loop

// Функция пшикает указанное число раз. Для этого мотор включается на некоторое время в одну сторону, затем в другую.
void pshik(uint8_t cnt) {
const uint16_t pressTime = 1000;    // Время включения двигателя для нажатия
const uint16_t releaseTime = 1000;  // Время включения двигателя на отпускание
const uint16_t pause = 200;         // Пауза после нажатия перед отпусканием
const uint16_t pauseBetween = 500;  // Пауза между пшиками

  digitalWrite(pinRelease, LOW);   // Это чтобы перебдеть
  for(uint8_t i = 0; i < cnt; i++) {
    // Нажатие
    digitalWrite(pinPress, HIGH);
    delay(pressTime);
    digitalWrite(pinPress, LOW);
    
    delay(pause); // Пауза перед отпусканием

    // Отпускание
    digitalWrite(pinRelease,HIGH);
    delay(releaseTime);
    digitalWrite(pinRelease, LOW);
    if(i < cnt-1) {
      delay(pauseBetween);    // Пауза после пшика (перед следующим), если он не последний
    }
  }

}

// Мигнем светодиодом указанное число раз
void ledBlink(uint8_t cnt) {

  for(uint8_t i = 0; i < cnt; i++) {
    LED_ON();
    delay(200);
    LED_OFF();
    delay(200);
  }
}

// Переход в спячку с пробуждением по пину PB2 (PCINT)
void system_sleep() {

  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT2);                   // Use PB2 as interrupt pin
  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
  sleep_cpu();                            // sleep

  cli();                                  // Disable interrupts
  PCMSK &= ~_BV(PCINT2);                  // Turn off PB2 as interrupt pin
  sleep_disable();                        // Clear SE bit
  ADCSRA |= _BV(ADEN);                    // ADC on

  sei();                                  // Enable interrupts
}

ISR(PCINT0_vect) {
    // Тут делать ничего не нужно. Нам достаточно проснуться.
}
