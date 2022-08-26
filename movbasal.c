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

FILE* posiciones;

int k = 25600;
int f0 = 20000;

int interruptor1_m, interruptor1_nm, interruptor2_m, interruptor2_nm;
int i1, i2, n_pasos1, n_pasos2, tdelay1, tdelay2, sgn1, sgn2, count1, count2;
int num_mot, inmovil;

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
	

void posicionamiento_inicial() {

	int pos = -1;

	int d;

	tdelay1 = 1000*1000 / (2 * f0);
	tdelay2 = 1000*1000 / (2 * f0);


	if (num_mot == 1) {
		printf("Se va a colocar el motor inmóvil en el punto central de la máquina. Por favor, no se acerque ni desenchufe.\n");
		if (inmovil == 1) {

			//Posición en centro del motor 1

			digitalWrite(EN1, HIGH);
			digitalWrite(EN2, LOW);
			delay(200);

			d = 100 - pos1;

			//Miramos dirección
			if (d < 0) { //Negativa es hacia motor
				digitalWrite(DIR1, LOW);
				sgn1 = -1;
			}
			if (d > 0) { //Positiva es hacia motor
				digitalWrite(DIR1, HIGH);
				sgn1 = 1;
			}

			delay(200);

			n_pasos1 = fabs(d) * k / 2;

			while (i1 < n_pasos1) {
				delay(20);
			}

			n_pasos1 = -1;

			digitalWrite(DIR1, HIGH);
			digitalWrite(STEP1, HIGH);

			digitalWrite(DIR2, HIGH);
			digitalWrite(STEP2, HIGH);

			delay(200);
			digitalWrite(EN1, LOW);
			digitalWrite(EN2, LOW);

			delay(500);

			//Posición del motor móvil (2)

			while (pos < 0 || pos>200) {
				printf("¿En qué posición desea comenzar con el motor móvil? (0-200) mm\n");
				scanf("%i", &pos);
			}

			digitalWrite(EN1, LOW);
			digitalWrite(EN2, HIGH);
			delay(200);

			d = pos - pos2;

			//Miramos dirección
			if (d < 0) { //Negativa es hacia motor
				digitalWrite(DIR2, LOW);
				sgn2 = -1;
			}
			if (d > 0) { //Positiva es hacia motor
				digitalWrite(DIR2, HIGH);
				sgn2 = 1;
			}

			delay(200);

			n_pasos2 = fabs(d) * k / 2;

			while (i2 < n_pasos2) {
				delay(20);
			}

			n_pasos2 = -1;

			digitalWrite(DIR1, HIGH);
			digitalWrite(STEP1, HIGH);

			digitalWrite(DIR2, HIGH);
			digitalWrite(STEP2, HIGH);

			delay(200);
			digitalWrite(EN1, LOW);
			digitalWrite(EN2, LOW);

		}

		if (inmovil == 2) {

			//Posicion en centro del motor 2

			digitalWrite(EN1, LOW);
			digitalWrite(EN2, HIGH);
			delay(200);

			d = 100 - pos2;

			//Miramos dirección
			if (d < 0) { //Negativa es hacia motor
				digitalWrite(DIR2, LOW);
				sgn2 = -1;
			}
			if (d > 0) { //Positiva es hacia motor
				digitalWrite(DIR2, HIGH);
				sgn2 = 1;
			}

			delay(200);

			n_pasos2 = fabs(d) * k / 2;

			while (i2 < n_pasos2) {
				delay(20);
			}

			n_pasos2 = -1;

			digitalWrite(DIR1, HIGH);
			digitalWrite(STEP1, HIGH);

			digitalWrite(DIR2, HIGH);
			digitalWrite(STEP2, HIGH);

			delay(200);
			digitalWrite(EN1, LOW);
			digitalWrite(EN2, LOW);


			//Posición del motor móvil (1)

			while (pos < 0 || pos>200) {
				printf("¿En qué posición desea comenzar con el motor móvil? (0-200) mm\n");
				scanf("%i", &pos);
			}

			delay(500);

			digitalWrite(EN1, HIGH);
			digitalWrite(EN2, LOW);
			delay(200);

			d = pos - pos1;

			//Miramos dirección
			if (d < 0) { //Negativa es hacia motor
				digitalWrite(DIR1, LOW);
				sgn1 = -1;
			}
			if (d > 0) { //Positiva es hacia motor
				digitalWrite(DIR1, HIGH);
				sgn1 = 1;
			}

			delay(200);

			n_pasos1 = fabs(d) * k / 2;

			while (i1 < n_pasos1) {
				delay(20);
			}

			n_pasos1 = -1;

			digitalWrite(DIR1, HIGH);
			digitalWrite(STEP1, HIGH);

			digitalWrite(DIR2, HIGH);
			digitalWrite(STEP2, HIGH);

			delay(200);
			digitalWrite(EN1, LOW);
			digitalWrite(EN2, LOW);


		}
	}


	if (num_mot == 2) {
		while (pos < 0 || pos>200) {
			printf("¿En qué posición desea comenzar con el motor 1? (0-200) mm\n");
			scanf("%i", &pos);
		}

		delay(500);

		digitalWrite(EN1, HIGH);
		digitalWrite(EN2, LOW);
		delay(200);

		d = pos - pos1;

		//Miramos dirección
		if (d < 0) { //Negativa es hacia motor
			digitalWrite(DIR1, LOW);
			sgn1 = -1;
		}
		if (d > 0) { //Positiva es hacia no motor
			digitalWrite(DIR1, HIGH);
			sgn1 = 1;
		}

		printf("Se va a proceder a mover la máquina, por favor no se acerque ni desenchufe.\n");

		delay(200);


		n_pasos1 = fabs(d) * k / 2;

		while (i1 < n_pasos1) {
			delay(20);
		}

		n_pasos1 = -1;

		digitalWrite(DIR1, HIGH);
		digitalWrite(STEP1, HIGH);

		digitalWrite(DIR2, HIGH);
		digitalWrite(STEP2, HIGH);

		delay(200);
		digitalWrite(EN1, LOW);
		digitalWrite(EN2, LOW);

		delay(500);


		pos = -20;

		while (pos < 0 || pos>200) {
			printf("¿En qué posición desea comenzar con el motor 2? (0-200) mm\n");
			scanf("%i", &pos);
		}

		delay(500);

		digitalWrite(EN1, LOW);
		digitalWrite(EN2, HIGH);
		delay(200);

		d = pos - pos2;

		//Miramos dirección
		if (d < 0) { //Negativa es hacia motor
			digitalWrite(DIR2, LOW);
			sgn2 = -1;
		}
		if (d > 0) { //Positiva es hacia no motor
			digitalWrite(DIR2, HIGH);
			sgn2 = 1;
		}

		printf("Se va a proceder a mover la máquina, por favor no se acerque ni desenchufe.\n");

			delay(200);


		n_pasos2 = fabs(d) * k / 2;

		while (i2 < n_pasos2) {
			delay(20);
		}

		n_pasos2 = -1;

		digitalWrite(DIR1, HIGH);
		digitalWrite(STEP1, HIGH);

		digitalWrite(DIR2, HIGH);
		digitalWrite(STEP2, HIGH);

		delay(200);
		digitalWrite(EN1, LOW);
		digitalWrite(EN2, LOW);

		delay(500);
		
	}

}


