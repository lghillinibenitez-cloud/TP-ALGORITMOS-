#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

const int K = 5; 

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
    for (int i = 0; clave[i] != '\0'; i++) {
        clave[i] += K; 
    }
}[cite: 1]

bool autenticarMozo(int id, const char* clavePlana, Mozo &mozoOut) {
    FILE* arch = fopen("mozos.dat", "rb"); 
    
    if (!arch) {
        cout << "[ERROR] No se pudo abrir mozos.dat\n";
        return false;
    }


    char claveCifrada[20];
    strncpy(claveCifrada, clavePlana, sizeof(claveCifrada) - 1);
    claveCifrada[sizeof(claveCifrada) - 1] = '\0';
    encriptar(claveCifrada); 

    Mozo m;
    bool autenticado = false;

   
    while (fread(&m, sizeof(Mozo), 1, arch) == 1) { 
        if (m.idMozo == id && strcmp(m.password, claveCifrada) == 0) { 
            mozoOut = m;
            autenticado = true;
            break; 
        }
    }

    fclose(arch);
    return autenticado;
}

bool descontarStock(int codProd, int cantidad, float &precioUnitario) {
    FILE* arch = fopen("inventario.dat", "rb+");
    
    if (!arch) {
        cout << "[ERROR] No se pudo abrir inventario.dat\n";
        return false;
    }

    Producto p;
    bool exito = false;

    while (fread(&p, sizeof(Producto), 1, arch) == 1) {
        if (p.codigo == codProd) {
            if (p.stock >= cantidad) {
                p.stock -= cantidad;
                precioUnitario = p.precio;

                fseek(arch, -static_cast<long>(sizeof(Producto)), SEEK_CUR);
                fwrite(&p, sizeof(Producto), 1, arch);
                exito = true;
            } 
            else {
                cout << "[AVISO] Stock insuficiente. Disponible: " << p.stock << "\n";
            }
            break; 
        }
    }

    fclose(arch);
    return exito;
}

void registrarComanda(const char* nombreArchivo, const Comanda &cmd) {
    FILE* arch = fopen(nombreArchivo, "ab");
    
    if (!arch) {
        cout << "[ERROR] No se pudo escribir en " << nombreArchivo << "\n";
        return;
    }

    fwrite(&cmd, sizeof(Comanda), 1, arch);
    fclose(arch);
}

void ordenarArchivoPorMozo(const char* nombreArchivo) {
    FILE* arch = fopen(nombreArchivo, "rb");
    if (!arch) {
        return;
    }

    fseek(arch, 0, SEEK_END);
    long totalBytes = ftell(arch);
    int totalRegistros = totalBytes / sizeof(Comanda);
    if (totalRegistros <= 1) {
        fclose(arch);
        return;
    }

    Comanda* comandas = new Comanda[totalRegistros];
    fseek(arch, 0, SEEK_SET);
    fread(comandas, sizeof(Comanda), totalRegistros, arch);
    fclose(arch);

    for (int i = 0; i < totalRegistros - 1; i++) {
        for (int j = 0; j < totalRegistros - i - 1; j++) {
            if (comandas[j].idMozo > comandas[j + 1].idMozo) {
                Comanda aux = comandas[j];
                comandas[j] = comandas[j + 1];
                comandas[j + 1] = aux;
            }
        }
    }

    arch = fopen(nombreArchivo, "wb");
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

    while (true) {
        cout << "\n--- Nueva Comanda ---\n";
        int idIngresado;
        char passIngresada[20];

        cout << "Ingrese ID del Mozo (o 0 para finalizar jornada): ";
        cin >> idIngresado;

        if (idIngresado == 0) {
            break;
        }

        cout << "Ingrese Clave: ";
        cin >> passIngresada;

        Mozo mozoActual;
        if (!autenticarMozo(idIngresado, passIngresada, mozoActual)) {
            cout << "[ACCESO DENEGADO] ID o contrasenia incorrectos.\n";
            continue; // Salta al siguiente ciclo de ingreso
        }

        cout << "Bienvenido/a, " << mozoActual.nombre << ".\n"; 

        int codProd, cant;
        cout << "Codigo de producto: ";
        cin >> codProd;
        cout << "Cantidad: ";
        cin >> cant;

        if (cant <= 0) {
            cout << "[ERROR] La cantidad ingresada debe ser mayor a 0.\n";
            continue; 
        }

        float precioUnitario = 0.0f;
        if (descontarStock(codProd, cant, precioUnitario)) {
            float totalVenta = precioUnitario * cant;
            float comision = totalVenta * 0.10f;

            Comanda nuevaCmd;
            strncpy(nuevaCmd.fecha, fecha, sizeof(nuevaCmd.fecha));
            nuevaCmd.idMozo = mozoActual.idMozo; 
            nuevaCmd.codigoProducto = codProd;
            nuevaCmd.cantidad = cant;
            nuevaCmd.comision = comision;

            registrarComanda(nombreArchivo, nuevaCmd);
            cout << "Comanda registrada con exito. Comision generada: $" << comision << "\n";
        } 
        else {
            cout << "[RECHAZADO] No se pudo procesar la comanda por stock o codigo inexistente.\n";
        }
    }

    cout << "\nFinalizando operaciones del dia...\n";
    ordenarArchivoPorMozo(nombreArchivo);
    cout << "Archivo " << nombreArchivo << " ordenado correctamente por ID de mozo.\n";

    return 0;
}
