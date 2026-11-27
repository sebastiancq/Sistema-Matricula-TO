#pragma once

#include "Persona.h"
#include <vector>

// Guarda los codigos de los cursos que ya aprobo, que vienen del archivo de
// datos.
class Estudiante : public Persona
{
private:
    std::string carrera;
    std::vector<std::string> aprobados;

public:
    Estudiante(const std::string& nombre, const std::string& codigo,
               const std::string& carrera)
        : Persona(nombre, codigo), carrera(carrera)
    {
    }

    std::string tipo() const override { return "Estudiante"; }
    std::string etiquetaDato() const override { return "Carrera"; }
    std::string datoPropio() const override { return carrera; }
    void setDatoPropio(const std::string& nuevo) override { carrera = nuevo; }

    void agregarAprobado(const std::string& curso) { aprobados.push_back(curso); }
    const std::vector<std::string>& getAprobados() const { return aprobados; }
    bool aprobo(const std::string& curso) const
    {
        for (const std::string& a : aprobados)
            if (a == curso)
                return true;
        return false;
    }
};
