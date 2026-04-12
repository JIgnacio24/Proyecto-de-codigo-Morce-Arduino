// Pines
const int buttonPin = 7; // Pin del botón
const int buzzerPin = 8; // Pin del buzzer

// Variables
String morseInput = "";   // Almacena las señales Morse del botón
unsigned long pressStartTime = 0; // Tiempo de inicio de una pulsación
unsigned long releaseStartTime = 0; // Tiempo de inicio al soltar el botón
bool buttonState = LOW;    // Estado actual del botón
bool lastButtonState = LOW; // Estado anterior del botón

// Definición del código Morse
char morseChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
String morseCode[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", // A-Z
  "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----." // 0-9
};

// Función para convertir texto a Morse
String textToMorse(String text) {
  String morse = "";
  text.toUpperCase(); // Convertir texto a mayúsculas
  for (int i = 0; i < text.length(); i++) {
    char c = text[i];
    for (int j = 0; j < sizeof(morseChars) - 1; j++) {
      if (c == morseChars[j]) {
        morse += morseCode[j] + " ";
        break;
      }
    }
  }
  return morse;
}

// Función para convertir Morse a texto
char morseToChar(String morse) {
  for (int i = 0; i < sizeof(morseChars) - 1; i++) {
    if (morseCode[i] == morse) {
      return morseChars[i];
    }
  }
  return '?'; // Retornar '?' si no encuentra coincidencia
}

// Función para reproducir código Morse con el buzzer
void playMorse(String morse) {
  for (int i = 0; i < morse.length(); i++) {
    if (morse[i] == '.') {
      tone(buzzerPin, 1000); // Emitir tono (punto)
      delay(200);
      noTone(buzzerPin);     // Detener el tono
      delay(200);
    } else if (morse[i] == '-') {
      tone(buzzerPin, 1000); // Emitir tono (guion)
      delay(500);
      noTone(buzzerPin);     // Detener el tono
      delay(200);
    } else if (morse[i] == ' ') {
      delay(1000); // Pausa entre letras
    }
  }
  noTone(buzzerPin); // Asegurarse de que el buzzer esté apagado
}

void setup() {
  pinMode(buttonPin, INPUT_PULLUP); // Configurar el botón como entrada con resistencia pull-up
  pinMode(buzzerPin, OUTPUT);      // Configurar el buzzer como salida
  noTone(buzzerPin);               // Asegurarse de que el buzzer inicie apagado
  Serial.begin(9600);             // Iniciar comunicación serial
  Serial.println("Selecciona un modo:");
  Serial.println("1: Escribe una palabra en el Monitor Serial para que el buzzer la reproduzca en Morse.");
  Serial.println("2: Usa el botón para enviar señales Morse. El Arduino identificará la letra.");
}

void loop() {
  // Verifica si hay entrada en el Monitor Serial
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // Leer la entrada
    input.trim(); // Eliminar espacios adicionales
    if (input == "1") {
      Serial.println("Modo 1: Escribe una palabra para reproducirla en Morse.");
      while (!Serial.available());
      String word = Serial.readStringUntil('\n');
      word.trim();
      String morse = textToMorse(word);
      Serial.print("Reproduciendo: ");
      Serial.println(morse);
      playMorse(morse);
    } else if (input == "2") {
      Serial.println("Modo 2: Usa el botón para enviar señales Morse.");
      while (true) {
        buttonState = digitalRead(buttonPin); // Leer el estado del botón

        if (buttonState == LOW && lastButtonState == HIGH) {
          // Inicia una pulsación
          pressStartTime = millis();
          releaseStartTime = 0;
        } else if (buttonState == HIGH && lastButtonState == LOW) {
          // Termina una pulsación
          unsigned long pressDuration = millis() - pressStartTime;
          if (pressDuration < 300) {
            morseInput += "."; // Punto
          } else {
            morseInput += "-"; // Guion
          }
          releaseStartTime = millis();
        }

        // Detectar una pausa larga para finalizar una letra
        if (releaseStartTime > 0 && millis() - releaseStartTime > 1000 && morseInput.length() > 0) {
          char decodedChar = morseToChar(morseInput);
          Serial.print("Letra recibida: ");
          Serial.println(decodedChar);
          morseInput = ""; // Reiniciar el almacenamiento del código Morse
          releaseStartTime = 0;
        }

        lastButtonState = buttonState; // Actualizar el estado del botón

        // Salir del modo 2 si se escribe algo en el Monitor Serial
        if (Serial.available() > 0) {
          Serial.readStringUntil('\n'); // Limpiar buffer
          Serial.println("Saliendo del Modo 2.");
          break;
        }
      }
    } else {
      Serial.println("Modo no válido. Escribe 1 o 2.");
    }
  }
}
