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

// structs agregados para la parte b
struct Producto {
    int codigo;
    char descripcion[50];
    float precio;
    int stockActual;
};

struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

// struct auX para ordenar las ventas
struct VentaAux {
    char fecha[11];
    Comanda comanda;
};

void encriptar( char clave []){
for (int i = 0; clave[i] != '\0'; i++){
    clave[i] += K;
   }
}

// actualiza el stock directo en inventario.dat sin cargar toda la lista:
// busca el producto, retrocede un registro con fseek y pisa solo ese registro
bool descontarStock(int codProd, int cantidad) {
    FILE* arch = fopen("inventario.dat", "rb+");

    if (arch == NULL) {
        return false;
    }

    Producto p;
    bool encontrado = false;

    while (!encontrado && fread(&p, sizeof(Producto), 1, arch) == 1) {
        if (p.codigo == codProd) {
            p.stockActual -= cantidad;

            fseek(arch, -static_cast<long>(sizeof(Producto)), SEEK_CUR);
            fwrite(&p, sizeof(Producto), 1, arch);
            encontrado = true;
        }
    }

    fclose(arch);
    return encontrado;
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

    // ---------------------------------------------------------
    // ACA EMPIEZA LA PARTE B 
    // ---------------------------------------------------------

    // 1 volvemos a abrir comandas para procesar las ventas
    FILE* archComandas = fopen("comandas_historicas.dat", "rb");
    VentaAux ventas[1000];
    int cantVentas = 0;
    ComandaHistorica comVieja;

    if (archComandas != NULL) {
        while (fread(&comVieja, sizeof(ComandaHistorica), 1, archComandas) == 1) {
            // copiamos los datos al struct nuevo
            strcpy(ventas[cantVentas].fecha, comVieja.fecha);
            ventas[cantVentas].comanda.codigoProducto = comVieja.codigoProducto;
            ventas[cantVentas].comanda.cantidad = comVieja.cantidad;
            ventas[cantVentas].comanda.comision = comVieja.comision;

            // buscamos el id del mozo comparando los nombres
            for (int i = 0; i < cantidadMozos; i++) {
                if (strcmp(mozos[i].nombre, comVieja.nombreMozo) == 0) {
                    ventas[cantVentas].comanda.idMozo = mozos[i].idMozo;
                    break;
                }
            }

            // descontamos el stock directamente en inventario.dat
            if (!descontarStock(comVieja.codigoProducto, comVieja.cantidad)) {
                cout << "[AVISO] Producto " << comVieja.codigoProducto
                     << " no encontrado en inventario.dat" << endl;
            }
            
            cantVentas++;
        }
        fclose(archComandas);
    }

    // 2 ordenamos las ventas (burbujeo por fecha y despues por id de mozo)
    for (int i = 0; i < cantVentas - 1; i++) {
        for (int j = 0; j < cantVentas - i - 1; j++) {
            int comp = strcmp(ventas[j].fecha, ventas[j+1].fecha);
            
            // si la fecha es mayor, o si es la misma fecha pero el id es mayor
            if (comp > 0 || (comp == 0 && ventas[j].comanda.idMozo > ventas[j+1].comanda.idMozo)) {
                VentaAux temp = ventas[j];
                ventas[j] = ventas[j+1];
                ventas[j+1] = temp;
            }
        }
    }

    // 3 separamos por dia creando los archivos (corte de control)
    char fechaActual[11];
    strcpy(fechaActual, "");
    FILE* archDia = NULL;

    for (int i = 0; i < cantVentas; i++) {
        // si cambia la fecha,cerramos el archivo anterior y abrimos uno nuevo
        if (strcmp(fechaActual, ventas[i].fecha) != 0) {
            if (archDia != NULL) {
                fclose(archDia);
            }
            strcpy(fechaActual, ventas[i].fecha);
            
            char nombreArch[50];
            sprintf(nombreArch, "comandas_%s.dat", fechaActual);
            archDia = fopen(nombreArch, "wb");
        }
        
        // escribimos la venta en el archivo del dia
        if (archDia != NULL) {
            fwrite(&ventas[i].comanda, sizeof(Comanda), 1, archDia);
        }
    }
    
    // cerramos el ultimo archivo que quedo abierto
    if (archDia != NULL) {
        fclose(archDia);
    }

    return 0;
}



