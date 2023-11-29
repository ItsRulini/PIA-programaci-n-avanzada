#include <Windows.h>
#include <iostream>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <CommCtrl.h>
#include "resource.h"


LRESULT CALLBACK cVentanaRegistro(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK cVentanaAgenda(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK cVentanaDetallesCita(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK cVentanaInformacionDoctor(HWND, UINT, WPARAM, LPARAM);

bool llenarDatos = false;

struct Doctor {
	char nombre[50], cedula[50], clave[50], password[50];
	Doctor* siguiente, * anterior;
};

struct Clientes {
	char nombreCliente[50], nombreMascota[50], motivoConsulta[300];
	int indiceEspecieMascota = 0, indiceDeCita = 0, indiceEstatusCita = 0;
	long long int telefonoCliente = 0;
	float costoConsulta = 0;
	int anio = 0, mes = 0, dia = 0, hora = 0, minuto = 0, segundo = 0;
	long long fechaEnOrden = 0;
	Clientes* siguiente, * anterior;
};

//Declaración de las listas a utilizar y sus respectivas funciones
Doctor* listaDoctores = NULL; //Lista de doctores variable global que va a tener a todos los usuarios (doctores)
void insertarElementosDoctores(char[], char[], char[], char[]);
Doctor* buscarDoctores(char[], char[]);
void eliminarDoctor();

Doctor* doctorActual; // Variable para saber el doctor con el que se inicó sesión
void llenarDatosDoctor(HWND);

Clientes* listaClientes = NULL; //Lista de clientes
void insertarClientes(char[], long long int, char[], char[], float, int, int, int, int, int, int, int, int);
int indiceGlobalDeCitas = 0; // Indice de citas
Clientes* buscarClientes(int);
void eliminarCliente(int);
void ordenarCitas();
long long partition(Clientes* [], int, int);
void quickSort(Clientes* [], int, int);
void swap(Clientes*&, Clientes*&);

Clientes* nodoSeleccionado; // Variable para saber el nodo seleccionado en las diferenets funciones
void SetDetallesCitaData(HWND); //Funcion para enviar los datos por si seleccionó algún elemento de la listbox
Clientes* filtrarCitas(int, int, int);

HBITMAP doctorHBitmap = NULL;  // Para la primera foto
HBITMAP doctorHBitmapNueva = NULL;  // Para la segunda foto

// Operando en archivos binarios para clientes
void guardarDatosEnArchivoBinario();
void cargarDatosDesdeArchivoBinario();

// Operando en archivos binarios para doctores
void guardarDoctoresEnArchivoBinario();
void cargarDoctoresDesdeArchivoBinario();

// Función para acceder al identificador del mapa de bits actual
HBITMAP GetDoctorHBitmap() {
	return (doctorHBitmapNueva != NULL) ? doctorHBitmapNueva : doctorHBitmap;
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, PSTR cmdLine, int cShow) {
	//Creando la ventana
	//hInst es la instancia de la ventana, el MAKEINTRESOURCE es el ID de la ventana, NULL es la ventana padre de donde proviene
	//CALLBACK es la función que espera a que sea llamada
	cargarDatosDesdeArchivoBinario(); //Llamando para cargar las citas
	cargarDoctoresDesdeArchivoBinario(); //Llamando para cargar los datos de los doctores

	HWND hVentanaRegistro = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN_WINDOW), NULL, cVentanaRegistro);

	MSG msg; //Variable para los mensajes recibidos en la ventana
	ZeroMemory(&msg, sizeof(MSG));

	ShowWindow(hVentanaRegistro, cShow);

	while (GetMessage(&msg, NULL, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
char doctorFoto[1000];
LRESULT CALLBACK cVentanaRegistro(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_COMMAND: {
		//HIWORD = QUE ; LOWORD = QUIEN
		if (LOWORD(wParam) == IDC_REGISTRO && HIWORD(wParam) == BN_CLICKED) { //Si le da clic a registrarse
			
			//Obteniendo todos los datos para el registro
			HWND hFullName = GetDlgItem(hwnd, IDC_NOMBRE_DOCTOR);
			HWND hCedula = GetDlgItem(hwnd, IDC_CEDULA);
			HWND hClaveUsuario = GetDlgItem(hwnd, IDC_CLAVE_USUARIO);
			HWND hPwrd = GetDlgItem(hwnd, IDC_PASSWORD); //Obtiene lo que tiene 'PASSWORD'

			//Obteniendo las longitudes de cada una para ver si contiene algo
			int iFullNameLength = GetWindowTextLength(hFullName); //Obtiene la longitud del nombre del doctor
			int iCedulaLength = GetWindowTextLength(hCedula);
			int iClaveUsuarioLength = GetWindowTextLength(hClaveUsuario);
			int iPasswordLength = GetWindowTextLength(hPwrd);

			//Creando variables para almacenar los valores de dichas strings
			char nombre[50], cedula[50], claveUser[50], password[50];

			//Obteniendo las longitudes
			GetWindowText(hFullName, nombre, iFullNameLength + 1);
			GetWindowText(hCedula, cedula, iCedulaLength + 1);
			GetWindowText(hClaveUsuario, claveUser, iClaveUsuarioLength + 1);
			GetWindowText(hPwrd, password, iPasswordLength + 1); //Guarda PASSWORD en "password"

			if (iFullNameLength < 1 || iCedulaLength < 1 || iClaveUsuarioLength < 1 || iPasswordLength < 1 || strlen(doctorFoto) == 0) {
				MessageBox(NULL, "Debe rellenar todos los campos", "Advertencia", MB_OK);
			}	
			else {

				bool validarNombre = true; // Validación de las letras en el nombre del doctor
				for (int i = 0; nombre[i] != '\0'; i++)
				{
					if (!isalpha(nombre[i]) && !isspace(nombre[i])) {
						validarNombre = false;
						break;
					}
				}
				if (!validarNombre)
				{
					MessageBox(hwnd, "El nombre solo debe contener letras y espacios.", "Advertencia", MB_OK);
				}
				else
				{
					insertarElementosDoctores(nombre, cedula, claveUser, password);
					MessageBox(NULL, "Usuario registrado, ahora puede iniciar sesión", "Registro exitoso", MB_OK);
				}
				
			}
			// Borra el contenido de los EditBox
			SetWindowText(hFullName, ""); // Borra el contenido del EditBox de nombre
			SetWindowText(hCedula, ""); // Borra el contenido del EditBox de cédula
			SetWindowText(hClaveUsuario, ""); // Borra el contenido del EditBox de clave de usuario
			SetWindowText(hPwrd, ""); // Borra el contenido del EditBox de contraseña
		}

		if (LOWORD(wParam) == IDC_CARGAR_FOTO) {
			OPENFILENAME ofn;
			char szFileName[MAX_PATH] = "";

			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = sizeof(szFileName);
			ofn.lpstrFilter = "Archivos de Imagen\0*.bmp;\0";
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

			// carga de mapa de bits
			HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);

			if (GetOpenFileName(&ofn)) {
				HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);

				if (hBitmap != NULL) {
					HWND hwndStatic = GetDlgItem(hwnd, IDC_FOTO);
					SendMessage(hwndStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

					doctorHBitmap = hBitmap;  // Guarda el identificador del mapa de bits en la variable global
					strcpy_s(doctorFoto, szFileName);

					//MessageBox(hwnd, "Imagen cargada exitosamente", "CARGA EXITOSA", MB_OK);
				}
				else {
					MessageBox(hwnd, "Error al cargar la imagen", "ERROR", MB_OK | MB_ICONERROR);
				}
			}
		}

		if (LOWORD(wParam) == IDC_INICIO_SESION && HIWORD(wParam) == BN_CLICKED) { //Botón de inicio de sesión
			HWND hClaveUsuario = GetDlgItem(hwnd, IDC_CLAVE_USUARIO2);
			HWND hPassword = GetDlgItem(hwnd, IDC_PASSWORD2);

			int iClaveUsuarioLength = GetWindowTextLength(hClaveUsuario);
			int iPasswordLength = GetWindowTextLength(hPassword);

			char claveUser[50], password[50];

			GetWindowText(hClaveUsuario, claveUser, iClaveUsuarioLength + 1);
			GetWindowText(hPassword, password, iPasswordLength + 1); //Guarda PASSWORD en "password"

			if (iClaveUsuarioLength < 1 || iPasswordLength < 1 || (strlen(doctorFoto) == 0)) {
				MessageBox(NULL, "Debe rellenar todos los campos", "Advertencia", MB_OK);
			}
			else {
				if (buscarDoctores(claveUser, password) == nullptr) { //No se encontró el doctor
					MessageBox(NULL, "No se encontró al doctor", "Advertencia", MB_OK);

					SetWindowText(hClaveUsuario, "");
					SetWindowText(hPassword, "");
				}
				else {
					doctorActual = buscarDoctores(claveUser, password);
					MessageBox(NULL, "Bienvenido" , "Inicio de sesión exitoso", MB_OK);
					char message[100];
					sprintf_s(message, "Doctor encontrado: %s", doctorActual->nombre); // O cualquier otro dato relevante
					MessageBox(NULL, message, "Información de usuario", MB_OK);
					EndDialog(hwnd, 0); //DestroyWindow(hwnd);  
					HWND hMenuWnd =  CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_AGENDA), hwnd, cVentanaAgenda);
					//Muestra la nueva ventana
					ShowWindow(hMenuWnd, SW_SHOW);
				}
			}
		}

	}break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
	break;
	case WM_DESTROY:
		guardarDatosEnArchivoBinario();
		guardarDoctoresEnArchivoBinario();
		PostQuitMessage(1); //Si la pongo en otra ventana se acaba todo el programa
	break;
	}
	return FALSE;
}
bool filtradorDeCitas = false;
Clientes* filtrarCita = nullptr;
int longitud = 0;
Clientes* todosLosNodos = nullptr;
int tamanio = 0;
bool mostrarCitas = false;

