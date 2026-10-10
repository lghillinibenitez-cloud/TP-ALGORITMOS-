#include <iostream>
#include <cstdio>

using namespace std;

// estructura que uso para guardar los datos
struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

void copiar_texto(char destino[], const char origen[]) {
    int i = 0;
    while (origen[i] != '\0') {
        destino[i] = origen[i];  
                                 
        i++;
    }
    destino[i] = '\0';
}
 
void pegar_al_final(char destino[], const char origen[]) {
    int i = 0;
    while (destino[i] != '\0') {
        i++;
    } //cuando i llega al caracter vacio se romple el while 
    int j = 0;
    while (origen[j] != '\0') { // despues de buscar el final de la plabra pego el texto nuevo
        destino[i] = origen[j];
        i++;
        j++;
    } // el nuevo destino arranca en el ultimo caracter de i y sigue con el primer origen de j
    destino[i] = '\0'; 
}

void armarNombreDia(char resultado[], const char fecha[]) {
    copiar_texto(resultado, "comandas_");
    pegar_al_final(resultado, fecha);
    pegar_al_final(resultado, ".dat");
}
//arma el nombre del archivo de toda la semana 
void armarNombreSemana(char resultado[], const char semanaMes[]) {
    copiar_texto(resultado, "comandas_semana_");
    pegar_al_final(resultado, semanaMes);
    pegar_al_final(resultado, ".dat");
}


int main() {
    // Un maximo logico de dias que puede tener una semana
    const int MAX_DIAS = 7; 
    
    char identificadorSemana[20];
    char nombreArchivoSemana[60];
    int cantidadDias = 0;

    cout << "========================================" << endl;
    cout << "       BUFFET ALBERT EINSTEIN           " << endl;
    cout << "      MODULO DE CIERRE SEMANAL          " << endl;
    cout << "========================================" << endl;

    cout << "Ingrese identificador de semana (ej: s1-06): ";
    cin >> identificadorSemana;
    armarNombreSemana(nombreArchivoSemana, identificadorSemana);

    cout << "Cuantos dias desea consolidar? (maximo 7): ";
    cin >> cantidadDias;
      while (cantidadDias > MAX_DIAS || cantidadDias < 1) { //verificar que la cantidad de dias no supere 7 dias y no sea menor a 1
        cout << "supera la cantidad de dias por favor ponga un numero del 1 al 7: ";
        cin >> cantidadDias;
    }

    
    FILE* archivosDias[MAX_DIAS]; 
    Comanda registroActual[MAX_DIAS];
    bool archivoActivo[MAX_DIAS];
    int diasValidos = 0;

    
    for (int d = 0; d < cantidadDias; d++) {
        char fechaDia[15];
        char nombreArchivoDia[60];

        cout << "Ingrese fecha del dia " << (d + 1) << " (formato DD-MM-AAAA): ";
        cin >> fechaDia;
        armarNombreDia(nombreArchivoDia, fechaDia);

        FILE* f = fopen(nombreArchivoDia, "rb");// busco el archivo y guardo la direccion de memoria 
        if (f != NULL) {
            
            if (fread(&registroActual[diasValidos], sizeof(Comanda), 1, f) == 1) {
                archivosDias[diasValidos] = f;
                archivoActivo[diasValidos] = true;
                diasValidos++;
                cout << "-> Archivo '" << nombreArchivoDia << "' listo para apareo." << endl;
            } else {
                fclose(f); 
                cout << "-> Archivo '" << nombreArchivoDia << "' esta vacio. Se omite." << endl;
            }
        } else {
            cout << "-> Aviso: No se encontro '" << nombreArchivoDia << "'." << endl;
        }
    }

    if (diasValidos == 0) {
        cout << "\nNo hay datos para procesar. Fin del programa." << endl;
        return 0;
    }

    
    FILE* fSemana = fopen(nombreArchivoSemana, "wb");
    if (fSemana == NULL) {//condicion por si ocurre un error al crear un archivo
        cout << "Error critico al crear " << nombreArchivoSemana << endl;
        return 1;
    }

    bool quedanDatos = true;
    int totalComandas = 0;

    while (quedanDatos) {
        quedanDatos = false;
        int indiceMinimo = -1;// guardo la ubicacion donde esta el minMozo
        int minMozo = 9999999;// guardo el valor del mozo mas chico

        // Buscamos cual de los archivos activos tiene el idMozo mas chico
        for (int i = 0; i < diasValidos; i++) {
            if (archivoActivo[i]) {
                quedanDatos = true;
                if (registroActual[i].idMozo < minMozo) {
                    minMozo = registroActual[i].idMozo;
                    indiceMinimo = i;
                }
            }
        }

        // Si encontramos un dato menor, lo grabamos y avanzamos SOLO ese archivo
        if (quedanDatos && indiceMinimo != -1) {
            fwrite(&registroActual[indiceMinimo], sizeof(Comanda), 1, fSemana);
            totalComandas++;

            // Avanzamos leyendo el siguiente registro del mismo archivo
            if (fread(&registroActual[indiceMinimo], sizeof(Comanda), 1, archivosDias[indiceMinimo]) != 1) { //si no hay mas datos para leer 
                
                archivoActivo[indiceMinimo] = false;
                fclose(archivosDias[indiceMinimo]); //cierro el archivo y libero memoria
            }
        }
    }

    fclose(fSemana);  

    cout << "\n========================================" << endl;
    cout << "Cierre completado mediante Apareo." << endl;
    cout << "Archivo generado: " << nombreArchivoSemana << endl;
    cout << "Total de ventas consolidadas: " << totalComandas << endl;
    cout << "========================================" << endl;

    return 0;
}
