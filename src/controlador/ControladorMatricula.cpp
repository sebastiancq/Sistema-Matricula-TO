#include "controlador/ControladorMatricula.h"
#include "modelo/FuncionesGenericas.h"
#include "modelo/PersonaFactory.h"
#include "modelo/RepositorioCsv.h"
#include <QDate>
#include <QFile>
#include <QSaveFile>
#include <sstream>

using namespace std;

static QString q(const string& texto)
{
    return QString::fromStdString(texto);
}

// La matricula y el retiro se registran con la fecha del dia.
static string hoy()
{
    return QDate::currentDate().toString("dd/MM/yyyy").toStdString();
}

static QString carreraDe(const Curso& c)
{
    return c.esGeneral() ? "General" : q(c.getCarrera());
}

ControladorMatricula::ControladorMatricula(VistaPrincipal* vista,
                                           SistemaMatricula* modelo,
                                           QObject* parent)
    : QObject(parent), vista(vista), modelo(modelo)
{
    QStringList tipos;
    for (const string& tipo : PersonaFactory::tiposRegistrados())
        tipos << q(tipo);
    vista->ponerTiposPersona(tipos);

    // Observer: cada cambio del modelo actualiza la parte de la vista que lo
    // muestra. Los cursos y la ficha del estudiante se actualizan siempre,
    // porque muestran nombres de docentes, aulas, cupos y listas de espera.
    // Las aulas cuentan sus cursos, asi que cambian tambien con ellos.
    modelo->agregarObservador(
        [this](Cambio cambio)
        {
            if (cambio == Cambio::Personas)
                actualizarPersonas();
            if (cambio == Cambio::Aulas || cambio == Cambio::Cursos)
                actualizarAulas();
            actualizarCursos();
            actualizarMatriculas();
        });

    connect(vista, &VistaPrincipal::solicitarRegistroPersona, this,
            &ControladorMatricula::onRegistrarPersona);
    connect(vista, &VistaPrincipal::solicitarEdicionPersona, this,
            &ControladorMatricula::onEditarPersona);
    connect(vista, &VistaPrincipal::solicitarEliminarPersona, this,
            &ControladorMatricula::onEliminarPersona);
    connect(vista, &VistaPrincipal::solicitarDetallePersona, this,
            &ControladorMatricula::onDetallePersona);
    connect(vista, &VistaPrincipal::solicitarNuevaCarrera, this,
            &ControladorMatricula::onNuevaCarrera);
    connect(vista, &VistaPrincipal::solicitarRegistroAula, this,
            &ControladorMatricula::onRegistrarAula);
    connect(vista, &VistaPrincipal::solicitarEdicionAula, this,
            &ControladorMatricula::onEditarAula);
    connect(vista, &VistaPrincipal::solicitarEliminarAula, this,
            &ControladorMatricula::onEliminarAula);
    connect(vista, &VistaPrincipal::solicitarDetalleAula, this,
            &ControladorMatricula::onDetalleAula);
    connect(vista, &VistaPrincipal::solicitarCrearCurso, this,
            &ControladorMatricula::onCrearCurso);
    connect(vista, &VistaPrincipal::solicitarCargaCurso, this,
            &ControladorMatricula::onCargarCurso);
    connect(vista, &VistaPrincipal::solicitarEdicionCurso, this,
            &ControladorMatricula::onEditarCurso);
    connect(vista, &VistaPrincipal::solicitarEliminarCurso, this,
            &ControladorMatricula::onEliminarCurso);
    connect(vista, &VistaPrincipal::solicitarDetalleCurso, this,
            &ControladorMatricula::onDetalleCurso);
    connect(vista, &VistaPrincipal::estudianteCambiado, this,
            &ControladorMatricula::actualizarMatriculas);
    connect(vista, &VistaPrincipal::solicitarMatricula, this,
            &ControladorMatricula::onMatricular);
    connect(vista, &VistaPrincipal::solicitarRetiro, this,
            &ControladorMatricula::onRetirar);
    connect(vista, &VistaPrincipal::solicitarAbrir, this,
            &ControladorMatricula::onAbrir);
    connect(vista, &VistaPrincipal::solicitarGuardar, this,
            &ControladorMatricula::onGuardar);
}