LRESULT CALLBACK cVentanaAgenda(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_INITDIALOG: {
		//Cargar imagen
		HBITMAP hBitmap = GetDoctorHBitmap();

		if (hBitmap != NULL) {
			HWND hwndStatic = GetDlgItem(hwnd, IDC_FOTO_AGENDA);
			SendMessage(hwndStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
		}

		HWND hNombreDelDoctor = GetDlgItem(hwnd, IDC_NOMBRE_AGENDA);

		SetWindowText(hNombreDelDoctor, doctorActual->nombre);

		// Imprimiendo las listas
		HWND hListCitas = GetDlgItem(hwnd, IDC_LISTA_CITAS);

		if (listaClientes == nullptr) { //Cambiar por la lista de citas
			SendMessage(hListCitas, LB_ADDSTRING, NULL, (LPARAM)"Sin citas por el momento");
		}
		else {
			// Si la lista no está vacía, llena la list box con los datos de la lista
			ordenarCitas(); // Función para ordenar los datos de la lista con ls citas de fechas más próximas a las más lejanas

			if (filtradorDeCitas)
			{
				
				for (int cantidad = 0; cantidad < longitud; cantidad++)
				{
					char estatus[20];
					switch (filtrarCita[cantidad].indiceEstatusCita)
					{
					case 0: strcpy_s(estatus, " - Pendiente"); break; // pendiente
					case 1: strcpy_s(estatus, " - Efectuada"); break; // efectuada
					case 2: strcpy_s(estatus, " - Cancelada"); break; // cancelada
					}

					char nombreYEstatus[50], contenedor[50];
					strcpy_s(contenedor, filtrarCita[cantidad].nombreMascota);
					strcat_s(contenedor, estatus);
					strcpy_s(nombreYEstatus, contenedor);
					//SendMessage(hListCitas, LB_ADDSTRING, 0, (LPARAM)actual->nombreMascota);
					SendMessage(hListCitas, LB_ADDSTRING, 0, (LPARAM)nombreYEstatus);
					//cantidad++;
				}

			}
			else // Si no estamos filtrando citas, solo imprimiremos todas las citas que se han agendado
			{
				Clientes* actual = listaClientes;
				tamanio = 0;
				mostrarCitas = false;
				while (actual != nullptr) {
					char estatus[20];
					switch (actual->indiceEstatusCita)
					{
					case 0: strcpy_s(estatus, " - Pendiente"); break; // pendiente
					case 1: strcpy_s(estatus, " - Efectuada"); break; // efectuada
					case 2: strcpy_s(estatus, " - Cancelada"); break; // cancelada
					}

					char nombreYEstatus[50], contenedor[50];
					strcpy_s(contenedor, actual->nombreMascota);
					strcat_s(contenedor, estatus);
					strcpy_s(nombreYEstatus, contenedor);
					
					time_t tiempoActual = time(0);
					struct tm tiempoUsuario;

					time(&tiempoActual); //Obtiene la fecha y hora actual

					if (localtime_s(&tiempoUsuario, &tiempoActual) == 0) {
						tiempoUsuario.tm_year = actual->anio - 1900;
						tiempoUsuario.tm_mon = actual->mes - 1;
						tiempoUsuario.tm_mday = actual->dia;
						tiempoUsuario.tm_hour = actual->hora;
						tiempoUsuario.tm_min = actual->minuto;
						tiempoUsuario.tm_sec = actual->segundo;

					}

					if (difftime(mktime(&tiempoUsuario), tiempoActual) > 0) {
						SendMessage(hListCitas, LB_ADDSTRING, 0, (LPARAM)nombreYEstatus);
						
					}
					else
					{
						mostrarCitas = true;
						tamanio++; // Variable que cuenta la cantidad de citas pasadas que hay
					}
					actual = actual->siguiente;
				}
			}
			
		}
	}break;
	case WM_COMMAND:{
		HWND hListCitas = GetDlgItem(hwnd, IDC_LISTA_CITAS);
		int indiceSeleccionado = SendMessage(hListCitas, LB_GETCURSEL, 0, 0); //Indice que seleccionemos de la listbox

		switch (LOWORD(wParam))
		{

		case ID_AGENDA:
		{
			llenarDatos = false;
			filtradorDeCitas = false;
			nodoSeleccionado = nullptr;
			mostrarCitas = false;
			EndDialog(hwnd, 0);
			HWND hVentanaAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_AGENDA), hwnd, cVentanaAgenda);
			ShowWindow(hVentanaAgenda, SW_SHOW);
		}
			break;
		case ID_INFODOCTOR:
		{
			EndDialog(hwnd, 0);
			HWND hVentanaInfoDoctor = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_INFO_DOCTOR), hwnd, cVentanaInformacionDoctor);
			llenarDatosDoctor(hVentanaInfoDoctor);
			ShowWindow(hVentanaInfoDoctor, SW_SHOW);
		}
			break;
		case ID_MANEJODECITAS:
		{
			llenarDatos = false;
			filtradorDeCitas = false;
			nodoSeleccionado = nullptr;
			mostrarCitas = false;
			EndDialog(hwnd, 0);
			HWND hManejoCitas = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DETALLES_CITA), hwnd, cVentanaDetallesCita);
			ShowWindow(hManejoCitas, SW_SHOW);
		}
			break;
		case ID_SALIR:
		{
			EndDialog(hwnd, 0);
			HWND hVentanaRegistro = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN_WINDOW), hwnd, cVentanaRegistro);
			ShowWindow(hVentanaRegistro, SW_SHOW);
		}
			break;
		}


		if (LOWORD(wParam) == IDC_VER_DETALLES_CITA && HIWORD(wParam) == BN_CLICKED) {
			
			if (indiceSeleccionado != LB_ERR) { //indiceSeleccionado existente

				// Accede al nodo correspondiente en la lista
				nodoSeleccionado = buscarClientes(indiceSeleccionado); //Busca el nodo del indice seleccionado
				//Indica que el usuario seleccionó un elemento del listbox, por lo tanto en la siguiente ventana se mostrarán los datos de la cita seleccionada
				
				if (nodoSeleccionado != nullptr) { //Si existe el nodo
					llenarDatos = true;
					MessageBox(NULL, "Enviando datos", "Advertencia", MB_OK);
					// Crea la ventana de detalles de la cita
					//llenarDatos = true;
					EndDialog(hwnd, 0);
					HWND hDetallesCita = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DETALLES_CITA), hwnd, cVentanaDetallesCita);
					// Pasa la información del nodo seleccionado a la ventana de detalles
					//EndDialog(hwnd, 0);
					SetDetallesCitaData(hDetallesCita);
					ShowWindow(hDetallesCita, SW_SHOW);
				}
				else {
					MessageBox(NULL, "No se encontró la cita", "Advertencia", MB_OK);
				}
			}
			else {
				MessageBox(NULL, "No seleccionó ningún elemento", "Advertencia", MB_OK);
			}

		}

		if (LOWORD(wParam) == IDC_AGENDAR && HIWORD(wParam) == BN_CLICKED) {
			llenarDatos = false;
			mostrarCitas = false;
			//indiceGlobalDeCitas++;
			EndDialog(hwnd, 0); //DestroyWindow(hwnd); Por si suceden cositas
			//Redireccionando a la ventana de registro
			HWND hAgendar = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DETALLES_CITA), hwnd, cVentanaDetallesCita);
			ShowWindow(hAgendar, SW_SHOW);
		}


		if (LOWORD(wParam) == IDC_FILTRAR && HIWORD(wParam) == BN_CLICKED)
		{
			SYSTEMTIME selectedDate;
			HWND hCalendar = GetDlgItem(hwnd, IDC_CALENDARIO); // Reemplaza IDC_CALENDARIO con el ID de tu control de calendario

			// Asegúrate de que hCalendar sea válido
			if (hCalendar != NULL) {
				SendMessage(hCalendar, MCM_GETCURSEL, 0, (LPARAM)&selectedDate);
				// Ahora selectedDate contiene la fecha seleccionada en el control de calendario
				
				//Inicializando variables que contengan cada cosa
				char anio[10], mes[10], dia[15];// horas[15], minutos[10], segundos[10];
				_itoa_s(selectedDate.wYear, anio, 10);
				_itoa_s(selectedDate.wMonth, mes, 10);
				_itoa_s(selectedDate.wDay, dia, 10);

				char fechaCompleta[25];// horaCompleta[25];
				strcat_s(dia, "/"); strcat_s(dia, mes); strcat_s(dia, "/"); strcat_s(dia, anio); // DIA/MES/AÑO
				strcpy_s(fechaCompleta, dia);

				MessageBox(NULL, fechaCompleta, "Fecha", MB_OK);
				filtrarCita = filtrarCitas(selectedDate.wYear, selectedDate.wMonth, selectedDate.wDay); //Funcion para filtrar fechas

				if (filtrarCita == nullptr)
				{
					MessageBox(NULL, "No se encontraron citas en la fecha seleccionada", "Advertencia", MB_OK);
				}
				else
				{
					filtradorDeCitas = true; // Quiere decir que si se encontraron citas puestas ese día
					MessageBox(NULL, "Filtrando las fechas", "Advertencia", MB_OK);
					EndDialog(hwnd, 0);
					HWND hVentanaDeAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_AGENDA), hwnd, cVentanaAgenda);
					ShowWindow(hVentanaDeAgenda, SW_SHOW);
				}	

			}

		}

		if (LOWORD(wParam) == IDC_ELIMINAR && HIWORD(wParam) == BN_CLICKED) {
			if (indiceSeleccionado != LB_ERR) { //indiceSeleccionado existente
				
				nodoSeleccionado = buscarClientes(indiceSeleccionado); //Busca el nodo del indice seleccionado
				//Indica que el usuario seleccionó un elemento del listbox, por lo tanto se podrá eliminar ese nodo
				if (nodoSeleccionado != nullptr) { //Si existe el nodo
					int respuesta = MessageBox(hwnd, "¿Estás seguro de que deseas eliminar esta cita? Los datos no se podrán recuperar", "Advertencia", MB_YESNO | MB_ICONQUESTION);

					if (respuesta == IDYES) {
						eliminarCliente(indiceSeleccionado); //Si el usuario elige que sí, eliminará la cita que haya seleccionado
						filtradorDeCitas = false;
						EndDialog(hwnd, 0);
						HWND hVentanaAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_AGENDA), hwnd, cVentanaAgenda);
						ShowWindow(hVentanaAgenda, SW_SHOW);
					}
				}
				else {
					MessageBox(NULL, "No se encontró la cita", "Advertencia", MB_OK);
				}
				
			}
			else {
				MessageBox(NULL, "No seleccionó ningún elemento", "Advertencia", MB_OK);
			}
		}

	}break;
	case WM_CLOSE: {
		EndDialog(hwnd, 0);
		//Redireccionando a la ventana de registro
		HWND hRegistro = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN_WINDOW), hwnd, cVentanaRegistro);
		ShowWindow(hRegistro, SW_SHOW);
		return TRUE;
	}break;
	}
	return FALSE;
}

