#pragma once

#include <iostream>
#include <string>
#include <vector>

// Un bloque de clase: dia de la semana, de 0 para el lunes a 4 para el
// viernes, y horas enteras de inicio y fin, entre las 7 y las 22. No hay
// clases los sabados.
struct Bloque
{
    int dia;
    int inicio;
    int fin;
};

// Horario semanal de un curso, formado por bloques que no se superponen. En
// el archivo de datos se guarda como texto: dia abreviado y horas, con los
// bloques separados por comas.
class Horario
{
private:
    std::vector<Bloque> bloques;

public:
    // Lanza DatosInvalidos si el bloque esta fuera de 7 a 22 o se superpone
    // con otro bloque del mismo horario.
    void agregar(const Bloque& bloque);

    // Lanza DatosInvalidos si el texto no respeta el formato.
    static Horario desdeTexto(const std::string& texto);
    static const std::vector<std::string>& nombresDias();

    const std::vector<Bloque>& getBloques() const { return bloques; }
    bool estaVacio() const { return bloques.empty(); }

    // Dos horarios se cruzan si comparten al menos una hora del mismo dia.
    bool seCruzaCon(const Horario& otro) const;

    // Si se cruzan, deja en comun las horas que comparten el primer dia en
    // que coinciden.
    bool seCruzaCon(const Horario& otro, Bloque& comun) const;

    // Texto para los mensajes con el nombre del dia y las horas del bloque.
    static std::string describir(const Bloque& bloque);

    friend std::ostream& operator<<(std::ostream& salida,
                                    const Horario& horario);
};