void ControladorMatricula::actualizarPersonas()
{
    QStringList carreras;
    for (const string& c : modelo->getCarreras())
        carreras << q(c);
    vista->ponerCarreras(carreras);

    QList<QStringList> filas;
    QList<QPair<QString, QString>> estudiantes, docentes;
    for (const auto& p : modelo->getPersonas())
    {
        filas.append({ q(p->tipo()), q(p->getCodigo()), q(p->getNombre()),
                       q(p->datoPropio()) });
        if (dynamic_cast<const Estudiante*>(p.get()))
            estudiantes.append({ q(p->getCodigo()), q(p->getNombre()) });
        if (dynamic_cast<const Docente*>(p.get()))
            docentes.append({ q(p->getCodigo()), q(p->getNombre()) });
    }
    vista->mostrarPersonas(filas);
    vista->ponerDocentes(docentes);
    vista->ponerEstudiantes(estudiantes);
}

void ControladorMatricula::actualizarAulas()
{
    QList<QStringList> filas;
    for (const auto& a : modelo->getAulas())
        filas.append({ q(a->getCodigo()), QString::number(a->getCapacidad()),
                       QString::number(modelo->cursosEnAula(*a).size()) });
    vista->mostrarAulas(filas);
}

void ControladorMatricula::actualizarCursos()
{
    QList<QStringList> filas;
    for (const auto& c : modelo->getCursos())
    {
        const Docente* d = c->getDocente();
        filas.append(
            { q(c->getCodigo()), q(c->getNombre()), carreraDe(*c),
              QString::number(c->getSemestre()),
              QString::number(c->getCreditos()), q(c->getAula().getCodigo()),
              QString::number(c->getVacantes()), q(aTexto(c->getHorario())),
              d ? q(d->getNombre()) : "", QString::number(c->getInscritos()),
              QString::number(c->enEspera()) });
    }
    vista->mostrarCursos(filas);
}

// Muestra las matriculas del estudiante elegido con su grilla semanal, sus
// creditos y las listas de espera en las que esta. La lista de cursos para
// matricular solo trae los generales y los de su carrera.
void ControladorMatricula::actualizarMatriculas()
{
    auto* e = dynamic_cast<const Estudiante*>(
        modelo->buscarPersona(vista->estudianteElegido().toStdString()));
    if (e == nullptr)
    {
        vista->ponerCursosMatricula({});
        vista->mostrarMatriculas("Registre un estudiante para matricularlo.",
                                 {}, {});
        return;
    }

    QList<OpcionCurso> opciones;
    for (const auto& c : modelo->getCursos())
        if (c->esGeneral() || c->getCarrera() == e->datoPropio())
            opciones.append(
                { q(c->getCodigo()),
                  QString("%1 %2, %3 créd., %4")
                      .arg(q(c->getCodigo()), q(c->getNombre()))
                      .arg(c->getCreditos())
                      .arg(c->cuposLibres() == 1
                               ? QString("1 cupo")
                               : QString("%1 cupos").arg(c->cuposLibres())),
                  c->getSemestre() });
    vista->ponerCursosMatricula(opciones);

    QList<QStringList> filas;
    QList<BloqueVista> horario;
    for (const Matricula& m : modelo->getMatriculas())
    {
        if (m.estudiante != e)
            continue;
        const Curso* c = m.curso;
        filas.append({ q(m.fecha), q(c->getCodigo()), q(c->getNombre()),
                       QString::number(c->getCreditos()),
                       q(aTexto(c->getHorario())) });
        for (const Bloque& b : c->getHorario().getBloques())
            horario.append({ b.dia, b.inicio, b.fin,
                             q(c->getCodigo() + "\n" + c->getNombre() + "\n" +
                               c->getAula().getCodigo()) });
    }

    QString cantidad = filas.size() == 1
                           ? QString("1 curso")
                           : QString("%1 cursos").arg(filas.size());
    QString resumen = QString("%1, %2: %3 créditos en %4. Cursos aprobados: "
                              "%5.")
                          .arg(q(e->getNombre()), q(e->datoPropio()))
                          .arg(modelo->creditosDe(*e))
                          .arg(cantidad)
                          .arg(e->getAprobados().size());
    for (const auto& c : modelo->getCursos())
    {
        auto espera = c->estudiantesEnEspera();
        for (size_t i = 0; i < espera.size(); i++)
            if (espera[i] == e)
                resumen += QString("\nEn espera de %1: puesto %2 de %3.")
                               .arg(q(c->getNombre()))
                               .arg(i + 1)
                               .arg(espera.size());
    }
    vista->mostrarMatriculas(resumen, filas, horario);
}

