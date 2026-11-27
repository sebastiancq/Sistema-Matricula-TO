#pragma once

#include "Curso.h"

// Asociacion entre un estudiante y un curso. No es duena de ninguno: los dos
// pertenecen al sistema.
struct Matricula
{
    const Estudiante* estudiante;
    const Curso* curso;
    std::string fecha;
};
