/*
  Sala Inteligente — sensor de presença + aviso de intervalo
  Placa: Arduino Uno / Nano      Comunicação: USB (Serial, 115200 baud)

  Ligações:
    Sensor PIR (HC-SR501)   VCC -> 5V   GND -> GND   OUT -> D2
    Buzzer ativo            (+)   -> D8  (-)   -> GND
    LED verde (presença)    D4 -> resistor 220 ohm -> LED -> GND
    LED amarelo (intervalo) D5 -> resistor 220 ohm -> LED -> GND

  O Arduino não tem relógio próprio: o site envia a hora do computador
  assim que a conexão é aberta (comando HORA=). Se a placa for reiniciada,
  basta reconectar no site para sincronizar de novo.
  Para manter a hora sem o site, use um módulo RTC (ex.: DS3231).

  Comandos aceitos (uma linha terminada em \n):
    HORA=HH:MM:SS            acerta o relógio
    INTS=HH:MM,HH:MM,...     define os horários de intervalo (até 6; vazio apaga)
    ALARME=1 | ALARME=0      arma ou desarma o aviso automático
    BUZ=1 | BUZ=0            toca ou para o sinal (teste)
    STATUS                   pede o estado atual

  A cada segundo (e a cada comando ou mudança de presença) a placa envia
  uma linha JSON com o estado, que o site lê e mostra no painel.
*/

// ---------- Pinos ----------
const uint8_t PIN_PIR           = 2;
const uint8_t PIN_BUZZER        = 8;
const uint8_t PIN_LED_PRESENCA  = 4;
const uint8_t PIN_LED_INTERVALO = 5;

// ---------- Ajustes ----------
const uint32_t TEMPO_SEM_MOVIMENTO_S = 30;    // segundos sem movimento até a sala ser considerada vazia
const uint32_t DURACAO_ALARME_MS     = 5000;  // quanto tempo o sinal de intervalo toca
const uint32_t PERIODO_STATUS_MS     = 1000;  // intervalo entre envios de status
const uint8_t  MAX_INTERVALOS        = 6;

// ---------- Estado ----------
uint16_t intervalos[MAX_INTERVALOS];          // minutos desde 00:00
uint8_t  qtdIntervalos = 0;

bool     horaSincronizada  = false;
uint32_t segundosDoDia     = 0;
uint32_t ultimoTickRelogio = 0;
int16_t  ultimoMinutoVisto = -1;

bool     alarmeArmado = true;
bool     buzinando    = false;
uint32_t inicioBuzina = 0;

bool     presenca        = false;
bool     houveMovimento  = false;
uint32_t ultimoMovimento = 0;
uint32_t ultimoStatus    = 0;

char     linha[64];
uint8_t  tamLinha = 0;

// ---------- Setup e loop ----------
void setup() {
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_PRESENCA, OUTPUT);
  pinMode(PIN_LED_INTERVALO, OUTPUT);
  Serial.begin(115200);
  ultimoTickRelogio = millis();
}

void loop() {
  atualizarRelogio();
  lerPir();
  lerSerial();
  verificarIntervalo();
  atualizarBuzina();

  if (millis() - ultimoStatus >= PERIODO_STATUS_MS) {
    ultimoStatus = millis();
    enviarStatus();
  }
}

// ---------- Relógio ----------
void atualizarRelogio() {
  uint32_t decorrido = (millis() - ultimoTickRelogio) / 1000UL;
  if (decorrido > 0) {
    segundosDoDia = (segundosDoDia + decorrido) % 86400UL;
    ultimoTickRelogio += decorrido * 1000UL;
  }
}

// ---------- Presença ----------
// O PIR leva de 30 a 60 s para se estabilizar depois de ligado.
void lerPir() {
  bool movimento = digitalRead(PIN_PIR) == HIGH;

  if (movimento) {
    ultimoMovimento = millis();
    houveMovimento = true;
    if (!presenca) {
      presenca = true;
      digitalWrite(PIN_LED_PRESENCA, HIGH);
      enviarStatus();
    }
  } else if (presenca && (millis() - ultimoMovimento) >= TEMPO_SEM_MOVIMENTO_S * 1000UL) {
    presenca = false;
    digitalWrite(PIN_LED_PRESENCA, LOW);
    enviarStatus();
  }
}

// ---------- Intervalos ----------
void verificarIntervalo() {
  if (!horaSincronizada) return;

  int16_t minutoAtual = (int16_t)(segundosDoDia / 60UL);
  if (minutoAtual == ultimoMinutoVisto) return;   // só confere quando o minuto muda
  ultimoMinutoVisto = minutoAtual;

  if (!alarmeArmado) return;
  for (uint8_t i = 0; i < qtdIntervalos; i++) {
    if (intervalos[i] == minutoAtual) {
      iniciarBuzina();
      break;
    }
  }
}