LRESULT CALLBACK cVentanaDetallesCita(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG: {

		HBITMAP hBitmap = GetDoctorHBitmap();

		if (hBitmap != NULL) {
			HWND hwndStatic = GetDlgItem(hwnd, IDC_FOTO_MANEJO);
			SendMessage(hwndStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
		}

		HWND hNombreDelDoctor = GetDlgItem(hwnd, IDC_NOMBRE_MANEJO);

		SetWindowText(hNombreDelDoctor, doctorActual->nombre);

		HWND hComboEspecieMascota = GetDlgItem(hwnd, IDC_COMBO_ESPECIE);
		HWND hComboEstatusCita = GetDlgItem(hwnd, IDC_ESTATUS);

		SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Perro");
		SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Felino");
		SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Ave");
		SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Reptil");
		SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Anfibio");
		SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Insecto");

		SendMessage(hComboEstatusCita, CB_ADDSTRING, 0, (LPARAM)"Pendiente");
		SendMessage(hComboEstatusCita, CB_ADDSTRING, 0, (LPARAM)"Efectuada");
		SendMessage(hComboEstatusCita, CB_ADDSTRING, 0, (LPARAM)"Cancelada");

		SYSTEMTIME selectedFecha, selectedHora;
		int fecha, hora;

		HWND hFecha = GetDlgItem(hwnd, IDC_DATETIME_FECHA); // hwnd es el identificador de la ventana principal
		HWND hHora = GetDlgItem(hwnd, IDC_DATETIME_HORA);
		fecha = SendDlgItemMessage(hwnd, IDC_DATETIME_FECHA, DTM_GETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&selectedFecha);
		hora = SendDlgItemMessage(hwnd, IDC_DATETIME_HORA, DTM_GETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&selectedHora);

		time_t tiempoActual = time(0);
		struct tm tiempoUsuario;

		time(&tiempoActual); //Obtiene la fecha y hora actual

		if (nodoSeleccionado != nullptr)
		{
			if (localtime_s(&tiempoUsuario, &tiempoActual) == 0) {
				tiempoUsuario.tm_year = nodoSeleccionado->anio - 1900;
				tiempoUsuario.tm_mon = nodoSeleccionado->mes - 1;
				tiempoUsuario.tm_mday = nodoSeleccionado->dia;
				tiempoUsuario.tm_hour = nodoSeleccionado->hora;
				tiempoUsuario.tm_min = nodoSeleccionado->minuto;
				tiempoUsuario.tm_sec = nodoSeleccionado->segundo;

				if (difftime(mktime(&tiempoUsuario), tiempoActual) < 0) {
					//double xd = difftime(mktime(&tiempoUsuario), tiempoActual);
					MessageBox(NULL, "La fecha y hora ingresadas son anteriores a la fecha y hora actual.", "Advertencia", MB_OK);
				}

			}
		}

		if ((llenarDatos && nodoSeleccionado->indiceEstatusCita == 2) || difftime(mktime(&tiempoUsuario), tiempoActual) < 0) // Si está cancelada la cita o ya pasó la fecha
		{
			HWND hNombreCliente = GetDlgItem(hwnd, IDC_NOMBRE_DUENO);
			HWND hTelefono = GetDlgItem(hwnd, IDC_TELEFONO);
			HWND hMotivoConsulta = GetDlgItem(hwnd, IDC_MOTIVO_CONSULTA);
			HWND hCosto = GetDlgItem(hwnd, IDC_COSTO); //Obtiene lo que tiene 'COSTO'
			HWND hNombreMascota = GetDlgItem(hwnd, IDC_NOMBRE_MASCOTA);

			HWND hComboEspecieMascota = GetDlgItem(hwnd, IDC_COMBO_ESPECIE); //Obtiene el combobox
			HWND hComboEstatusCita = GetDlgItem(hwnd, IDC_ESTATUS);
			//MessageBox(NULL, "Una cita cancelada no puede modificarse", "Advertencia", MB_OK);
			

			HWND hAplicar = GetDlgItem(hwnd, IDC_APLICAR_CITA);

			EnableWindow(hNombreCliente, FALSE);
			EnableWindow(hTelefono, FALSE);
			EnableWindow(hMotivoConsulta, FALSE);
			EnableWindow(hCosto, FALSE);
			EnableWindow(hNombreMascota, FALSE);
			EnableWindow(hComboEspecieMascota, FALSE);
			EnableWindow(hComboEstatusCita, FALSE);
			EnableWindow(hFecha, FALSE);
			EnableWindow(hHora, FALSE);
			EnableWindow(hAplicar, FALSE);
		}

	}break;
	case WM_COMMAND: {

		switch (LOWORD(wParam))
		{

		case ID_AGENDA:
		{
			llenarDatos = false;
			filtradorDeCitas = false;
			nodoSeleccionado = nullptr;
			//mostrarCitas = false;
			EndDialog(hwnd, 0);
			HWND hVentanaAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_AGENDA), hwnd, cVentanaAgenda);
			ShowWindow(hVentanaAgenda, SW_SHOW);
		}
		break;
		case ID_INFODOCTOR:
		{
			EndDialog(hwnd, 0);
			HWND hVentanaInfoDoctor = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_INFO_DOCTOR), hwnd, cVentanaInformacionDoctor);
			llenarDatosDoctor(hVentanaInfoDoctor);
			ShowWindow(hVentanaInfoDoctor, SW_SHOW);
		}
		break;
		case ID_MANEJODECITAS:
		{
			llenarDatos = false;
			filtradorDeCitas = false;
			nodoSeleccionado = nullptr;
			//mostrarCitas = false;
			EndDialog(hwnd, 0);
			HWND hManejoCitas = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DETALLES_CITA), hwnd, cVentanaDetallesCita);
			ShowWindow(hManejoCitas, SW_SHOW);
		}
		break;
		case ID_SALIR:
		{
			EndDialog(hwnd, 0);
			HWND hVentanaRegistro = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN_WINDOW), hwnd, cVentanaRegistro);
			ShowWindow(hVentanaRegistro, SW_SHOW);
		}
		break;
		}


		if (LOWORD(wParam) == IDC_APLICAR_CITA && HIWORD(wParam) == BN_CLICKED) {

			HWND hNombreCliente = GetDlgItem(hwnd, IDC_NOMBRE_DUENO);
			HWND hTelefono = GetDlgItem(hwnd, IDC_TELEFONO);
			HWND hMotivoConsulta = GetDlgItem(hwnd, IDC_MOTIVO_CONSULTA);
			HWND hCosto = GetDlgItem(hwnd, IDC_COSTO); //Obtiene lo que tiene 'COSTO'
			HWND hNombreMascota = GetDlgItem(hwnd, IDC_NOMBRE_MASCOTA);

			HWND hComboEspecieMascota = GetDlgItem(hwnd, IDC_COMBO_ESPECIE); //Obtiene el combobox
			HWND hComboEstatusCita = GetDlgItem(hwnd, IDC_ESTATUS);

			SYSTEMTIME selectedFecha, selectedHora;
			int fecha, hora;

			fecha = SendDlgItemMessage(hwnd, IDC_DATETIME_FECHA, DTM_GETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&selectedFecha);
			hora = SendDlgItemMessage(hwnd, IDC_DATETIME_HORA, DTM_GETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&selectedHora);

			//Obteniendo las longitudes de cada una para ver si contiene algo
			int iNombreClienteLength = GetWindowTextLength(hNombreCliente); //Obtiene la longitud del nombre del cliente
			int iTelefonoLength = GetWindowTextLength(hTelefono);
			int iMotivoConsultaLength = GetWindowTextLength(hMotivoConsulta);
			int iCostoLength = GetWindowTextLength(hCosto);
			int iNombreMascotaLength = GetWindowTextLength(hNombreMascota);

			//Creando variables para almacenar los valores de dichas strings
			char nombreCliente[50], telefono[50], motivoConsulta[300], costo[50], nombreMascota[50];

			//Obteniendo las longitudes
			GetWindowText(hNombreCliente, nombreCliente, iNombreClienteLength + 1);
			GetWindowText(hTelefono, telefono, iTelefonoLength + 1);
			GetWindowText(hMotivoConsulta, motivoConsulta, iMotivoConsultaLength + 1);
			GetWindowText(hCosto, costo, iCostoLength + 1);
			GetWindowText(hNombreMascota, nombreMascota, iNombreMascotaLength + 1);

			int indiceEspecieMascota = SendMessage(hComboEspecieMascota, CB_GETCURSEL, 0, 0); //Indice del combobox de la especie de mascota
			int indiceEstatusCita = SendMessage(hComboEstatusCita, CB_GETCURSEL, 0, 0);

			time_t tiempoActual = time(0);
			struct tm tiempoUsuario;
			
			time(&tiempoActual); //Obtiene la fecha y hora actual

			if (localtime_s(&tiempoUsuario, &tiempoActual) == 0) {
				tiempoUsuario.tm_year = selectedFecha.wYear - 1900;
				tiempoUsuario.tm_mon = selectedFecha.wMonth - 1;
				tiempoUsuario.tm_mday = selectedFecha.wDay;
				tiempoUsuario.tm_hour = selectedHora.wHour;
				tiempoUsuario.tm_min = selectedHora.wMinute;
				tiempoUsuario.tm_sec = selectedHora.wSecond;

				if (difftime(mktime(&tiempoUsuario), tiempoActual) < 0) {
					MessageBox(NULL, "La fecha y hora ingresadas son anteriores a la fecha y hora actual.", "Advertencia", MB_OK);
				}
				else {
					if (iNombreClienteLength < 1 || iTelefonoLength < 1 || iMotivoConsultaLength < 1 || iCostoLength < 1 || iNombreMascotaLength < 1
						|| indiceEspecieMascota == CB_ERR || indiceEstatusCita == CB_ERR) {
						MessageBox(NULL, "Debe rellenar todos los campos", "Advertencia", MB_OK);
					}
					else {

						bool validarNombre = true; // Validación de las letras en el nombre del cliente
						for (int i = 0; nombreCliente[i] != '\0'; i++)
						{
							if (!isalpha(nombreCliente[i]) && !isspace(nombreCliente[i])) {
								validarNombre = false;
								break;
							}
						}

						bool validarTelefono = true; // Validación de los números del teléfono
						for (int i = 0; telefono[i] != '\0'; i++)
						{
							if (!isdigit(telefono[i])) {
								validarTelefono = false;
								break;
							}
						}

						bool validarLongitud = true;
						if (strlen(telefono) != 10)
						{
							validarLongitud = false;
						}

						bool validarNombreMascota = true; // Validación de los números del teléfono
						for (int i = 0; nombreMascota[i] != '\0'; i++)
						{
							if (!isalpha(nombreMascota[i]) && !isspace(nombreMascota[i]))
							{
								validarNombreMascota = false;
								break;
							}
						}

						bool validarPrecio = true, punto = false;

						for (int i = 0; costo[i] != '\0'; i++)
						{
							if (costo[i] == '.' && !punto)
							{
								punto = true;
								continue;
							}
							else if (!isdigit(costo[i]))
							{
								validarPrecio = false;
								break;
							}
						}

						if (!validarNombre || !validarTelefono || !validarLongitud || !validarNombreMascota || !validarPrecio)
						{
							if (!validarNombre)
							{
								MessageBox(hwnd, "ERROR: El nombre del cliente sólo debe contener letras y espacios.", "Advertencia", MB_OK);
							}

							if (!validarTelefono)
							{
								MessageBox(hwnd, "ERROR: El número telefónico sólo debe estar compuesto de dígitos.", "Advertencia", MB_OK);
							}

							if (!validarLongitud)
							{
								MessageBox(hwnd, "ERROR: El número telefónico debe contener exactamente 10 dígitos.", "Advertencia", MB_OK);
							}

							if (!validarNombreMascota)
							{
								MessageBox(hwnd, "ERROR: El nombre de la mascota sólo debe contener letras y espacios.", "Advertencia", MB_OK);
							}

							if (!validarPrecio)
							{
								MessageBox(hwnd, "ERROR: El costo sólo debe estar compuesto de dígitos y un solo punto decimal.", "Advertencia", MB_OK);
							}

						}
						else
						{
							long long int telefonoCliente = atoll(telefono);
							float costoConsulta = atof(costo);

							insertarClientes(nombreCliente, telefonoCliente, nombreMascota, motivoConsulta, costoConsulta, indiceEspecieMascota, indiceEstatusCita, selectedFecha.wYear, selectedFecha.wMonth, selectedFecha.wDay, selectedHora.wHour, selectedHora.wMinute, selectedHora.wSecond);
						}
					}
				}

			}else {
				MessageBox(NULL, "Error al obtener la fecha y hora actual.", "Advertencia", MB_OK);
			}
		}

		if (LOWORD(wParam) == IDC_REGRESAR && HIWORD(wParam) == BN_CLICKED) {
			llenarDatos = false;
			filtradorDeCitas = false;
			nodoSeleccionado = nullptr;
			EndDialog(hwnd, 0); //DestroyWindow(hwnd); Por si suceden cositas
			//Redireccionando a la ventana de registro
			HWND hRegistro = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_AGENDA), hwnd, cVentanaAgenda);
			ShowWindow(hRegistro, SW_SHOW);
		}

	}break;
	case WM_CLOSE: {

		int respuesta = MessageBox(hwnd, "¿Estás seguro de que deseas cerrar esta ventana? Los datos no guardados no se verán reflejados", "Advertencia", MB_YESNO | MB_ICONQUESTION);

		if (respuesta == IDYES) {
			// Si el usuario elige "Sí", cierra la ventana	//DestroyWindow(hwnd);
			EndDialog(hwnd, 0); //DestroyWindow(hwnd); Por si suceden cositas
			//Redireccionando a la ventana de registro
			HWND hRegistro = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_AGENDA), hwnd, cVentanaAgenda);
			ShowWindow(hRegistro, SW_SHOW);
		}
		return TRUE;
	} break;
	}
	//llenarDatos = false;
	return FALSE;
}

