#include <iostream>
#include <cstdio>
#include <cstring> 

using namespace std;

const  int K = 5; 

struct ComandaHistorica{
    char fecha[11];
    char nombreMozo[50];
    int codigoProducto;
    int cantidad; 
    float comision;
};

struct Mozo {
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};

void encriptar( char clave []){
for (int i = 0; clave[i] != '\0'; i++){
    clave[i] += K;
   }
}

int main() {
    FILE* archivo = fopen( "comandas_historicas.dat", "rb");

    if (archivo == NULL) {
        cout << "No se pudo abrir comandas_historicas.dat" << endl;
        return 1;
    }

    Mozo mozos[100];
    int cantidadMozos = 0;

    ComandaHistorica comanda;

    while (fread(&comanda, sizeof(ComandaHistorica), 1, archivo) == 1){

        int posicion = -1;
        // buscamos si el mozo ya existe
        for (int i = 0; i < cantidadMozos; i++){
            if (strcmp(mozos[i].nombre, comanda.nombreMozo) == 0){
                posicion = i;
                break;
            }
        }
        // si el mozo no exite, lo creamos
        if (posicion == -1) {
            mozos[cantidadMozos].idMozo = cantidadMozos + 1;
            strcpy(mozos[cantidadMozos].nombre, comanda.nombreMozo);
            mozos[cantidadMozos].totalComision= comanda.comision;
            // la contraseña inicial es el ID convertido a texto
            char clave[20];

            sprintf(clave, "%d", mozos[cantidadMozos].idMozo);

            // encriptamos la contraseña con K=5
            encriptar(clave);

            strcpy(mozos[cantidadMozos].password, clave);

            cantidadMozos++;

        } else{
            // si ya existe, acumulamos su comision
            mozos[posicion].totalComision += comanda.comision;
        }
    }

    fclose(archivo);

    //creamos el archivo de mozos
    FILE* archivoMozos = fopen("mozos.dat", "wb");

    if (archivoMozos == NULL){
        cout << "No se pudo crear mozos.dat" << endl;
        return 1;
    }

    // guardamos todos los mozos 
    for (int i = 0; i < cantidadMozos; i++){
        fwrite(&mozos[i], sizeof(Mozo), 1, archivoMozos);
    }
    fclose(archivoMozos);

    cout << "Mozos generados:" << endl;

    for (int i = 0; i < cantidadMozos; i++){
        cout << "ID: " << mozos[i].idMozo << endl;
        cout << "Nombre: " << mozos[i].nombre << endl;
        cout << "Password: " << mozos[i].password << endl;
        cout << "Comision: " << mozos[i].totalComision << endl;
        cout << "------------------------" << endl;
    }
    
    return 0;


    }