// Codigo y nombre de cada curso, separados por comas.
QString ControladorMatricula::listaDeCursos(const vector<string>& codigos) const
{
    QStringList lista;
    for (const string& codigo : codigos)
    {
        const Curso* c = modelo->buscarCurso(codigo);
        lista << q(codigo + (c ? " " + c->getNombre() : ""));
    }
    return lista.join(", ");
}

QString ControladorMatricula::nombres(
    const vector<const Estudiante*>& estudiantes) const
{
    QStringList lista;
    for (const Estudiante* e : estudiantes)
        lista << q(e->getNombre());
    return lista.join(", ");
}

void ControladorMatricula::onRegistrarPersona()
{
    QString tipo = vista->tipoPersona();
    QString nombre = vista->nombrePersona();
    if (intentar("No se pudo registrar",
                 [&]
                 {
                     modelo->registrarPersona(PersonaFactory::crearPersona(
                         tipo.toStdString(), nombre.toStdString(),
                         vista->codigoPersona().toStdString(),
                         vista->datoPersona().toStdString()));
                 }))
    {
        vista->limpiarPersona();
        vista->mostrarResultado("Se registró a " + nombre + " como " +
                                tipo.toLower() + ".");
    }
}

void ControladorMatricula::onEditarPersona()
{
    QString nombre = vista->nombrePersona();
    if (intentar("No se pudo guardar",
                 [&]
                 {
                     modelo->editarPersona(vista->codigoPersona().toStdString(),
                                           nombre.toStdString(),
                                           vista->datoPersona().toStdString());
                 }))
    {
        vista->limpiarPersona();
        vista->mostrarResultado("Se guardaron los cambios de " + nombre + ".");
    }
}

void ControladorMatricula::onEliminarPersona()
{
    QString nombre = vista->nombrePersona();
    if (!vista->confirmar("Eliminar persona",
                          "¿Eliminar a " + nombre + " del sistema?"))
        return;
    if (intentar(
            "No se pudo eliminar", [&]
            { modelo->eliminarPersona(vista->codigoPersona().toStdString()); }))
    {
        vista->limpiarPersona();
        vista->mostrarResultado("Se eliminó a " + nombre + " del sistema.");
    }
}

void ControladorMatricula::onDetallePersona(const QString& codigo)
{
    Persona* p = modelo->buscarPersona(codigo.toStdString());
    if (p == nullptr)
        return;
    QString tituloLista;
    QStringList cursos;
    if (auto* e = dynamic_cast<Estudiante*>(p))
    {
        for (const Curso* c : modelo->cursosDe(*e))
            cursos << q(c->getNombre() + ", " + aTexto(c->getHorario()));
        tituloLista = QString("Cursos matriculados (%1 créditos):")
                          .arg(modelo->creditosDe(*e));
    }
    else if (auto* d = dynamic_cast<Docente*>(p))
    {
        for (const Curso* c : modelo->cursosDeDocente(*d))
            cursos << q(c->getNombre() + ", " + aTexto(c->getHorario()));
        tituloLista = "Cursos a cargo:";
    }
    vista->mostrarDetalle(q(p->descripcion()), tituloLista, cursos);
}

