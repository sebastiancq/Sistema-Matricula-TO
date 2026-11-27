#pragma once

#include <iostream>
#include <string>

// Clase base abstracta de todas las personas del sistema. Cada tipo concreto
// define su dato propio: la carrera, el departamento o el area.
class Persona
{
protected:
    std::string nombre;
    std::string codigo;

public:
    Persona(const std::string& nombre, const std::string& codigo);
    virtual ~Persona() = default;

    const std::string& getCodigo() const { return codigo; }
    const std::string& getNombre() const { return nombre; }
    void setNombre(const std::string& nuevo) { nombre = nuevo; }

    virtual std::string tipo() const = 0;
    virtual std::string etiquetaDato() const = 0;
    virtual std::string datoPropio() const = 0;
    virtual void setDatoPropio(const std::string& nuevo) = 0;

    // Texto del detalle. Lo arma con los metodos virtuales, asi sirve para
    // cualquier tipo de persona.
    std::string descripcion() const;

    // Dos personas son la misma si tienen el mismo codigo.
    friend bool operator==(const Persona& a, const Persona& b)
    {
        return a.codigo == b.codigo;
    }

    friend std::ostream& operator<<(std::ostream& salida,
                                    const Persona& persona);
};
