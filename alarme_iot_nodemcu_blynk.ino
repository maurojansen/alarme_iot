/*
 * Projeto: Alarme IOT de 2 setores com NodeMCU, sensores PIR e Blynk
 * Criação: 16/12/2019 - Mauro Jansen, William, Dionilson
 * Projeto de TCC do curso de Eletrotécnica Subsequente
 * Instituto Federal de Educação, Ciencia e Tecnologia do MA - IFMA - Campus Alcântara
 * Histórico de alterações:
 16/12/2019 - Mauro Jansen 
 - documentei e organizei o código
 - nome do wifi padrão = hotspot_iot e senha = 12345678
 - acrescentei campainha no pino D3 (GPIO 0) e envio de notificação de campainha acionada
 - envio de notificações de alarme ativado/desativado
 - ativei função pânico com botão V2
 - adicionei um LED no app para indicar se alarme está disparado
 
*/
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Você deve obter seu Token de autenticação no aplicativo Blynk
// Vá até a opção de Configurações do Projeto (nut icon).
//char auth[] = "44JUWnFvrUoU8-dNOmnMpLQXvvUHxa6L";   // Alarme TCC v.1 - Mauro
//char auth[] = "lFHZ56VqyC9McLDGCWeXlqN9U3aNcY4v";   // Alarme TCC v.1 - William
char auth[] = "yT8em9Sq67DDpf_njB04gpPf4WnlHFrp";   // Alarme TCC v.1 - tcc.eletrotecnica@gmail.com / 123456

// --- variáveis para o Blynk
// Coloque aqui os dados para conexão à rede WiFi
// Para redes abertas, coloque a senha como ""
//char ssid[] = "hostspot_iot";
//char pass[] = "12345678";
char ssid[] = "SMASTER";
char pass[] = "ninamimi2019";
//BlynkTimer timer; // Create a Timer object called "timer"! 

//--- Declaração das variáveis referentes aos pinos digitais do NodeMCU
int pinSirene = D2;       // GPIO  4 
int pinOnOff = D1;        // GPIO  5
int pinSetor1 = D6;       // GPIO 12
int pinSetor2 = D5;       // GPIO 14
int pinLed = D7;          // GPIO 13
int pinCampainha = D3;    // GPIO  0
//--- portas livres:
//int pinXXX = D4;        // GPIO  2

//--- Declaração das variáveis referentes aos pinos virtuais Blynk
int pinBlkOnOff = V1;

//--- Variaveis de uso geral
int valorSetor1 = 0;
int valorSetor2 = 0;
int valorOnOff = 1;     // botão on/off local (inverso)
int valorVP=0;    // valor obtido de pino virtual blynk
int alarmeAtivo = 0;
int qtDisparos = 0;
int sensorAtivado;
unsigned long tempo = 0;      // variável unsigned long porque milis retorna número grande
unsigned long tmpLigAuto = 20000;
int online = 1;
int sireneLigada=0;

/*
 * Função comunicaBlynk, a ser executada de acordo com um timer para evitar desconexão
 * Implementar esta função se precisar de alguma comunicação específica com o Blynk
 */
/*void comunicaBlynk() {

}
*/

/*
 * Esta função será executada cada vez que a conexão com o Blynk for estabelecida
 */
BLYNK_CONNECTED() {
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();

  // You can also update individual virtual pins like this:
  //Blynk.syncVirtual(V0, V2);

  // Let's write your hardware uptime to Virtual Pin 2
  //int value = millis() / 1000;
  //Blynk.virtualWrite(V1, alarmeAtivo);
}


/*
 * Função para ativar ou desativar alarme
 * Parâmetro: origem (1=hardware 2=Blynk)
 */
void ativaDesativaAlarme(int origem) {
    // se alarme esta ativo, então desativa
    if (alarmeAtivo == 1) {
      desativaAlarme(origem);
    }// senão (está inativo), ativa
    else {
      ativaAlarme(origem);
    }
}


/*
 * Função padrão BLYNK_WRITE(X) - é acionada quando um pino virtual muda de valor no aplicativo Blynk 
 * Pino virtual V1: Ativa/desativa o alarme
 */
BLYNK_WRITE(V1) {
  valorVP = param.asInt(); // guarda valor obtido do pino V1 em uma variável
  Serial.println("V1: "+String(valorVP));  
  if(valorVP==1)
    ativaAlarme(2);   // executa função local para ativar ou desativar alarme
  else
    desativaAlarme(2);
}


/*
 * Função padrão BLYNK_WRITE(X) - é acionada quando um pino virtual muda de valor no aplicativo Blynk 
 * Pino virtual V2: Pânico
 */
BLYNK_WRITE(V2) {
  valorVP = param.asInt(); // guarda valor obtido do pino V2 em uma variável
  Serial.println("V2: "+String(valorVP));  
  if(valorVP==1)
    disparaAlarme(0);
  else
    desligarSirene();  
}

/*
 * Função ativaAlarme - ativa o alarme 
 */
void ativaAlarme(int origem) {
  Serial.println("Ativando");  
  digitalWrite(pinLed, HIGH); //liga led de alarme ativo
  alarmeAtivo = 1; //ativa alarme
  if(online==1) {  // se alarme está online, envia notificação para o app
    Blynk.notify("Alarme ativado!");
    if(origem==1)    // se o comando veio do hardware
      Blynk.virtualWrite(V1, alarmeAtivo);  // atualiza status no app com o status do alarme
  }
}