void ControladorMatricula::onNuevaCarrera(const QString& nombre)
{
    if (intentar("No se pudo agregar la carrera",
                 [&] { modelo->registrarCarrera(nombre.toStdString()); }))
    {
        vista->elegirCarrera(nombre);
        vista->mostrarResultado("Se agregó la carrera " + nombre + ".");
    }
}

void ControladorMatricula::onRegistrarAula()
{
    QString codigo = vista->codigoAula();
    if (intentar("No se pudo registrar el aula",
                 [&]
                 {
                     modelo->registrarAula(codigo.toStdString(),
                                           vista->capacidadAula());
                 }))
    {
        vista->limpiarAula();
        vista->mostrarResultado("Se registró el aula " + codigo + ".");
    }
}

void ControladorMatricula::onEditarAula()
{
    QString codigo = vista->codigoAula();
    if (intentar("No se pudo guardar el aula",
                 [&]
                 {
                     modelo->editarAula(codigo.toStdString(),
                                        vista->capacidadAula());
                 }))
    {
        vista->limpiarAula();
        vista->mostrarResultado("Se guardaron los cambios del aula " + codigo +
                                ".");
    }
}

void ControladorMatricula::onEliminarAula()
{
    QString codigo = vista->codigoAula();
    if (!vista->confirmar("Eliminar aula",
                          "¿Eliminar el aula " + codigo + " del sistema?"))
        return;
    if (intentar("No se pudo eliminar el aula",
                 [&] { modelo->eliminarAula(codigo.toStdString()); }))
    {
        vista->limpiarAula();
        vista->mostrarResultado("Se eliminó el aula " + codigo + ".");
    }
}

void ControladorMatricula::onDetalleAula(const QString& codigo)
{
    Aula* a = modelo->buscarAula(codigo.toStdString());
    if (a == nullptr)
        return;
    QStringList cursos;
    for (const Curso* c : modelo->cursosEnAula(*a))
        cursos << q(c->getCodigo() + " " + c->getNombre() + ", " +
                    aTexto(c->getHorario()));
    vista->mostrarDetalle(QString("Aula %1\nCapacidad: %2 asientos")
                              .arg(codigo)
                              .arg(a->getCapacidad()),
                          "Cursos que la usan:", cursos);
}

// Los bloques del formulario forman el horario; si alguno se superpone con
// otro, Horario::agregar lo rechaza antes de llegar al sistema.
DatosCurso ControladorMatricula::datosCursoDeVista() const
{
    Horario horario;
    for (const BloqueVista& b : vista->bloquesHorario())
        horario.agregar({ b.dia, b.inicio, b.fin });
    const Curso* actual =
        modelo->buscarCurso(vista->codigoCurso().toStdString());
    return { vista->codigoCurso().toStdString(),
             vista->nombreCurso().toStdString(),
             vista->carreraCurso().toStdString(),
             vista->creditos(),
             vista->semestre(),
             vista->aula().toStdString(),
             vista->vacantes(),
             horario,
             vista->docente().toStdString(),
             actual ? actual->getPrerrequisitos() : vector<string>() };
}

void ControladorMatricula::onCrearCurso()
{
    QString nombre = vista->nombreCurso();
    if (intentar("No se pudo crear el curso",
                 [&] { modelo->crearCurso(datosCursoDeVista()); }))
    {
        vista->limpiarCurso();
        vista->mostrarResultado("Se creó el curso " + nombre + ".");
    }
}

void ControladorMatricula::onCargarCurso(const QString& codigo)
{
    Curso* c = modelo->buscarCurso(codigo.toStdString());
    if (c == nullptr)
        return;
    QList<BloqueVista> bloques;
    for (const Bloque& b : c->getHorario().getBloques())
        bloques.append({ b.dia, b.inicio, b.fin, "" });
    vista->cargarCurso(
        { q(c->getCodigo()), q(c->getNombre()), q(c->getCarrera()),
          c->getCreditos(), c->getSemestre(), q(c->getAula().getCodigo()),
          c->getVacantes(), bloques,
          c->getDocente() ? q(c->getDocente()->getCodigo()) : "" });
}

