#pragma once

#include "SistemaMatricula.h"
#include <iostream>

// Guarda y recupera el sistema completo en texto separado por punto y coma.
// Cada linea empieza con el tipo de registro:
//   carrera;nombre
//   persona;tipo;codigo;nombre;dato
//   aprobados;estudiante;codigos separados por espacios
//   aula;codigo;capacidad
//   curso;codigo;nombre;carrera;creditos;semestre;aula;vacantes;horario;
//         docente;prerrequisitos separados por espacios
//   matricula;estudiante;curso;fecha
//   espera;curso;estudiante
// Un curso sin carrera es general. Los prerrequisitos y la linea aprobados
// pueden faltar, asi los archivos anteriores se siguen abriendo. Las lineas que empiezan con # son
// comentarios.
class RepositorioCsv
{
public:
    static void guardar(const SistemaMatricula& sistema, std::ostream& salida);

    // Si el archivo tiene un error, lanza DatosInvalidos con el numero de
    // linea y el sistema conserva sus datos anteriores.
    static void cargar(std::istream& entrada, SistemaMatricula& sistema);
};
