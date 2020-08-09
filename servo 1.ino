/*
      ver. MFVC-01 - versão firmeware 4
      Cesar Costa  *-|-*      Julho 2020
      Implantação função em C para funcionamento dos CI's:
      LCD-020N004L  www.vishay.com Vishay -20 x 4 Character LCD
      HX711 -  Conversor AD para célula de carga
      A4988- Drive para motor de passo  
      MPU-6050 - Modulo giroscopio
      Micro switch e LED - Botões para acionar as funções.
      //____Premissas___\\
      Medidor da força de vinco e armação do cartucho: 
      Para medir o vinco estou usando o motor 1 que dobra o vinco inclinando a balança e mede a força de resistencia.
      Para medir a armação do cartucho vou usar o motor 2 para suspender uma plataforma que vai forçar armação contra a balança deitada 
      Para austar o tamanho do cartucho com a balança vou usar o motor 3 regulando altura entre base e balaça
      //____Premissas___\\ end
      
      #### proximas tarefas###
      #13/07/20 - incluir modulo giroscopio ao lado da celula de carga para controlar posição da balança
            giroscopio vai concertar ou pelo menos certificar dos passos terem ocoridos criando erro se a leitura for diferente
      #Trocar servo motor por motor de passo (criar nova Classe para objeto  Sugiro "C_servo();",
            o mesmo pode evoluir de passos para servo)
            Incluir as variaveis com limite de passos; limite de velocidade; direção; fins de cursos
      #para proxima versão corrigir display automatico  distancia entre  HOLD MAX = DE DEMAIS     
*/
//==========================================================================================================================================================
// --- Mapeamento de Hardware ---
// PIN 0 AND 1 SERIAL DATA  //desativado serial
#include <LiquidCrystal.h>  // lcd(12, 11, 5, 4, 3, 2); //pinos do LCD 12=4-RS , 11=6-E , 5=11-D4 , 4= 12-D5 , 3=13-D6 , 2=14-D7
#include <EEPROM.h>         // escrever as taras para recuperar dados
#define  M1_LADO 9       // Define pino 9 como lado do drive motor 
#define  M1_PASSO 10     // Define pino 10 como passo para drive motor
#define  M1_VOLTA 1600        // mumero de passos por revolução 360º  , no programa já vai dividir por 4 para a balança
#define  ADDO  7            //Data Out CI HX711
#define  ADSK  6            //SCK   CI HX711
#define  LED  13            //Led indicador power on
#define  ATUADOR 8           //acionador do rele do atuador   "trocado julho de pino 9 para pino 8"
#define  TRAVA  A0          //laranja- botão swith pull down
#define  MENU  A1           //amarelo- botão swith pull down
#define  ESQUERDA A2        //verde- botão swith pull down
#define  DIREITA  A3        //Azul- botão swith pull down
#define  SAIR  A4           //Roxo- botão swith pull down
#define  FIM_CURSO  A5       // fim de curso do M1

//==========================================================================================================================================================
// --- Protótipo das Funções Auxiliares ---

