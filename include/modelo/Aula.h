#pragma once

#include <string>

// Aula del local: existe por su cuenta y varios cursos la usan en horarios
// distintos. El codigo la identifica y no cambia; la capacidad si se puede
// corregir.
class Aula
{
private:
    std::string codigo;
    int capacidad;

public:
    Aula(const std::string& codigo, int capacidad)
        : codigo(codigo), capacidad(capacidad)
    {
    }

    const std::string& getCodigo() const { return codigo; }
    int getCapacidad() const { return capacidad; }
    void setCapacidad(int nueva) { capacidad = nueva; }
};
