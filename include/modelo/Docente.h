#pragma once

#include "Persona.h"

class Docente : public Persona
{
private:
    std::string departamento;

public:
    Docente(const std::string& nombre, const std::string& codigo,
            const std::string& departamento)
        : Persona(nombre, codigo), departamento(departamento)
    {
    }

    std::string tipo() const override { return "Docente"; }
    std::string etiquetaDato() const override { return "Departamento"; }
    std::string datoPropio() const override { return departamento; }
    void setDatoPropio(const std::string& nuevo) override
    {
        departamento = nuevo;
    }
};
