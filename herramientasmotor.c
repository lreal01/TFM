#include <wiringPi.h>
#include <pthread.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

//MOTOR IZDA
int EN1 = 25; //cable blanco (GPIO26)
int DIR1 = 29; //cable amarillo (GPIO21)
int STEP1 = 28; //cable rojo (GPIO20)
int FINAL1_M = 24; //Final carrera lado motor (GPIO19)
int FINAL1_NM = 22; //Final carrera lado no motor (GPIO6)
int pos1;


//MOTOR DCHA
int EN2 = 21; //cable blanco (GPIO5)
int DIR2 = 31; //cable amarillo (GPIO1)
int STEP2 = 30; //cable rojo (GPIO0)
int FINAL2_M = 3; //Final carrera (GPIO22)
int FINAL2_NM = 7; //Final carrera (GPIO4)
int pos2;

FILE *posiciones;

int k = 25600;
int f0 = 20000;

int interruptor1_m, interruptor1_nm, interruptor2_m, interruptor2_nm;
int i1, i2, n_pasos1, n_pasos2, tdelay1, tdelay2,sgn1,sgn2, count1,count2;
int num_mot;

int reset;

PI_THREAD(Lectura_interruptores){
	interruptor1_m = 0;
	interruptor1_nm = 0;
	interruptor2_m = 0;
	interruptor2_nm = 0;
	for(;;){
		int j = 0;
		int a1 = 0;
		int a2 = 0;
		int a3 = 0;
		int a4 = 0;
		while(j<5){
			delay(20);
			a1 += digitalRead(FINAL1_M);
			a2 += digitalRead(FINAL1_NM);
			a3 += digitalRead(FINAL2_M);
			a4 += digitalRead(FINAL2_NM);
			j++;
		}
		if(a1==5){
			interruptor1_m = 1;
			pos1=0;
			//printf("Motor 1 en extremo motor.\n");
			if(reset==0){n_pasos1 = -1;}
		}
		else{interruptor1_m = 0;}
		if(a2==5){
			interruptor1_nm = 1;
			pos1=200;
			//printf("Motor 1 en extremo no motor.\n");
			if(reset==0){n_pasos1 = -1;}
		}
		else {interruptor1_nm = 0; }
		if(a3==5){
			interruptor2_m = 1;
			pos2=0;
			//printf("Motor 2 en extremo motor.\n");
			if(reset==0){n_pasos2 = -1;}
		}
		else {interruptor2_m = 0; }
		if(a4==5){
			interruptor2_nm = 1;
			pos2=200;
			//printf("Motor 2 en extremo no motor.\n");
			if(reset==0){n_pasos2 = -1;}
		}
		else {interruptor2_nm = 0; }
	}
}

PI_THREAD(Motor1){
	for(;;){
		i1 = 0;
		while(i1<n_pasos1){
			digitalWrite(STEP1, LOW);
			delayMicroseconds(tdelay1);
			digitalWrite(STEP1, HIGH);
			delayMicroseconds(tdelay1);
			i1++;
			if (i1 % 12800 == 0) {
				pos1 = pos1 + sgn1;
			}
		}
		delay(500);
	}
}

PI_THREAD(Motor2) {
	n_pasos2 = 0;
	for (;;) {
		i2 = 0;
		while (i2 < n_pasos2) {
			digitalWrite(STEP2, LOW);
			delayMicroseconds(tdelay2);
			digitalWrite(STEP2, HIGH);
			delayMicroseconds(tdelay2);
			i2++;
			if (i2 % 12800 == 0) {
				pos2 = pos2 + sgn2;
			}
		}
		delay(500);
	}
}

PI_THREAD(Guardado_Posicion) {
	int j;
	for (;;) {
		while (n_pasos1 > 0 || n_pasos2 > 0) {
			delay(500);
			posiciones = fopen("posiciones.txt", "w");
			while (posiciones == NULL) { //Por si hubiera ocurrido un error creando el archivo
				posiciones = fopen("posiciones.txt", "w");
			}
			fprintf(posiciones, "%i,%i", pos1, pos2);
			fclose(posiciones);
			delay(1500);
			j=1;
		}
		if(j==1){
			
			posiciones = fopen("posiciones.txt", "w");
			while (posiciones == NULL) { //Por si hubiera ocurrido un error creando el archivo
				posiciones = fopen("posiciones.txt", "w");
			}
			fprintf(posiciones, "%i,%i", pos1, pos2);
			fclose(posiciones);
		}
		j=0;
	}
}

