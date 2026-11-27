#include "modelo/Curso.h"

using namespace std;

Curso::Curso(const DatosCurso& datos, const Aula* aula, const Docente* docente)
    : codigo(datos.codigo)
{
    actualizar(datos, aula, docente);
}

void Curso::actualizar(const DatosCurso& datos, const Aula* nuevaAula,
                       const Docente* nuevoDocente)
{
    nombre = datos.nombre;
    carrera = datos.carrera;
    creditos = datos.creditos;
    semestre = datos.semestre;
    aula = nuevaAula;
    vacantes = datos.vacantes;
    horario = datos.horario;
    docente = nuevoDocente;
    prerrequisitos = datos.prerrequisitos;
}

bool Curso::ocuparCupo()
{
    if (cuposLibres() == 0)
        return false;
    inscritos++;
    return true;
}

void Curso::liberarCupo()
{
    if (inscritos > 0)
        inscritos--;
}

void Curso::agregarAEspera(const Estudiante* estudiante)
{
    listaEspera.encolar(estudiante);
}

bool Curso::estaEnEspera(const Estudiante* estudiante) const
{
    return listaEspera.contiene([estudiante](const Estudiante* e)
                                { return e == estudiante; });
}

const Estudiante* Curso::siguienteEnEspera()
{
    return listaEspera.estaVacia() ? nullptr : listaEspera.desencolar();
}

vector<const Estudiante*> Curso::estudiantesEnEspera() const
{
    vector<const Estudiante*> lista;
    listaEspera.recorrer([&lista](const Estudiante* e) { lista.push_back(e); });
    return lista;
}
