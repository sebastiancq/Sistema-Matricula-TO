#pragma once

#include "Aula.h"
#include "Cola.h"
#include "Docente.h"
#include "Estudiante.h"
#include "Horario.h"
#include <vector>

// Datos que llegan del formulario o del archivo para crear o editar un
// curso. La carrera vacia indica un curso general, comun a todas las
// carreras. El aula y el docente van por su codigo; el docente puede estar
// vacio. Las vacantes no pueden superar la capacidad del aula. Los
// prerrequisitos son codigos de otros cursos y pueden faltar.
struct DatosCurso
{
    std::string codigo;
    std::string nombre;
    std::string carrera;
    int creditos;
    int semestre;
    std::string aula;
    int vacantes;
    Horario horario;
    std::string docente;
    std::vector<std::string> prerrequisitos = {};
};

// Composicion: el horario y la lista de espera son parte del curso.
// Agregacion: el aula y el docente existen por su cuenta y el curso solo los
// referencia.
class Curso
{
private:
    std::string codigo;
    std::string nombre;
    std::string carrera;
    int creditos;
    int semestre;
    const Aula* aula;
    int vacantes;
    Horario horario;
    const Docente* docente;
    std::vector<std::string> prerrequisitos;
    int inscritos = 0;
    Cola<const Estudiante*> listaEspera;

public:
    Curso(const DatosCurso& datos, const Aula* aula, const Docente* docente);

    // Cambia todo menos el codigo, los inscritos y la lista de espera. El
    // sistema revisa antes que los cambios sean validos.
    void actualizar(const DatosCurso& datos, const Aula* aula,
                    const Docente* docente);

    const std::string& getCodigo() const { return codigo; }
    const std::string& getNombre() const { return nombre; }
    const std::string& getCarrera() const { return carrera; }
    bool esGeneral() const { return carrera.empty(); }
    int getCreditos() const { return creditos; }
    int getSemestre() const { return semestre; }
    const Aula& getAula() const { return *aula; }
    int getVacantes() const { return vacantes; }
    const Horario& getHorario() const { return horario; }
    const Docente* getDocente() const { return docente; }
    const std::vector<std::string>& getPrerrequisitos() const
    {
        return prerrequisitos;
    }
    int getInscritos() const { return inscritos; }
    int cuposLibres() const { return vacantes - inscritos; }
    int enEspera() const { return listaEspera.tamanio(); }

    bool ocuparCupo();
    void liberarCupo();

    void agregarAEspera(const Estudiante* estudiante);
    bool estaEnEspera(const Estudiante* estudiante) const;
    // Devuelve nullptr si nadie espera.
    const Estudiante* siguienteEnEspera();
    // En el orden en que entraron a la cola.
    std::vector<const Estudiante*> estudiantesEnEspera() const;
};
