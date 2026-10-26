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

// convierte un numero entero positivo a texto sin usar sprintf:
// con % 10 se obtiene el ultimo digito, con / 10 se lo descarta,
// y cada digito se convierte a su caracter sumandole '0'
void numeroATexto(int numero, char texto[]) {
    int cantDigitos = 0;

    if (numero == 0) {
        texto[cantDigitos] = '0';
        cantDigitos++;
    }

    // los digitos salen de derecha a izquierda (al reves)
    while (numero > 0) {
        texto[cantDigitos] = '0' + numero % 10;
        cantDigitos++;
        numero = numero / 10;
    }
    texto[cantDigitos] = '\0';

    // damos vuelta el texto para dejarlo en el orden correcto
    for (int i = 0; i < cantDigitos / 2; i++) {
        char aux = texto[i];
        texto[i] = texto[cantDigitos - 1 - i];
        texto[cantDigitos - 1 - i] = aux;
    }
}

void encriptar( char clave []){
for (int i = 0; clave[i] != '\0'; i++){
    clave[i] += K;
   }
}

// Busqueda binaria directamente sobre inventario.dat (ordenado por codigo).
// En cada paso lee solo el registro del medio con fseek, sin cargar el archivo.
// Devuelve la posicion (numero de registro) del producto, o -1 si no existe.
// Si lo encuentra, deja los datos del producto en p.
long buscarProductoBinaria(FILE* arch, int codProd, Producto &p) {
    fseek(arch, 0, SEEK_END);
    long cantRegistros = ftell(arch) / static_cast<long>(sizeof(Producto));

    long inf = 0;
    long sup = cantRegistros - 1;

    while (inf <= sup) {
        long medio = inf + (sup - inf) / 2;

        fseek(arch, medio * static_cast<long>(sizeof(Producto)), SEEK_SET);
        if (fread(&p, sizeof(Producto), 1, arch) != 1) {
            return -1;
        }

        if (p.codigo == codProd) {
            return medio;
        }
        else if (p.codigo < codProd) {
            inf = medio + 1;
        }
        else {
            sup = medio - 1;
        }
    }

    return -1;
}

// actualiza el stock directo en inventario.dat sin cargar toda la lista:
// ubica el producto con busqueda binaria y pisa solo ese registro
bool descontarStock(int codProd, int cantidad) {
    FILE* arch = fopen("inventario.dat", "rb+");

    if (arch == NULL) {
        return false;
    }

    Producto p;
    long pos = buscarProductoBinaria(arch, codProd, p);

    if (pos >= 0) {
        p.stockActual -= cantidad;

        fseek(arch, pos * static_cast<long>(sizeof(Producto)), SEEK_SET);
        fwrite(&p, sizeof(Producto), 1, arch);
    }

    fclose(arch);
    return pos >= 0;
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

            numeroATexto(mozos[cantidadMozos].idMozo, clave);

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