LRESULT CALLBACK cVentanaInformacionDoctor(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		HBITMAP hBitmap = GetDoctorHBitmap();

		if (hBitmap != NULL) {
			HWND hwndStatic = GetDlgItem(hwnd, IDC_FOTO_DOCTOR);
			SendMessage(hwndStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
		}
	}break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{

		case ID_AGENDA:
		{
			EndDialog(hwnd, 0);
			HWND hVentanaAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_AGENDA), hwnd, cVentanaAgenda);
			ShowWindow(hVentanaAgenda, SW_SHOW);
		}
		break;
		case ID_INFODOCTOR:
		{
			EndDialog(hwnd, 0);
			HWND hVentanaInfoDoctor = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_INFO_DOCTOR), hwnd, cVentanaInformacionDoctor);
			llenarDatosDoctor(hVentanaInfoDoctor);
			ShowWindow(hVentanaInfoDoctor, SW_SHOW);
		}
		break;
		case ID_MANEJODECITAS:
		{
			EndDialog(hwnd, 0);
			HWND hManejoCitas = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DETALLES_CITA), hwnd, cVentanaDetallesCita);
			ShowWindow(hManejoCitas, SW_SHOW);
		}
		break;
		case ID_SALIR:
		{
			EndDialog(hwnd, 0);
			HWND hVentanaRegistro = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN_WINDOW), hwnd, cVentanaRegistro);
			ShowWindow(hVentanaRegistro, SW_SHOW);
		}
		break;
		}

		if (LOWORD(wParam) == IDC_CARGAR_FOTO)
		{
			OPENFILENAME ofn;
			char szFileName[MAX_PATH] = "";

			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = sizeof(szFileName);
			ofn.lpstrFilter = "Archivos de Imagen\0*.bmp;\0";
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

			if (GetOpenFileName(&ofn)) {
				// Ahora szFileName contiene la ruta del archivo seleccionado

				HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);

				if (hBitmap != NULL) {
					// Actualiza la variable global con el nuevo mapa de bits
					doctorHBitmapNueva = hBitmap;

					// Puedes mostrar la imagen en un control de la ventana si es necesario
					// ...
					MessageBox(hwnd, "Nueva imagen adjuntada exitosamente", "CARGA EXITOSA", MB_OK);
				}
				else {
					MessageBox(hwnd, "Error al cargar la nueva imagen", "ERROR", MB_OK | MB_ICONERROR);
				}
			}
		}

		if (LOWORD(wParam) == IDC_ACTUALIZAR_INFO_DOC && HIWORD(wParam) == BN_CLICKED)
		{
			HWND hFullName = GetDlgItem(hwnd, IDC_NOMBRE_DEL_DOCTOR);
			HWND hCedula = GetDlgItem(hwnd, IDC_CEDULA_DOCTOR);
			HWND hClaveUsuario = GetDlgItem(hwnd, IDC_CLAVE_USUARIO_DOCTOR);
			HWND hPassword = GetDlgItem(hwnd, IDC_CONTRASENA_DOCTOR); //Obtiene lo que tiene 'PASSWORD'

			//Obteniendo las longitudes de cada una para ver si contiene algo
			int iFullNameLength = GetWindowTextLength(hFullName); //Obtiene la longitud del nombre del doctor
			int iCedulaLength = GetWindowTextLength(hCedula);
			int iClaveUsuarioLength = GetWindowTextLength(hClaveUsuario);
			int iPasswordLength = GetWindowTextLength(hPassword);

			//Creando variables para almacenar los valores de dichas strings
			char nombre[50], cedula[50], claveUser[50], password[50];

			//Obteniendo las longitudes
			GetWindowText(hFullName, nombre, iFullNameLength + 1);
			GetWindowText(hCedula, cedula, iCedulaLength + 1);
			GetWindowText(hClaveUsuario, claveUser, iClaveUsuarioLength + 1);
			GetWindowText(hPassword, password, iPasswordLength + 1); //Guarda PASSWORD en "password"

			if (iFullNameLength < 1 || iCedulaLength < 1 || iClaveUsuarioLength < 1 || iPasswordLength < 1) {
				MessageBox(NULL, "Debe rellenar todos los campos", "Advertencia", MB_OK);
			}
			else {

				bool validarNombre = true; // Validación de las letras en el nombre del doctor
				for (int i = 0; nombre[i] != '\0'; i++)
				{
					if (!isalpha(nombre[i]) && !isspace(nombre[i])) {
						validarNombre = false;
						break;
					}
				}
				if (!validarNombre)
				{
					MessageBox(hwnd, "ERROR: El nombre solo debe contener letras y espacios.", "Advertencia", MB_OK);
				}
				else
				{
					llenarDatos = true; // Estableciendo que vamos a llenar datos
					insertarElementosDoctores(nombre, cedula, claveUser, password);
					MessageBox(NULL, "Cambios realizados", "Actualización exitosa", MB_OK);
					llenarDatos = false;
					EndDialog(hwnd, 0);
					HWND hVentanaInfoDoctor = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_INFO_DOCTOR), hwnd, cVentanaInformacionDoctor);
					llenarDatosDoctor(hVentanaInfoDoctor);
					ShowWindow(hVentanaInfoDoctor, SW_SHOW);
				}

			}
			// Borra el contenido de los EditBox
			SetWindowText(hFullName, ""); // Borra el contenido del EditBox de nombre
			SetWindowText(hCedula, ""); // Borra el contenido del EditBox de cédula
			SetWindowText(hClaveUsuario, ""); // Borra el contenido del EditBox de clave de usuario
			SetWindowText(hPassword, ""); // Borra el contenido del EditBox de contraseña
		}

		if (LOWORD(wParam) == IDC_ELIMINAR_DOCTOR && HIWORD(wParam) == BN_CLICKED)
		{
			int respuesta = MessageBox(hwnd, "¿Estás seguro de que deseas eliminar esta cuenta? Los datos no se podrán recuperar", "Advertencia", MB_YESNO | MB_ICONQUESTION);

			if (respuesta == IDYES) {
				eliminarDoctor(); //Si el usuario elige que sí, eliminará la cita que haya seleccionado

				EndDialog(hwnd, 0);
				HWND hVentanaInicio = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN_WINDOW), hwnd, cVentanaRegistro);
				ShowWindow(hVentanaInicio, SW_SHOW);
			}
		}

	} break;

	}

	llenarDatos = false;
	return FALSE;
}

