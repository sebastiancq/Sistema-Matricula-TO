#include "modelo/SistemaMatricula.h"
#include "modelo/Excepciones.h"
#include "modelo/FuncionesGenericas.h"

using namespace std;

void SistemaMatricula::agregarRegla(unique_ptr<ReglaMatricula> regla)
{
    reglas.push_back(std::move(regla));
}

SistemaMatricula SistemaMatricula::copiaVacia() const
{
    SistemaMatricula copia;
    for (const auto& regla : reglas)
        copia.agregarRegla(unique_ptr<ReglaMatricula>(regla->clonar()));
    return copia;
}

// Los objetos no cambian de lugar al mover los unique_ptr, asi que las
// matriculas siguen apuntando a sus estudiantes y cursos.
void SistemaMatricula::reemplazarDatos(SistemaMatricula& otro)
{
    carreras = std::move(otro.carreras);
    personas = std::move(otro.personas);
    aulas = std::move(otro.aulas);
    cursos = std::move(otro.cursos);
    matriculas = std::move(otro.matriculas);
    notificar(Cambio::Personas);
    notificar(Cambio::Aulas);
    notificar(Cambio::Cursos);
    notificar(Cambio::Matriculas);
}

void SistemaMatricula::agregarObservador(function<void(Cambio)> observador)
{
    observadores.push_back(observador);
}

void SistemaMatricula::notificar(Cambio cambio)
{
    for (auto& observador : observadores)
        observador(cambio);
}

Estudiante& SistemaMatricula::estudiante(const string& codigo) const
{
    auto* encontrado = dynamic_cast<Estudiante*>(buscarPersona(codigo));
    if (encontrado == nullptr)
        throw NoEncontrado("No hay un estudiante con el código " + codigo +
                           ".");
    return *encontrado;
}

Curso& SistemaMatricula::curso(const string& codigo) const
{
    Curso* encontrado = buscarCurso(codigo);
    if (encontrado == nullptr)
        throw NoEncontrado("No hay un curso con el código " + codigo + ".");
    return *encontrado;
}

// Un codigo vacio significa que el curso aun no tiene docente.
const Docente* SistemaMatricula::docente(const string& codigo) const
{
    if (codigo.empty())
        return nullptr;
    auto* encontrado = dynamic_cast<const Docente*>(buscarPersona(codigo));
    if (encontrado == nullptr)
        throw NoEncontrado("No hay un docente con el código " + codigo + ".");
    return encontrado;
}

bool SistemaMatricula::existeCarrera(const string& nombre) const
{
    for (const string& c : carreras)
        if (c == nombre)
            return true;
    return false;
}

void SistemaMatricula::registrarCarrera(const string& nombre)
{
    if (nombre.empty())
        throw DatosInvalidos("Escriba el nombre de la carrera.");
    if (existeCarrera(nombre))
        throw DatosInvalidos("La carrera " + nombre + " ya está registrada.");
    carreras.push_back(nombre);
    notificar(Cambio::Personas);
}

void SistemaMatricula::registrarPersona(unique_ptr<Persona> persona)
{
    if (persona->getCodigo().empty() || persona->getNombre().empty() ||
        persona->datoPropio().empty())
        throw DatosInvalidos("Complete el código, el nombre y el campo " +
                             persona->etiquetaDato() + ".");
    for (const auto& p : personas)
        if (*p == *persona)
            throw CodigoDuplicado(persona->getCodigo());
    if (dynamic_cast<Estudiante*>(persona.get()) &&
        !existeCarrera(persona->datoPropio()))
        throw NoEncontrado("La carrera " + persona->datoPropio() +
                           " no está registrada.");
    personas.push_back(std::move(persona));
    notificar(Cambio::Personas);
}

// Un estudiante solo cambia de carrera si no lleva ni espera cursos de la
// carrera anterior; los cursos generales no lo impiden.
void SistemaMatricula::editarPersona(const string& codigo, const string& nombre,
                                     const string& dato)
{
    Persona* persona = buscarPersona(codigo);
    if (persona == nullptr)
        throw NoEncontrado("No hay una persona con el código " + codigo + ".");
    if (nombre.empty() || dato.empty())
        throw DatosInvalidos("Complete el nombre y el campo " +
                             persona->etiquetaDato() + ".");
    auto* e = dynamic_cast<Estudiante*>(persona);
    if (e && dato != e->datoPropio())
    {
        if (!existeCarrera(dato))
            throw NoEncontrado("La carrera " + dato + " no está registrada.");
        for (const auto& c : cursos)
            if (!c->esGeneral() &&
                (estaMatriculado(*e, *c) || c->estaEnEspera(e)))
                throw EnUso(e->getNombre() + " tiene matrícula o está en " +
                            "la lista de espera de " + c->getNombre() +
                            ", curso de su carrera actual. Retire esa " +
                            "inscripción antes de cambiar de carrera.");
    }
    persona->setNombre(nombre);
    persona->setDatoPropio(dato);
    notificar(Cambio::Personas);
}

