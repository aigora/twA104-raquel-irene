/*
  Este ejemplo permite apagar, encender o poner a parpadear el led interno del Arduino.

  Solicita por pantalla al usuario que indice la acción que desea realizar (apagar, encender o
  parpadear) y la envía por el puerto serie al Arduino.

  El programa necesita que se le pase como parámetro el puerto serie a utilizar.
  Por ejemplo:
	serie COM3

*/

#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <locale.h>

/*

*/


int main(int argc, char* argv[])
{
	HANDLE hdPort;			// Manejador del puerto serie
	DCB paramsPort;			// Parámetros del puerto serie
	COMMTIMEOUTS tm;		// Tiempos de espera para el puerto serie
	char action;			// Acción a enviar al Arduino
	DWORD numBytes;			// Numero de bytes enviados al Arduino o recibos del Arduino
	char serialName[20];	// Nombre del puerto serie
	char temperature[20];	// Temperatura enviadao a o recibida de Arduino. Se utiliza una cadena de texto porque
							// hace más simple la programación aunque no sea lo más eficiente. Como máximo la cadena puede tener 19 caracteres. 
							// La cadena se reserva de 20 para poder contener los 19 caracteres máximos más
							// el byte de fin de cadena. Ese byte no se envia o recibe del arduino. 
	float faux;				// Variable flotante auxiliar. Se utiliza sólo para comprobar que la cadena "temperature" tiene un
							// formato válido
	int res;				// Resultado de algunas funciones


	// Se establece que el idoma es el español. Esto es sólo para que por la 
	// consola de Windows se representen bien los carácteres especiales del 
	// español (tildes, ñ, etc.)
	setlocale(LC_ALL, "spanish");

	// Se comprueba que se haya llamado al programa con un parámetro.
	// argc contiene el número de parámetros + 1, por eso se compara con 2
	if (argc != 2)
	{
		printf("Error: Número de parámetros incorrecto.\n");
		return -1;
	}

	// Los puertos serie en Windows se nombran como "COM1", "COM2", "COM3", etc.
	// Pero solo los nueve primeros se pueden abrir usando directamente ese nombre.
	// A partir del 10, tiene que nombrarse como "\\.\COM10" "\\.\COM11" etc.
	// Por eso, al parámetro indicado por el usuario se le va a añadir la cadena
	// "\\." para completar el nombre.

	// Se almacena en serialName el nombre del puerto concatenando la cadena
	// inicial al nombre del puerto indicado por el usuario como parámetro.
	// Al utiliar sprintf_s se asegura que aunque el usuario introduzca un nombre
	// de puerto muy largo nunca se van a copiar más de 19 bytes y por tanto
	// no va desbordar el serialName.
	sprintf_s(serialName, 19, "\\\\.\\%s", argv[1]);


	// Se abre el puerto indicando que se va a utilizar para lectura y escritura
	hdPort = CreateFile(serialName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hdPort == INVALID_HANDLE_VALUE)
	{
		printf("Error: Imposible abrir el puerto serie\n");
		return 1;
	}

	// Una vez abierto hay que configurarlo para que su velocidad coincida con la que se configura
	// en Arduino.

	// El puerto serie tiene bastantes parámetros de configuración. Para evitar tener que indicar todos
	// lo más cómodo es leer la configuración que tiene actualmente, modificar sólo lo que nos interesa
	// y volver a escribir la configuración

	// Se lee la configuración de los parámetros del puerto serie
	if (GetCommState(hdPort, &paramsPort) == 0)
	{
		printf("Error: Imposible configurar el puerto serie\n");
		return 2;
	}

	// Se modifican los parámetros necesarios para comunicar con Arduino
	paramsPort.BaudRate = CBR_9600;		// Velocidad: 9600 baudios
	paramsPort.Parity = NOPARITY;		// Sin paridad 
	paramsPort.ByteSize = 8;			// Numero de bits por byte 8
	paramsPort.StopBits = ONESTOPBIT;	// 1 bit de stop

	// Se vuelve a escribir la configuración
	if (SetCommState(hdPort, &paramsPort) == 0)
	{
		printf("Error: Imposible configurar el puerto serie\n");
		return 2;
	}


	// Ahora también hay que configurar los tiempos de espera por la respuesta del Arduino. Se va
	// a configurar un tiempo de 500 ms. Es decir, cuando se va a leer por el puerto serie, se espera
	// como máximo 500 ms por si llega algo.

	// El proceso de configuración es similar al anterior. Primero se leen los valores actuales de los
	// tiempos de espera, después se modifican los que hagan falta y por último se vuelven a escribir.

	// Lectura de valores
	if (GetCommTimeouts(hdPort, &tm) == 0)
	{
		printf("Error: Imposible configurar el puerto serie\n");
		return 2;
	}

	// Se ajustan los valores
	tm.ReadIntervalTimeout = 0;
	tm.ReadTotalTimeoutMultiplier = 0;
	tm.ReadTotalTimeoutConstant = 500;

	// Se escriben los nuevos valores
	if (SetCommTimeouts(hdPort, &tm) == 0)
	{
		printf("Error: Imposible configurar el puerto serie\n");
		return 2;
	}

	// Bucle infinito de solicitud al usuario de la acción a realizar
	do {

		// Se le muestra al usuario la opciones que tiene
		printf("\nIndique la acción a realizar\n");
		printf("1 - Encender led\n");
		printf("2 - Apagar led\n");
		printf("3 - Forzar parpadeo led\n");
		printf("4 - Leer temperatura\n");
		printf("5 - Establecer temperatura deseada\n");
		printf("0 - Salir\n");
		printf("> ");

		// Se lee la acción indicada por el usuario. 
		// _getch lee el carácter introducido en la consola pero no lo escribe, por eso
		// despues se hace el _putch. De esta manera el usuario si ve en pantalla la tecla
		// pulsada
		action = _getch();
		_putch(action);

		// Según la acción...
		switch (action)
		{
		case '0':
			// Ha indicado salir. Se cierra el puerto y se finaliza
			CloseHandle(hdPort);
			return 0;

		case '1':
		case '2':
		case '3':
			// Para cualquiera de estas acciones se hace lo mismo, enviarla por el puerto serie al 
			// Arduino. Se escribe en el puerto, pasando como dirección del buffer a escribir directamente
			// la dirección de la acción e indicando que sólo se va a escribir 1 byte.
			if (WriteFile(hdPort, &action, 1, &numBytes, NULL) == 0)
			{
				printf("\nError: Imposible enviar la acción.\n");
				return 4;
			}
			break;

		case '4':
			// Aquí tambien hay que enviar la acción al Arduino, pero después hay que leer la respuesta
			// del Arduino con la temperatura
			if (WriteFile(hdPort, &action, 1, &numBytes, NULL) == 0)
			{
				printf("\nError: Imposible enviar la acción.\n");
				return 4;
			}

			// Se lee por el puerto la temperatura recibida. Se manda leer como máximo 19
			// bytes, porque tiene que quedar al menos uno libre para poner el final de la
			// cadena de texto
			if (ReadFile(hdPort, temperature, 19, &numBytes, NULL) == 0)
			{
				printf("\nError: Imposible leer la temperatura.\n");
				return 5;
			}

			// Se añade el final a la cadena de texto recibida. El final ira en la posición
			// que corresponda a los bytes leídos. Por ejemplo, si se recibe "12.35", el 
			// valor de numBytes será 5 porque se han leido 5 caracteres y la marca de final
			// de cadena habrá que ponerla en la posición de numBytes, porque las cadenas en
			// C empiezan a contarse en 0. Es decir quedará
			// temperature[0] = '1'
			// temperature[1] = '2'
			// temperature[2] = '.'
			// temperature[3] = '3'
			// temperature[4] = '5'
			// temperature[5] = '\0'  			
			temperature[numBytes] = '\0';

			// Se escribe el resultado en la pantalla
			printf("\nTemperatura: %s ºC\n", temperature);
			break;


		case '5':
			// Se le pide al usuario la temperatura deseada. Se mete en un bucle porque
			// se va a pedir de forma continua hasta que el usuario proporcione un valor
			// de coma flotante válido. 
			res = 0;
			while (res != 1)
			{
				printf("\nIntroduzca la temperatura deseada en ºC: ");


				// Se recoge el valor en formato string
				res = scanf_s("%19s", temperature, (unsigned)_countof(temperature));
				if (res == 1)
				{
					// Antes de aceptar el string indicado por el usuario se comprueba que sea un
					// valor en como flotante válido. Para ello utilizo la función sscanf_s.
					res = sscanf_s(temperature, "%f", &faux);
				}
				if (res != 1)
				{
					printf("Error: valor de temperatura inválido. Introduzca un valor de coma flotante válido\n");
				}
			}

			// Se envia la acción al Arduino
			if (WriteFile(hdPort, &action, 1, &numBytes, NULL) == 0)
			{
				printf("\nError: Imposible enviar la acción.\n");
				return 4;
			}

			// Se envia el valor de temperatura deseado. 
			if (WriteFile(hdPort, temperature, strlen(temperature), &numBytes, NULL) == 0)
			{
				printf("\nError: Imposible enviar la consigna de temperatura.\n");
				return 5;
			}

			break;
		case EOF:
			printf("\nError: Fallo en la lectura desde consola\n");
			return 3;

		default:
			printf("\nError: '%c' no es una acción válida\n", action);
		}

	} while (TRUE);

	system("pause");
	return 0;
}