void insertarElementosDoctores(char nombre[], char cedula[], char claveU[], char contrasena[]) {

	if (llenarDatos) // Si sí se van a actualizar los datos, se actualiza el nodo
	{
		Doctor* actualizarDoctor = doctorActual;

		strcpy_s(actualizarDoctor->nombre, nombre);
		strcpy_s(actualizarDoctor->cedula, cedula);
		strcpy_s(actualizarDoctor->clave, claveU);
		strcpy_s(actualizarDoctor->password, contrasena);
		
		doctorActual = actualizarDoctor;
	}
	else
	{
		Doctor* nuevoDoctor = new Doctor;
		strcpy_s(nuevoDoctor->nombre, nombre);
		strcpy_s(nuevoDoctor->cedula, cedula);
		strcpy_s(nuevoDoctor->clave, claveU);
		strcpy_s(nuevoDoctor->password, contrasena);

		nuevoDoctor->siguiente = nullptr;
		nuevoDoctor->anterior = nullptr;

		if (listaDoctores == nullptr) {
			// La lista está vacía, el nuevo doctor será el primero.
			listaDoctores = nuevoDoctor;
		}
		else {
			// Agrega el nuevo doctor al final de la lista.
			Doctor* ultimo = listaDoctores;
			while (ultimo->siguiente != nullptr) {
				ultimo = ultimo->siguiente;
			}
			nuevoDoctor->anterior = ultimo;
			ultimo->siguiente = nuevoDoctor;
		}
	}
	
}

