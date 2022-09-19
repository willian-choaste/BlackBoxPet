/*-------- Definições de acesso App Blynk ----------- */
#define BLYNK_TEMPLATE_ID "TMPLIFh0Jm-U"                    // ID Template para acesso App Blynk
#define BLYNK_DEVICE_NAME "Quickstart Template"             // Device name para acesso App Blynk
#define BLYNK_AUTH_TOKEN "XfkmJ5DEFC6b5FFQEN0DWU0pWkDiav2b" // Token de autenticação para acesso App Blynk
#define BLYNK_PRINT Serial

/*-------- Bibliotecas ----------- */
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <NTPClient.h> 

char auth[] = BLYNK_AUTH_TOKEN;

/*-------- Configurações rede Wi-fi ----------- */
char ssdi[] = "rede";
char pass[] = "senha";

/* -------- Configurações de relógio on-line ----------- */
WiFiUDP udp;
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000); // Cria um objeto "NTP" com as configurações.utilizada no Brasil
String horaAtual;                                     // Variável que armazena o horario atual
String horaProgramada1 = "08:00:00";                  // Definição primeiro horario de alimentação
String horaProgramada2 = "16:33:00";                  // Definição segundo horario de alimentação

/* -------- Declaração Portas Digitais ----------- */
#define pinServo                13 // Porta Digital gpio13
#define sensorNivelMin          27 // Porta Digital gpio27
#define sensorNivelMed          26 // Porta Digital gpio26
#define sensorNivelMax          25 // Porta Digital gpio25
#define chaveAutoManual         35 // Porta Digital gpio35
#define ligaManual              34 // Porta Digital gpio34
#define sinalBotaoAutoManualApp 18 // Porta Digital gpio18
#define sinalBotaoManualApp     19 // Porta Digital gpio19

/* -------- Declaração Variavéis Globais ----------- */
Servo servo1;                         // Variável Servo
int angulo                   = 120;   // Posição Servo
bool flagFaltaRacao          = false; // Inicializa está variavel com valor false
bool flagAtivaModoManual     = false; // Inicializa está variavel com valor false
bool flagStopServo           = false; // Inicializa está variavel com valor false
bool flagAtivaModoManualApp  = false; // Inicializa está variavel com valor false
bool flagHabilitaAlimentacao = false; // Inicializa está variavel com valor false

// Funções para leitura pinos virtuiasi App Blynk
BLYNK_WRITE(V0){
  int pinValue = param.asInt(); // atribui o valor de entrada do pino V1 a uma variável
  digitalWrite(sinalBotaoAutoManualApp, pinValue);
}

BLYNK_WRITE(V1){
  int pinValue = param.asInt(); // atribui o valor de entrada do pino V2 a uma variável
  digitalWrite(sinalBotaoManualApp, pinValue);
}

// Configuração Setup inicial
void setup() {

  Serial.begin(115200);                     // Abre a porta serial a 115200 bps
  servo1.attach(pinServo);                  // Defini pinServo como pino de controle para servo1 
  Blynk.begin(auth, ssdi, pass);            // Realiza conexão com a internet e acesso ao App Blynk
  ntp.begin();                              // Inicia o protocolo relogio
  ntp.forceUpdate();                        // Atualização relogio
  pinMode(sensorNivelMin, INPUT_PULLDOWN);  // Defini este pino como INPUT_PULLDOWN
  pinMode(sensorNivelMed, INPUT_PULLDOWN);  // Defini este pino como INPUT_PULLDOWN
  pinMode(sensorNivelMax, INPUT_PULLDOWN);  // Defini este pino como INPUT_PULLDOWN
  pinMode(chaveAutoManual, INPUT);          // Defini este pino como INPUT
  pinMode(ligaManual, INPUT);               // Defini este pino como INPUT
  pinMode(sinalBotaoManualApp, OUTPUT);     // Defini este pino como OUTPUT
  pinMode(sinalBotaoAutoManualApp, OUTPUT); // Defini este pino como OUTPUT
}

// Função rotina de execução servo motor
void rotinaServo(){
    Serial.print("\n---- Inicio ----");

    angulo = 45;          // Defini a variavel angulo com valor igual a 45º.
    servo1.write(angulo); // Comanda servo1 para posição com angulo igual a 45º
    Serial.print("\nangulo:");
    Serial.println(angulo);
    delay(5000);

    angulo = 120;         // Defini a variavel angulo com valor igual a 45º.
    servo1.write(angulo); // Comanda servo1 para posição com angulo igual a 120º.
    Serial.print("angulo:");
    Serial.println(angulo);
    delay(1000);

    Serial.print("---- Fim ----\n");
    delay(2000);
}

