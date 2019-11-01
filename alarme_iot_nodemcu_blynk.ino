/*
Projeto Arduino - Alarme com Arduino e sensor de movimento PIR
Por Jota
----------------------------------------
--=<| www.ComoFazerAsCoisas.com.br |>=--
----------------------------------------
*/
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "44JUWnFvrUoU8-dNOmnMpLQXvvUHxa6L";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "xaomi_mauro";
char pass[] = "1q2w3e4r";

//Declaração das variáveis referentes aos pinos digitais.
int pinBuzzer = D2;       // 4; 
int pinSensorPIR = D6;    // 12;
int pinLed = D7;          //  13;
int pinOnOff = D1;        // 5;
int pinSensorPIR2 = D5;   // 14;

//Variaveis de uso geral
int valorSensorPIR = 0;
int valorOnOff = 1;
int alarmeAtivo = 0;
int qtDisparos = 0;
int sensorAtivado;
int valorSensorPIR2 = 0;
unsigned long tempo = 0;
unsigned long tmpLigAuto = 20000;

void setup() {
  Serial.begin(9600); //Inicializando o serial monitor

   //Definido pinos como de entrada ou de saída
  pinMode(pinBuzzer,OUTPUT);
  pinMode(pinSensorPIR,INPUT_PULLUP);
  pinMode(pinLed,OUTPUT);
  pinMode(pinOnOff,INPUT_PULLUP);
  pinMode(pinSensorPIR2,INPUT_PULLUP);
   tempo = millis();

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

   
}
 
void loop() {  
    Blynk.run();

  int presenca=0;

  //Lê o estado do botão de armar/desarmar
  valorOnOff = digitalRead(pinOnOff);
  // se botão foi pressionado 
  if (valorOnOff == 0) {
    //se alarme esta ativo então desativa
    if (alarmeAtivo == 1){
      desativaAlarme();
    }
    else {
      ativaAlarme();
    }
    delay (500);  
  }

  //Lendo o valor do sensor PIR. Este sensor pode assumir 2 valores
  //1 quando detecta algum movimento e 0 quando não detecta.
  valorSensorPIR = digitalRead(pinSensorPIR);
    valorSensorPIR2 = digitalRead(pinSensorPIR2);
    
  Serial.print("PIR: ");  
  Serial.print(valorSensorPIR);
  Serial.print("  PIR2: ");  
  Serial.print(valorSensorPIR2);
  Serial.print("  tempo:");  
  Serial.print(tempo);
  Serial.print("  milis:");  
  Serial.println(millis());
   
  //Verificando se ocorreu detecção de movimentos
  presenca=(valorSensorPIR == 1  || valorSensorPIR2 == 1);
  if(presenca>0)    // se houve presença
    tempo=millis();   // atualiza marcação de tempo
  if (alarmeAtivo==1 && presenca==1) { 

    disparaAlarme();
    
    
    if(valorSensorPIR == 1)
        Blynk.notify("Setor 1 violado!!!");
    if(valorSensorPIR2 == 1)
        Blynk.notify("Setor 2 violado!!!");

    
    qtDisparos = qtDisparos + 1;
    Serial.println(qtDisparos);
 // } else {
    //desligarAlarme();
  }    
  // se o alarme desativado por mais de X segundos sem movimento, entao ativa
  // OBS: falta tratar quando o millis() virar (zerar)
   if ( millis()-tempo > tmpLigAuto && alarmeAtivo == 0 && presenca==0 ){
    ativaAlarme();
   }
//   delay(1000);
}

void ativaAlarme() {
      digitalWrite(pinLed, HIGH); //liga led de alarme ativo
      alarmeAtivo = 1; //ativa alarme
}

void desativaAlarme() {
       desligarAlarme();
      tempo = 0;
      tempo = millis();
      digitalWrite(pinLed, LOW);  //desliga led de alarme ativo
      alarmeAtivo=0; //desativa alarme
      qtDisparos = 0;
}

void disparaAlarme() {
  //Ligando o led
//  digitalWrite(pinLed, HIGH);
   
  //Ligando o buzzer com uma frequencia de 1500 hz.
  tone(pinBuzzer,1500);
  //delay(4000); //tempo que o led fica acesso e o buzzer toca
   
  
}

void desligarAlarme() {
  //Desligando o led
//  digitalWrite(pinLed, LOW);
   
  //Desligando o buzzer
  noTone(pinBuzzer);
}