Doctor* buscarDoctores(char clave[], char password[]) {
	Doctor* actual = listaDoctores;

	while (actual != nullptr) {
		if (strcmp(actual->clave, clave) == 0 && strcmp(actual->password, password) == 0)
			return actual; //Se encontró al doctor y se retorna el doctor actual
		actual = actual->siguiente;
	}
	return nullptr; //No se encontró al doctor
}

void eliminarDoctor()
{
	Doctor* nodoAEliminar = doctorActual;

	if (nodoAEliminar != nullptr) {
		if (nodoAEliminar->anterior != nullptr) {
			nodoAEliminar->anterior->siguiente = nodoAEliminar->siguiente;
		}
		else {
			// El nodo a eliminar es el primero de la lista
			listaDoctores = nodoAEliminar->siguiente;
			if (listaDoctores != nullptr) {
				listaDoctores->anterior = nullptr;
			}
		}

		if (nodoAEliminar->siguiente != nullptr) {
			nodoAEliminar->siguiente->anterior = nodoAEliminar->anterior;
		}

		delete nodoAEliminar;

		MessageBox(NULL, "Doctor eliminado", "Eliminación exitosa", MB_OK);
	}
	else {
		MessageBox(NULL, "Doctor no encontrado", "Error", MB_OK);
	}
}

void llenarDatosDoctor(HWND hwnd)
{
	HWND hNombreDoctor = GetDlgItem(hwnd, IDC_NOMBRE_DEL_DOCTOR);
	HWND hCedula = GetDlgItem(hwnd, IDC_CEDULA_DOCTOR);
	HWND hClaveUsuario = GetDlgItem(hwnd, IDC_CLAVE_USUARIO_DOCTOR);
	HWND hPassword = GetDlgItem(hwnd, IDC_CONTRASENA_DOCTOR);

	// Asignando todos los valores a sus respecctivos lugares
	Doctor* doctorSeleccionado = doctorActual;

	SetWindowText(hNombreDoctor, doctorSeleccionado->nombre);
	SetWindowText(hCedula, doctorSeleccionado->cedula);
	SetWindowText(hClaveUsuario, doctorSeleccionado->clave);
	SetWindowText(hPassword, doctorSeleccionado->password);
	//Nombre de doctor en el texto estático
	HWND hNombreDelDoctor = GetDlgItem(hwnd, IDC_NAME_DOCTOR);

	SetWindowText(hNombreDelDoctor, doctorSeleccionado->nombre);
}

void SetDetallesCitaData(HWND hwnd) {
	//Llenando todo el mugrero
	HWND hNombreCliente = GetDlgItem(hwnd, IDC_NOMBRE_DUENO);
	HWND hTelefono = GetDlgItem(hwnd, IDC_TELEFONO);
	HWND hMotivoConsulta = GetDlgItem(hwnd, IDC_MOTIVO_CONSULTA);
	HWND hCosto = GetDlgItem(hwnd, IDC_COSTO); //Obtiene lo que tiene 'COSTO'
	HWND hNombreMascota = GetDlgItem(hwnd, IDC_NOMBRE_MASCOTA);

	HWND hComboEspecieMascota = GetDlgItem(hwnd, IDC_COMBO_ESPECIE); //Obtiene el combobox
	HWND hEstatusCita = GetDlgItem(hwnd, IDC_ESTATUS);

	/*SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Perro");
	SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Felino");
	SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Ave");
	SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Reptil");
	SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Anfibio");
	SendMessage(hComboEspecieMascota, CB_ADDSTRING, 0, (LPARAM)"Insecto");

	SendMessage(hEstatusCita, CB_ADDSTRING, 0, (LPARAM)"Pendiente");
	SendMessage(hEstatusCita, CB_ADDSTRING, 0, (LPARAM)"Efectuada");
	SendMessage(hEstatusCita, CB_ADDSTRING, 0, (LPARAM)"Cancelada");*/

	SYSTEMTIME cargarFecha;
	cargarFecha.wYear = nodoSeleccionado->anio; // Año //nodoSeleccionado->anio
	cargarFecha.wMonth = nodoSeleccionado->mes; // Mes
	cargarFecha.wDay = nodoSeleccionado->dia; // Día
	//NO CARGAN, CHECALO
	SYSTEMTIME cargarHora;
	cargarHora.wHour = nodoSeleccionado->hora; // Hora
	cargarHora.wMinute = nodoSeleccionado->minuto; // Minutos
	cargarHora.wSecond = nodoSeleccionado->segundo;

	if (nodoSeleccionado != nullptr) {
		char telefono[50], costo[50];
		snprintf(telefono, sizeof(telefono), "%lld", nodoSeleccionado->telefonoCliente); //El teléfono del cliente se transforma a char
		sprintf_s(costo, "%.2f", nodoSeleccionado->costoConsulta); //El costo de la consula se transforma a char
		// Rellena los controles con los datos del nodo seleccionado
		SetWindowText(hNombreCliente, nodoSeleccionado->nombreCliente);
		SetWindowText(hTelefono, telefono);
		SetWindowText(hMotivoConsulta, nodoSeleccionado->motivoConsulta);
		SetWindowText(hCosto, costo);
		SetWindowText(hNombreMascota, nodoSeleccionado->nombreMascota);
		//Indice del combobox (espcie de la mascota)
		SendMessage(hComboEspecieMascota, CB_SETCURSEL, (WPARAM)nodoSeleccionado->indiceEspecieMascota, NULL);
		SendMessage(hEstatusCita, CB_SETCURSEL, (WPARAM)nodoSeleccionado->indiceEstatusCita, NULL);
		/*DateTime_SetSystemtime(GetDlgItem(hwnd, IDC_DATETIME_FECHA), GDT_VALID, &cargarFecha);
		DateTime_SetSystemtime(GetDlgItem(hwnd, IDC_DATETIME_HORA), GDT_VALID, &cargarHora);*/
		char anio[10], mes[10], dia[15], horas[15], minutos[10], segundos[10];
		_itoa_s(cargarFecha.wYear, anio, 10);
		_itoa_s(cargarFecha.wMonth, mes, 10);
		_itoa_s(cargarFecha.wDay, dia, 10);
		_itoa_s(cargarHora.wHour, horas, 10);
		_itoa_s(cargarHora.wMinute, minutos, 10);
		_itoa_s(cargarHora.wSecond, segundos, 10);

		char fechaCompleta[15], horaCompleta[15];
		strcat_s(dia, "/"); strcat_s(dia, mes); strcat_s(dia, "/"); strcat_s(dia, anio); // DIA/MES/AÑO
		strcpy_s(fechaCompleta, dia);
		strcat_s(horas, ":"); strcat_s(horas, minutos); strcat_s(horas, ":"); strcat_s(horas, segundos); // HORA:MINUTOS:SEGUNDOS
		strcpy_s(horaCompleta, horas);

		HWND hTextoFecha = GetDlgItem(hwnd, IDC_STATIC_FECHA);
		HWND hTextoHora = GetDlgItem(hwnd, IDC_STATIC_HORA);

		SetWindowText(hTextoFecha, fechaCompleta);
		SetWindowText(hTextoHora, horaCompleta);
	}
}