void reset_pos(){
	
	printf("Comienzo reset.\n");
	
	sgn1 = -1;
	sgn2 = -1;
	
	tdelay1 = 1000*1000/(2*f0);
	tdelay2 = 1000*1000/(2*f0);
	
	//Todo a cero por si acaso
	digitalWrite(STEP1, HIGH);
	digitalWrite(DIR1,HIGH);
	digitalWrite(EN1, LOW);
	
	digitalWrite(STEP2, HIGH);
	digitalWrite(DIR2,HIGH);
	digitalWrite(EN2, LOW);
	
	delay(200);
	
	//Enciendo motores
	digitalWrite(EN1, HIGH);
	digitalWrite(EN2, HIGH);
	delay(200);
	
	digitalWrite(DIR1,LOW);
	digitalWrite (DIR2,LOW);
	
	delay(20);
	
	n_pasos1 = 10000;
	n_pasos2 = 10000;
	while(interruptor1_m==0||interruptor2_m==0){
		i1 = 0;
		i2 = 0;
	}
	
	//Por si acaso
	n_pasos1 = -1;
	n_pasos2 = -1;
	digitalWrite(STEP1, HIGH);
	digitalWrite(STEP2, HIGH);
	
	delay(20);
	
	
	tdelay1 = 1000*1000/(2*500);
	tdelay2 = 1000*1000/(2*500);
	
	digitalWrite(DIR1,HIGH);
	digitalWrite(DIR2,HIGH);
	
	delay(20);
	
	reset=1;
	n_pasos1 = 10000;
	n_pasos2 = 10000;
	while(interruptor1_m==1||interruptor2_m==1){
		i1 = 0;
		i2 = 0;
	}
	reset=0;
	n_pasos1 = -1;
	n_pasos2 = -1;
	
	//Por si acaso
	digitalWrite(STEP1, HIGH);
	digitalWrite(STEP2, HIGH);
	
	delay(20);
	
	//Todo a cero por si acaso
	digitalWrite(STEP1, HIGH);
	digitalWrite(DIR1,HIGH);
	digitalWrite(EN1, LOW);
	
	digitalWrite(STEP2, HIGH);
	digitalWrite(DIR2,HIGH);
	digitalWrite(EN2, LOW);
	
	pos1 = 0;
	pos2 = 0;
	
	printf("Final reset.\n");
}

void lectura_posicion(){
	char data[7];
	
	posiciones = fopen("posiciones.txt","r"); 
	while(posiciones==NULL){ //Por si hubiera ocurrido un error creando el archivo
		posiciones = fopen("posiciones.txt","r"); 
	}
	 
	int j=0;
	while(!feof(posiciones)){
		fread(&data[j],sizeof(char),1,posiciones);
		//printf("%c %i\n",data[j],j);
		j++;
	}
	fclose(posiciones);
	
	sscanf(data, "%d,%d\n",&pos1,&pos2);

	if (pos1 < 0 || pos1>200){
		printf("ERROR FATAL. POR FAVOR, REALICE UN RESETEO.");
	}
	if (pos2 < 0 || pos2>200) {
		printf("ERROR FATAL. POR FAVOR, REALICE UN RESETEO.");
	}
	
	//printf("pos 1 %i\n",pos1);
	//printf("pos 2 %i\n",pos2);
	
 }
	
