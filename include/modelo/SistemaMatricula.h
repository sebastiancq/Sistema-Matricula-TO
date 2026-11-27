#pragma once

#include "Matricula.h"
#include "ReglaMatricula.h"
#include <functional>
#include <memory>
#include <vector>

// Parte del sistema que cambio, para que cada observador actualice solo lo
// que le corresponde.
enum class Cambio
{
    Personas,
    Aulas,
    Cursos,
    Matriculas
};

// Modelo del sistema: guarda carreras, personas, aulas, cursos y matriculas,
// aplica sus reglas y avisa de cada cambio a sus observadores. Una regla que
// no se cumple se informa con una excepcion derivada de ErrorMatricula.
class SistemaMatricula
{
private:
    std::vector<std::string> carreras;
    std::vector<std::unique_ptr<Persona>> personas;
    std::vector<std::unique_ptr<Aula>> aulas;
    std::vector<std::unique_ptr<Curso>> cursos;
    std::vector<Matricula> matriculas;
    std::vector<std::function<void(Cambio)>> observadores;
    std::vector<std::unique_ptr<ReglaMatricula>> reglas;

    void notificar(Cambio cambio);
    bool existeCarrera(const std::string& nombre) const;
    Estudiante& estudiante(const std::string& codigo) const;
    Curso& curso(const std::string& codigo) const;
    const Docente* docente(const std::string& codigo) const;

    // Aplica todas las reglas de matricula, en el orden en que se agregaron.
    void comprobarReglas(const Estudiante& estudiante,
                         const Curso& curso) const;
    // Revisa los campos del curso y devuelve el aula elegida.
    const Aula& validarDatosCurso(const DatosCurso& datos) const;
    // Revisa que el aula y el docente esten libres en ese horario. Al editar,
    // el curso no se compara consigo mismo.
    void comprobarRecursos(const DatosCurso& datos, const Docente* docente,
                           const Curso* editado = nullptr) const;
    void inscribir(const Estudiante& estudiante, Curso& curso,
                   const std::string& fecha);
    // Llena los cupos libres con la lista de espera. Quien ya no cumple las
    // reglas sale de la cola sin matricularse.
    std::vector<const Estudiante*>
    promoverDesdeEspera(Curso& curso, const std::string& fecha);

public:
    void agregarRegla(std::unique_ptr<ReglaMatricula> regla);
    // Sistema sin datos con una copia de las mismas reglas, para cargar un
    // archivo sin tocar los datos actuales hasta saber que es valido.
    SistemaMatricula copiaVacia() const;
    // Pasa a este sistema los datos de otro, por ejemplo los de un archivo
    // ya cargado y revisado, y avisa a los observadores.
    void reemplazarDatos(SistemaMatricula& otro);
    void agregarObservador(std::function<void(Cambio)> observador);

    // Las carreras se registran antes que sus estudiantes y cursos.
    void registrarCarrera(const std::string& nombre);

    void registrarPersona(std::unique_ptr<Persona> persona);
    void editarPersona(const std::string& codigo, const std::string& nombre,
                       const std::string& dato);
    void eliminarPersona(const std::string& codigo);
    Persona* buscarPersona(const std::string& codigo) const;

    void registrarAula(const std::string& codigo, int capacidad);
    void editarAula(const std::string& codigo, int capacidad);
    void eliminarAula(const std::string& codigo);
    Aula* buscarAula(const std::string& codigo) const;
    std::vector<const Curso*> cursosEnAula(const Aula& aula) const;

    void crearCurso(const DatosCurso& datos);
    // Los matriculados deben seguir cumpliendo las reglas con los datos
    // nuevos. Si aumentan las vacantes, entran los primeros de la lista de
    // espera; devuelve quienes entraron.
    std::vector<const Estudiante*> editarCurso(const DatosCurso& datos,
                                               const std::string& fecha);
    // Solo se elimina un curso sin matriculados ni lista de espera.
    void eliminarCurso(const std::string& codigo);
    Curso* buscarCurso(const std::string& codigo) const;

    void matricular(const std::string& codigoEstudiante,
                    const std::string& codigoCurso, const std::string& fecha);
    // Devuelve la posicion del estudiante en la lista de espera.
    int ponerEnEspera(const std::string& codigoEstudiante,
                      const std::string& codigoCurso);
    // Devuelve quienes pasaron de la lista de espera al cupo liberado.
    std::vector<const Estudiante*> retirar(const std::string& codigoEstudiante,
                                           const std::string& codigoCurso,
                                           const std::string& fecha);

    bool estaMatriculado(const Estudiante& estudiante,
                         const Curso& curso) const;
    int creditosDe(const Estudiante& estudiante) const;
    std::vector<const Curso*> cursosDe(const Estudiante& estudiante) const;
    std::vector<const Estudiante*> estudiantesDe(const Curso& curso) const;
    std::vector<const Curso*> cursosDeDocente(const Docente& docente) const;

    const std::vector<std::string>& getCarreras() const { return carreras; }
    const std::vector<std::unique_ptr<Persona>>& getPersonas() const
    {
        return personas;
    }
    const std::vector<std::unique_ptr<Aula>>& getAulas() const
    {
        return aulas;
    }
    const std::vector<std::unique_ptr<Curso>>& getCursos() const
    {
        return cursos;
    }
    const std::vector<Matricula>& getMatriculas() const { return matriculas; }
};