// Si al subir las vacantes entra alguien de la lista de espera, se avisa.
void ControladorMatricula::onEditarCurso()
{
    QString nombre = vista->nombreCurso();
    vector<const Estudiante*> entraron;
    if (!intentar(
            "No se pudo guardar el curso", [&]
            { entraron = modelo->editarCurso(datosCursoDeVista(), hoy()); }))
        return;
    vista->limpiarCurso();
    QString texto = "Se guardaron los cambios de " + nombre + ".";
    if (!entraron.empty())
        texto += " Pasaron de la lista de espera a la matrícula: " +
                 nombres(entraron) + ".";
    vista->mostrarResultado(texto);
}

void ControladorMatricula::onEliminarCurso()
{
    QString nombre = vista->nombreCurso();
    if (!vista->confirmar("Eliminar curso",
                          "¿Eliminar el curso " + nombre + " del sistema?"))
        return;
    if (intentar(
            "No se pudo eliminar el curso",
            [&] { modelo->eliminarCurso(vista->codigoCurso().toStdString()); }))
    {
        vista->limpiarCurso();
        vista->mostrarResultado("Se eliminó el curso " + nombre + ".");
    }
}

// Muestra los matriculados y la lista de espera en el orden de la cola.
void ControladorMatricula::onDetalleCurso(const QString& codigo)
{
    Curso* c = modelo->buscarCurso(codigo.toStdString());
    if (c == nullptr)
        return;
    QString docente =
        c->getDocente() ? q(c->getDocente()->getNombre()) : "sin asignar";
    QString descripcion =
        QString("%1 %2\nCarrera: %3\nAula %4: %5 de %6 vacantes ocupadas\n"
                "Horario: %7\nDocente: %8\nPrerrequisitos: %9")
            .arg(q(c->getCodigo()), q(c->getNombre()), carreraDe(*c),
                 q(c->getAula().getCodigo()))
            .arg(c->getInscritos())
            .arg(c->getVacantes())
            .arg(q(aTexto(c->getHorario())), docente,
                 c->getPrerrequisitos().empty()
                     ? "ninguno"
                     : listaDeCursos(c->getPrerrequisitos()));
    QStringList lista;
    for (const Estudiante* e : modelo->estudiantesDe(*c))
        lista << q(aTexto(*e));
    auto espera = c->estudiantesEnEspera();
    for (size_t i = 0; i < espera.size(); i++)
        lista << QString("En espera, puesto %1: %2")
                     .arg(i + 1)
                     .arg(q(aTexto(*espera[i])));
    vista->mostrarDetalle(descripcion,
                          "Matriculados y lista de espera:", lista);
}

// Si el curso esta lleno, ofrece la lista de espera.
void ControladorMatricula::onMatricular()
{
    string estudiante = vista->estudianteElegido().toStdString();
    string curso = vista->cursoElegido().toStdString();
    if (estudiante.empty() || curso.empty())
    {
        vista->mostrarError("Datos incompletos",
                            "Elija un estudiante y un curso.");
        return;
    }
    QString nombreCurso = q(modelo->buscarCurso(curso)->getNombre());
    QString nombreEstudiante =
        q(modelo->buscarPersona(estudiante)->getNombre());
    try
    {
        modelo->matricular(estudiante, curso, hoy());
        vista->mostrarResultado("Se matriculó a " + nombreEstudiante + " en " +
                                nombreCurso + ".");
    }
    catch (const CursoSinCupo& e)
    {
        if (!vista->confirmar("Curso lleno", q(e.what()) + "\n¿Agregar a " +
                                                 nombreEstudiante +
                                                 " a la lista de espera?"))
            return;
        int puesto = 0;
        if (intentar("No se pudo agregar a la lista de espera", [&]
                     { puesto = modelo->ponerEnEspera(estudiante, curso); }))
            vista->mostrarResultado(
                QString("%1 quedó en el puesto %2 de la lista de espera de %3.")
                    .arg(nombreEstudiante)
                    .arg(puesto)
                    .arg(nombreCurso));
    }
    catch (const FaltaPrerrequisito& e)
    {
        vista->mostrarError("No se pudo matricular",
                            "Para llevar " + nombreCurso + ", " +
                                nombreEstudiante + " debe aprobar antes " +
                                listaDeCursos(e.getFaltantes()) + ".");
    }
    catch (const ErrorMatricula& e)
    {
        vista->mostrarError("No se pudo matricular", q(e.what()));
    }
}