void Motor1();                // movimentar motor de acordo com a direção, velocidade e diferença de M1possN M1possA -
void Medir();               //função para ler e imprimir os valores encontrados e acender led se 0
void LerMedia();            // calcula a x millis medida e posição
void LerAtual();            //calcula valor atual de angulo e peso
void ExibirLCD();           //exibi no LCD para as Funções LerMedia e LerAtual
void EEPROMWriteLong(int address, long value); //4 Bytes, grava o valor long na memoria
unsigned long EEPROMReadLong(int address); //4 Bytes, faz leitura do endereço de memoria e retorna o valor em unsigned long
unsigned long ReadCount();  //conversão AD do HX711
void Zerar1();              // zera no ponto inicial em cima
void Zerar2();              // identifica o valor no ponto dois para compesar com a Função F(X)=sin( X "angulo inclinação")* zerar2 "variavel encontrada"
void Trava();               // Trava ou destrava o atuador segura o papel
void MoveLivre();           //função para movimentar motor1
void GramaNewton();         // função para gravar na memoria fixa
int Teclas();              // ler as teclas no loop
void keyboard_menu();
void menu1();
void menu2();
void menu3();
void menu4();
//==========================================================================================================================================================
// --- Variáveis Globais ---
#define   menu_max   4
unsigned long convert=0;                      //  variavel recebida do HX711
unsigned long previousMillis = 0;           //  will store last time LED was updated HX711
unsigned long M1_previousMillis = 0;            // ultimo passo do motor 1
const long interval = 20;                   //  original 100 constants won't change:
long varB = 8364550;                        //  variavel B da equação "F(x)=a*x+b" = Tara zero
int varA = 901;                 //  variavel A da equação aplicando "varB-convert/varA" 0,001111 unidade de grama901= n
long grama = 0;                            //  VARIAVEL PARA TER PESO DA BALANÇA ATUALIADO
int gramaMax = 0;                         // peso maximo registrado
float gramaMaxNewton = 0;
int maxima = 0;                             // angulo do peso maximo
int varGrama = 0;                         //  variavel compensando inclinação da balança
float varNewton = 0;  
unsigned long media = 0;                             //
int contagem = 0;                           //
int unidadeM;                             // 1= GRAMA  0 = NEWTON
unsigned long currentMillis = 0;            //
bool zerar1 = LOW;                          //  registrador de execução 1 posição
long zerar2 = 1;                           //  registrador de execução 2 posição
int ponto1 = 0;                             //  definição do 1º ponto  , inicio da medida
int ponto2 = 90;                             //  definição do 2º ponto  , final da medida
int val;
int valD = 0;
int M1possN = 0;                                //  varievel de nova posição para motor1
int M1possA = 0;                                   //  variavel para posição atual do motor1
int M1_RPM =2;                                  // VELOCIDADE DO MOTOR 1 DE ROTAÇÃO POR MINUTO
unsigned long M1_tempo = (M1_VOLTA*M1_RPM)/60000;            // VELOCIDADE DO MOTOR 1 NUMERO DE PASSOS p/ REVOLUÇÃO *RPM / 60000 milisegundos
bool dr;                                    //  registrador do botão direito
bool es;                                    //  registrador do bottão esquerdo
// menus
int QTDEdeMenus = 3;            //  numero de opções no menu principal
int menuPricipal = 1;           //  posição atual do menu principal
bool sair;
int teclas = 0;
int menu_num = 1;
int sub_menu = 1;
unsigned long  tempo;

// ---Variáveis alocação de Memoria ---
int  M_varB =10 ;         //define o ponto de memmoria para salvar variavel long
int  M_zerar1 =20;        //define o ponto de memmoria para salvar variavel bit 
int  M_zerar2 =30 ;       //define o ponto de memmoria para salvar variavel long
int M_unidadeM=40;