// Un estudiante con matriculas o en espera, o un docente con cursos a cargo,
// no se puede eliminar: otros datos apuntan a el.
void SistemaMatricula::eliminarPersona(const string& codigo)
{
    Persona* persona = buscarPersona(codigo);
    if (persona == nullptr)
        throw NoEncontrado("No hay una persona con el código " + codigo + ".");

    if (auto* e = dynamic_cast<Estudiante*>(persona))
    {
        if (!cursosDe(*e).empty())
            throw EnUso("No se puede eliminar a " + e->getNombre() +
                        " porque tiene matrículas registradas. Retire " +
                        "primero sus matrículas.");
        for (const auto& c : cursos)
            if (c->estaEnEspera(e))
                throw EnUso("No se puede eliminar a " + e->getNombre() +
                            " porque está en la lista de espera de " +
                            c->getNombre() + ".");
    }
    if (auto* d = dynamic_cast<Docente*>(persona))
        if (!cursosDeDocente(*d).empty())
            throw EnUso("No se puede eliminar a " + d->getNombre() +
                        " porque tiene cursos a su cargo.");

    for (size_t i = 0; i < personas.size(); i++)
        if (personas[i].get() == persona)
        {
            personas.erase(personas.begin() + i);
            break;
        }
    notificar(Cambio::Personas);
}

Persona* SistemaMatricula::buscarPersona(const string& codigo) const
{
    return buscarPor(personas, [&codigo](const Persona& p)
                     { return p.getCodigo() == codigo; });
}

const Aula& SistemaMatricula::validarDatosCurso(const DatosCurso& datos) const
{
    if (datos.codigo.empty() || datos.nombre.empty() || datos.aula.empty())
        throw DatosInvalidos("Complete el código, el nombre y el aula.");
    if (datos.creditos < 1 || datos.semestre < 1 || datos.vacantes < 1)
        throw DatosInvalidos("Los créditos, el semestre y las vacantes "
                             "deben ser mayores que 0.");
    if (datos.horario.estaVacio())
        throw DatosInvalidos("Agregue al menos un bloque al horario.");
    if (!datos.carrera.empty() && !existeCarrera(datos.carrera))
        throw NoEncontrado("La carrera " + datos.carrera +
                           " no está registrada.");
    const Aula* aula = buscarAula(datos.aula);
    if (aula == nullptr)
        throw NoEncontrado("El aula " + datos.aula + " no está registrada.");
    if (datos.vacantes > aula->getCapacidad())
        throw DatosInvalidos("El aula " + aula->getCodigo() + " tiene " +
                             to_string(aula->getCapacidad()) +
                             " asientos; las vacantes no pueden ser más.");
    return *aula;
}

void SistemaMatricula::comprobarRecursos(const DatosCurso& datos,
                                         const Docente* docente,
                                         const Curso* editado) const
{
    Bloque comun;
    for (const auto& c : cursos)
    {
        if (c.get() == editado ||
            !datos.horario.seCruzaCon(c->getHorario(), comun))
            continue;
        string cruce = Horario::describir(comun);
        if (c->getAula().getCodigo() == datos.aula)
            throw AulaOcupada(datos.aula, c->getNombre(), cruce);
        if (docente != nullptr && c->getDocente() == docente)
            throw DocenteOcupado(docente->getNombre(), c->getNombre(), cruce);
    }
}

void SistemaMatricula::registrarAula(const string& codigo, int capacidad)
{
    if (codigo.empty() || capacidad < 1)
        throw DatosInvalidos("Escriba el código del aula y una capacidad "
                             "mayor que 0.");
    if (buscarAula(codigo) != nullptr)
        throw CodigoDuplicado(codigo, "un aula");
    aulas.push_back(make_unique<Aula>(codigo, capacidad));
    notificar(Cambio::Aulas);
}