// Função de verificação nível de ração
void nivelRacao(){
  
  bool statusSensorNivelMin = digitalRead(sensorNivelMin);
  bool statusSensorNivelMed = digitalRead(sensorNivelMed);
  bool statusSensorNivelMax = digitalRead(sensorNivelMax);
  
  if(statusSensorNivelMax == HIGH && statusSensorNivelMed == HIGH){ // Verifica se sensor nivel maximo e sensor nivel medio estão atuados
    Serial.print("Nivel Maximo \n"); // Exibe mensangem no monitor serial
    Blynk.virtualWrite(V4, 100);     // Escreve o valor 100 no pino virtual V4
    flagFaltaRacao = false;          // Defini o valor como falso para a variavel flagFaltaRacao
    delay(1000);
  }
  else if(statusSensorNivelMax == LOW && statusSensorNivelMed == HIGH){ // Verifica se sensor nivel maximo esta desatuado e sensor nivel medio atuado
   	Serial.print("Nivel Medio \n"); // Exibe mensangem no monitor serial
    Blynk.virtualWrite(V4, 65);     // Escreve o valor 65 no pino virtual V4
    flagFaltaRacao = false;         // Defini o valor como falso para a variavel flagFaltaRacao
    delay(1000);
  } 
  else if(statusSensorNivelMed == LOW && statusSensorNivelMin == HIGH){ // Verifica se sensor nivel meido esta desatuado e sensor nivel minimo atuado
   	Serial.print("Nivel Minimo \n"); // Exibe mensangem no monitor serial
    Blynk.virtualWrite(V4,30);       // Escreve o valor 30 no pino virtual V4
    flagFaltaRacao = false;          // Defini o valor como falso para a variavel flagFaltaRacao
    delay(100);
  }   
  else{
    Serial.print("Falta racao \n"); // Exibe mensangem no monitor serial
    Blynk.virtualWrite(V4, 0);      // Escreve o valor 0 no pino virtual V4
    flagFaltaRacao = true;          // Defini o valor como verdadeiro para a variavel flagFaltaRacao
    delay(1000);    
  }
}

// Função de operação em modo manual fisico
void modoManualFisico(){

  int statusBotaoLigaManual = digitalRead(ligaManual); 

  if(statusBotaoLigaManual == HIGH && flagFaltaRacao == false){ // Verifica se o botão de acionamento manual esta atuado e se varivel flagFaltaRacao esta com status falso    
    Serial.print("Manual em curso...\n");
    delay(1000);      
    rotinaServo(); // Executa a função rotina servo           
  }
  else if(flagAtivaModoManual == false){    
    Serial.print("Manual desabilitado\n");
    delay(1000);
  }  
}

// Função de operação em modo manual via App Blynk
void modoManualAppBlynk(){

  int statusBotaoLigaManualApp = digitalRead(sinalBotaoManualApp);
 
  if(statusBotaoLigaManualApp == HIGH && flagFaltaRacao == false){
    Serial.print("Manual via APP em curso...\n");

    flagHabilitaAlimentacao = true; // Atribui valor verdadeiro para a variavel flagHabilitaAlimentacao 

    rotinaServo();                  // Executa a função rotina servo

    Blynk.virtualWrite(V0, 0);                  // Atribui valor zero para o pino virtual V0 
    digitalWrite(sinalBotaoAutoManualApp, LOW); // Atribui valor LOW para o pino sinalBotaoAutoManualApp 

    Blynk.virtualWrite(V1, 0);              // Atribui valor zero para o pino virtual V1    
    digitalWrite(sinalBotaoManualApp, LOW); // Atribui valor LOW para o pino sinalBotaoManualApp

    flagAtivaModoManualApp = false; // Atribui valor falso para a variavel flagAtivaModoManualApp  
  }
  else if(flagAtivaModoManualApp == false){    
    Serial.print("Manual via APP desabilitado\n");
    delay(1000);
  } 
}

// Definição de operação em modo automatico
void modoAutomatico(){
  if(horaAtual == horaProgramada1 || horaAtual == horaProgramada2){ // Verifica se a hora atual é igual a hora programada 1 ou 2
    rotinaServo(); // Executa a função rotina servo
    Serial.print("Alimentação efetuada!!!\n");
    delay(1000);
  }
}

//Definição modo de funcionamento
void modoFuncionamento(){
  
  nivelRacao();
 
  int statusChaveAutoManual = digitalRead(chaveAutoManual);
  int statusFuncionamentoBlynk = digitalRead(sinalBotaoAutoManualApp);

  horaAtual = ntp.getFormattedTime();

  Serial.printf("Horario: %s\n", horaAtual);
  delay(100);
  
  if(statusChaveAutoManual == HIGH){ // Verifica se chave para comando manual fisico esta atuada
    Serial.print("MODO MANUAL \n");
    delay(1000);
    modoManualFisico();
  }
  else if(statusFuncionamentoBlynk == HIGH){ // Verifica se o botão para comando manual via App Blynk esta atuado
    Serial.print("MODO MANUAL APP BLYNK \n");
    delay(1000);
    modoManualAppBlynk();
  }  
  else{ // Executa modo automatico
    Serial.print("MODO AUTOMATICO\n"); 
    delay(100);    
    modoAutomatico();           
  } 
}

// Loop de execução do programa
void loop() {
  Blynk.run();         // Gerencia o funcionamento com a aplicação blynk
  modoFuncionamento(); // Executa função Modo de funcionamento
}