LiquidCrystal lcd(12, 11, 62, 63, 64, 65, 5, 4, 3, 2); //pinos do LCD 12=4-RS , 11=6-E , 5=11-D4 , 4= 12-D5 , 3=13-D6 , 2=14-D7
bool atuador;                          // CONTROLE DO ATUADOR
//==========================================================================================================================================================
// --- Configurações Iniciais ---
void setup()
{
  lcd.begin(20, 4);                 //  iniciar modulo de LCD 
  pinMode(ADDO, INPUT);      //  entrada para receber os dados
  pinMode(ADSK, OUTPUT);            //  saída para SCK
  pinMode(M1_LADO, OUTPUT);             //  saida do led
  pinMode(M1_PASSO, OUTPUT);             //  saida do led
  digitalWrite(M1_PASSO, LOW); // completa o passo do motor 
  pinMode(LED, OUTPUT);             //  saida do led
  pinMode(TRAVA, INPUT_PULLUP);     //  botão pullup
  pinMode(MENU, INPUT_PULLUP);      //  botão pullup
  pinMode(ESQUERDA, INPUT_PULLUP);  //  botão pullup
  pinMode(DIREITA, INPUT_PULLUP);   //  botão pullup
  pinMode(SAIR, INPUT_PULLUP);      //  botão pullup
  pinMode(FIM_CURSO ,INPUT_PULLUP); //  Swich do fim de curso Motor1
  pinMode(ATUADOR, OUTPUT);         //  Saida para atuador
  valD = 0;                         //  Podição atual do motor1 
  varB = EEPROMReadLong(M_varB);    //  carrega variavel com informações existente na memoria 
  zerar1 = EEPROM.read(M_zerar1);   //  carrega variavel com informações existente na memoria 
  zerar2= EEPROMReadLong(M_zerar2); //  carrega variavel com informações existente na memoria 
  unidadeM= EEPROM.read(M_unidadeM);//  carrega variavel com informações existente na memoria 


  // Incluir Função para acertar posição do motor de passo pelo fim de curso 
  //"fazer movimentação para localizar a posição do motor e certificar que o mesmo esta em ponto 0
      



} //end setup

//==========================================================================================================================================================
// --- Loop Infinito ---
void loop()
{
  keyboard_menu();                      //fazer leitura do 
  switch (menu_num)
  {
    case 1: menu1(); break;
    case 2: menu2(); break;
    case 3: menu3(); break;
    case 4: menu4(); break;
  } //end switch
  if (!digitalRead(TRAVA))Trava();
     
  
} //end loop

//==========================================================================================================================================================

// --- Funções ---
//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
// --- Movimentar o motor um passo, registra passição atual do motor no var: M1possA
void Motor1()  {
  
if (micros() - M1_previousMillis >= M1_tempo && M1possN != M1possA){

      if (M1possN > M1possA){
            digitalWrite(M1_LADO,LOW);
            M1possA = M1possA +1;}
      else {
            digitalWrite (M1_LADO, HIGH);
               M1possA = M1possA -1;}
      M1_previousMillis=micros();
      digitalWrite(M1_PASSO, HIGH);
      }
digitalWrite(M1_PASSO, LOW);
}// end Motor1


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
// --- Teclas --- faz a leitura do teclado, uma tecla por vez!
int Teclas() {
  if (!digitalRead(MENU))return 1;
  if (!digitalRead(ESQUERDA))return 2;
  if (!digitalRead(DIREITA))return 3;
  if (!digitalRead(SAIR))return 4;
  if (!digitalRead(TRAVA))return 5;
  else   return 0;
}//end Teclas


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
// -- Função do modulo HX711 - celula de carga
unsigned long ReadCount()
{          
  unsigned long Count = 0;
  unsigned char i;
  digitalWrite(ADSK, LOW);
  while(digitalRead(ADDO)) ;
  
  for (i = 0; i < 24; i++)
  {
    digitalWrite(ADSK, HIGH);
    Count = Count << 1;
    digitalWrite(ADSK, LOW);
    if (digitalRead(ADDO)) Count++;
  } //end for
      
  digitalWrite(ADSK, HIGH);
  Count = Count ^ 0x800000;
  digitalWrite(ADSK, LOW); 
  return (Count);

} //end ReadCount


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
void LerMedia() {  // imprimir constantemente o peso da balança nas linhas 2 e 3
  currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    contagem = contagem + 1; //passar contando o quantas vezes soma o peso
    convert = convert + ReadCount(); // soma o pesso para tirar a media
   
    //    if (contagem == 10) { //exibe o resultado a cada 10 mediçoes
    //      contagem = 0 ; // zera a contagem
    //      convert = convert / 10;

    if (contagem == 2) { //exibe o resultado a cada 2 mediçoes
      contagem = 0 ; // zera a contagem
      convert = convert / 2;


      if (convert < varB) {
        convert = varB - convert; //
        grama = convert / varA;
        grama = grama * -1;
      }
      if (convert > varB) {
        convert = convert - varB; //
        grama = convert / varA;
      }
      if (convert == varB) {
        grama = 0;
      }
      convert = 0 ; //zera o convert
    }
  }
  if ( grama < 1 && grama > -1) { //acender led ao lado do display
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }//end if
  varGrama = grama - sin(radians (val)) * zerar2; // teste com seno para calcular diferença da balança
     if (varGrama > gramaMax){
      gramaMax = varGrama;
      maxima = val;
     }
  ExibirLCD();
}//end LerMedia


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