void insertarClientes(char nombreCliente[], long long int telefonoCliente, char nombreMascota[], char motivoConsulta[], float costo, int indiceEspecieMascota, int indiceEstatusCita, int anio, int mes, int dia, int hora, int minuto, int segundo) {
	int index = -1; // Inicializado como un nuevo nodo

	if (llenarDatos) {
		index = nodoSeleccionado->indiceDeCita; // Viene de un nodo ya existente
		//MessageBox(NULL, "Se encontró el maldito nodo", "Si", MB_OK);
		if (filtradorDeCitas)
		{
			index = longitud; // Si se van a filtrar las citas vamos a disminuir el indice de la cita en uno, ya que es un arreglo de citas que va a empezar desde dicha posicion
		}
	}

	// Buscar si el nodo ya existe en la lista
	Clientes* nodoExistente = buscarClientes(index);

	if (nodoExistente != nullptr) { // Si se encuentra un nodo con el mismo nombre de mascota, sobreescribe los datos
		strcpy_s(nodoExistente->nombreCliente, nombreCliente);
		nodoExistente->telefonoCliente = telefonoCliente;
		strcpy_s(nodoExistente->nombreMascota, nombreMascota);
		strcpy_s(nodoExistente->motivoConsulta, motivoConsulta);
		nodoExistente->costoConsulta = costo;
		nodoExistente->indiceEspecieMascota = indiceEspecieMascota;
		nodoExistente->indiceEstatusCita = indiceEstatusCita;
		nodoExistente->anio = anio;
		nodoExistente->mes = mes;
		nodoExistente->dia = dia;
		nodoExistente->hora = hora;
		nodoExistente->minuto = minuto;
		nodoExistente->segundo = segundo;
		nodoExistente->fechaEnOrden = anio * 10000000000LL + //Dato para obtener la fecha en un formato de 20231120151100
			mes * 100000000 +																		// añoMesDiaHoraMinutosSegundos
			dia * 1000000 +
			hora * 10000 +
			minuto * 100 +
			segundo;
		MessageBox(NULL, "Datos modificados", "Advertencia", MB_OK);
	}
	else { // Si no se encuentra un nodo existente, crea uno nuevo
		Clientes* nuevo_cliente = new Clientes;
		strcpy_s(nuevo_cliente->nombreCliente, nombreCliente);
		nuevo_cliente->telefonoCliente = telefonoCliente;
		strcpy_s(nuevo_cliente->nombreMascota, nombreMascota);
		strcpy_s(nuevo_cliente->motivoConsulta, motivoConsulta);
		nuevo_cliente->costoConsulta = costo;
		nuevo_cliente->indiceEspecieMascota = indiceEspecieMascota;
		nuevo_cliente->indiceDeCita = indiceGlobalDeCitas;
		nuevo_cliente->indiceEstatusCita = indiceEstatusCita;
		nuevo_cliente->anio = anio;
		nuevo_cliente->mes = mes;
		nuevo_cliente->dia = dia;
		nuevo_cliente->hora = hora;
		nuevo_cliente->minuto = minuto;
		nuevo_cliente->segundo = segundo;
		nuevo_cliente->fechaEnOrden = anio * 10000000000LL + //Dato para obtener la fecha en un formato de 20231120151100
			mes * 100000000 +																		// añoMesDiaHoraMinutosSegundos
			dia * 1000000 +
			hora * 10000 +
			minuto * 100 +
			segundo;
		/*char fechaGrande[50];
		snprintf(fechaGrande, sizeof(fechaGrande), "%lld", nuevo_cliente->fechaEnOrden);
		MessageBox(NULL, fechaGrande, "Fecha en número long long", MB_OK);*/

		nuevo_cliente->siguiente = nullptr;
		nuevo_cliente->anterior = nullptr;

		indiceGlobalDeCitas++;

		if (listaClientes == nullptr) {
			listaClientes = nuevo_cliente;
		}
		else {
			Clientes* ultimo = listaClientes;
			while (ultimo->siguiente != nullptr) {
				ultimo = ultimo->siguiente;
			}
			nuevo_cliente->anterior = ultimo;
			ultimo->siguiente = nuevo_cliente;
		}
		MessageBox(NULL, "Cita agendada", "Registro exitoso", MB_OK);
	}
	
	llenarDatos = false;
	//guardarDatosEnArchivoBinario();
}

Clientes* buscarClientes(int indiceSeleccionado) {
	if (filtradorDeCitas)
	{
		longitud = indiceSeleccionado;
		Clientes* filtroActual = listaClientes;

		while (filtroActual != nullptr) {
			if (filtroActual->indiceDeCita == filtrarCita[indiceSeleccionado].indiceDeCita) {
				// Aquí has encontrado la cita deseada, puedes devolver este nodo
				return filtroActual;
			}
			filtroActual = filtroActual->siguiente; // Avanza al siguiente nodo
		}

		// Si no se encontró ninguna coincidencia, puedes devolver nullptr o manejar el caso en consecuencia
		return nullptr;
	}
	else
	{
		Clientes* actual = listaClientes;

		/*char indicetilin[50], indiceActual[50];
		_itoa_s(indiceSeleccionado, indicetilin, 10);
		MessageBox(NULL, indicetilin, "Indice Seleccionado de la listbox", MB_OK);*/

		if (mostrarCitas)
		{
			if (!llenarDatos)
			{
				indiceSeleccionado += tamanio;
			}
			
		}

		while (actual != nullptr) {
			if (actual->indiceDeCita == indiceSeleccionado) {
				/*_itoa_s(actual->indiceDeCita, indiceActual, 10);
				MessageBox(NULL, indiceActual, "Indice de la lista", MB_OK);*/
				return actual;	//Se encontró a la mascota
			}
			actual = actual->siguiente;
		}

		return nullptr;
	}
	
}

void eliminarCliente(int indiceSeleccionado) {
	Clientes* nodoAEliminar = buscarClientes(indiceSeleccionado);

	if (nodoAEliminar != nullptr) {
		if (nodoAEliminar->anterior != nullptr) {
			nodoAEliminar->anterior->siguiente = nodoAEliminar->siguiente;
		}
		else {
			// El nodo a eliminar es el primero de la lista
			listaClientes = nodoAEliminar->siguiente;
			if (listaClientes != nullptr) {
				listaClientes->anterior = nullptr;
			}
		}

		if (nodoAEliminar->siguiente != nullptr) {
			nodoAEliminar->siguiente->anterior = nodoAEliminar->anterior;
		}

		delete nodoAEliminar;
		//Actualizando indices
		Clientes* actualizar = listaClientes;
		while (actualizar != nullptr) {
			if (actualizar->indiceDeCita != 0) {
				actualizar->indiceDeCita--;
			}
			actualizar = actualizar->siguiente;
		}
		indiceGlobalDeCitas--;
		MessageBox(NULL, "Cliente eliminado", "Eliminación exitosa", MB_OK);
	}
	else {
		MessageBox(NULL, "Cliente no encontrado", "Error", MB_OK);
	}
}


