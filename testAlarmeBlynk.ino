/*
Projeto Arduino - Alarme com Arduino e sensor de movimento PIR
Por Jota
versão offline
----------------------------------------
--=<| www.ComoFazerAsCoisas.com.br |>=--
----------------------------------------
*/
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
//char auth[] = "44JUWnFvrUoU8-dNOmnMpLQXvvUHxa6L";   // Alarme TCC v.1 - Mauro
char auth[] = "lFHZ56VqyC9McLDGCWeXlqN9U3aNcY4v";   // Alarme TCC v.1 - William


// --- variáveis para o Blynk
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "xaomi_mauro";
char pass[] = "1q2w3e4r";
BlynkTimer timer; // Create a Timer object called "timer"! 

//Declaração das variáveis referentes aos pinos digitais.
int pinSensorPIR = D6;    // 12;
int pinLed = D7;          //  13;
int pinOnOff = D1;        // 5;

//Declaração das variáveis referentes aos pinos virtuais Blynk
int pinBlkOnOff = V1;


//Variaveis de uso geral
int valorSensorPIR = 0;
int valorOnOff = 1;   // botão on/off local (inverso)
int valorBlkOnOff=0;  // botão on/off blynk
int valorBlkOnOff_ant=0;  // botão on/off blynk (último valor)
int alarmeAtivo = 0;
int qtDisparos = 0;
int sensorAtivado;
int valorSensorPIR2 = 0;
unsigned long tempo = 0;
unsigned long tmpLigAuto = 20000;
int online = 1;
int sireneLigada=0;
int tsOnOff_local=0;
int tsOnOff_blynk=0;



void comunicaBlynk() {
  /* // -- botão ativar/desativar alarme no Blynk
  valorBlkOnOff=V3.asInt();   // obtem valor
  if(valorBlkOnOff<>valorBlkOnOff_ant) {  // se mudou valor
    valorBlkOnOff_ant=valorBlkOnOff;
    tsOnOff_blynk=millis();   // guarda timestamp    
    ativaDesativaAlarme();
  }
  */
  // -- se alarme disparado, envia notificação para o celular
  //if(
     // -- alarme está ativo, botão blynk desligado, ultimo acionado=botão blynk

     // -- se alarme está 
}

// This function will run every time Blynk connection is established
BLYNK_CONNECTED() {
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();

  // You can also update individual virtual pins like this:
  //Blynk.syncVirtual(V0, V2);

  // Let's write your hardware uptime to Virtual Pin 2
  //int value = millis() / 1000;
  //Blynk.virtualWrite(V1, alarmeAtivo);
}




void ativaDesativaAlarme(int origem) {  // origem: 1=hardware 2=Blynk
    //se alarme esta ativo então desativa
    if (alarmeAtivo == 1){
      desativaAlarme();
      if(origem==1 && online==1)
        Blynk.virtualWrite(V1, 0);  // atualiza status no app
    }
    else {
      ativaAlarme();
      if(origem==1 && online==1)
        Blynk.virtualWrite(V1, 1);  // atualiza status no app
    }
}

BLYNK_WRITE(V1) {
  valorBlkOnOff = param.asInt(); // guarda valor obtido do pino V1 em uma variável
  Serial.println("V1: "+String(valorBlkOnOff));  
  if(valorBlkOnOff=1) {
    ativaDesativaAlarme(2);
  }
  
}

BLYNK_WRITE(V2) {
  valorBlkOnOff = param.asInt(); // guarda valor obtido do pino V1 em uma variável
  Serial.println("V2: "+String(valorBlkOnOff));  
  if(valorBlkOnOff=1) {
    ativaDesativaAlarme(2);
  }
  
}

void ativaAlarme() {
      Serial.println("Ativando");  
      digitalWrite(pinLed, HIGH); //liga led de alarme ativo
      alarmeAtivo = 1; //ativa alarme
}

void desativaAlarme() {
      Serial.println("Desativando");  
       desligarAlarme();
      tempo = 0;
      tempo = millis();
      digitalWrite(pinLed, LOW);  // desliga led de alarme ativo
      alarmeAtivo=0; //desativa alarme
      qtDisparos = 0;
}

void disparaAlarme(int setor) {
  //Ligando o led
//  digitalWrite(pinLed, HIGH);
   
  //Ligando o buzzer com uma frequencia de 1500 hz.
  //tone(pinBuzzer,1500);
  //delay(4000); //tempo que o led fica acesso e o buzzer toca

  // --- ativa sirene
  if(sireneLigada==0) {
    sireneLigada=1;
    // digitalWrite(pinBuzzer, HIGH);
    if(online=1)
        Blynk.notify("Setor "+String(setor)+" violado!!! Sirene Ligada!");
  }  
}

void desligarAlarme() {
  //Desligando o led
//  digitalWrite(pinLed, LOW);
   
  //Desligando o buzzer
  //noTone(pinBuzzer);

  // -- desativa sirene
  // digitalWrite(pinBuzzer, LOW);
  sireneLigada=0;
}

void setup() {
  Serial.begin(9600); //Inicializando o serial monitor

   //Definido pinos como de entrada ou de saída
  pinMode(pinSensorPIR,INPUT_PULLUP);
  pinMode(pinLed,OUTPUT);
  pinMode(pinOnOff,INPUT_PULLUP);
   tempo = millis();

  if(online==1) {
    Blynk.begin(auth, ssid, pass);
    // You can also specify server:
    //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
    //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
    timer.setInterval(1000L, comunicaBlynk); //  Here you set interval (1sec) and which function to call 
  }
   
}

void loop() {  

  if(online==1) {
      Blynk.run();
      timer.run();
  }

  int presenca=0;

  //Lê o estado do botão de armar/desarmar
  valorOnOff = digitalRead(pinOnOff);
  // se botão foi pressionado 
  if (valorOnOff == 0) {
    tsOnOff_local=millis();

    //se alarme esta ativo então desativa
    ativaDesativaAlarme(1);
    delay(500);  
  }

  //

  //Lendo o valor do sensor PIR. Este sensor pode assumir 2 valores
  //1 quando detecta algum movimento e 0 quando não detecta.
  valorSensorPIR = digitalRead(pinSensorPIR);
    
  Serial.print("PIR: ");  
  Serial.print(valorSensorPIR);
  Serial.print("  tempo:");  
  Serial.print(tempo);
  Serial.print("  milis:");  
  Serial.println(millis());
   
  //Verificando se ocorreu detecção de movimentos
  presenca=valorSensorPIR == 1;
  if(presenca>0)    // se houve presença
    tempo=millis();   // atualiza marcação de tempo
  if (alarmeAtivo==1 && presenca==1 && sireneLigada==0) { 
    Serial.println("-------- DISPARO");

    if(valorSensorPIR == 1)
      disparaAlarme(1);
     
   
 // } else {
    //desligarAlarme();
  }    
  // se o alarme desativado por mais de X segundos sem movimento, entao ativa
  // OBS: falta tratar quando o millis() virar (zerar)
   if( millis()-tempo > tmpLigAuto && alarmeAtivo == 0 && presenca==0 ){
    ativaAlarme();
   }
//   delay(1000);
}
