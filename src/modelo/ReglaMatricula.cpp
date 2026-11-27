#include "modelo/ReglaMatricula.h"
#include "modelo/Excepciones.h"

using namespace std;

void ReglaCarrera::comprobar(const Estudiante& estudiante, const Curso& curso,
                             const vector<const Curso*>&) const
{
    if (!curso.esGeneral() && curso.getCarrera() != estudiante.datoPropio())
        throw OtraCarrera(curso.getNombre(), curso.getCarrera(),
                          estudiante.getNombre(), estudiante.datoPropio());
}

void ReglaCruceHorario::comprobar(const Estudiante& estudiante,
                                  const Curso& curso,
                                  const vector<const Curso*>& actuales) const
{
    Bloque comun;
    for (const Curso* c : actuales)
        if (curso.getHorario().seCruzaCon(c->getHorario(), comun))
            throw CruceHorario(estudiante.getNombre(), curso.getNombre(),
                               c->getNombre(), Horario::describir(comun));
}

void ReglaLimiteCreditos::comprobar(const Estudiante& estudiante,
                                    const Curso& curso,
                                    const vector<const Curso*>& actuales) const
{
    int creditos = 0;
    for (const Curso* c : actuales)
        creditos += c->getCreditos();
    if (creditos + curso.getCreditos() > maximo)
        throw LimiteCreditos(estudiante.getNombre(), curso.getNombre(),
                             creditos, curso.getCreditos(), maximo);
}

void ReglaPrerrequisito::comprobar(const Estudiante& estudiante,
                                   const Curso& curso,
                                   const vector<const Curso*>&) const
{
    vector<string> faltantes;
    for (const string& requisito : curso.getPrerrequisitos())
        if (!estudiante.aprobo(requisito))
            faltantes.push_back(requisito);
    if (!faltantes.empty())
        throw FaltaPrerrequisito(estudiante.getNombre(), curso.getNombre(),
                                 faltantes);
}
