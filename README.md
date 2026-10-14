TP Integrador: El Buffet de Alberto
Materia: Algoritmos y Estructuras de Datos (UTN FRBA)
Grupo: 4

Integrantes y Roles:
* Anabela Ovejero Valdez, normalizacion A
* Victoria Gomez, normalizacion B
* Valentina Medina Enriú, Cierre
* Luca Ghillini Benites, Ventas
* Mateo Reyes, Resumen

----------------------------------------------------------------

Como compilar y ejecutar el proyecto

Importante: Los archivos provistos por la catedra (comandas_historicas.dat e inventario.dat) no estan subidos al repo por regla del TP. Tienen que estar en la misma carpeta que los .cpp para arrancar.

1) Compilacion
Se compila cada programa por separado en la terminal usando g++:

g++ -O2 -o normalizacion normalizacion.cpp
g++ -O2 -o ventas ventas.cpp
g++ -O2 -o cierre cierre.cpp
g++ -O2 -o resumen resumen.cpp

2) Orden de ejecucion
Los programas se ejecutan en este orden estricto porque se van pasando los archivos .dat entre si:

- ./normalizacion (Arma mozos.dat, separa los dias y actualiza inventario)
- ./ventas (Valida al mozo, descuenta stock y guarda la venta en el dia)
- ./cierre (Junta los dias de la semana y genera comandas_semana_sX-mm.dat)
- ./resumen (Lee el archivo de la semana y muestra las comisiones y el total)

--------------------------------------------------------------------

Resolucion de las partes clave

* Seguridad y Login: Usamos un corrimiento (Cifrado Cesar) sumando K=5 al codigo ASCII de cada caracter. En ventas.cpp se encripta la clave ingresada y se compara directamente con la guardada en el archivo binario.

* Actualizacion de Stock: Al hacer una venta, abrimos el inventario con rb+, buscamos el producto, retrocedemos una posicion con fseek y pisamos unicamente el stock viejo.

* Cierre Semanal: Usamos Apareo Multiple para juntar las planillas de los dias. Se lee en paralelo y se va grabando en el archivo semanal el idMozo mas chico, asi nos aseguramos que quede totalmente ordenado.

* Resumen Final: Como el archivo semanal ya viene ordenado desde el cierre, aplicamos la estructura de Corte de Control por idMozo. Leemos secuencialmente y vamos sumando comisiones para sacar el subtotal por mozo y el total general del buffet.
