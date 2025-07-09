#include <ESP32Servo.h> 

#define LED_VERMELHO 0
#define LED_AMARELO 16
#define LED_VERDE 5
#define BOTAO 12
#define SERVO_PIN 26
#define TRIG_PIN 14
#define ECHO_PIN 27


Servo catraca;

bool liberado = false;
int voltas = 0;
bool correndo = false;
bool primeiraPassagem = true;

const float distanciaVolta = 107.0; // cm
float velocidades[3];               // Velocidades por volta

unsigned long tempoAnterior = 0;
unsigned long tempoAtual = 0;

void setup() {
  Serial.begin(115200);

  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(BOTAO, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  

  catraca.setPeriodHertz(50);
  catraca.attach(SERVO_PIN, 500, 2400);
  catraca.write(0);  // Catraca fechada

  digitalWrite(LED_VERMELHO, HIGH); // Começa no vermelho
}

long medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH, 30000);
  long distancia = duracao * 0.034 / 2;  // cm

  return distancia;
}

void calcularTempoVolta() {
  tempoAtual = millis();
  unsigned long tempoVolta = tempoAtual - tempoAnterior;
  float tempoSegundos = tempoVolta / 1000.0;

  Serial.print("⏱️ Tempo da volta: ");
  Serial.print(tempoSegundos);
  Serial.println(" segundos");

  float velocidade = distanciaVolta / tempoSegundos;
  velocidades[voltas - 1] = velocidade;

  Serial.print("🚗 Velocidade média: ");
  Serial.print(velocidade);
  Serial.println(" cm/s");

  tempoAnterior = tempoAtual;
}

void mostrarResultados() {
  Serial.println("==========================================");
  Serial.println("       🏎️  TREINAMENTO - VOLTAS LIVRES");
  Serial.println("==========================================\n");

  Serial.println("Resumo das voltas:");
  Serial.println("------------------------------------------");
  Serial.println("| Volta | Tempo (s) | Velocidade (cm/s) |");
  Serial.println("------------------------------------------");

  float tempos[3];
  float somaTempos = 0;
  float somaVelocidades = 0;

  float maisRapida = velocidades[0];
  float maisLenta = velocidades[0];
  int voltaMaisRapida = 1;
  int voltaMaisLenta = 1;

  for (int i = 0; i < 3; i++) {
    tempos[i] = distanciaVolta / velocidades[i];
    somaTempos += tempos[i];
    somaVelocidades += velocidades[i];

    Serial.print("|   ");
    Serial.print(i + 1);
    Serial.print("   |   ");
    Serial.print(tempos[i], 2);
    Serial.print("    |      ");
    Serial.print(velocidades[i], 2);
    Serial.println("        |");

    if (velocidades[i] > maisRapida) {
      maisRapida = velocidades[i];
      voltaMaisRapida = i + 1;
    }

    if (velocidades[i] < maisLenta) {
      maisLenta = velocidades[i];
      voltaMaisLenta = i + 1;
    }
  }

  Serial.println("------------------------------------------\n");

  Serial.print("🏆 Volta mais rápida: Volta ");
  Serial.print(voltaMaisRapida);
  Serial.print(" (");
  Serial.print(maisRapida, 2);
  Serial.println(" cm/s)");

  Serial.print("🐢 Volta mais lenta : Volta ");
  Serial.print(voltaMaisLenta);
  Serial.print(" (");
  Serial.print(maisLenta, 2);
  Serial.println(" cm/s)");

  Serial.print("\n🕒 Tempo total: ");
  Serial.print(somaTempos, 2);
  Serial.println(" segundos");

  Serial.print("📊 Velocidade média geral: ");
  Serial.print(somaVelocidades / 3.0, 2);
  Serial.println(" cm/s");

  Serial.println("\n==========================================\n");
}

void loop() {
  if (digitalRead(BOTAO) == LOW && !liberado) {
    long distancia = medirDistancia();

    if (distancia > 0 && distancia < 50) {
      Serial.println("Carro detectado, Corrida Preparada");

      digitalWrite(LED_VERMELHO, LOW);
      digitalWrite(LED_AMARELO, HIGH);
      delay(3000);

      digitalWrite(LED_AMARELO, LOW);
      digitalWrite(LED_VERDE, HIGH);
      Serial.println("🏁 Valendo!");

      catraca.write(90);
      

      liberado = true;
      correndo = true;
      primeiraPassagem = true;
    } else {
      Serial.println("🚫 Nenhum carro detectado, catraca não liberada");
      delay(1000);
    }
  }

  if (digitalRead(BOTAO) == HIGH) {
    liberado = false;
  }

  int passando = medirDistancia();

  if (passando > 0 && passando <= 20 && correndo) {

    if (primeiraPassagem) {
      tempoAnterior = millis();  // Começa o cronômetro real aqui
      primeiraPassagem = false;
      Serial.println("🚦 Primeira passagem detectada.");
    } else {
      voltas++;
      Serial.print("🔁 Volta ");
      Serial.println(voltas);

      calcularTempoVolta();  // Salva tempo e velocidade da volta
    }

    delay(3000);  // Antirrepique
  }

  if (voltas == 3) {
    Serial.println("🏁 Corrida encerrada!");
    correndo = false;
    voltas = 0;
    catraca.write(0);
    mostrarResultados();
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
    delay(3000);
  }

  delay(10);
}