void LerAtual() {  // atualizar o peso da balança nas linhas 2 e 3
  convert =  ReadCount(); // soma o pesso para tirar a media de 2
  if (convert < varB) {
    convert = varB - convert; //
    grama = convert / varA;
    grama = grama * -1;
  }
  else if (convert > varB) {
    convert = convert - varB; //
    grama = convert / varA;
  }
  else {
    grama = 0;
  }
  if ( grama ==0) { //acender led ao lado do display
    digitalWrite(LED, HIGH);
  } else {
    digitalWrite(LED, LOW);
  }//end if
   varGrama = grama - sin(radians (val)) * zerar2; //  multipica com seno para calcular diferença da balança
     if (varGrama > gramaMax){
      gramaMax = varGrama;
      maxima = val;
     }
  ExibirLCD();
}//end LerAtual


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

void ExibirLCD() // inicio da função  exibe para o LerMedia e LerAtual
{
    //tempo = millis(); //marcador de tempo   //17 miles segundos
  
  if(unidadeM==0){ //exibir em newton
    varNewton=varGrama*0.009807;
    gramaMaxNewton=gramaMax*0.009807;
    lcd.setCursor(0, 2);
  lcd.write("D= ");
  lcd.write("     ");
  lcd.setCursor(3, 2);
  lcd.print(val);
  lcd.setCursor(6, 2);
  lcd.write("/peso=        ");
  lcd.setCursor(13, 2);
  lcd.print(varNewton,3);
  lcd.setCursor(0, 3);
  lcd.write("MAX=                ");
  lcd.setCursor(7, 3);
  lcd.print(gramaMaxNewton,3);      
  }else{  //exibir em grama
    lcd.setCursor(0, 2);
  lcd.write("D= ");
  lcd.write("     ");
  lcd.setCursor(3, 2);
  lcd.print(val);
  lcd.setCursor(6, 2);
  lcd.write("/peso=        ");
  lcd.setCursor(13, 2);
  lcd.print(varGrama);
  lcd.setCursor(0, 3);
  lcd.write("MAX=                ");
  lcd.setCursor(7, 3);
  lcd.print(gramaMax);      
  }
 // tempo = millis() - tempo; //finaliza marcação de tempo  17miles segundos 11/07/20
  
}  //end ExibirLCD


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

