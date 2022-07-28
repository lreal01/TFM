#include <stdio.h>
#include <stdlib.h>

int main()
{
	int modo = 0;
	printf("\n¡Bienvenido!\n");
	
	while(modo!=4){
		printf("\nEstá en el menú principal.\nSeleccione el modo que desea emplear:\n");
		printf("1. Experimento con movimiento de bases.\n");
		printf("2. Experimento con movimiento de pared.\n");
		printf("3. Herramientas de movimiento de motores.\n");
		printf("4. Apagar.\n");
		scanf("%i",&modo);
		
		if(modo==1){
			system("./movbasal");
			}
		if(modo==2){
			system("./movparedes");
			}
		if(modo==3){
			system("./herramientas");
			}
	}
	
	printf("¡Hasta la próxima!\n");
	
	return 0;
}
