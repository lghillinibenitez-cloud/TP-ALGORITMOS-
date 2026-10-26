#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <string>

using namespace std;

const int K = 5; 

struct Mozo {
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};

struct Producto {
    int codigo;
    char nombre[50];
    float precio;
    int stock;
};

struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

void encriptar(char clave[]) {
    for (int i = 0; clave[i] != '\0'; i++) {
        clave[i] += K; 
    }
}

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

bool descontarStock(int codProd, int cantidad, float &precioUnitario) {
    FILE* arch = fopen("inventario.dat", "rb+");
    
    if (!arch) {
        cout << "[ERROR] No se pudo abrir inventario.dat\n";
        return false;
    }

    Producto p;
    bool exito = false;

    long pos = buscarProductoBinaria(arch, codProd, p);

    if (pos >= 0) {
        if (p.stock >= cantidad) {
            p.stock -= cantidad;
            precioUnitario = p.precio;

            fseek(arch, pos * static_cast<long>(sizeof(Producto)), SEEK_SET);
            fwrite(&p, sizeof(Producto), 1, arch);
            exito = true;
        } 
        else {
            cout << "[AVISO] Stock insuficiente. Disponible: " << p.stock << "\n";
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
    size_t leidos = fread(comandas, sizeof(Comanda), totalRegistros, arch);
    fclose(arch);
    if ((int)leidos != totalRegistros) {
        cout << "[ERROR] No se pudo leer completo " << nombreArchivo << "\n";
        delete[] comandas;
        return;
    }

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

// Lee un entero validando TODO lo ingresado (rechaza "abc", "12abc", "1.5").
// Devuelve false solo si se cierra la entrada (EOF).
bool leerEntero(int &valor) {
    string texto;
    while (cin >> texto) {
        char* fin;
        long v = strtol(texto.c_str(), &fin, 10);
        if (*fin == '\0' && v >= INT_MIN && v <= INT_MAX) {
            valor = static_cast<int>(v);
            return true;
        }
        cout << "[ERROR] Ingrese un numero valido: ";
    }
    return false;
}

// Lee una fecha con formato DD-MM-AAAA y la copia en fecha[11].
// Devuelve false solo si se cierra la entrada (EOF).
bool leerFecha(char fecha[]) {
    string texto;
    while (cin >> texto) {
        bool valida = (texto.size() == 10 && texto[2] == '-' && texto[5] == '-');
        for (int i = 0; valida && i < 10; i++) {
            if (i != 2 && i != 5 && (texto[i] < '0' || texto[i] > '9')) {
                valida = false;
            }
        }
        if (valida) {
            strcpy(fecha, texto.c_str());
            return true;
        }
        cout << "[ERROR] Formato invalido. Use DD-MM-AAAA: ";
    }
    return false;
}

int main() {
    char fecha[11];
    cout << "=== SISTEMA DE CARGA DE COMANDAS ===\n";
    cout << "Ingrese la fecha del dia (DD-MM-AAAA): ";
    if (!leerFecha(fecha)) {
        return 1;
    }

    char nombreArchivo[50];
    snprintf(nombreArchivo, sizeof(nombreArchivo), "comandas_%s.dat", fecha);

    while (true) {
        cout << "\n--- Nueva Comanda ---\n";
        int idIngresado;
        string passIngresada;

        cout << "Ingrese ID del Mozo (o 0 para finalizar jornada): ";
        if (!leerEntero(idIngresado) || idIngresado == 0) {
            break;
        }

        cout << "Ingrese Clave: ";
        if (!(cin >> passIngresada)) {
            break;
        }

        Mozo mozoActual;
        // una clave de 20 o mas caracteres no puede coincidir con ninguna guardada
        if (passIngresada.size() >= sizeof(mozoActual.password) ||
            !autenticarMozo(idIngresado, passIngresada.c_str(), mozoActual)) {
            cout << "[ACCESO DENEGADO] ID o contrasenia incorrectos.\n";
            continue; // Salta al siguiente ciclo de ingreso
        }

        cout << "Bienvenido/a, " << mozoActual.nombre << ".\n"; 

        int codProd, cant;
        cout << "Codigo de producto: ";
        if (!leerEntero(codProd)) {
            break;
        }
        cout << "Cantidad: ";
        if (!leerEntero(cant)) {
            break;
        }

        if (cant <= 0) {
            cout << "[ERROR] La cantidad ingresada debe ser mayor a 0.\n";
            continue; 
        }

        float precioUnitario = 0.0f;
        if (descontarStock(codProd, cant, precioUnitario)) {
            float totalVenta = precioUnitario * cant;
            float comision = totalVenta * 0.10f;

            Comanda nuevaCmd;
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
