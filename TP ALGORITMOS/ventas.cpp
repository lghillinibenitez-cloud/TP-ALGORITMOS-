#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

const int K = 5; // Constante de desplazamiento para el cifrado Cesar[cite: 1]

struct Mozo {
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};[cite: 1]

struct Producto {
    int codigo;
    char nombre[50];
    float precio;
    int stock;
};

struct Comanda {
    char fecha[11];
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

void encriptar(char clave[]) {
    // Rama iterativa: recorre caracter por caracter hasta encontrar el fin de cadena
    for (int i = 0; clave[i] != '\0'; i++) {
        clave[i] += K; // Aplica el desplazamiento K[cite: 1]
    }
}[cite: 1]

bool autenticarMozo(int id, const char* clavePlana, Mozo &mozoOut) {
    FILE* arch = fopen("mozos.dat", "rb"); //[cite: 1]
    
    // Rama: el archivo mozos.dat no existe o no se pudo abrir
    if (!arch) {
        cout << "[ERROR] No se pudo abrir mozos.dat\n";
        return false;
    }

    // Preparacion del buffer y encriptacion previa a la comparacion
    char claveCifrada[20];
    strncpy(claveCifrada, clavePlana, sizeof(claveCifrada) - 1);
    claveCifrada[sizeof(claveCifrada) - 1] = '\0';
    encriptar(claveCifrada); //[cite: 1]

    Mozo m;
    bool autenticado = false;

    // Rama iterativa: lectura secuencial registro por registro[cite: 1]
    while (fread(&m, sizeof(Mozo), 1, arch) == 1) { //[cite: 1]
        // Rama condicional: verifica coincidencia simultanea de ID y clave cifrada[cite: 1]
        if (m.idMozo == id && strcmp(m.password, claveCifrada) == 0) { //[cite: 1]
            mozoOut = m;
            autenticado = true;
            break; // Interrumpe el ciclo prematuramente al encontrar coincidencia
        }
    }

    fclose(arch);
    return autenticado;
}

bool descontarStock(int codProd, int cantidad, float &precioUnitario) {
    FILE* arch = fopen("inventario.dat", "rb+");
    
    // Rama: error al abrir el archivo en modo lectura/escritura
    if (!arch) {
        cout << "[ERROR] No se pudo abrir inventario.dat\n";
        return false;
    }

    Producto p;
    bool exito = false;

    // Rama iterativa: busqueda lineal del producto por codigo
    while (fread(&p, sizeof(Producto), 1, arch) == 1) {
        // Rama condicional: valida si el registro actual es el producto buscado
        if (p.codigo == codProd) {
            // Rama anidada (True): hay existencia suficiente para cubrir el pedido
            if (p.stock >= cantidad) {
                p.stock -= cantidad;
                precioUnitario = p.precio;

                // Reposiciona el cursor un registro atras para sobreescribir el stock actualizado
                fseek(arch, -static_cast<long>(sizeof(Producto)), SEEK_CUR);
                fwrite(&p, sizeof(Producto), 1, arch);
                exito = true;
            } 
            // Rama anidada (False): el producto existe pero la cantidad pedida supera el stock
            else {
                cout << "[AVISO] Stock insuficiente. Disponible: " << p.stock << "\n";
            }
            break; // Se detiene la busqueda tras evaluar el producto correspondiente
        }
    }

    fclose(arch);
    return exito;
}

void registrarComanda(const char* nombreArchivo, const Comanda &cmd) {
    FILE* arch = fopen(nombreArchivo, "ab");
    
    // Rama: fallo al abrir/crear el archivo binario en modo anexo
    if (!arch) {
        cout << "[ERROR] No se pudo escribir en " << nombreArchivo << "\n";
        return;
    }

    // Escritura directa al final del archivo si la apertura fue correcta
    fwrite(&cmd, sizeof(Comanda), 1, arch);
    fclose(arch);
}

void ordenarArchivoPorMozo(const char* nombreArchivo) {
    FILE* arch = fopen(nombreArchivo, "rb");
    
    // Rama: el archivo diario no existe (por ejemplo, si no se cargo ninguna comanda)
    if (!arch) {
        return;
    }

    fseek(arch, 0, SEEK_END);
    long totalBytes = ftell(arch);
    int totalRegistros = totalBytes / sizeof(Comanda);

    // Rama: si hay 0 o 1 registro, el archivo ya se encuentra ordenado trivialmente
    if (totalRegistros <= 1) {
        fclose(arch);
        return;
    }

    Comanda* comandas = new Comanda[totalRegistros];
    fseek(arch, 0, SEEK_SET);
    fread(comandas, sizeof(Comanda), totalRegistros, arch);
    fclose(arch);

    // Rama iterativa: ordenamiento por metodo burbuja
    for (int i = 0; i < totalRegistros - 1; i++) {
        for (int j = 0; j < totalRegistros - i - 1; j++) {
            // Rama condicional: si el ID actual es mayor al siguiente, realiza el swap
            if (comandas[j].idMozo > comandas[j + 1].idMozo) {
                Comanda aux = comandas[j];
                comandas[j] = comandas[j + 1];
                comandas[j + 1] = aux;
            }
        }
    }

    arch = fopen(nombreArchivo, "wb");
    // Rama: apertura correcta para reescribir los datos ordenados
    if (arch) {
        fwrite(comandas, sizeof(Comanda), totalRegistros, arch);
        fclose(arch);
    }

    delete[] comandas;
}

int main() {
    char fecha[11];
    cout << "=== SISTEMA DE CARGA DE COMANDAS ===\n";
    cout << "Ingrese la fecha del dia (DD-MM-AAAA): ";
    cin >> fecha;

    char nombreArchivo[50];
    snprintf(nombreArchivo, sizeof(nombreArchivo), "comandas_%s.dat", fecha);

    // Rama iterativa: ciclo principal de ingreso de operaciones
    while (true) {
        cout << "\n--- Nueva Comanda ---\n";
        int idIngresado;
        char passIngresada[20];

        cout << "Ingrese ID del Mozo (o 0 para finalizar jornada): ";
        cin >> idIngresado;

        // Rama condicional de salida: bandera 0 para terminar la carga del dia
        if (idIngresado == 0) {
            break;
        }

        cout << "Ingrese Clave: ";
        cin >> passIngresada;

        Mozo mozoActual;
        // Rama condicional: fallo de autenticacion (mozo inexistente o pass incorrecta)
        if (!autenticarMozo(idIngresado, passIngresada, mozoActual)) {
            cout << "[ACCESO DENEGADO] ID o contrasenia incorrectos.\n";
            continue; // Salta al siguiente ciclo de ingreso
        }

        cout << "Bienvenido/a, " << mozoActual.nombre << ".\n"; //[cite: 1]

        int codProd, cant;
        cout << "Codigo de producto: ";
        cin >> codProd;
        cout << "Cantidad: ";
        cin >> cant;

        // Rama condicional: validacion de entrada numerica de cantidad
        if (cant <= 0) {
            cout << "[ERROR] La cantidad ingresada debe ser mayor a 0.\n";
            continue; // Descarta la operacion invalida y reinicia el ciclo
        }

        float precioUnitario = 0.0f;
        // Rama condicional (True): descuento exitoso de inventario
        if (descontarStock(codProd, cant, precioUnitario)) {
            float totalVenta = precioUnitario * cant;
            float comision = totalVenta * 0.10f;

            Comanda nuevaCmd;
            strncpy(nuevaCmd.fecha, fecha, sizeof(nuevaCmd.fecha));
            nuevaCmd.idMozo = mozoActual.idMozo; //[cite: 1]
            nuevaCmd.codigoProducto = codProd;
            nuevaCmd.cantidad = cant;
            nuevaCmd.comision = comision;

            registrarComanda(nombreArchivo, nuevaCmd);
            cout << "Comanda registrada con exito. Comision generada: $" << comision << "\n";
        } 
        // Rama condicional (False): producto inexistente o stock no disponible
        else {
            cout << "[RECHAZADO] No se pudo procesar la comanda por stock o codigo inexistente.\n";
        }
    }

    cout << "\nFinalizando operaciones del dia...\n";
    ordenarArchivoPorMozo(nombreArchivo);
    cout << "Archivo " << nombreArchivo << " ordenado correctamente por ID de mozo.\n";

    return 0;
}