// creador_inventario.cpp
#include <cstdio>
struct Producto { int codigo; char nombre[50]; float precio; int stock; };

int main() {
    FILE* f = fopen("inventario.dat", "wb");
    Producto items[] = {
        {101, "Hamburguesa Doble", 4500.0f, 20},
        {102, "Papas Fritas", 2000.0f, 15},
        {103, "Gaseosa", 1500.0f, 30}
    };
    fwrite(items, sizeof(Producto), 3, f);
    fclose(f);
    return 0;
}