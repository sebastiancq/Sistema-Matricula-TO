#pragma once

#include <stdexcept>

// Cola generica FIFO con nodos enlazados. Al ser un template, su implementacion
// completa queda en la cabecera: el compilador la necesita para generar
// cada Cola<T>.
template <typename T> class Cola
{
private:
    struct Nodo
    {
        T dato;
        Nodo* siguiente;
        Nodo(const T& dato) : dato(dato), siguiente(nullptr) {}
    };

    Nodo* primero;
    Nodo* ultimo;
    int cantidad;

public:
    Cola() : primero(nullptr), ultimo(nullptr), cantidad(0) {}
    ~Cola()
    {
        while (!estaVacia())
            desencolar();
    }

    // Copiar la cola duplicaria la propiedad de los nodos.
    Cola(const Cola&) = delete;
    Cola& operator=(const Cola&) = delete;

    bool estaVacia() const { return primero == nullptr; }
    int tamanio() const { return cantidad; }

    void encolar(const T& dato)
    {
        Nodo* nuevo = new Nodo(dato);
        if (estaVacia())
            primero = nuevo;
        else
            ultimo->siguiente = nuevo;
        ultimo = nuevo;
        cantidad++;
    }

    T desencolar()
    {
        if (estaVacia())
            throw std::out_of_range("La cola está vacía.");
        Nodo* viejo = primero;
        T dato = viejo->dato;
        primero = primero->siguiente;
        if (primero == nullptr)
            ultimo = nullptr;
        delete viejo;
        cantidad--;
        return dato;
    }

    // Aplica la accion a cada elemento, del primero al ultimo.
    template <typename Accion> void recorrer(Accion accion) const
    {
        for (Nodo* n = primero; n != nullptr; n = n->siguiente)
            accion(n->dato);
    }

    // Recorre la cola sin modificarla; el criterio llega como lambda.
    template <typename Criterio> bool contiene(Criterio criterio) const
    {
        for (Nodo* n = primero; n != nullptr; n = n->siguiente)
            if (criterio(n->dato))
                return true;
        return false;
    }
};
