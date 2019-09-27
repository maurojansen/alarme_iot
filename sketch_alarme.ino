/*
Projeto Arduino - Alarme com Arduino e sensor de movimento PIR
Por Jota
----------------------------------------
--=<| www.ComoFazerAsCoisas.com.br |>=--
----------------------------------------
*/
 
//Declaração das variáveis referentes aos pinos digitais.
int pinBuzzer = 7;
int pinSensorPIR = 8;
int pinLed = 9;
int pinOnOff = 5;
int pinSensorPIR2 = 6;

//Variaveis de uso geral
int valorSensorPIR = 0;
int valorOnOff = 1;
int alarmeAtivo = 0;
int qtDisparos = 0;
int sensorAtivado;
int valorSensorPIR2 = 0;

void setup() {
  Serial.begin(9600); //Inicializando o serial monitor
 
  //Definido pinos como de entrada ou de saída
  pinMode(pinBuzzer,OUTPUT);
  pinMode(pinSensorPIR,INPUT);
  pinMode(pinLed,OUTPUT);
  pinMode(pinOnOff,INPUT_PULLUP);
  pinMode(pinSensorPIR2,INPUT_PULLUP);
}
 
void loop() {  

  //Lê o estado do botão de armar/desarmar
  valorOnOff = digitalRead(pinOnOff);
  // se botão foi pressionado 
  if (valorOnOff == 0) {
    //se alarme esta ativo então desativa
    if (alarmeAtivo == 1){
      desligarAlarme();
      digitalWrite(pinLed, LOW);  //desliga led de alarme ativo
      alarmeAtivo=0; //desativa alarme
      qtDisparos = 0;
    }
    else {
      digitalWrite(pinLed, HIGH); //liga led de alarme ativo
      alarmeAtivo = 1; //ativa alarme
    }
    delay (500);  
  }

  //Lendo o valor do sensor PIR. Este sensor pode assumir 2 valores
  //1 quando detecta algum movimento e 0 quando não detecta.
  valorSensorPIR = digitalRead(pinSensorPIR);
    valorSensorPIR2 = digitalRead(pinSensorPIR2);
    
  Serial.print("Valor do Sensor PIR: ");  
  Serial.print(valorSensorPIR);
  Serial.print("Valor do Sensor PIR2: ");  
  Serial.println(valorSensorPIR2);
   
  //Verificando se ocorreu detecção de movimentos
  //if (alarmeAtivo == 1 && ( valorSensorPIR == 1  || valorSensorPIR2 == 1) ) { 
  if (alarmeAtivo == 1 &&  valorSensorPIR == 1  ) { 
    ligarAlarme();
    
    qtDisparos = qtDisparos + 1;
    Serial.println(qtDisparos);
 // } else {
    //desligarAlarme();
  }    

}
 
void ligarAlarme() {
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