// La capacidad no puede quedar por debajo de las vacantes de algun curso
// que use el aula.
void SistemaMatricula::editarAula(const string& codigo, int capacidad)
{
    Aula* aula = buscarAula(codigo);
    if (aula == nullptr)
        throw NoEncontrado("No hay un aula con el código " + codigo + ".");
    if (capacidad < 1)
        throw DatosInvalidos("La capacidad debe ser mayor que 0.");
    for (const Curso* c : cursosEnAula(*aula))
        if (c->getVacantes() > capacidad)
            throw EnUso(c->getNombre() + " usa el aula " + codigo + " con " +
                        to_string(c->getVacantes()) +
                        " vacantes; la capacidad no puede ser menor.");
    aula->setCapacidad(capacidad);
    notificar(Cambio::Aulas);
}

// Un aula que usa algun curso no se elimina: el curso quedaria sin aula.
void SistemaMatricula::eliminarAula(const string& codigo)
{
    Aula* aula = buscarAula(codigo);
    if (aula == nullptr)
        throw NoEncontrado("No hay un aula con el código " + codigo + ".");
    vector<const Curso*> usan = cursosEnAula(*aula);
    if (!usan.empty())
    {
        string otros;
        if (usan.size() == 2)
            otros = " y otro curso";
        else if (usan.size() > 2)
            otros = " y otros " + to_string(usan.size() - 1) + " cursos";
        throw EnUso("No se puede eliminar el aula " + codigo +
                    " porque la usa " + usan[0]->getNombre() + otros + ".");
    }
    for (size_t i = 0; i < aulas.size(); i++)
        if (aulas[i].get() == aula)
        {
            aulas.erase(aulas.begin() + i);
            break;
        }
    notificar(Cambio::Aulas);
}

Aula* SistemaMatricula::buscarAula(const string& codigo) const
{
    return buscarPor(aulas, [&codigo](const Aula& a)
                     { return a.getCodigo() == codigo; });
}

vector<const Curso*> SistemaMatricula::cursosEnAula(const Aula& aula) const
{
    vector<const Curso*> lista;
    for (const auto& c : cursos)
        if (&c->getAula() == &aula)
            lista.push_back(c.get());
    return lista;
}

void SistemaMatricula::crearCurso(const DatosCurso& datos)
{
    const Aula& aula = validarDatosCurso(datos);
    if (buscarCurso(datos.codigo) != nullptr)
        throw CodigoDuplicado(datos.codigo, "un curso");
    const Docente* elegido = docente(datos.docente);
    comprobarRecursos(datos, elegido);
    cursos.push_back(make_unique<Curso>(datos, &aula, elegido));
    notificar(Cambio::Cursos);
}

// Las reglas de matricula se aplican a cada matriculado con el curso ya
// cambiado y sin contar el curso actual entre los que lleva.
vector<const Estudiante*> SistemaMatricula::editarCurso(const DatosCurso& datos,
                                                        const string& fecha)
{
    Curso& actual = curso(datos.codigo);
    const Aula& aula = validarDatosCurso(datos);
    const Docente* elegido = docente(datos.docente);
    comprobarRecursos(datos, elegido, &actual);
    if (datos.vacantes < actual.getInscritos())
        throw DatosInvalidos(
            actual.getNombre() + " tiene " + to_string(actual.getInscritos()) +
            (actual.getInscritos() == 1 ? " estudiante matriculado"
                                        : " estudiantes matriculados") +
            "; las vacantes no pueden ser menos.");
    Curso propuesto(datos, &aula, elegido);
    for (const Estudiante* e : estudiantesDe(actual))
    {
        vector<const Curso*> otros;
        for (const Curso* c : cursosDe(*e))
            if (c != &actual)
                otros.push_back(c);
        for (const auto& regla : reglas)
            regla->comprobar(*e, propuesto, otros);
    }
    actual.actualizar(datos, &aula, elegido);
    notificar(Cambio::Cursos);
    return promoverDesdeEspera(actual, fecha);
}

void SistemaMatricula::eliminarCurso(const string& codigo)
{
    Curso& c = curso(codigo);
    if (c.getInscritos() > 0)
        throw EnUso("No se puede eliminar " + c.getNombre() +
                    " porque tiene estudiantes matriculados. Retire primero "
                    "sus matrículas.");
    if (c.enEspera() > 0)
        throw EnUso("No se puede eliminar " + c.getNombre() +
                    " porque tiene estudiantes en lista de espera.");
    for (size_t i = 0; i < cursos.size(); i++)
        if (cursos[i].get() == &c)
        {
            cursos.erase(cursos.begin() + i);
            break;
        }
    notificar(Cambio::Cursos);
}

Curso* SistemaMatricula::buscarCurso(const string& codigo) const
{
    return buscarPor(cursos, [&codigo](const Curso& c)
                     { return c.getCodigo() == codigo; });
}