// segundos até o próximo intervalo; -1 se não há hora ou horários
long segundosParaProximo() {
  if (!horaSincronizada || qtdIntervalos == 0) return -1;

  long menor = 86400L;
  for (uint8_t i = 0; i < qtdIntervalos; i++) {
    long diff = (long)intervalos[i] * 60L - (long)segundosDoDia;
    if (diff <= 0) diff += 86400L;
    if (diff < menor) menor = diff;
  }
  return menor;
}

void definirIntervalos(char *lista) {
  qtdIntervalos = 0;
  char *item = strtok(lista, ",");
  while (item != NULL && qtdIntervalos < MAX_INTERVALOS) {
    int h, m;
    if (sscanf(item, "%d:%d", &h, &m) == 2 && h >= 0 && h < 24 && m >= 0 && m < 60) {
      intervalos[qtdIntervalos++] = h * 60 + m;
    }
    item = strtok(NULL, ",");
  }
}

// ---------- Buzzer e LED de intervalo ----------
void iniciarBuzina() {
  buzinando = true;
  inicioBuzina = millis();
}

void pararBuzina() {
  buzinando = false;
  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED_INTERVALO, LOW);
}

void atualizarBuzina() {
  if (!buzinando) return;

  uint32_t t = millis() - inicioBuzina;
  if (t >= DURACAO_ALARME_MS) {
    pararBuzina();
    return;
  }
  bool liga = ((t / 300UL) % 2UL) == 0;     // pisca a cada 300 ms
  digitalWrite(PIN_BUZZER, liga ? HIGH : LOW);
  digitalWrite(PIN_LED_INTERVALO, liga ? HIGH : LOW);
}

// ---------- Comunicação serial ----------
void lerSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      linha[tamLinha] = '\0';
      if (tamLinha > 0) tratarComando(linha);
      tamLinha = 0;
    } else if (tamLinha < sizeof(linha) - 1) {
      linha[tamLinha++] = c;
    }
  }
}

void tratarComando(char *cmd) {
  if (strncmp(cmd, "HORA=", 5) == 0) {
    int h, m, s;
    if (sscanf(cmd + 5, "%d:%d:%d", &h, &m, &s) == 3 &&
        h >= 0 && h < 24 && m >= 0 && m < 60 && s >= 0 && s < 60) {
      segundosDoDia = (uint32_t)h * 3600UL + (uint32_t)m * 60UL + (uint32_t)s;
      ultimoTickRelogio = millis();
      ultimoMinutoVisto = (int16_t)(segundosDoDia / 60UL);
      horaSincronizada = true;
    }
  } else if (strncmp(cmd, "INTS=", 5) == 0) {
    definirIntervalos(cmd + 5);
  } else if (strncmp(cmd, "ALARME=", 7) == 0) {
    alarmeArmado = (cmd[7] == '1');
  } else if (strncmp(cmd, "BUZ=", 4) == 0) {
    if (cmd[4] == '1') iniciarBuzina(); else pararBuzina();
  }
  // "STATUS" e qualquer outro comando só provocam o envio do estado.
  enviarStatus();
}

// Exemplo de linha enviada:
// {"presenca":1,"semMov":4,"hora":34210,"prox":1190,"armado":1,"buzina":0,"ints":[570,720]}
void enviarStatus() {
  atualizarRelogio();

  Serial.print(F("{\"presenca\":"));
  Serial.print(presenca ? 1 : 0);

  Serial.print(F(",\"semMov\":"));
  if (houveMovimento) Serial.print((millis() - ultimoMovimento) / 1000UL);
  else Serial.print(-1);

  Serial.print(F(",\"hora\":"));
  if (horaSincronizada) Serial.print(segundosDoDia);
  else Serial.print(-1);

  Serial.print(F(",\"prox\":"));
  Serial.print(segundosParaProximo());

  Serial.print(F(",\"armado\":"));
  Serial.print(alarmeArmado ? 1 : 0);

  Serial.print(F(",\"buzina\":"));
  Serial.print(buzinando ? 1 : 0);

  Serial.print(F(",\"ints\":["));
  for (uint8_t i = 0; i < qtdIntervalos; i++) {
    if (i > 0) Serial.print(',');
    Serial.print(intervalos[i]);
  }
  Serial.println(F("]}"));
}
// Quando o Arduino é reiniciado, o site precisa enviar a hora de novo para sincronizar.
// Comprar os componentes para montagem do circuito: https://www.filipeflop.com/produto/sala-inteligente-sensor-de-presenca-e-aviso-de-intervalo/