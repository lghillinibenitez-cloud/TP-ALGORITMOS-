#include <iostream>
#include <cstdio>

using namespace std;

// Estructura de la comanda
struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

int main() {
    char nombreArchivo[100];

    cout << "==========================================" << endl;
    cout << "   BUFFET ALBERT EINSTEIN - RESUMEN      " << endl;
    cout << "==========================================" << endl;

    cout << "Ingrese el nombre del archivo semanal (ej: comandas_semana_s1-06.dat): ";
    cin >> nombreArchivo;

    FILE* archivo = fopen(nombreArchivo, "rb");
    if (archivo == NULL) {
        cout << "\nError: No se pudo abrir el archivo " << nombreArchivo << endl;
        cout << "Verifique que el archivo exista en la carpeta actual." << endl;
        return 1;
    }

    Comanda reg;
    int totalProductosBuffet = 0;

    //primer registro
    size_t leidos = fread(&reg, sizeof(Comanda), 1, archivo);

    if (leidos == 1) {
        cout << "\n------------------------------------------" << endl;
        cout << "DETALLE DE VENTAS POR MOZO:" << endl;
        cout << "------------------------------------------" << endl;

        //corte de control principal
        while (leidos == 1) {
            int mozoActual = reg.idMozo;
            int productosMozo = 0;
            float comisionMozo = 0.0f;

            //acumula ventas del mismo mozo
            while (leidos == 1 && reg.idMozo == mozoActual) {
                productosMozo += reg.cantidad;
                comisionMozo += reg.comision;

                // lectura del siguiente registro
                leidos = fread(&reg, sizeof(Comanda), 1, archivo);
            }

            //totales acumulados por mozo
            cout << "Mozo ID: " << mozoActual << endl;
            cout << "  - Productos vendidos: " << productosMozo << endl;
            cout << "  - Comision a pagar:   $" << comisionMozo << endl;
            cout << "------------------------------------------" << endl;

            // suma al total general del buffet
            totalProductosBuffet += productosMozo;
        }

        // total general del buffet
        cout << "==========================================" << endl;
        cout << "TOTAL DE PRODUCTOS VENDIDOS POR EL BUFFET: " << totalProductosBuffet << endl;
        cout << "==========================================" << endl;

    } else {
        cout << "\nEl archivo semanal esta vacio." << endl;
    }

    fclose(archivo);
    return 0;
}