/*
 * Função desativaAlarme - desativa o alarme 
 */
void desativaAlarme(int origem) {
      Serial.println("Desativando");  
       desligarSirene();
      tempo = 0;
      tempo = millis();
      digitalWrite(pinLed, LOW);  // desliga led de alarme ativo
      alarmeAtivo=0; //desativa alarme
      qtDisparos = 0;
  if(online==1) {  // se alarme está online, envia notificação para o app
    Blynk.notify("Alarme desativado!");
    if(origem==1)    // se o comando veio do hardware
      Blynk.virtualWrite(V1, alarmeAtivo);  // atualiza status no app com o status do alarme
  }
}


/*
 * Função disparaAlarme - dispara o alarme, ligando a sirene
 * Parâmetro: número do setor que foi violado ou 0=pânico
 */
void disparaAlarme(int setor) {
  //Ligando o buzzer com uma frequencia de 1500 hz.
  //tone(pinSirene,1500);
  //delay(4000); //tempo que o led fica acesso e o buzzer toca

  // --- ativa sirene
  if(sireneLigada==0) {
    sireneLigada=1;
    digitalWrite(pinSirene, HIGH);
    if(online==1) {  // se alarme está online, envia notificação para o app
      if(setor>0)
        Blynk.notify("Setor "+String(setor)+" violado!!! Sirene Ligada!");
      else
        Blynk.notify("Função pânico acionada!");
      delay(1000);
      Blynk.virtualWrite(V3, 255);  // atualiza LED no app com o status da sirene
    }    
  }  
}


/*
 * Função desligarSirene - desliga a sirene do alarme
 */
void desligarSirene() {
  //Desligando o buzzer
  //noTone(pinSirene);

  // -- desativa sirene
  digitalWrite(pinSirene, LOW);
  sireneLigada=0;
  Blynk.virtualWrite(V3, 0);  // atualiza LED no app com o status da sirene
}


/*
 * Função setup - configuração de portas
 */
void setup() {
  Serial.begin(9600); //Inicializando o serial monitor

  //-- Definindo pinos de entrada ou de saída
  pinMode(pinSirene,OUTPUT);
  pinMode(pinSetor1,INPUT_PULLUP);  // 0=setor fechado não violado(estado normal), 1=setor aberto (pullup)
  pinMode(pinSetor2,INPUT_PULLUP);  // 0=setor fechado não violado(estado normal), 1=setor aberto (pullup)
  pinMode(pinLed,OUTPUT);
  pinMode(pinOnOff,INPUT_PULLUP);   // 0=acionado, 1=desligado
  pinMode(pinCampainha,INPUT_PULLUP);   // 0=acionado, 1=desligado
  tempo = millis();

  //-- se estiver online, inicia e conecta com o Blynk
  if(online==1) {
    Blynk.begin(auth, ssid, pass);
    // You can also specify server:
    //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
    //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);
    //timer.setInterval(1000L, comunicaBlynk); //  Here you set interval (1sec) and which function to call 
  }

  desativaAlarme(1);  // inicia com alarme desativado localmente
   
}

void loop() {  

  if(online==1) {
      Blynk.run();
      //timer.run();
  }

  int presenca=0;

  //-- Lê o estado do botão de armar/desarmar
  valorOnOff = digitalRead(pinOnOff);
  // se botão foi pressionado 
  if(valorOnOff == 0) {
    Serial.println("botão pressionado");  
    ativaDesativaAlarme(1);   // ativa ou desativa o alarme
    delay(500);   // delay 0,5s p/ evitar leitura repetida do pressionamento do botão
  }

  //Lendo o valor do sensor PIR. Este sensor pode assumir 2 valores
  //1 quando detecta algum movimento e 0 quando não detecta.
  valorSetor1 = digitalRead(pinSetor1);
  valorSetor2 = digitalRead(pinSetor2);

  if(valorOnOff== 0 || valorSetor1>0 || valorSetor2>0 || sireneLigada>0) {
    Serial.print("CT: ");
    Serial.print(valorOnOff);
    Serial.print("  S1: ");
    Serial.print(valorSetor1);
    Serial.print("  S2: ");  
    Serial.print(valorSetor2);
    Serial.print("  Sirene: ");  
    Serial.println(sireneLigada);
  }

  //-- Lê o estado do botão da campainha
  valorOnOff = digitalRead(pinCampainha);
  // se campainha foi acionada
  if(valorOnOff == 0 && online==1) {
    Serial.println("campainha acionada");  
    Blynk.notify("Campainha acionada!");
    delay(500);   // delay 0,5s p/ evitar leitura repetida do pressionamento do botão
  }
   
  //Verificando se ocorreu detecção de movimentos
  presenca=(valorSetor1 == 1  || valorSetor2 == 1);
  if(presenca>0)    // se houve presença
    tempo=millis();   // atualiza marcação de tempo
  if (alarmeAtivo==1 && presenca==1 && sireneLigada==0) { 
    Serial.println("-------- DISPARO");
    disparaAlarme(1);
  }    
  // se o alarme desativado por mais de X segundos sem movimento, entao ativa
  // OBS: falta tratar quando o millis() virar (zerar)
   if( millis()-tempo > tmpLigAuto && alarmeAtivo == 0 && presenca==0 ){
    Serial.print("  tempo:");  
    Serial.print(tempo);
    Serial.print("  milis:");  
    Serial.println(millis());
    ativaDesativaAlarme(1);
   }
//   delay(1000);
}