void desplazamiento(){
	int d=-1000;
	
	tdelay1 = 1000*1000/(2*f0);
	tdelay2 = 1000*1000/(2*f0);
	
	//Elección de motor a mover

	while(num_mot!=1&&num_mot!=2){
		printf("¿Qué motor desea mover?\n");
		printf("1. Motor 1\n");
		printf("2. Motor 2\n");
		scanf("%i",&num_mot);
	}


	if(num_mot==1){

		while((pos1+d)<-200||(pos1+d)>200){
			printf("¿Qué distancia desea desplazar? \n Introduzca un valor 0-200 mm. Signo positivo movimiento hacia 200, signo negativo hacia 0.\n");
			scanf("%i",&d);
		}

		//Habilito el motor 1
		
		digitalWrite(EN1, HIGH);
		digitalWrite(EN2, LOW);
		delay(200);
		
		//Miramos dirección
		if(d<0){ //Negativa es hacia motor
			digitalWrite(DIR1, LOW);
			sgn1 = -1;
		}
		 if(d>0){ //Positiva es hacia motor
			digitalWrite(DIR1, HIGH);
			sgn1 = 1;
		}
		
		delay(200);
		
		n_pasos1 = fabs(d)*k/2;
		//printf("npasos= %i\n",n_pasos1);
		//printf("i1= %i\n",i1);

		while(i1<n_pasos1){
			delay(20);
		}
		n_pasos1=-1;

	}

	if(num_mot==2){
		while((pos2+d)<-200||(pos2+d)>200){
			printf("¿Qué distancia desea desplazar? \n Introduzca un valor 0-200 mm. Signo positivo movimiento hacia 200, signo negativo hacia 0.\n");
			scanf("%i",&d);
		}

		//Habilito el motor 2
		
		digitalWrite(EN2, HIGH);
		digitalWrite(EN1, LOW);
		delay(200);
		
		//Miramos dirección
		if(d<0){ //Negativa es hacia motor
			digitalWrite(DIR2, LOW);
			sgn2 = -1;
		}
		 if(d>0){ //Positiva es hacia motor
			digitalWrite(DIR2, HIGH);
			sgn2 = 1;
		}
		
		delay(200);
		
		n_pasos2 = fabs(d)*k/2;
		//printf("npasos= %i\n",n_pasos2);
		//printf("i2= %i\n",i2);

		while(i2<n_pasos2){
			delay(20);
		}
		n_pasos2=-1;

	}

	digitalWrite (DIR1, HIGH);
	digitalWrite (STEP1, HIGH);
	
	digitalWrite (DIR2, HIGH);
	digitalWrite (STEP2, HIGH);
	
	delay(200);
	digitalWrite (EN1, LOW);
	digitalWrite (EN2, LOW);
	
	
}

void posicionamiento(){
	
	int pos = -1;
	
	tdelay1 = 1000*1000/(2*f0);
	tdelay2 = 1000*1000/(2*f0);
	
	//Elección de motores a mover
	if(pos1==pos2){
		while(num_mot!=1&&num_mot!=2&&num_mot!=3){
			printf("¿Qué motor desea mover?\n");
			printf("1. Motor 1\n");
			printf("2. Motor 2\n");
			printf("3. Ambos simultáneamente\n");
			scanf("%i",&num_mot);
		}
	}
	if(pos1!=pos2){
		while(num_mot!=1&&num_mot!=2){
			printf("¿Qué motor desea mover?\n");
			printf("1. Motor 1\n");
			printf("2. Motor 2\n");
			scanf("%i",&num_mot);
		}
	}
	
	if(num_mot==1){ //se mueve solo motor1
		while(pos<0||pos>200){
			printf("¿A qué posición desea mover el motor 1? (0-200) mm\n");
			scanf("%i",&pos);
		}
		
		int d = pos-pos1; //calculo la distancia a mover
		
		//Habilito el motor 1
		
		digitalWrite(EN1, HIGH);
		digitalWrite(EN2, LOW);
		delay(200);
		
		//Miramos dirección
		if(d<0){ //Negativa es hacia motor
			digitalWrite(DIR1, LOW);
			sgn1 = -1;
		}
		 if(d>0){ //Positiva es hacia motor
			digitalWrite(DIR1, HIGH);
			sgn1 = 1;
		}
		
		delay(200);
		
		n_pasos1 = fabs(d)*k/2;
		//printf("npasos= %i\n",n_pasos1);
		//printf("i1= %i\n",i1);

		while(i1<n_pasos1){
			delay(20);
		}
		n_pasos1=-1;
		
	}
	
	if(num_mot==2){ //se mueve solo motor 2
		while(pos<0||pos>200){
			printf("¿A qué posición desea mover el motor 2? (0-200) mm\n");
			scanf("%i",&pos);
		}
		
		int d = pos-pos2;
		
		//Habilito el motor 2
		
		digitalWrite(EN1, LOW);
		digitalWrite(EN2, HIGH);
		delay(200);
		
		//Miramos dirección
		if(d<0){ //Negativa es hacia motor
			digitalWrite(DIR2, LOW);
			sgn2 = -1;
		}
		if(d>0){ //Positiva es hacia motor
			digitalWrite(DIR2, HIGH);
			sgn2 =1;
		}
		
		delay(200);
		
		n_pasos2 = fabs(d)*k/2;
		
		while(i2<n_pasos2){
			delay(20);
		}

		n_pasos2 = -1;
		
	}
	
	if(num_mot==3){ //se mueven los dos motores
		while(pos<0||pos>200){
			printf("¿A qué posición desea mover los dos motores? (0-200) mm\n");
			scanf("%i",&pos);
		}
		
		int d = pos-pos1;
		
		//Habilito los dos motores
		
		digitalWrite(EN1, HIGH);
		digitalWrite(EN2, HIGH);
		delay(200);
		
		//Miramos dirección
		if(d<0){ //Negativa es hacia motor
			digitalWrite(DIR1, LOW);
			digitalWrite(DIR2, LOW);
			sgn1 = -1;
			sgn2 = -1;
		}
		if(d>0){ //Positiva es hacia motor
			digitalWrite(DIR1, HIGH);
			digitalWrite(DIR2, HIGH);
			sgn1 = 1;
			sgn2 = 1;
		}
		
		delay(200);
		
		n_pasos1 = fabs(d)*k/2;
		n_pasos2 = fabs(d)*k/2;
		
		while(i1<n_pasos1&&i2<n_pasos1){
			delay(20);
		}

		n_pasos1 = -1;
		n_pasos2 = -1;
		
		
	}
	
	digitalWrite (DIR1, HIGH);
	digitalWrite (STEP1, HIGH);
	
	digitalWrite (DIR2, HIGH);
	digitalWrite (STEP2, HIGH);
	
	delay(200);
	digitalWrite (EN1, LOW);
	digitalWrite (EN2, LOW);
	
	lectura_posicion();
	printf("Posicionamiento terminado.\n Actualmente los motores se encuentran en:\n");
	printf("Motor 1: %i mm Motor 2: %i mm\n", pos1, pos2);
	
	
}