void Medir() {  //inicio da função de leitura
  /*   *criar variaveis para:
    1º Gerar array para apresentar Grafico tempo/angulo X peso
  */
  //variaveis locais
  int grafico[90];
  int leitura[2];
  int qtdeLeitura = 0;
  int motor;
   
 unsigned long desceMile = 0;
  int fim = 0;
// Checar se o micros esta proximo do final, 
  while(micros()> 4284967296){
    lcd.clear();
    lcd.print("  Aguarde !");
  }
  
  // checar posição de motor paralela a folha e balança zerada
  if (!zerar1 || val != 0) {
    lcd.clear();
    lcd.print("  Erro == 02");// faltou zerar a balança ou iniciar da posição 0
    lcd.setCursor(0, 2);
    lcd.print(" ZERAR PONTOS 1 e 2 ");
    delay(1200);
    menu_num = 2;
    return;
  }
  //checar atuador ativo
  else if (!atuador) {
    lcd.clear();
    lcd.print("  Erro == 03");// faltou travar amostra
    lcd.setCursor(0, 2);
    lcd.print("   TRAVAR AMOSTRA   ");
    delay(1200);    
    return;
  } else {
gramaMax = 0;
    //LeD indicador de display
    delay(1000);
    //registar tempo de inicio
    tempo = millis(); //marcador de tempo
      M1_tempo=14968;  // totaal  marcador de tempo 8128 com motor 3,7 angulo
      
    //iniciar descida da celula de carga
    for (int i=0; i <= 90 ; i++) //descer por passo e registar a cada medida no array
    {
          val=i;
          LerAtual();
          grafico[i] = varGrama;        //incli a informação na tabela a cada grau
       M1possN = map(i, 0, 90, 0, M1_VOLTA/4); // ajustar o valor para inclinação do Motor compensando diferença
         while(M1possN!=M1possA){
         Motor1();
         } 
       motor=M1possN;
    }

    delay(1500);
    
    fim = varGrama;
    //ao finalizar curso comparar com tempo subtraindo resultado para imprimir na tela
    tempo = millis() - tempo; //finaliza marcação de tempo 
  
    //exibir media  qtde de posições acima de 1 grama
    for (int i = 0 ; i <= 90 ; i++) {
      if (grafico[i] > 0) {
        media = media + grafico[i];
        qtdeLeitura++;
        val=0;
      }
    }
    media = media / qtdeLeitura;
  
    //retornar motor para inicio
        M1_tempo=11000;
    
          M1possN = 0; // implantar forma de auto ajuste pelo numero de revoluções
           while(M1possN!=M1possA){
            Motor1();
           }
         
   
   
      Trava();
      delay (250);
      
        //finalizar  exibindo balança #tempo , media, Max, final
      
   
      if(unidadeM==0){
         varNewton=fim*0.009807;
    gramaMaxNewton=gramaMax*0.009807;
           lcd.clear();
      lcd.print("**   RESULTADOS   **");
      lcd.setCursor(0,1);
      lcd.print("HOLD MAX = ");      
      lcd.print(gramaMaxNewton,3);       
      lcd.setCursor(0,2);    
      lcd.print("ANG. MAX = ");
      lcd.print(maxima);
      lcd.setCursor(0,3);
      lcd.print("HOLD 90  = ");
      lcd.print(varNewton,3);
      }else{
      lcd.clear();
      lcd.print("**   RESULTADOS   **");
      lcd.setCursor(0,1);
      lcd.print("HOLD MAX = ");      
      lcd.print(gramaMax);       
      lcd.setCursor(0,2);    
      lcd.print("ANG. MAX = ");
      lcd.print(maxima);
      lcd.print("$");
      lcd.print(motor);
      lcd.setCursor(0,3);
      lcd.print("HOLD 90  = ");
      lcd.print(fim);
      lcd.print("$");
      lcd.print(tempo);
      }
      bool aguardar=HIGH;
      while(aguardar){  
            if(!digitalRead(TRAVA)){
                  aguardar=LOW;
                  Trava();
            }else if(!digitalRead(SAIR))aguardar=LOW;
      }//end while
      lcd.clear();
      delay(200);

  }
 sub_menu=2;
  //imprimir array otimizando linha
}//end Medir