void SistemaMatricula::comprobarReglas(const Estudiante& estudiante,
                                       const Curso& curso) const
{
    vector<const Curso*> actuales = cursosDe(estudiante);
    for (const auto& regla : reglas)
        regla->comprobar(estudiante, curso, actuales);
}

// La matricula repetida se revisa antes del cupo para no ocupar un lugar que
// no se va a usar.
void SistemaMatricula::inscribir(const Estudiante& estudiante, Curso& curso,
                                 const string& fecha)
{
    if (estaMatriculado(estudiante, curso))
        throw MatriculaDuplicada(estudiante.getNombre(), curso.getNombre());
    comprobarReglas(estudiante, curso);
    if (!curso.ocuparCupo())
        throw CursoSinCupo(curso.getNombre());
    matriculas.push_back({ &estudiante, &curso, fecha });
    notificar(Cambio::Matriculas);
}

void SistemaMatricula::matricular(const string& codigoEstudiante,
                                  const string& codigoCurso,
                                  const string& fecha)
{
    inscribir(estudiante(codigoEstudiante), curso(codigoCurso), fecha);
}

// Solo se espera por un curso lleno, y solo si al entrar se cumplirian las
// demas reglas.
int SistemaMatricula::ponerEnEspera(const string& codigoEstudiante,
                                    const string& codigoCurso)
{
    Estudiante& e = estudiante(codigoEstudiante);
    Curso& c = curso(codigoCurso);
    if (estaMatriculado(e, c))
        throw MatriculaDuplicada(e.getNombre(), c.getNombre());
    if (c.estaEnEspera(&e))
        throw DatosInvalidos(e.getNombre() +
                             " ya está en la lista de espera de " +
                             c.getNombre() + ".");
    if (c.cuposLibres() > 0)
        throw DatosInvalidos(c.getNombre() +
                             " todavía tiene cupos disponibles; matricule "
                             "al estudiante directamente.");
    comprobarReglas(e, c);
    c.agregarAEspera(&e);
    notificar(Cambio::Cursos);
    return c.enEspera();
}

vector<const Estudiante*> SistemaMatricula::retirar(
    const string& codigoEstudiante, const string& codigoCurso,
    const string& fecha)
{
    Estudiante& e = estudiante(codigoEstudiante);
    Curso& c = curso(codigoCurso);
    bool encontrada = false;
    for (size_t i = 0; i < matriculas.size() && !encontrada; i++)
        if (matriculas[i].estudiante == &e && matriculas[i].curso == &c)
        {
            matriculas.erase(matriculas.begin() + i);
            encontrada = true;
        }
    if (!encontrada)
        throw NoEncontrado(e.getNombre() + " no tiene matrícula en " +
                           c.getNombre() + ".");
    c.liberarCupo();
    notificar(Cambio::Matriculas);
    return promoverDesdeEspera(c, fecha);
}

vector<const Estudiante*> SistemaMatricula::promoverDesdeEspera(
    Curso& curso, const string& fecha)
{
    vector<const Estudiante*> promovidos;
    while (curso.cuposLibres() > 0)
    {
        const Estudiante* siguiente = curso.siguienteEnEspera();
        if (siguiente == nullptr)
            break;
        try
        {
            inscribir(*siguiente, curso, fecha);
            promovidos.push_back(siguiente);
        }
        catch (const ErrorMatricula&)
        {
        }
    }
    notificar(Cambio::Cursos);
    return promovidos;
}

bool SistemaMatricula::estaMatriculado(const Estudiante& estudiante,
                                       const Curso& curso) const
{
    for (const Matricula& m : matriculas)
        if (m.estudiante == &estudiante && m.curso == &curso)
            return true;
    return false;
}

int SistemaMatricula::creditosDe(const Estudiante& estudiante) const
{
    int total = 0;
    for (const Curso* c : cursosDe(estudiante))
        total += c->getCreditos();
    return total;
}

vector<const Curso*> SistemaMatricula::cursosDe(
    const Estudiante& estudiante) const
{
    vector<const Curso*> lista;
    for (const Matricula& m : matriculas)
        if (m.estudiante == &estudiante)
            lista.push_back(m.curso);
    return lista;
}

vector<const Estudiante*> SistemaMatricula::estudiantesDe(
    const Curso& curso) const
{
    vector<const Estudiante*> lista;
    for (const Matricula& m : matriculas)
        if (m.curso == &curso)
            lista.push_back(m.estudiante);
    return lista;
}

vector<const Curso*> SistemaMatricula::cursosDeDocente(
    const Docente& docente) const
{
    vector<const Curso*> lista;
    for (const auto& c : cursos)
        if (c->getDocente() == &docente)
            lista.push_back(c.get());
    return lista;
}
