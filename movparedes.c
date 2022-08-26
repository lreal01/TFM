#include <wiringPi.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

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

FILE* posiciones;

int k = 25600; // número de pasos en los que se sivide una vuelta
int f0 = 20000; //20 kHz para los posicionamientos rápidos

int interruptor1_m, interruptor1_nm, interruptor2_m, interruptor2_nm;
int i1, i2, n_pasos1, n_pasos2, tdelay1, tdelay2, sgn1, sgn2, count1, count2;
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
		while(i1 < n_pasos1){
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
			delay(500);
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
	
	return;
	
	//printf("pos 1 %i\n",pos1);
	//printf("pos 2 %i\n",pos2);
	
 }
	

void posicionamiento_inicial() {

	int pos = -1;

	tdelay1 = 1000*1000 / (2 * f0);
	tdelay2 = 1000*1000 / (2 * f0);

	while (pos < 0 || pos>200) {
		printf("¿En qué posición desea comenzar? (0-200) mm\n");
		scanf("%i", &pos);
	}

	int d = pos - pos1;

	//Habilito los dos motores

	digitalWrite(EN1, HIGH);
	digitalWrite(EN2, HIGH);
	delay(200);

	//Miramos dirección
	if (d < 0) { //Negativa es hacia motor
		digitalWrite(DIR1, LOW);
		digitalWrite(DIR2, LOW);
		sgn1 = -1;
		sgn2 = -1;
	}
	if (d > 0) { //Positiva es hacia motor
		digitalWrite(DIR1, HIGH);
		digitalWrite(DIR2, HIGH);
		sgn1 = 1;
		sgn2 = 1;
	}

	delay(200);

	n_pasos1 = fabs(d) * k / 2;
	n_pasos2 = fabs(d) * k / 2;

	while (i1 < n_pasos1 && i2 < n_pasos1) {
		delay(20);
	}

	n_pasos1 = -1;
	n_pasos2 = -1;

	digitalWrite(DIR1, HIGH);
	digitalWrite(STEP1, HIGH);

	digitalWrite(DIR2, HIGH);
	digitalWrite(STEP2, HIGH);

	delay(200);
	digitalWrite(EN1, LOW);
	digitalWrite(EN2, LOW);

}

void desplazamiento() {
	int d = -1000;
	int tasa = -7;


	printf("¿Qué distancia desea desplazar? Introduzca un valor 0-200 mm.\n");
	printf("Signo positivo -> compresión    Signo negativo -> extensión\n");
	scanf("%i", &d);
	while ((pos1 + d) < -200 || (pos1 + d) > 200) {
		printf("Pruebe de nuevo.\n");
		printf("¿Qué distancia desea desplazar? Introduzca un valor 0-200 mm.\n");
		printf("Signo positivo->compresión    Signo negativo -> extensión\n");
		scanf("%i", &d);
	}

	printf("¿Cuál será la tasa de deformación? Introduzca un valor 1-20000 mm/h.\n");
	scanf("%i", &tasa);
	while (tasa<1||tasa>20000) {
		printf("Pruebe de nuevo.\n");
		printf("¿Cuál será la tasa de deformación? Introduzca un valor 1-20000 mm/h.\n");
		printf("Signo positivo->compresión    Signo negativo -> extensión\n");
		scanf("%i", &tasa);
	}


	int f = tasa * k / (2 * 3600); //frecuencia en s^-1
	tdelay1 = 1000 * 1000 / (2 * f);
	tdelay2 = 1000 * 1000 / (2 * f);

	//Habilito el motore

	digitalWrite(EN1, HIGH);
	digitalWrite(EN2, HIGH);
	delay(200);

	//Miramos dirección
	if (d < 0) { //Negativa es hacia motor
		digitalWrite(DIR1, LOW);
		digitalWrite(DIR2, LOW);
		sgn1 = -1;
		sgn2 = -1;
	}
	if (d > 0) { //Positiva es hacia no motor
		digitalWrite(DIR1, HIGH);
		digitalWrite(DIR2, HIGH);
		sgn1 = 1;
		sgn2 = 1;
	}

	delay(200);

	n_pasos1 = fabs(d) * k / 2;
	n_pasos2 = fabs(d) * k / 2;

	while (i1 < n_pasos1 && i2 < n_pasos1) {
		delay(20);
	}

	n_pasos1 = -1;
	n_pasos2 = -1;

	digitalWrite(DIR1, HIGH);
	digitalWrite(STEP1, HIGH);

	digitalWrite(DIR2, HIGH);
	digitalWrite(STEP2, HIGH);

	delay(200);
	digitalWrite(EN1, LOW);
	digitalWrite(EN2, LOW);

	lectura_posicion();

}



