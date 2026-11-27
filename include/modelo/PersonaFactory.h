#pragma once

#include "Persona.h"
#include <functional>
#include <map>
#include <memory>
#include <vector>

// Recibe nombre, codigo y el dato propio de cada tipo, como la carrera o el
// departamento.
using CreadorPersona = std::function<std::unique_ptr<Persona>(
    const std::string&, const std::string&, const std::string&)>;

// Factory Method: crea personas a partir del tipo elegido en el formulario o
// leido del archivo. Cada tipo se registra con su creador.
class PersonaFactory
{
private:
    static std::map<std::string, CreadorPersona>& creadores();

public:
    static void registrarTipo(const std::string& tipo, CreadorPersona creador);
    static std::vector<std::string> tiposRegistrados();

    // Lanza DatosInvalidos si el tipo no esta registrado.
    static std::unique_ptr<Persona> crearPersona(const std::string& tipo,
                                                 const std::string& nombre,
                                                 const std::string& codigo,
                                                 const std::string& extra);
};

// Registra en la fabrica cada subclase concreta de Persona con su creador.
void registrarTiposDePersona();