void ordenarCitas()
{
	Clientes* auxArr = listaClientes;
	int lng = 0;
	//longuitud de la lista ligada
	while (auxArr != nullptr) {
		lng++;
		auxArr = auxArr->siguiente;
	}

	Clientes** arreLista = new Clientes * [lng];

	auxArr = listaClientes;

	//copia la lista al arreglo
	for (int i = 0; i < lng; i++)
	{
		arreLista[i] = auxArr;
		auxArr = auxArr->siguiente;
	}

	quickSort(arreLista, 0, lng - 1);

	Clientes* temp = arreLista[0];
	arreLista[0]->indiceDeCita = 0;
	for (int i = 1; i < lng; i++) {
		arreLista[i]->indiceDeCita = i;
		temp->siguiente = arreLista[i];
		arreLista[i]->anterior = temp;
		temp = temp->siguiente;
	}
	temp->siguiente = nullptr; // El último nodo de la lista ordenada

	listaClientes = arreLista[0]; // Actualizar el inicio de listaClientes
	listaClientes->anterior = nullptr;
}


long long partition(Clientes* arr[], int start, int end) {

	//int indice = end-1;
	long long pivot = arr[end]->fechaEnOrden;

	int i = (start - 1);
	for (int j = start; j <= end; j++) {
		if (arr[j]->fechaEnOrden < pivot) {
			i++;
			swap(arr[i], arr[j]);
		}
	}
	swap(arr[i + 1], arr[end]);
	return (i + 1);
}

void quickSort(Clientes* arr[], int start, int end) {

	if (start < end) {
		long long pi = partition(arr, start, end);
		quickSort(arr, start, pi - 1);
		quickSort(arr, pi + 1, end);

		//print(arr, end + 1);
	}
}

void swap(Clientes*& a, Clientes*& b)
{
	Clientes* temp = b;
	b = a;
	a = temp;
}

Clientes* filtrarCitas(int anio, int mes, int dia)
{
	Clientes* filtro = listaClientes;
	//int
	longitud = 0;

	while (filtro != nullptr)
	{
		if (filtro->anio == anio && filtro->mes == mes && filtro->dia == dia)
		{
			longitud++; // Para saber cuantas citas coinciden con las fechas
		}

		filtro = filtro->siguiente;
	}

	if (longitud == 0) {
		return nullptr;
	}
	
	Clientes* arregloFiltro = new Clientes [longitud]; // Creando un arreglo con la longitud de los nodos que coinciden con dicha fecha
	int indice = 0;
	filtro = listaClientes;

	while (filtro != nullptr && indice < longitud) {
		if (filtro->anio == anio && filtro->mes == mes && filtro->dia == dia) {
			//filtro->indiceDeCita = indice; // Igualando los nodos filtrados para que se puedan visualizar con éxito en la lista
			arregloFiltro[indice] = *filtro; // Guardando los nodos coincidentes en un arreglo
			indice++;
		}
		filtro = filtro->siguiente;
	}

	return arregloFiltro;
}


void guardarDatosEnArchivoBinario() {
	std::ofstream archivoBinario("C:\\Users\\raula\\Downloads\\PIA DEFINITIVO\\PIA_PA\\PIA_PA\\datos.bin", std::ios::binary | std::ios::out);

	if (!archivoBinario.is_open()) {
		// Manejo de error si no se puede abrir el archivo
		MessageBox(NULL, "Error al abrir el archivo para guardar doctores.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	// Guarda el tamaño de la listaClientes
	int cantidadClientes = 0;
	Clientes* actual = listaClientes;
	while (actual != nullptr) {
		cantidadClientes++;
		actual = actual->siguiente;
	}
	archivoBinario.write(reinterpret_cast<const char*>(&cantidadClientes), sizeof(int));

	// Guarda cada cliente en el archivo
	actual = listaClientes;
	while (actual != nullptr) {
		archivoBinario.write(reinterpret_cast<const char*>(actual), sizeof(Clientes));
		actual = actual->siguiente;
	}

	archivoBinario.close();
}

void cargarDatosDesdeArchivoBinario() {
	std::ifstream archivoBinario("datos.bin", std::ios::binary | std::ios::in);

	if (!archivoBinario.is_open()) {
		// Manejo de error si no se puede abrir el archivo
		return;
	}

	// Limpiar la listaClientes antes de cargar nuevos datos
	while (listaClientes != nullptr) {
		Clientes* temp = listaClientes;
		listaClientes = listaClientes->siguiente;
		delete temp;
	}

	// Lee la cantidad de clientes desde el archivo
	int cantidadClientes;
	archivoBinario.read(reinterpret_cast<char*>(&cantidadClientes), sizeof(int));

	// Lee cada cliente desde el archivo y agrega a listaClientes
	Clientes clienteLeido;
	for (int i = 0; i < cantidadClientes; ++i) {
		 // Mejor con listaClientes
		archivoBinario.read(reinterpret_cast<char*>(&clienteLeido), sizeof(Clientes));

		Clientes* nuevo_cliente = new Clientes(clienteLeido);

		nuevo_cliente->siguiente = nullptr;
		nuevo_cliente->anterior = nullptr;

		indiceGlobalDeCitas++;

		if (listaClientes == nullptr) {
			listaClientes = nuevo_cliente;
		}
		else {
			Clientes* ultimo = listaClientes;
			while (ultimo->siguiente != nullptr) {
				ultimo = ultimo->siguiente;
			}
			nuevo_cliente->anterior = ultimo;
			ultimo->siguiente = nuevo_cliente;
		}

	}

	archivoBinario.close();
}


void guardarDoctoresEnArchivoBinario() {
	std::ofstream archivoBinario("C:\\Users\\raula\\Downloads\\PIA DEFINITIVO\\PIA_PA\\PIA_PA\\doctores.bin", std::ios::binary | std::ios::out);

	if (!archivoBinario.is_open()) {
		// Manejo de error si no se puede abrir el archivo
		MessageBox(NULL, "Error al abrir el archivo para guardar doctores.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	// Mensaje de éxito al abrir el archivo
	//MessageBox(NULL, "Archivo abierto con éxito.", "Éxito", MB_OK | MB_ICONINFORMATION);

	// Escribir los doctores en el archivo binario
	Doctor* actual = listaDoctores;
	while (actual != nullptr) {
		archivoBinario.write(reinterpret_cast<const char*>(actual), sizeof(Doctor));
		actual = actual->siguiente;
	}

	// Mensaje de éxito al guardar los datos en el archivo
	//MessageBox(NULL, "Datos guardados en el archivo.", "Éxito", MB_OK | MB_ICONINFORMATION);

	archivoBinario.close();
}

void cargarDoctoresDesdeArchivoBinario() {
	std::ifstream archivoBinario("doctores.bin", std::ios::binary | std::ios::in);

	if (!archivoBinario.is_open()) {
		// Manejo de error si no se puede abrir el archivo
		return;
	}

	// Limpiar la listaDoctores antes de cargar nuevos datos
	// (asumiendo que listaDoctores es una lista doblemente ligada)
	while (listaDoctores != nullptr) {
		Doctor* temp = listaDoctores;
		listaDoctores = listaDoctores->siguiente;
		delete temp;
	}

	// Leer cada doctor desde el archivo y agregarlo a la listaDoctores
	Doctor doctorLeido;
	while (archivoBinario.read(reinterpret_cast<char*>(&doctorLeido), sizeof(Doctor))) {
		Doctor* nuevoDoctor = new Doctor;
		memcpy(nuevoDoctor, &doctorLeido, sizeof(Doctor));

		nuevoDoctor->siguiente = listaDoctores;
		nuevoDoctor->anterior = nullptr;
		if (listaDoctores != nullptr) {
			listaDoctores->anterior = nuevoDoctor;
		}
		listaDoctores = nuevoDoctor;
	}

	archivoBinario.close();
}