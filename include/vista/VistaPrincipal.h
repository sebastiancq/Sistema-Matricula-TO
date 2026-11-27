#pragma once

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>

// Datos de un curso que la vista necesita para la lista de matricula.
struct OpcionCurso
{
    QString codigo;
    QString texto;
    int semestre;
};

// Bloque de un horario: dia, con 0 para el lunes, horas de inicio y fin, y
// el texto que se muestra en la grilla semanal.
struct BloqueVista
{
    int dia;
    int inicio;
    int fin;
    QString texto;
};

// Datos de un curso para llenar el formulario al editarlo.
struct CursoVista
{
    QString codigo;
    QString nombre;
    QString carrera;
    int creditos;
    int semestre;
    QString aula;
    int vacantes;
    QList<BloqueVista> bloques;
    QString docente;
};

// Vista: arma las cuatro pestanas, entrega lo que el usuario escribe y avisa
// con senales lo que pide. No conoce el modelo; el controlador le pasa los
// datos ya convertidos en texto.
class VistaPrincipal : public QWidget
{
    Q_OBJECT

public:
    explicit VistaPrincipal(QWidget* parent = nullptr);

    // Pestana Personas
    void ponerTiposPersona(const QStringList& tipos);
    void ponerCarreras(const QStringList& carreras);
    void elegirCarrera(const QString& carrera);
    void mostrarPersonas(const QList<QStringList>& filas);
    QString tipoPersona() const { return cmbTipo->currentText(); }
    QString codigoPersona() const { return edtCodigo->text().trimmed(); }
    QString nombrePersona() const { return edtNombre->text().trimmed(); }
    QString datoPersona() const;
    void limpiarPersona();

    // Pestana Aulas. Cada fila trae codigo, capacidad y cursos que la usan.
    void mostrarAulas(const QList<QStringList>& filas);
    QString codigoAula() const { return edtCodigoAula->text().trimmed(); }
    int capacidadAula() const { return spnCapacidadAula->value(); }
    void limpiarAula();

    // Pestana Cursos
    void ponerDocentes(const QList<QPair<QString, QString>>& docentes);
    void mostrarCursos(const QList<QStringList>& filas);
    QString codigoCurso() const { return edtCodigoCurso->text().trimmed(); }
    QString nombreCurso() const { return edtNombreCurso->text().trimmed(); }
    QString carreraCurso() const
    {
        return cmbCarreraCurso->currentData().toString();
    }
    int creditos() const { return spnCreditos->value(); }
    int semestre() const { return spnSemestre->value(); }
    QString aula() const { return cmbAula->currentData().toString(); }
    int vacantes() const { return spnVacantes->value(); }
    QList<BloqueVista> bloquesHorario() const;
    QString docente() const { return cmbDocente->currentData().toString(); }
    void cargarCurso(const CursoVista& curso);
    void limpiarCurso();

    // Pestana Matricula
    void ponerEstudiantes(const QList<QPair<QString, QString>>& estudiantes);
    void ponerCursosMatricula(const QList<OpcionCurso>& cursos);
    void mostrarMatriculas(const QString& resumen,
                           const QList<QStringList>& filas,
                           const QList<BloqueVista>& horario);
    QString estudianteElegido() const
    {
        return cmbEstudiante->currentData().toString();
    }
    QString cursoElegido() const { return cmbCurso->currentData().toString(); }
    QString cursoDeMatriculaElegida() const;

    void mostrarResultado(const QString& texto);
    void mostrarError(const QString& titulo, const QString& texto);
    bool confirmar(const QString& titulo, const QString& texto);
    void mostrarDetalle(const QString& descripcion, const QString& tituloLista,
                        const QStringList& lista);
    QString elegirArchivo(bool paraGuardar);

signals:
    void solicitarRegistroPersona();
    void solicitarEdicionPersona();
    void solicitarEliminarPersona();
    void solicitarDetallePersona(const QString& codigo);
    void solicitarNuevaCarrera(const QString& nombre);
    void solicitarRegistroAula();
    void solicitarEdicionAula();
    void solicitarEliminarAula();
    void solicitarDetalleAula(const QString& codigo);
    void solicitarCrearCurso();
    void solicitarCargaCurso(const QString& codigo);
    void solicitarEdicionCurso();
    void solicitarEliminarCurso();
    void solicitarDetalleCurso(const QString& codigo);
    void estudianteCambiado();
    void solicitarMatricula();
    void solicitarRetiro();
    void solicitarAbrir();
    void solicitarGuardar();

private slots:
    void cargarPersonaElegida();
    void actualizarCamposTipo(const QString& tipo);
    void pedirNuevaCarrera();
    void cargarAulaElegida();
    void aulaDeCursoCambiada();
    void agregarBloque();
    void filtrarPersonas();
    void filtrarCursos();
    void filtrarCursosMatricula();

private:
    QWidget* crearPestanaPersonas();
    QWidget* crearPestanaAulas();
    QWidget* crearPestanaCursos();
    QWidget* crearPestanaMatricula();
    void llenarTabla(QTableWidget* tabla, const QList<QStringList>& filas);
    void llenarGrilla(QTableWidget* grilla, const QList<BloqueVista>& bloques);
    void modoEdicionPersona(bool editando);
    void modoEdicionAula(bool editando);
    void modoEdicionCurso(bool editando);
    bool editandoCurso() const { return edtCodigoCurso->isReadOnly(); }

    QComboBox* cmbTipo;
    QLineEdit* edtCodigo;
    QLineEdit* edtNombre;
    QLabel* lblDato;
    QLineEdit* edtDato;
    QComboBox* cmbCarrera;
    QPushButton* btnNuevaCarrera;
    QPushButton* btnRegistrar;
    QPushButton* btnEditarPersona;
    QPushButton* btnEliminarPersona;
    QLineEdit* edtBuscarPersona;
    QTableWidget* tablaPersonas;

    QLineEdit* edtCodigoAula;
    QSpinBox* spnCapacidadAula;
    QPushButton* btnRegistrarAula;
    QPushButton* btnEditarAula;
    QPushButton* btnEliminarAula;
    QTableWidget* tablaAulas;

    QLineEdit* edtCodigoCurso;
    QLineEdit* edtNombreCurso;
    QComboBox* cmbCarreraCurso;
    QSpinBox* spnCreditos;
    QSpinBox* spnSemestre;
    QComboBox* cmbAula;
    QSpinBox* spnVacantes;
    QComboBox* cmbDia;
    QComboBox* cmbInicio;
    QComboBox* cmbFin;
    QTableWidget* tablaBloques;
    QComboBox* cmbDocente;
    QPushButton* btnCrearCurso;
    QPushButton* btnEditarCurso;
    QPushButton* btnEliminarCurso;
    QLineEdit* edtBuscarCurso;
    QComboBox* cmbFiltroSemestre;
    QTableWidget* tablaCursos;

    QComboBox* cmbEstudiante;
    QComboBox* cmbSemestreMatricula;
    QComboBox* cmbCurso;
    QLabel* lblResumen;
    QTableWidget* tablaMatriculas;
    QTableWidget* grillaEstudiante;
    QList<OpcionCurso> opcionesCurso;

    QLabel* lblResultado;
};