void movimiento() {

	int d1=-30000;
	int d2 = -300000;
	int tasa1 = -20;
	int tasa2 = -20;

	printf("Recuerde que movimiento hacia 0 es negativo y hacia 200 es positivo.\n");
	
	if(num_mot==1){ //Si se mueve solo un motor
		
		if(inmovil==1){ //Motor 1 no se mueve
			
			while ((d2 + pos2) < 0 || (d2 + pos2) > 200) {
				printf("¿Qué distancia desea mover el motor 2?\n");
				scanf("%i", &d2);
			}
		
		
			printf("¿Cuál será la tasa de deformación en el motor 2? Introduzca un valor 1-20000 mm/h.\n");
			scanf("%i", &tasa2);
			while (tasa2<1||tasa2>20000) {
				printf("Pruebe de nuevo.\n");
				printf("¿Cuál será la tasa de deformación? Introduzca un valor 1-20000 mm/h.\n");
				scanf("%i", &tasa2);
			}
		
			int f2 = tasa2 * k / (2 * 3600); //frecuencia en s^-1
			tdelay2 = 1000 * 1000 / (2 * f2);

			//Habilito motor 2

			digitalWrite(EN1, LOW);
			digitalWrite(EN2, HIGH);
			delay(200);

			//Miramos dirección
			if (d2 < 0) { //Negativa es hacia motor
				digitalWrite(DIR2, LOW);
				sgn2 = -1;
			}
			if (d2 > 0) { //Positiva es hacia no motor
				digitalWrite(DIR2, HIGH);
				sgn2 = 1;
			}

			delay(200);

			n_pasos2 = fabs(d2) * k / 2;

			while (i2 < n_pasos2) {
				delay(10);
			}

			n_pasos2 = -1;

			digitalWrite(DIR1, HIGH);
			digitalWrite(STEP1, HIGH);

			digitalWrite(DIR2, HIGH);
			digitalWrite(STEP2, HIGH);

			delay(200);
			digitalWrite(EN1, LOW);
			digitalWrite(EN2, LOW);
		}
		
		if(inmovil==2){ //Motor 2 no se mueve
			
			while ((d1 + pos1) < 0 || (d1 + pos1) > 200) {
				printf("¿Qué distancia desea mover el motor 1?\n");
				scanf("%i", &d1);
			}
		
		
			printf("¿Cuál será la tasa de deformación en el motor 1? Introduzca un valor 1-20000 mm/h.\n");
			scanf("%i", &tasa1);
			while (tasa1<1||tasa1>20000) {
				printf("Pruebe de nuevo.\n");
				printf("¿Cuál será la tasa de deformación? Introduzca un valor 1-20000 mm/h.\n");
				scanf("%i", &tasa1);
			}
		
			int f1 = tasa1 * k / (2 * 3600); //frecuencia en s^-1
			tdelay1 = 1000 * 1000 / (2 * f1);

			//Habilito motor 1

			digitalWrite(EN1, HIGH);
			digitalWrite(EN2, LOW);
			delay(200);

			//Miramos dirección
			if (d1 < 0) { //Negativa es hacia motor
				digitalWrite(DIR1, LOW);
				sgn1 = -1;
			}
			if (d1 > 0) { //Positiva es hacia no motor
				digitalWrite(DIR1, HIGH);
				sgn1 = 1;
			}


			delay(200);

			n_pasos1 = fabs(d1) * k / 2;


			while (i1 < n_pasos1) {
				delay(10);
			}

			n_pasos1 = -1;



			digitalWrite(DIR1, HIGH);
			digitalWrite(STEP1, HIGH);

			digitalWrite(DIR2, HIGH);
			digitalWrite(STEP2, HIGH);

			delay(200);
			digitalWrite(EN1, LOW);
			digitalWrite(EN2, LOW);
			
		}
		
		
	}
	
	
	if(num_mot==2){ //Si se mueven los dos motores
		
		while ((d1 + pos1) < 0 || (d1 + pos1) > 200) {
			printf("¿Qué distancia desea mover el motor 1?\n");
			scanf("%i", &d1);
		}

		while ((d2 + pos2) < 0 || (d2 + pos2) > 200) {
			printf("¿Qué distancia desea mover el motor 2?\n");
			scanf("%i", &d2);
		}
		
		
		printf("¿Cuál será la tasa de deformación en el motor 1? Introduzca un valor 1-20000 mm/h.\n");
		scanf("%i", &tasa1);
		while (tasa1<1||tasa1>20000) {
			printf("Pruebe de nuevo.\n");
			printf("¿Cuál será la tasa de deformación? Introduzca un valor 1-20000 mm/h.\n");
			scanf("%i", &tasa1);
		}
		
		printf("¿Cuál será la tasa de deformación en el motor 2? Introduzca un valor 1-20000 mm/h.\n");
		scanf("%i", &tasa2);
		while (tasa2<1||tasa2>20000) {
			printf("Pruebe de nuevo.\n");
			printf("¿Cuál será la tasa de deformación? Introduzca un valor 1-20000 mm/h.\n");
			scanf("%i", &tasa2);
		}
		
		int f1 = tasa1 * k / (2 * 3600); //frecuencia en s^-1
		int f2 = tasa2 * k / (2 * 3600); //frecuencia en s^-1
		tdelay1 = 1000 * 1000 / (2 * f1);
		tdelay2 = 1000 * 1000 / (2 * f2);

		//Habilito los dos motores

		digitalWrite(EN1, HIGH);
		digitalWrite(EN2, HIGH);
		delay(200);

		//Miramos dirección
		if (d1 < 0) { //Negativa es hacia motor
			digitalWrite(DIR1, LOW);
			sgn1 = -1;
		}
		if (d1 > 0) { //Positiva es hacia no motor
			digitalWrite(DIR1, HIGH);
			sgn1 = 1;
		}
		if (d2 < 0) { //Negativa es hacia motor
			digitalWrite(DIR2, LOW);
			sgn2 = -1;
		}
		if (d2 > 0) { //Positiva es hacia no motor
			digitalWrite(DIR2, HIGH);
			sgn2 = 1;
		}

		delay(200);

		n_pasos1 = fabs(d1) * k / 2;
		n_pasos2 = fabs(d2) * k / 2;

		while (i1 < n_pasos1 || i2 < n_pasos2) {
			delay(10);

			if (i1 >= n_pasos1) {
				n_pasos1 = -1;
			}
			if (i2 >= n_pasos2) {
				n_pasos2 = -1;
			}

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
}


int main(){
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
	
	
	printf("Movimiento de tectónica en dirección seleccionado.\n");
	
	
	//Preguntas usuario para obtencion de parametros:

	lectura_posicion();
	printf("Actualmente los motores se encuentran en:\n");
	printf("Motor 1: %i mm Motor 2: %i mm\n", pos1, pos2);
	printf("\n");

	num_mot=0;
	
	//Número de motores/bases que se van a mover
	printf("Seleccione el número de motores (1 o 2) que se va a utilizar:\n");
	scanf("%i",&num_mot);
	while(num_mot!=1&&num_mot!=2){
		printf("Pruebe de nuevo, solo hay dos motores disponibles:\n");
		scanf("%i",&num_mot);
	}

	inmovil = 0;
	
	if(num_mot==1){ //Si solo 1 motor, se selecciona el inmovil
		printf("Seleccione el motor fijo (1 o 2).\n");
		scanf("%i",&inmovil);
		while(inmovil!=1&&inmovil!=2){
			printf("Pruebe de nuevo, solo hay dos motores posibles:\n");
			scanf("%i",&inmovil);
		}
	}
	
	int mov = -1;
	
	//Reseteo o no de la posición última
	printf("¿Quiere comenzar el experimento desde la posición actual?\n");
	printf("Sí(1) No(0)\n");
	scanf("%i",&mov);
	while(mov!=1&&mov!=0){
		printf("Pruebe de nuevo.\n");
		printf("¿Quiere comenzar el experimento desde la posición actual?\n");
		printf("Sí(1) No(0)\n");
		scanf("%i",&mov);
	}

	if (mov == 0) {//Si se quiere cambiar la posición inicial

		posicionamiento_inicial(); //cambiar para los dos motores
		
		delay(1500);
		lectura_posicion();
		printf("Posicionamiento terminado.\n Actualmente los motores se encuentran en:\n");
		printf("Motor 1: %i mm Motor 2: %i mm\n", pos1, pos2);
	}

	printf("Puede proceder a colocar el material en el interior.\n");
	printf("Introduzca un 1 cuando haya acabado.");
	int seguridad = 0;
	while (seguridad != 1) {
		scanf("%i", &seguridad);
	}


	movimiento();

	
	printf("Experimento finalizado.\n");
		
	//FINAL, todo a cero 
	digitalWrite (STEP1, HIGH); //0
	digitalWrite (DIR1, HIGH); //0
	digitalWrite (EN1, LOW); //0
	digitalWrite (STEP2, HIGH); //0
	digitalWrite (DIR2, HIGH); //0
	digitalWrite (EN2, LOW); //0
	

		
	return 0;
}