int main()
{
	wiringPiSetup();

	//Pines
	pinMode(EN1, OUTPUT);
	pinMode(DIR1, OUTPUT);
	pinMode(STEP1, OUTPUT);

	pinMode(FINAL1_M, INPUT);
	pinMode(FINAL1_NM, INPUT);

	pinMode(EN2, OUTPUT);
	pinMode(DIR2, OUTPUT);
	pinMode(STEP2, OUTPUT);

	pinMode(FINAL2_M, INPUT);
	pinMode(FINAL2_NM, INPUT);

	//Todo a cero por si acaso
	digitalWrite(STEP1, HIGH);
	digitalWrite(DIR1, HIGH);
	digitalWrite(EN1, LOW);

	digitalWrite(STEP2, HIGH);
	digitalWrite(DIR2, HIGH);
	digitalWrite(EN2, LOW);

	//Arranque hilos
	piThreadCreate(Lectura_interruptores);
	piThreadCreate(Motor1);
	piThreadCreate(Motor2);
	piThreadCreate(Guardado_Posicion);

	int mov = -2;
	
	printf("Movimiento de tectónica extensional/compresiva seleccionado.\n");
	
	//Preguntas usuario para obtencion de parametros:

	lectura_posicion();
	printf("Actualmente los motores se encuentran en:\n");
	printf("Motor 1: %i mm Motor 2: %i mm\n", pos1, pos2);
	printf("\n");

	if (pos1 != pos2) { //Como solo una pared con ángulo 90º, ambos motores tienen que estar en la misma pos
		printf("Los motores no se encuentran en la misma posición. Por favor, haga un reset en las herramientas del sistema y vuelva a intentarlo.\n");
		return 0;
	}
	
	printf("¿Quiere comenzar el experimento desde el punto actual?\n");
	printf("Sí(1) No(0)\n");
	scanf("%i",&mov);
	while(mov!=1&&mov!=0){
		printf("Pruebe de nuevo.\n");
		printf("¿Quiere comenzar el experimento desde el punto actual?\n");
		printf("Sí(1) No(0)\n");
		scanf("%i",&mov);
	}
	
	if (mov == 0) { //Colocar posición inicial
		posicionamiento_inicial();
		
		delay(1500);
		lectura_posicion();
		printf("Posicionamiento terminado.\n Actualmente los motores se encuentran en:\n");
		printf("Motor 1: %i mm Motor 2: %i mm\n", pos1, pos2);
		
	}
	
	printf("Puede proceder a colocar el material en el interior.\n");
	printf("Introduzca un 1 cuando haya acabado.");
	int seguridad = 0;
	while (seguridad!=1) {
		scanf("%i", &seguridad);
	}

	desplazamiento();
	
	delay(1500);
	
	printf("Experimento concluido.\n");
	
	//Todo a cero para finalizar
	digitalWrite (DIR1, HIGH);
	digitalWrite (STEP1, HIGH);
	digitalWrite (EN1, LOW);
	
	digitalWrite (DIR2, HIGH);
	digitalWrite (STEP2, HIGH);
	digitalWrite (EN2, LOW);
	

	return 0;
}