//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
//Zerar balança em cima
void Zerar1() {
  LerMedia();

  varB = ReadCount(); //gravar variavel TARA a balança
  contagem = 0;
  convert = 0;
  zerar1 = HIGH;
      
  if( varB != EEPROMReadLong(M_varB )) EEPROMWriteLong(M_varB, varB);  
  if (zerar1 != EEPROM.read(M_zerar1))EEPROM.write(M_zerar1, zerar1); 
  gramaMax = 0; 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.println("      ZERANDO        ");
  lcd.println("   ponto  um    ");
  delay(500);
  LerMedia();
  previousMillis = currentMillis;

}// end Zerar1


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
//Zerar balança em baixo
void Zerar2() {

  if (zerar1 == HIGH) {
    LerMedia();
    zerar2 = grama;
        if  ( zerar2!= EEPROMReadLong(M_zerar2)) EEPROMWriteLong(M_zerar2 , zerar2); 
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println("      ZERANDO        ");
    lcd.println("  ponto dois  ");
    delay(500);
  }
  else {
    LerMedia();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   TEM QUE ZERAR       "); lcd.setCursor(0, 1);
    lcd.print("  PONTO  UM   ");
    lcd.setCursor(3, 3);
    lcd.print(" ERRO - 01");
    delay(1000);
    LerMedia();
  }

}// end Zerar2


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
void Trava() {

  while (!digitalRead(TRAVA)) delay(250);
  atuador = !atuador;
  if (atuador == HIGH) {
    digitalWrite(ATUADOR, HIGH);
    delay(100);
    lcd.begin(20, 4);
  } else {
    digitalWrite(ATUADOR, LOW);
    delay(100);
    lcd.begin(20, 4);
  }
}//end  TRAVA


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

void MoveLivre() {
  sair = HIGH; 
  lcd.setCursor(0, 0);
  lcd.print("   Move Livre      ");
  do {
      //
      if (!digitalRead(DIREITA) && val < 90) {
      lcd.setCursor(0, 1);
      lcd.print("> Direita >      ");
      delay(20);
      val = val + 1;
    }
    else if (!digitalRead(ESQUERDA) && val > 0) {
      lcd.setCursor(0, 1);
      lcd.print("< Esquerda <     ");
      delay(20);
      val = val - 1;
    } else {
      lcd.setCursor(0, 1);
      lcd.print("                    ");
          M1_tempo=800;
    }
    M1possN = map(val, 0, 90, 0, M1_VOLTA/4); // scale it to use it with the Motor (value between 0 and 180)              
    while(M1possN!=M1possA){
      Motor1();                  // sets the Motor position according to the scaled value
    }
    LerMedia();
    if (!digitalRead(SAIR)) {
      sair = LOW;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("  SAINDO MOVE LIVRE   ");
      delay(700);
    }
  } while (sair == HIGH);
  sub_menu = 1;
  return;

}// end MoveLivre


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
void GramaNewton(){
  sair= HIGH;
  lcd.setCursor(0, 0);
  lcd.print("    GRAMA/NEWTON    ");
  lcd.setCursor(0, 1);
  lcd.print("> Direita = GRAMA    "); //direita = grama = 1
  lcd.setCursor(0, 2);
  lcd.print("< Esquerda = NEWTON");  //esquerda = newton = 0
   do {
      if (!digitalRead(DIREITA) && unidadeM!=1) {
        while(!digitalRead(DIREITA)){}
      delay(20);
      unidadeM=1;
      EEPROMWriteLong(M_unidadeM , unidadeM); 
    }
    else if (!digitalRead(ESQUERDA) && unidadeM!=0) {
      while(!digitalRead(ESQUERDA)){}
      delay(20);
      unidadeM=0;
          if ( unidadeM!= EEPROM.read(M_unidadeM)) EEPROMWriteLong(M_unidadeM , unidadeM);                          
    } else {
      lcd.setCursor(0, 3);
      lcd.print(unidadeM);
      lcd.setCursor(2,3);
      if(unidadeM==0)lcd.print("NEWTON");
      else lcd.print("GRAMA       ");
    }
   
    if (!digitalRead(SAIR)) {
      sair = LOW;
      lcd.clear();
      lcd.setCursor(0, 0);
      
    }
  } while (sair == HIGH);
  sub_menu = 1;
  return;
}// end GramaNewton


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

void EEPROMWriteLong(int address, long value) {
   byte four = (value & 0xFF);
   byte three = ((value >> 8) & 0xFF);
   byte two = ((value >> 16) & 0xFF);
   byte one = ((value >> 24) & 0xFF);
   EEPROM.write(address, four);
   EEPROM.write(address + 1, three);
   EEPROM.write(address + 2, two);
   EEPROM.write(address + 3, one);
}//end EEPROMWriteLong


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO


