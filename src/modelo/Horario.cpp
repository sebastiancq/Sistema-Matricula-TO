#include "modelo/Horario.h"
#include "modelo/Excepciones.h"
#include <algorithm>
#include <sstream>

using namespace std;

const vector<string>& Horario::nombresDias()
{
    static const vector<string> dias = { "LU", "MA", "MI", "JU", "VI" };
    return dias;
}

static bool seSuperponen(const Bloque& a, const Bloque& b)
{
    return a.dia == b.dia && a.inicio < b.fin && b.inicio < a.fin;
}

void Horario::agregar(const Bloque& bloque)
{
    if (bloque.dia < 0 || bloque.dia > 4 || bloque.inicio < 7 ||
        bloque.fin > 22 || bloque.inicio >= bloque.fin)
        throw DatosInvalidos("Cada bloque debe ser de lunes a viernes, entre "
                             "las 7:00 y las 22:00, y terminar después de "
                             "su hora de inicio.");
    for (const Bloque& b : bloques)
        if (seSuperponen(b, bloque))
            throw DatosInvalidos("El bloque se superpone con el del " +
                                 describir(b).substr(3) + ", ya agregado.");
    bloques.push_back(bloque);
}

static int leerHora(const string& texto)
{
    if (texto.size() != 2 || texto[0] < '0' || texto[0] > '9' ||
        texto[1] < '0' || texto[1] > '9')
        return -1;
    return (texto[0] - '0') * 10 + (texto[1] - '0');
}

// Cada bloque tiene el dia abreviado y las horas de inicio y fin con dos
// cifras; los bloques se separan con comas.
Horario Horario::desdeTexto(const string& texto)
{
    const string formato = "El horario se escribe como LU 07-09, MI 07-09.";
    Horario horario;
    stringstream entrada(texto);
    string parte;
    while (getline(entrada, parte, ','))
    {
        stringstream bloque(parte);
        string dia, horas;
        if (!(bloque >> dia >> horas) || horas.size() != 5 || horas[2] != '-')
            throw DatosInvalidos(formato);
        int indiceDia = -1;
        for (size_t d = 0; d < nombresDias().size(); d++)
            if (nombresDias()[d] == dia)
                indiceDia = static_cast<int>(d);
        horario.agregar({ indiceDia, leerHora(horas.substr(0, 2)),
                          leerHora(horas.substr(3, 2)) });
    }
    if (horario.estaVacio())
        throw DatosInvalidos(formato);
    return horario;
}

bool Horario::seCruzaCon(const Horario& otro) const
{
    Bloque comun;
    return seCruzaCon(otro, comun);
}

bool Horario::seCruzaCon(const Horario& otro, Bloque& comun) const
{
    for (const Bloque& a : bloques)
        for (const Bloque& b : otro.bloques)
            if (seSuperponen(a, b))
            {
                comun = { a.dia, max(a.inicio, b.inicio), min(a.fin, b.fin) };
                return true;
            }
    return false;
}

string Horario::describir(const Bloque& bloque)
{
    static const vector<string> dias = { "lunes", "martes", "miércoles",
                                         "jueves", "viernes" };
    return "el " + dias[bloque.dia] + " de " + to_string(bloque.inicio) +
           ":00 a " + to_string(bloque.fin) + ":00";
}

ostream& operator<<(ostream& salida, const Horario& horario)
{
    for (size_t i = 0; i < horario.bloques.size(); i++)
    {
        const Bloque& b = horario.bloques[i];
        if (i > 0)
            salida << ", ";
        salida << Horario::nombresDias()[b.dia] << ' '
               << (b.inicio < 10 ? "0" : "") << b.inicio << '-'
               << (b.fin < 10 ? "0" : "") << b.fin;
    }
    return salida;
}
