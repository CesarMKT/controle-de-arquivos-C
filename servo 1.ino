
int m1[] = {A0,A1,38,0}; //define as portas do primeiro motor no eixo X e seus passos (step,dir,eneable)
int m2[] = {A6,A7,A2,0}; //define as portas do segundo motor no eixo Y e seus passos (step,dir,eneable)

unsigned long M1_previousMillis = 0;
int M1possN = 0;                                //  varievel de nova posição para motor1
int M1possA = 0;                                   //  variavel para posição atual do motor1
int M1_RPM =2;     // VELOCIDADE DO MOTOR 1 DE ROTAÇÃO POR MINUTO
int M1_VOLTA = 1557;
unsigned long M1_tempo = 300;            // VELOCIDADE DO MOTOR 1 NUMERO DE PASSOS p/ REVOLUÇÃO *RPM / 60000 milisegundos


void setup(){

    for (int x; x<3; x++){
        pinMode(m1[x], OUTPUT);
        pinMode(m2[x], OUTPUT);
    }

    digitalWrite(m1[2],LOW);
    digitalWrite(m2[2],LOW);
    M1possN = 1557;

}

void loop(){

if (M1possA==M1possN && M1possN == 1557) M1possN=0;
else if (M1possA==M1possN && M1possN ==0) M1possN = 1557; 
Motor1();

}

void Motor1()  {
  
if (micros() - M1_previousMillis >= M1_tempo && M1possN != M1possA){

      if (M1possN > M1possA){
            digitalWrite(m1[1],LOW);
            M1possA = M1possA +1;}
      else {
            digitalWrite (m1[1], HIGH);
               M1possA = M1possA -1;}
      M1_previousMillis=micros();
      digitalWrite(m1[0], HIGH);
      }
digitalWrite(m1[0], LOW);
}
