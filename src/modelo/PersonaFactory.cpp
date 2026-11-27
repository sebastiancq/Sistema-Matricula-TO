#include "modelo/PersonaFactory.h"
#include "modelo/Administrativo.h"
#include "modelo/Docente.h"
#include "modelo/Estudiante.h"
#include "modelo/Excepciones.h"

using namespace std;

map<string, CreadorPersona>& PersonaFactory::creadores()
{
    static map<string, CreadorPersona> tabla;
    return tabla;
}

void PersonaFactory::registrarTipo(const string& tipo, CreadorPersona creador)
{
    creadores()[tipo] = creador;
}

vector<string> PersonaFactory::tiposRegistrados()
{
    vector<string> tipos;
    for (const auto& par : creadores())
        tipos.push_back(par.first);
    return tipos;
}

unique_ptr<Persona> PersonaFactory::crearPersona(const string& tipo,
                                                 const string& nombre,
                                                 const string& codigo,
                                                 const string& extra)
{
    auto it = creadores().find(tipo);
    if (it == creadores().end())
        throw DatosInvalidos("Tipo de persona no reconocido: " + tipo + ".");
    return it->second(nombre, codigo, extra);
}

void registrarTiposDePersona()
{
    PersonaFactory::registrarTipo(
        "Estudiante",
        [](const string& nombre, const string& codigo,
           const string& carrera) -> unique_ptr<Persona>
        { return make_unique<Estudiante>(nombre, codigo, carrera); });

    PersonaFactory::registrarTipo(
        "Docente",
        [](const string& nombre, const string& codigo,
           const string& departamento) -> unique_ptr<Persona>
        { return make_unique<Docente>(nombre, codigo, departamento); });

    PersonaFactory::registrarTipo(
        "Administrativo",
        [](const string& nombre, const string& codigo,
           const string& area) -> unique_ptr<Persona>
        { return make_unique<Administrativo>(nombre, codigo, area); });
}
