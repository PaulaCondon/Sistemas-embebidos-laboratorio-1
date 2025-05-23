#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Dirección del módulo I2C

#define MOISTURE_PIN A0
#define RELAY_PIN 7
#define BUTTON_MODE_PIN 8
#define BUTTON_HOWOFTEN_PIN 9
#define BUTTON_HOWLONG_PIN 10
#define BUTTON_MOISTURE_PIN 11
#define BUTTON_PANTALLA_PIN 12

// Variables configurables
unsigned long howOften = 3600000; // en milisegundos (default: 1 hora)
unsigned long howLong = 10000;    // duración del riego en ms
int minMoisture = 30;             // humedad mínima %
bool modoProgramado = true;
int pantallaActual = 0;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_MODE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_HOWOFTEN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_HOWLONG_PIN, INPUT_PULLUP);
  pinMode(BUTTON_MOISTURE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PANTALLA_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Sistema Riego");
  delay(1000);
  lcd.clear();
}

void loop() {
  static bool lastMode = HIGH, lastOften = HIGH, lastLong = HIGH, lastMoist = HIGH, lastPantalla = HIGH;
  int valorBruto = analogRead(MOISTURE_PIN);
  int humedad = map(valorBruto, 1023, 0, 0, 100);

  // === Cambio de pantalla con botón ===
  int estadoPantalla = digitalRead(BUTTON_PANTALLA_PIN);
  if (estadoPantalla == LOW && lastPantalla == HIGH) {
    pantallaActual = (pantallaActual + 1) % 3;
    lcd.clear();
    delay(300);
  }
  lastPantalla = estadoPantalla;

  switch (pantallaActual) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Modo:");
      lcd.print(modoProgramado ? "Prog-ON " : "Manual ");
      lcd.setCursor(0, 1);
      lcd.print("Humedad: ");
      lcd.print(humedad);
      lcd.print("%   ");
      break;

    case 1:
      lcd.setCursor(0, 0);
      lcd.print("H.O:");
      lcd.print(howOften / 60000);
      lcd.print("m ");
      lcd.setCursor(0, 1);
      lcd.print("H.L:");
      lcd.print(howLong / 1000);
      lcd.print("s     ");
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.print("MinMois:");
      lcd.print(minMoisture);
      lcd.print("%    ");
      lcd.setCursor(0, 1);
      lcd.print("Actual: ");
      lcd.print(humedad);
      lcd.print("% ");
      break;
  }

  // === Activar riego si necesario ===
  if (modoProgramado && humedad < minMoisture) {
    digitalWrite(RELAY_PIN, HIGH);
    delay(howLong);
    digitalWrite(RELAY_PIN, LOW);
  }

  // === Lectura de botones (con switch-case) ===
  int botones[] = {BUTTON_MODE_PIN, BUTTON_HOWOFTEN_PIN, BUTTON_HOWLONG_PIN, BUTTON_MOISTURE_PIN};
  for (int i = 0; i < 4; i++) {
    int estado = digitalRead(botones[i]);
    switch (i) {
      case 0: // MODE
        if (estado == LOW && lastMode == HIGH) {
          modoProgramado = !modoProgramado;
          delay(300);
        }
        lastMode = estado;
        break;

      case 1: // HOW OFTEN
        if (estado == LOW && lastOften == HIGH) {
          howOften += 60000; // +1 minuto
          delay(300);
        }
        lastOften = estado;
        break;

      case 2: // HOW LONG
        if (estado == LOW && lastLong == HIGH) {
          howLong += 5000; // +5 segundos
          delay(300);
        }
        lastLong = estado;
        break;

      case 3: // MOISTURE
        if (estado == LOW && lastMoist == HIGH) {
          minMoisture += 5; // +5%
          delay(300);
        }
        lastMoist = estado;
        break;
    }
  }

  delay(100);
}
