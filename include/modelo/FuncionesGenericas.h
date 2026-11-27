#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <vector>

// Convierte en texto cualquier valor que tenga operator<<, como un Horario o
// una Persona.
template <typename T> std::string aTexto(const T& valor)
{
    std::ostringstream salida;
    salida << valor;
    return salida.str();
}

// Busca el primer elemento que cumple el criterio. Sirve igual para personas
// y para cursos.
template <typename T, typename Criterio>
T* buscarPor(const std::vector<std::unique_ptr<T>>& elementos,
             Criterio criterio)
{
    for (const auto& e : elementos)
        if (criterio(*e))
            return e.get();
    return nullptr;
}