void ControladorMatricula::onRetirar()
{
    string curso = vista->cursoDeMatriculaElegida().toStdString();
    if (curso.empty())
    {
        vista->mostrarError(
            "Sin matrícula elegida",
            "Elija en la tabla la matrícula que desea retirar.");
        return;
    }
    QString nombreCurso = q(modelo->buscarCurso(curso)->getNombre());
    QString nombreEstudiante =
        q(modelo->buscarPersona(vista->estudianteElegido().toStdString())
              ->getNombre());
    if (!vista->confirmar("Retirar matrícula", "¿Retirar a " +
                                                   nombreEstudiante + " de " +
                                                   nombreCurso + "?"))
        return;
    vector<const Estudiante*> promovidos;
    if (!intentar("No se pudo retirar",
                  [&]
                  {
                      promovidos = modelo->retirar(
                          vista->estudianteElegido().toStdString(), curso,
                          hoy());
                  }))
        return;
    QString texto =
        "Se retiró a " + nombreEstudiante + " de " + nombreCurso + ".";
    if (!promovidos.empty())
        texto += " El cupo liberado pasó a " + nombres(promovidos) +
                 ", de la lista de espera.";
    vista->mostrarResultado(texto);
}

bool ControladorMatricula::cargarArchivo(const QString& ruta)
{
    QFile archivo(ruta);
    if (!archivo.open(QIODevice::ReadOnly))
    {
        vista->mostrarError("No se pudo abrir",
                            "No se pudo leer " + ruta + ".");
        return false;
    }
    istringstream entrada(archivo.readAll().toStdString());
    if (!intentar("Archivo con errores",
                  [&] { RepositorioCsv::cargar(entrada, *modelo); }))
        return false;
    vista->mostrarResultado(
        QString("Datos cargados: %1 personas, %2 aulas, %3 cursos y %4 "
                "matrículas.")
            .arg(modelo->getPersonas().size())
            .arg(modelo->getAulas().size())
            .arg(modelo->getCursos().size())
            .arg(modelo->getMatriculas().size()));
    return true;
}

bool ControladorMatricula::escribir(const QString& ruta) const
{
    ostringstream salida;
    RepositorioCsv::guardar(*modelo, salida);
    QSaveFile archivo(ruta);
    return archivo.open(QIODevice::WriteOnly) &&
           archivo.write(QByteArray::fromStdString(salida.str())) >= 0 &&
           archivo.commit();
}

bool ControladorMatricula::guardarArchivo(const QString& ruta)
{
    if (!escribir(ruta))
    {
        vista->mostrarError("No se pudo guardar",
                            "No se pudo escribir " + ruta + ".");
        return false;
    }
    vista->mostrarResultado("Datos guardados en " + ruta + ".");
    return true;
}

// Observer: la sesion es otro observador del modelo, igual que la vista.
void ControladorMatricula::guardarSesionEn(const QString& ruta)
{
    modelo->agregarObservador(
        [this, ruta](Cambio)
        {
            if (!escribir(ruta))
                vista->mostrarError("No se pudo guardar la sesión",
                                    "No se pudo escribir " + ruta +
                                        ". Use Guardar datos para no perder "
                                        "los cambios.");
        });
}

void ControladorMatricula::onAbrir()
{
    QString ruta = vista->elegirArchivo(false);
    if (!ruta.isEmpty() &&
        vista->confirmar("Abrir datos", "Los datos actuales se reemplazarán "
                                        "por los del archivo. ¿Continuar?"))
        cargarArchivo(ruta);
}

void ControladorMatricula::onGuardar()
{
    QString ruta = vista->elegirArchivo(true);
    if (!ruta.isEmpty())
        guardarArchivo(ruta);
}
