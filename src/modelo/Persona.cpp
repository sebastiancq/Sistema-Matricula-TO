#include "modelo/Persona.h"
#include <sstream>

using namespace std;

Persona::Persona(const string& nombre, const string& codigo)
    : nombre(nombre), codigo(codigo)
{
}

string Persona::descripcion() const
{
    ostringstream salida;
    salida << tipo() << ": " << *this << '\n'
           << etiquetaDato() << ": " << datoPropio();
    return salida.str();
}

ostream& operator<<(ostream& salida, const Persona& persona)
{
    return salida << persona.nombre << " (" << persona.codigo << ")";
}
