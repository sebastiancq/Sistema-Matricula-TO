#pragma once

#include "Persona.h"

class Administrativo : public Persona
{
private:
    std::string area;

public:
    Administrativo(const std::string& nombre, const std::string& codigo,
                   const std::string& area)
        : Persona(nombre, codigo), area(area)
    {
    }

    std::string tipo() const override { return "Administrativo"; }
    std::string etiquetaDato() const override { return "Área"; }
    std::string datoPropio() const override { return area; }
    void setDatoPropio(const std::string& nuevo) override { area = nuevo; }
};
