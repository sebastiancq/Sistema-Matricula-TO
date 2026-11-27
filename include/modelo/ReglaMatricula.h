#pragma once

#include "Curso.h"
#include <vector>

// Strategy: cada regla de matricula es un objeto con la misma interfaz. El
// sistema recorre su lista de reglas sin saber cuales son.
class ReglaMatricula
{
public:
    virtual ~ReglaMatricula() = default;

    // Lanza una excepcion derivada de ErrorMatricula si el estudiante, que ya
    // lleva los cursos indicados, no puede llevar tambien este curso.
    virtual void comprobar(const Estudiante& estudiante, const Curso& curso,
                           const std::vector<const Curso*>& actuales) const = 0;

    // Retorno covariante: cada regla devuelve una copia de su propio tipo.
    virtual ReglaMatricula* clonar() const = 0;
};

// Solo se llevan cursos generales o de la carrera del estudiante.
class ReglaCarrera : public ReglaMatricula
{
public:
    void comprobar(const Estudiante& estudiante, const Curso& curso,
                   const std::vector<const Curso*>& actuales) const override;
    ReglaCarrera* clonar() const override { return new ReglaCarrera(*this); }
};

// El curso no puede cruzarse con el horario de los que ya lleva.
class ReglaCruceHorario : public ReglaMatricula
{
public:
    void comprobar(const Estudiante& estudiante, const Curso& curso,
                   const std::vector<const Curso*>& actuales) const override;
    ReglaCruceHorario* clonar() const override
    {
        return new ReglaCruceHorario(*this);
    }
};

// El estudiante debe haber aprobado todos los prerrequisitos del curso.
class ReglaPrerrequisito : public ReglaMatricula
{
public:
    void comprobar(const Estudiante& estudiante, const Curso& curso,
                   const std::vector<const Curso*>& actuales) const override;
    ReglaPrerrequisito* clonar() const override
    {
        return new ReglaPrerrequisito(*this);
    }
};

// La suma de creditos no puede pasar del maximo.
class ReglaLimiteCreditos : public ReglaMatricula
{
private:
    int maximo;

public:
    explicit ReglaLimiteCreditos(int maximo) : maximo(maximo) {}
    void comprobar(const Estudiante& estudiante, const Curso& curso,
                   const std::vector<const Curso*>& actuales) const override;
    ReglaLimiteCreditos* clonar() const override
    {
        return new ReglaLimiteCreditos(*this);
    }
};
