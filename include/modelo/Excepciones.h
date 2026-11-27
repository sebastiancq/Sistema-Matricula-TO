#pragma once

#include <stdexcept>
#include <string>
#include <vector>

// Jerarquia de errores del sistema. Todas derivan de ErrorMatricula, asi el
// controlador puede capturar un caso concreto o todos con un solo catch.
class ErrorMatricula : public std::runtime_error
{
public:
    explicit ErrorMatricula(const std::string& mensaje)
        : std::runtime_error(mensaje)
    {
    }
};

class DatosInvalidos : public ErrorMatricula
{
public:
    explicit DatosInvalidos(const std::string& mensaje)
        : ErrorMatricula(mensaje)
    {
    }
};

// El segundo dato dice que se repite: una persona, un aula o un curso.
class CodigoDuplicado : public ErrorMatricula
{
public:
    explicit CodigoDuplicado(const std::string& codigo,
                             const std::string& entidad = "una persona")
        : ErrorMatricula("Ya existe " + entidad + " con el código " + codigo +
                         ".")
    {
    }
};

class NoEncontrado : public ErrorMatricula
{
public:
    explicit NoEncontrado(const std::string& mensaje) : ErrorMatricula(mensaje)
    {
    }
};

// Una persona, un aula o un curso no se puede eliminar, ni un estudiante
// cambiar de carrera, mientras otros datos dependan de ellos.
class EnUso : public ErrorMatricula
{
public:
    explicit EnUso(const std::string& mensaje) : ErrorMatricula(mensaje) {}
};

class MatriculaDuplicada : public ErrorMatricula
{
public:
    MatriculaDuplicada(const std::string& estudiante, const std::string& curso)
        : ErrorMatricula(estudiante + " ya tiene matrícula en " + curso + ".")
    {
    }
};

// Quien la captura puede ofrecer la lista de espera.
class CursoSinCupo : public ErrorMatricula
{
public:
    explicit CursoSinCupo(const std::string& curso)
        : ErrorMatricula(curso + " no tiene cupos disponibles.")
    {
    }
};

class OtraCarrera : public ErrorMatricula
{
public:
    OtraCarrera(const std::string& curso, const std::string& carreraCurso,
                const std::string& estudiante,
                const std::string& carreraEstudiante)
        : ErrorMatricula(curso + " es un curso de " + carreraCurso + " y " +
                         estudiante + " estudia " + carreraEstudiante + ".")
    {
    }
};

class LimiteCreditos : public ErrorMatricula
{
public:
    LimiteCreditos(const std::string& estudiante, const std::string& curso,
                   int actuales, int creditosCurso, int maximo)
        : ErrorMatricula(estudiante + " tiene " + std::to_string(actuales) +
                         " créditos matriculados. Con los " +
                         std::to_string(creditosCurso) + " de " + curso +
                         " sumaría " +
                         std::to_string(actuales + creditosCurso) +
                         " y el máximo por semestre es " +
                         std::to_string(maximo) + ".")
    {
    }
};

// El texto del cruce lo arma Horario::describir: dia y horas en que
// coinciden los dos cursos.
class CruceHorario : public ErrorMatricula
{
public:
    CruceHorario(const std::string& estudiante, const std::string& curso,
                 const std::string& otroCurso, const std::string& cruce)
        : ErrorMatricula(curso + " tiene clase " + cruce +
                         ", a la misma hora que " + otroCurso +
                         ", curso en el que " + estudiante +
                         " ya tiene matrícula.")
    {
    }
};

// Guarda los codigos que faltan; el controlador los muestra con su nombre.
class FaltaPrerrequisito : public ErrorMatricula
{
private:
    std::vector<std::string> faltantes;

    static std::string unir(const std::vector<std::string>& codigos)
    {
        std::string texto;
        for (size_t i = 0; i < codigos.size(); i++)
            texto += (i == 0 ? "" : ", ") + codigos[i];
        return texto;
    }

public:
    FaltaPrerrequisito(const std::string& estudiante, const std::string& curso,
                       const std::vector<std::string>& faltantes)
        : ErrorMatricula("Para llevar " + curso + ", " + estudiante +
                         " debe aprobar antes " + unir(faltantes) + "."),
          faltantes(faltantes)
    {
    }

    const std::vector<std::string>& getFaltantes() const { return faltantes; }
};

// Dos cursos no pueden usar la misma aula a la misma hora.
class AulaOcupada : public ErrorMatricula
{
public:
    AulaOcupada(const std::string& aula, const std::string& otroCurso,
                const std::string& cruce)
        : ErrorMatricula("El aula " + aula + " ya está asignada a " +
                         otroCurso + " " + cruce + ".")
    {
    }
};

// Un docente no puede dictar dos cursos a la misma hora.
class DocenteOcupado : public ErrorMatricula
{
public:
    DocenteOcupado(const std::string& docente, const std::string& otroCurso,
                   const std::string& cruce)
        : ErrorMatricula(docente + " ya dicta " + otroCurso + " " + cruce +
                         ".")
    {
    }
};