unsigned long EEPROMReadLong(int address) {
   long four = EEPROM.read(address);
   long three = EEPROM.read(address + 1);
   long two = EEPROM.read(address + 2);
   long one = EEPROM.read(address + 3);
   return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}//end EEPROMReadLong


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

void keyboard_menu()
{
  if (!digitalRead(ESQUERDA) && sub_menu == 1)
  {
    while(!digitalRead(ESQUERDA)){}
    delay(150);
    lcd.clear();
     if (menu_num > 1) menu_num -= 1;
  } //end ESQUERDA
  if (!digitalRead(DIREITA) && sub_menu == 1)
  {
    while(!digitalRead(DIREITA)){}
    delay(150);
    lcd.clear();
    if (menu_num < menu_max) menu_num += 1;
  } //end DIREITA
  if (!digitalRead(MENU))
  {
    while(!digitalRead(MENU)){}
    delay(150);
    lcd.clear();
    if (sub_menu <= 2) sub_menu = 2;
  } //end MENU
  if (!digitalRead(SAIR))
  {
    while(!digitalRead(SAIR)){}
    delay(150);
    lcd.clear();
    if (sub_menu > 0) sub_menu = 1;
  } //end SAIR

} //end keyboard_menu


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

void menu1()
{
  switch (sub_menu)
  {
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("     AUTOMATICO    >");
      
      break;
    case 2:
      lcd.setCursor(0,0);
      lcd.print("     AUTOMATICO    ");
      lcd.setCursor(0,1);
      lcd.print("                   ");
      if (!digitalRead(MENU))Medir();
      break;
  }


} //end menu1


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

void menu2()
{
  switch (sub_menu)
  {
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("<       ZERAR      >");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      break;
    case 2:
      delay(150);
      lcd.setCursor(0, 0);
      lcd.print("        ZERAR       ");
      lcd.setCursor(0, 1);
      lcd.print("  RETIRE A AMOSTRA  ");
      lcd.setCursor(0, 2);
      lcd.print("PARA CALIBRAR -TARA-");
      lcd.setCursor(0, 3);
      lcd.print("   pressione enter  ");
      if (!digitalRead(MENU)) {
        if (M1possA != 0) {
          M1_tempo=2000;
          M1possN=0;
          do{
           
            Motor1();
          }while(M1possA !=M1possN);

            LerMedia();
            delay(10);
          }
         
        }
        delay(550);
        Zerar1();
        LerMedia();
        lcd.clear();
        delay(150);
        LerMedia();
        for (int i = 0 ; i <= 90; i++) {
        val=map(i,0,90,0,M1_VOLTA/4);
        M1_tempo=8000;
        
        M1possN=val;
        do{
         
          Motor1();
          }while(M1possA !=M1possN);
          valD = i;
          LerMedia();
          delay(10);          
        }
        delay(550);  
        Zerar2();
        M1_tempo=5000;
        
        M1possN=M1_VOLTA/4;
        do{
          
          Motor1();
          }while(M1possA !=M1possN);
        delay(300);
        valD = 45;
        val = 0;
        
        M1possN=0;
        do{
          
          Motor1();
          }while(M1possA !=M1possN);

        LerMedia();
        lcd.clear();
        menu_num = 1;
       // end if MENU Zerando
      break;
}
} //end menu2


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

void menu3()
{
  switch (sub_menu)
  {
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("<   GRAMA/NEWTON   >");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      break;
    case 2:
      GramaNewton();
      break;
  }


} //end menu3


//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO


void menu4()
{
  switch (sub_menu)
  {
    case 1:
      lcd.setCursor(0, 0);
      lcd.print("<  MOVIMENTO LIVRE  ");
      lcd.setCursor(0, 1);
      lcd.print("                ");
      break;
    case 2:
      MoveLivre();
      break;
  }


} //end menu4