int main(){
	
	wiringPiSetup();
	
	//Pines
	pinMode(EN1,OUTPUT);
	pinMode(DIR1,OUTPUT);
	pinMode(STEP1,OUTPUT);
	
	pinMode(FINAL1_M,INPUT);
	pinMode(FINAL1_NM,INPUT);
	
	pinMode(EN2,OUTPUT);
	pinMode(DIR2,OUTPUT);
	pinMode(STEP2,OUTPUT);
	
	pinMode(FINAL2_M,INPUT);
	pinMode(FINAL2_NM,INPUT);
	
	//Todo a cero por si acaso
	digitalWrite(STEP1, HIGH);
	digitalWrite(DIR1,HIGH);
	digitalWrite(EN1, LOW);
	
	digitalWrite(STEP2, HIGH);
	digitalWrite(DIR2,HIGH);
	digitalWrite(EN2, LOW);
	
	lectura_posicion();
	
	//Arranque hilos
	piThreadCreate(Lectura_interruptores);
	piThreadCreate(Motor1);
	piThreadCreate(Motor2);
	piThreadCreate(Guardado_Posicion);
	
	int modo = 0;
	num_mot = 0;
	sgn1 = 0;
	sgn2 = 0;
	
	reset =0;
		 	
	printf("\nHa elegido el programa de herramientas.\n");
	
	while(modo != 5){
		printf("Escoja:\n");
		printf("1. Realizar reset. RECOMENDADO.\n");
		printf("2. Desplazar un motor una distancia.\n");
		printf("3. Colocar motores en posición determinada.\n");
		printf("4. Conocer posición de los motores.\n");
		printf("5. Apagado.\n");
		scanf("%i",&modo);
		
		if(modo == 1){
			reset_pos();
			num_mot = 0;
		}
		
		if(modo == 2){
			lectura_posicion();
			printf("Actualmente los motores se encuentran en:\n");
			printf("Motor 1: %i mm Motor 2: %i mm\n", pos1, pos2);
			printf("\n");
			
			desplazamiento();
			num_mot = 0;
			
		}
		
		if(modo == 3){
			lectura_posicion();
			printf("Actualmente los motores se encuentran en:\n");
			printf("Motor 1: %i mm Motor 2: %i mm\n", pos1, pos2);
			printf("\n");
			
			posicionamiento();
			num_mot = 0;
			
		}
		if(modo == 4){
			lectura_posicion();
			printf("Actualmente los motores se encuentran en:\n");
			printf("Motor 1: %i mm Motor 2: %i mm\n", pos1, pos2);
			num_mot = 0;
			
		}
		if(modo == 5){

			digitalWrite(DIR1, HIGH);
			digitalWrite(STEP1, HIGH);

			digitalWrite(DIR2, HIGH);
			digitalWrite(STEP2, HIGH);

			delay(200);
			digitalWrite(EN1, LOW);
			digitalWrite(EN2, LOW);

		}
		
		
	}
	
	return 0;
	
}
