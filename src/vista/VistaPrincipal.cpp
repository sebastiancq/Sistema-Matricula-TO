#include "vista/VistaPrincipal.h"
#include "vista/DialogoDetalle.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QStyle>
#include <QTabWidget>
#include <QVBoxLayout>

static const QStringList DIAS = { "Lunes", "Martes", "Miércoles", "Jueves",
                                  "Viernes" };

static QString hora(int h)
{
    return QString("%1:00").arg(h, 2, 10, QChar('0'));
}

// Boton con un icono estandar del estilo de Qt.
static QPushButton* boton(QWidget* padre, const QString& texto,
                          QStyle::StandardPixmap icono)
{
    return new QPushButton(padre->style()->standardIcon(icono), texto, padre);
}

static QTableWidget* tabla(QWidget* padre, const QStringList& columnas)
{
    auto* t = new QTableWidget(0, columnas.size(), padre);
    t->setHorizontalHeaderLabels(columnas);
    t->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    t->horizontalHeader()->setStretchLastSection(true);
    t->verticalHeader()->hide();
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    return t;
}

// Campo de busqueda con el texto de ayuda y el boton para borrarlo.
static QLineEdit* buscador(QWidget* padre, const QString& ayuda)
{
    auto* campo = new QLineEdit(padre);
    campo->setPlaceholderText(ayuda);
    campo->setClearButtonEnabled(true);
    return campo;
}

// Texto sin tildes y en minusculas, para que la busqueda no dependa de
// como se escriban.
static QString sinTildes(const QString& texto)
{
    QString limpio;
    for (const QChar& c : texto.normalized(QString::NormalizationForm_D))
        if (c.category() != QChar::Mark_NonSpacing)
            limpio += c;
    return limpio.toLower();
}

// Una fila coincide si alguna de sus celdas contiene el texto buscado.
static bool filaContiene(QTableWidget* tabla, int fila, const QString& buscado)
{
    if (buscado.isEmpty())
        return true;
    for (int c = 0; c < tabla->columnCount(); c++)
    {
        QTableWidgetItem* item = tabla->item(fila, c);
        if (item && sinTildes(item->text()).contains(buscado))
            return true;
    }
    return false;
}

// Opcion de todos los semestres y una por cada semestre del 1 al 10. Cada
// opcion guarda su numero; la de todos guarda 0.
static QComboBox* filtroSemestre(QWidget* padre)
{
    auto* combo = new QComboBox(padre);
    combo->addItem("Todos los semestres", 0);
    for (int s = 1; s <= 10; s++)
        combo->addItem(QString("Semestre %1").arg(s), s);
    return combo;
}

// Al volver a llenar una lista se conserva la opcion que estaba elegida; si
// ya no existe, queda la primera.
static void elegirDato(QComboBox* combo, const QString& dato)
{
    int indice = combo->findData(dato);
    combo->setCurrentIndex(indice >= 0 ? indice : 0);
}

// Grilla de lunes a viernes con una fila por hora, de 7 a 22.
static QTableWidget* grillaSemanal(QWidget* padre)
{
    auto* grilla = new QTableWidget(15, DIAS.size(), padre);
    grilla->setHorizontalHeaderLabels(DIAS);
    QStringList horas;
    for (int h = 7; h < 22; h++)
        horas << hora(h) + " - " + hora(h + 1);
    grilla->setVerticalHeaderLabels(horas);
    grilla->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    grilla->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    grilla->setEditTriggers(QAbstractItemView::NoEditTriggers);
    grilla->setSelectionMode(QAbstractItemView::NoSelection);
    return grilla;
}

VistaPrincipal::VistaPrincipal(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("Sistema de registro y matrícula");

    auto* pestanas = new QTabWidget(this);
    pestanas->addTab(crearPestanaPersonas(), "Personas");
    pestanas->addTab(crearPestanaAulas(), "Aulas");
    pestanas->addTab(crearPestanaCursos(), "Cursos");
    pestanas->addTab(crearPestanaMatricula(), "Matrícula");

    auto* btnAbrir = boton(this, "Abrir datos…", QStyle::SP_DialogOpenButton);
    auto* btnGuardar =
        boton(this, "Guardar datos…", QStyle::SP_DialogSaveButton);
    lblResultado = new QLabel("Sin cambios aún", this);
    auto* barra = new QHBoxLayout();
    barra->addWidget(btnAbrir);
    barra->addWidget(btnGuardar);
    barra->addWidget(lblResultado, 1);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(pestanas);
    layout->addLayout(barra);

    connect(btnAbrir, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarAbrir);
    connect(btnGuardar, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarGuardar);
}

QWidget* VistaPrincipal::crearPestanaPersonas()
{
    auto* pagina = new QWidget(this);
    cmbTipo = new QComboBox(pagina);
    edtCodigo = new QLineEdit(pagina);
    edtCodigo->setValidator(new QRegularExpressionValidator(
        QRegularExpression("[A-Z0-9-]{1,10}"), pagina));
    edtNombre = new QLineEdit(pagina);
    lblDato = new QLabel("Carrera:", pagina);
    edtDato = new QLineEdit(pagina);
    cmbCarrera = new QComboBox(pagina);
    btnNuevaCarrera =
        boton(pagina, "Nueva carrera…", QStyle::SP_FileDialogNewFolder);
    auto* filaDato = new QHBoxLayout();
    filaDato->addWidget(edtDato);
    filaDato->addWidget(cmbCarrera, 1);
    filaDato->addWidget(btnNuevaCarrera);
    btnRegistrar = boton(pagina, "Registrar", QStyle::SP_FileDialogNewFolder);
    btnEditarPersona =
        boton(pagina, "Guardar cambios", QStyle::SP_DialogApplyButton);
    btnEliminarPersona = boton(pagina, "Eliminar", QStyle::SP_TrashIcon);
    auto* btnLimpiar = boton(pagina, "Limpiar", QStyle::SP_DialogResetButton);

    auto* botones = new QHBoxLayout();
    botones->addWidget(btnRegistrar);
    botones->addWidget(btnEditarPersona);
    botones->addWidget(btnEliminarPersona);
    botones->addWidget(btnLimpiar);
    auto* form = new QFormLayout();
    form->addRow("Tipo:", cmbTipo);
    form->addRow("Código:", edtCodigo);
    form->addRow("Nombre:", edtNombre);
    form->addRow(lblDato, filaDato);
    form->addRow(botones);
    auto* grupo = new QGroupBox("Datos de la persona", pagina);
    grupo->setLayout(form);
    grupo->setMaximumWidth(450);

    tablaPersonas = tabla(pagina, { "Tipo", "Código", "Nombre",
                                    "Carrera / Departamento / Área" });
    edtBuscarPersona =
        buscador(pagina, "Buscar por código, nombre, tipo o carrera");
    auto* derecha = new QVBoxLayout();
    derecha->addWidget(edtBuscarPersona);
    derecha->addWidget(new QLabel("Elija una persona para editarla; doble "
                                  "clic para ver el detalle.",
                                  pagina));
    derecha->addWidget(tablaPersonas);
    auto* layout = new QHBoxLayout(pagina);
    layout->addWidget(grupo);
    layout->addLayout(derecha, 1);
    modoEdicionPersona(false);

    connect(btnRegistrar, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarRegistroPersona);
    connect(btnEditarPersona, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarEdicionPersona);
    connect(btnEliminarPersona, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarEliminarPersona);
    connect(btnLimpiar, &QPushButton::clicked, this,
            [this]
            {
                if (nombrePersona().isEmpty() ||
                    confirmar("Limpiar", "¿Borrar los datos del formulario?"))
                    limpiarPersona();
            });
    connect(btnNuevaCarrera, &QPushButton::clicked, this,
            &VistaPrincipal::pedirNuevaCarrera);
    connect(cmbTipo, &QComboBox::currentTextChanged, this,
            &VistaPrincipal::actualizarCamposTipo);
    connect(tablaPersonas, &QTableWidget::itemSelectionChanged, this,
            &VistaPrincipal::cargarPersonaElegida);
    connect(edtBuscarPersona, &QLineEdit::textChanged, this,
            &VistaPrincipal::filtrarPersonas);
    connect(tablaPersonas, &QTableWidget::cellDoubleClicked, this,
            [this](int fila, int)
            {
                emit solicitarDetallePersona(
                    tablaPersonas->item(fila, 1)->text());
            });
    return pagina;
}

void VistaPrincipal::ponerTiposPersona(const QStringList& tipos)
{
    cmbTipo->addItems(tipos);
    cmbTipo->setCurrentText("Estudiante");
}

// La lista de carreras aparece en el formulario de personas y en el de
// cursos; en este ultimo la primera opcion es la de los cursos generales.
void VistaPrincipal::ponerCarreras(const QStringList& carreras)
{
    QString elegida = cmbCarrera->currentText();
    cmbCarrera->clear();
    cmbCarrera->addItems(carreras);
    cmbCarrera->setCurrentText(elegida);

    elegida = carreraCurso();
    cmbCarreraCurso->clear();
    cmbCarreraCurso->addItem("General (todas las carreras)", "");
    for (const QString& c : carreras)
        cmbCarreraCurso->addItem(c, c);
    elegirDato(cmbCarreraCurso, elegida);
}

// El estudiante elige su carrera de una lista; docentes y administrativos
// escriben su departamento o area.
void VistaPrincipal::actualizarCamposTipo(const QString& tipo)
{
    bool estudiante = tipo == "Estudiante";
    if (estudiante)
        lblDato->setText("Carrera:");
    else if (tipo == "Docente")
        lblDato->setText("Departamento:");
    else
        lblDato->setText("Área:");
    edtDato->setVisible(!estudiante);
    cmbCarrera->setVisible(estudiante);
    btnNuevaCarrera->setVisible(estudiante);
    if (estudiante)
        lblDato->setBuddy(cmbCarrera);
    else
        lblDato->setBuddy(edtDato);
}

void VistaPrincipal::pedirNuevaCarrera()
{
    QString nombre =
        QInputDialog::getText(this, "Nueva carrera", "Nombre de la carrera:")
            .trimmed();
    if (!nombre.isEmpty())
        emit solicitarNuevaCarrera(nombre);
}

QString VistaPrincipal::datoPersona() const
{
    if (tipoPersona() == "Estudiante")
        return cmbCarrera->currentText();
    return edtDato->text().trimmed();
}

void VistaPrincipal::elegirCarrera(const QString& carrera)
{
    cmbCarrera->setCurrentText(carrera);
}

// Al actualizar la tabla se conserva lo que se esta escribiendo. Si se
// estaba editando una persona, sigue elegida; si ya no existe, el
// formulario se limpia.
void VistaPrincipal::mostrarPersonas(const QList<QStringList>& filas)
{
    QString editada = edtCodigo->isReadOnly() ? codigoPersona() : "";
    tablaPersonas->blockSignals(true);
    llenarTabla(tablaPersonas, filas);
    filtrarPersonas();
    bool sigue = false;
    for (int f = 0; f < tablaPersonas->rowCount() && !editada.isEmpty(); f++)
        if (tablaPersonas->item(f, 1)->text() == editada)
        {
            tablaPersonas->selectRow(f);
            sigue = true;
        }
    tablaPersonas->blockSignals(false);
    if (!editada.isEmpty() && !sigue)
        limpiarPersona();
}

void VistaPrincipal::filtrarPersonas()
{
    QString buscado = sinTildes(edtBuscarPersona->text().trimmed());
    for (int f = 0; f < tablaPersonas->rowCount(); f++)
        tablaPersonas->setRowHidden(f,
                                    !filaContiene(tablaPersonas, f, buscado));
}

void VistaPrincipal::cargarPersonaElegida()
{
    int fila = tablaPersonas->currentRow();
    if (tablaPersonas->selectedItems().isEmpty() || fila < 0)
        return;
    cmbTipo->setCurrentText(tablaPersonas->item(fila, 0)->text());
    edtCodigo->setText(tablaPersonas->item(fila, 1)->text());
    edtNombre->setText(tablaPersonas->item(fila, 2)->text());
    edtDato->setText(tablaPersonas->item(fila, 3)->text());
    cmbCarrera->setCurrentText(tablaPersonas->item(fila, 3)->text());
    modoEdicionPersona(true);
}

// Al editar, el tipo y el codigo quedan fijos: identifican a la persona.
void VistaPrincipal::modoEdicionPersona(bool editando)
{
    cmbTipo->setEnabled(!editando);
    edtCodigo->setReadOnly(editando);
    btnRegistrar->setEnabled(!editando);
    btnEditarPersona->setEnabled(editando);
    btnEliminarPersona->setEnabled(editando);
}

void VistaPrincipal::limpiarPersona()
{
    tablaPersonas->clearSelection();
    edtCodigo->clear();
    edtNombre->clear();
    edtDato->clear();
    modoEdicionPersona(false);
}

QWidget* VistaPrincipal::crearPestanaAulas()
{
    auto* pagina = new QWidget(this);
    edtCodigoAula = new QLineEdit(pagina);
    edtCodigoAula->setValidator(new QRegularExpressionValidator(
        QRegularExpression("[A-Z0-9-]{1,10}"), pagina));
    spnCapacidadAula = new QSpinBox(pagina);
    spnCapacidadAula->setRange(1, 200);
    spnCapacidadAula->setValue(30);
    btnRegistrarAula =
        boton(pagina, "Registrar", QStyle::SP_FileDialogNewFolder);
    btnEditarAula =
        boton(pagina, "Guardar cambios", QStyle::SP_DialogApplyButton);
    btnEliminarAula = boton(pagina, "Eliminar", QStyle::SP_TrashIcon);
    auto* btnLimpiar = boton(pagina, "Limpiar", QStyle::SP_DialogResetButton);
    auto* botones = new QHBoxLayout();
    botones->addWidget(btnRegistrarAula);
    botones->addWidget(btnEditarAula);
    botones->addWidget(btnEliminarAula);
    botones->addWidget(btnLimpiar);
    auto* form = new QFormLayout();
    form->addRow("Código:", edtCodigoAula);
    form->addRow("Capacidad:", spnCapacidadAula);
    form->addRow(botones);
    auto* grupo = new QGroupBox("Datos del aula", pagina);
    grupo->setLayout(form);
    grupo->setMaximumWidth(450);

    tablaAulas = tabla(pagina, { "Código", "Capacidad", "Cursos asignados" });
    auto* derecha = new QVBoxLayout();
    derecha->addWidget(new QLabel("Elija un aula para editarla; doble clic "
                                  "para ver los cursos que la usan.",
                                  pagina));
    derecha->addWidget(tablaAulas);
    auto* layout = new QHBoxLayout(pagina);
    layout->addWidget(grupo, 0, Qt::AlignTop);
    layout->addLayout(derecha, 1);
    modoEdicionAula(false);

    connect(btnRegistrarAula, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarRegistroAula);
    connect(btnEditarAula, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarEdicionAula);
    connect(btnEliminarAula, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarEliminarAula);
    connect(btnLimpiar, &QPushButton::clicked, this,
            &VistaPrincipal::limpiarAula);
    connect(tablaAulas, &QTableWidget::itemSelectionChanged, this,
            &VistaPrincipal::cargarAulaElegida);
    connect(tablaAulas, &QTableWidget::cellDoubleClicked, this,
            [this](int fila, int)
            { emit solicitarDetalleAula(tablaAulas->item(fila, 0)->text()); });
    return pagina;
}

// La tabla de aulas y la lista del formulario de cursos salen de las mismas
// filas. Cada opcion de la lista guarda la capacidad para limitar las
// vacantes. El aula que se estaba editando sigue elegida.
void VistaPrincipal::mostrarAulas(const QList<QStringList>& filas)
{
    QString editada = edtCodigoAula->isReadOnly() ? codigoAula() : "";
    tablaAulas->blockSignals(true);
    llenarTabla(tablaAulas, filas);
    bool sigue = false;
    for (int f = 0; f < tablaAulas->rowCount() && !editada.isEmpty(); f++)
        if (tablaAulas->item(f, 0)->text() == editada)
        {
            tablaAulas->selectRow(f);
            sigue = true;
        }
    tablaAulas->blockSignals(false);
    if (!editada.isEmpty() && !sigue)
        limpiarAula();

    QString elegida = aula();
    cmbAula->blockSignals(true);
    cmbAula->clear();
    for (const QStringList& f : filas)
    {
        cmbAula->addItem(f[0] + ", " + f[1] + " asientos", f[0]);
        cmbAula->setItemData(cmbAula->count() - 1, f[1].toInt(),
                             Qt::UserRole + 1);
    }
    elegirDato(cmbAula, elegida);
    cmbAula->blockSignals(false);
    aulaDeCursoCambiada();
}

void VistaPrincipal::cargarAulaElegida()
{
    int fila = tablaAulas->currentRow();
    if (tablaAulas->selectedItems().isEmpty() || fila < 0)
        return;
    edtCodigoAula->setText(tablaAulas->item(fila, 0)->text());
    spnCapacidadAula->setValue(tablaAulas->item(fila, 1)->text().toInt());
    modoEdicionAula(true);
}

// Al editar, el codigo queda fijo: identifica al aula.
void VistaPrincipal::modoEdicionAula(bool editando)
{
    edtCodigoAula->setReadOnly(editando);
    btnRegistrarAula->setEnabled(!editando);
    btnEditarAula->setEnabled(editando);
    btnEliminarAula->setEnabled(editando);
}

void VistaPrincipal::limpiarAula()
{
    tablaAulas->clearSelection();
    edtCodigoAula->clear();
    spnCapacidadAula->setValue(30);
    modoEdicionAula(false);
}

QWidget* VistaPrincipal::crearPestanaCursos()
{
    auto* pagina = new QWidget(this);
    edtCodigoCurso = new QLineEdit(pagina);
    edtCodigoCurso->setValidator(new QRegularExpressionValidator(
        QRegularExpression("[A-Z0-9-]{1,10}"), pagina));
    edtNombreCurso = new QLineEdit(pagina);
    cmbCarreraCurso = new QComboBox(pagina);
    spnCreditos = new QSpinBox(pagina);
    spnCreditos->setRange(1, 10);
    spnCreditos->setValue(4);
    spnSemestre = new QSpinBox(pagina);
    spnSemestre->setRange(1, 10);
    cmbAula = new QComboBox(pagina);
    spnVacantes = new QSpinBox(pagina);
    spnVacantes->setRange(1, 200);
    spnVacantes->setValue(30);
    cmbDocente = new QComboBox(pagina);

    cmbDia = new QComboBox(pagina);
    for (int d = 0; d < DIAS.size(); d++)
        cmbDia->addItem(DIAS[d], d);
    cmbInicio = new QComboBox(pagina);
    for (int h = 7; h <= 21; h++)
        cmbInicio->addItem(hora(h), h);
    cmbFin = new QComboBox(pagina);
    for (int h = 8; h <= 22; h++)
        cmbFin->addItem(hora(h), h);
    cmbFin->setCurrentIndex(1);
    auto* btnAgregarBloque = boton(pagina, "Agregar", QStyle::SP_ArrowDown);
    auto* btnQuitarBloque =
        boton(pagina, "Quitar bloque", QStyle::SP_DialogDiscardButton);
    tablaBloques = tabla(pagina, { "Día", "Inicio", "Fin" });
    tablaBloques->setMaximumHeight(130);
    auto* filaBloque = new QHBoxLayout();
    filaBloque->addWidget(cmbDia);
    filaBloque->addWidget(new QLabel("de", pagina));
    filaBloque->addWidget(cmbInicio);
    filaBloque->addWidget(new QLabel("a", pagina));
    filaBloque->addWidget(cmbFin);
    filaBloque->addWidget(btnAgregarBloque);

    btnCrearCurso =
        boton(pagina, "Crear curso", QStyle::SP_FileDialogNewFolder);
    btnEditarCurso =
        boton(pagina, "Guardar cambios", QStyle::SP_DialogApplyButton);
    btnEliminarCurso = boton(pagina, "Eliminar", QStyle::SP_TrashIcon);
    auto* btnLimpiar = boton(pagina, "Limpiar", QStyle::SP_DialogResetButton);
    auto* botones = new QHBoxLayout();
    botones->addWidget(btnCrearCurso);
    botones->addWidget(btnEditarCurso);
    botones->addWidget(btnEliminarCurso);
    botones->addWidget(btnLimpiar);
    auto* form = new QFormLayout();
    form->addRow("Código:", edtCodigoCurso);
    form->addRow("Nombre:", edtNombreCurso);
    form->addRow("Carrera:", cmbCarreraCurso);
    form->addRow("Créditos:", spnCreditos);
    form->addRow("Semestre:", spnSemestre);
    form->addRow("Aula:", cmbAula);
    form->addRow("Vacantes:", spnVacantes);
    form->addRow("Horario:", filaBloque);
    form->addRow("", tablaBloques);
    form->addRow("", btnQuitarBloque);
    form->addRow("Docente:", cmbDocente);
    form->addRow(botones);
    auto* grupo = new QGroupBox("Datos del curso", pagina);
    grupo->setLayout(form);
    grupo->setMaximumWidth(450);

    cmbFiltroSemestre = filtroSemestre(pagina);
    tablaCursos = tabla(pagina, { "Código", "Nombre", "Carrera", "Sem.",
                                  "Créd.", "Aula", "Vacantes", "Horario",
                                  "Docente", "Inscritos", "En espera" });
    edtBuscarCurso =
        buscador(pagina, "Buscar por código, nombre, aula o docente");
    auto* filtros = new QHBoxLayout();
    filtros->addWidget(edtBuscarCurso, 1);
    filtros->addWidget(cmbFiltroSemestre);
    auto* derecha = new QVBoxLayout();
    derecha->addLayout(filtros);
    derecha->addWidget(
        new QLabel("Elija un curso para editarlo; doble clic para ver sus "
                   "estudiantes y su lista de espera.",
                   pagina));
    derecha->addWidget(tablaCursos);
    auto* layout = new QHBoxLayout(pagina);
    layout->addWidget(grupo);
    layout->addLayout(derecha, 1);

    modoEdicionCurso(false);

    connect(btnCrearCurso, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarCrearCurso);
    connect(btnEditarCurso, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarEdicionCurso);
    connect(btnEliminarCurso, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarEliminarCurso);
    connect(cmbAula, &QComboBox::currentIndexChanged, this,
            &VistaPrincipal::aulaDeCursoCambiada);
    connect(tablaCursos, &QTableWidget::itemSelectionChanged, this,
            [this]
            {
                int fila = tablaCursos->currentRow();
                if (!tablaCursos->selectedItems().isEmpty() && fila >= 0)
                    emit solicitarCargaCurso(
                        tablaCursos->item(fila, 0)->text());
            });
    connect(btnLimpiar, &QPushButton::clicked, this,
            &VistaPrincipal::limpiarCurso);
    connect(btnAgregarBloque, &QPushButton::clicked, this,
            &VistaPrincipal::agregarBloque);
    connect(btnQuitarBloque, &QPushButton::clicked, this,
            [this] { tablaBloques->removeRow(tablaBloques->currentRow()); });
    connect(cmbFiltroSemestre, &QComboBox::currentIndexChanged, this,
            &VistaPrincipal::filtrarCursos);
    connect(edtBuscarCurso, &QLineEdit::textChanged, this,
            &VistaPrincipal::filtrarCursos);
    connect(
        tablaCursos, &QTableWidget::cellDoubleClicked, this,
        [this](int fila, int)
        { emit solicitarDetalleCurso(tablaCursos->item(fila, 0)->text()); });
    return pagina;
}

void VistaPrincipal::ponerDocentes(
    const QList<QPair<QString, QString>>& docentes)
{
    QString elegido = docente();
    cmbDocente->clear();
    cmbDocente->addItem("Sin docente", "");
    for (const auto& d : docentes)
        cmbDocente->addItem(d.second, d.first);
    elegirDato(cmbDocente, elegido);
}

// Si se estaba editando un curso, sigue elegido tras actualizar la tabla; si
// ya no existe, el formulario se limpia.
void VistaPrincipal::mostrarCursos(const QList<QStringList>& filas)
{
    QString editado = editandoCurso() ? codigoCurso() : "";
    tablaCursos->blockSignals(true);
    llenarTabla(tablaCursos, filas);
    filtrarCursos();
    bool sigue = false;
    for (int f = 0; f < tablaCursos->rowCount() && !editado.isEmpty(); f++)
        if (tablaCursos->item(f, 0)->text() == editado)
        {
            tablaCursos->selectRow(f);
            sigue = true;
        }
    tablaCursos->blockSignals(false);
    if (!editado.isEmpty() && !sigue)
        limpiarCurso();
}

// Las vacantes no pueden superar la capacidad del aula. En un curso nuevo se
// proponen todos los asientos.
void VistaPrincipal::aulaDeCursoCambiada()
{
    int capacidad = cmbAula->currentData(Qt::UserRole + 1).toInt();
    if (capacidad < 1)
        return;
    spnVacantes->setMaximum(capacidad);
    if (!editandoCurso())
        spnVacantes->setValue(capacidad);
}

void VistaPrincipal::cargarCurso(const CursoVista& curso)
{
    modoEdicionCurso(true);
    edtCodigoCurso->setText(curso.codigo);
    edtNombreCurso->setText(curso.nombre);
    elegirDato(cmbCarreraCurso, curso.carrera);
    spnCreditos->setValue(curso.creditos);
    spnSemestre->setValue(curso.semestre);
    elegirDato(cmbAula, curso.aula);
    spnVacantes->setValue(curso.vacantes);
    elegirDato(cmbDocente, curso.docente);
    tablaBloques->setRowCount(0);
    for (const BloqueVista& b : curso.bloques)
    {
        cmbDia->setCurrentIndex(cmbDia->findData(b.dia));
        cmbInicio->setCurrentIndex(cmbInicio->findData(b.inicio));
        cmbFin->setCurrentIndex(cmbFin->findData(b.fin));
        agregarBloque();
    }
}

// Al editar, el codigo queda fijo: identifica al curso.
void VistaPrincipal::modoEdicionCurso(bool editando)
{
    edtCodigoCurso->setReadOnly(editando);
    btnCrearCurso->setEnabled(!editando);
    btnEditarCurso->setEnabled(editando);
    btnEliminarCurso->setEnabled(editando);
}

// Se muestran los cursos del semestre elegido que contienen el texto buscado.
void VistaPrincipal::filtrarCursos()
{
    int semestre = cmbFiltroSemestre->currentData().toInt();
    QString buscado = sinTildes(edtBuscarCurso->text().trimmed());
    for (int f = 0; f < tablaCursos->rowCount(); f++)
    {
        int delCurso = tablaCursos->item(f, 3)->text().toInt();
        bool visible = (semestre == 0 || delCurso == semestre) &&
                       filaContiene(tablaCursos, f, buscado);
        tablaCursos->setRowHidden(f, !visible);
    }
}

// La hora de fin debe ser posterior a la de inicio; que el bloque no se
// superponga con otro lo revisa el modelo al crear el curso.
void VistaPrincipal::agregarBloque()
{
    int dia = cmbDia->currentData().toInt();
    int inicio = cmbInicio->currentData().toInt();
    int fin = cmbFin->currentData().toInt();
    if (fin <= inicio)
    {
        mostrarError("Bloque inválido",
                     "La hora de fin debe ser posterior a la de inicio.");
        return;
    }
    // Cada celda guarda el numero que representa para leerlo despues.
    int f = tablaBloques->rowCount();
    tablaBloques->insertRow(f);
    const QStringList textos = { DIAS[dia], hora(inicio), hora(fin) };
    const int numeros[] = { dia, inicio, fin };
    for (int c = 0; c < 3; c++)
    {
        auto* item = new QTableWidgetItem(textos[c]);
        item->setData(Qt::UserRole, numeros[c]);
        tablaBloques->setItem(f, c, item);
    }
}

QList<BloqueVista> VistaPrincipal::bloquesHorario() const
{
    QList<BloqueVista> bloques;
    for (int f = 0; f < tablaBloques->rowCount(); f++)
        bloques.append({ tablaBloques->item(f, 0)->data(Qt::UserRole).toInt(),
                         tablaBloques->item(f, 1)->data(Qt::UserRole).toInt(),
                         tablaBloques->item(f, 2)->data(Qt::UserRole).toInt(),
                         "" });
    return bloques;
}

void VistaPrincipal::limpiarCurso()
{
    tablaCursos->clearSelection();
    modoEdicionCurso(false);
    edtCodigoCurso->clear();
    edtNombreCurso->clear();
    tablaBloques->setRowCount(0);
    cmbCarreraCurso->setCurrentIndex(0);
    cmbAula->setCurrentIndex(0);
    cmbDocente->setCurrentIndex(0);
    aulaDeCursoCambiada();
}

QWidget* VistaPrincipal::crearPestanaMatricula()
{
    auto* pagina = new QWidget(this);
    cmbEstudiante = new QComboBox(pagina);
    cmbSemestreMatricula = filtroSemestre(pagina);
    cmbCurso = new QComboBox(pagina);
    auto* btnMatricular =
        boton(pagina, "Matricular", QStyle::SP_DialogApplyButton);
    auto* btnRetirar = boton(pagina, "Retirar matrícula", QStyle::SP_TrashIcon);

    auto* form = new QFormLayout();
    form->addRow("Estudiante:", cmbEstudiante);
    form->addRow("Semestre:", cmbSemestreMatricula);
    form->addRow("Curso:", cmbCurso);
    form->addRow("", btnMatricular);
    auto* grupo = new QGroupBox("Matricular", pagina);
    grupo->setLayout(form);
    grupo->setMaximumWidth(450);

    lblResumen = new QLabel(pagina);
    lblResumen->setWordWrap(true);
    tablaMatriculas =
        tabla(pagina, { "Fecha", "Código", "Curso", "Créd.", "Horario" });
    grillaEstudiante = grillaSemanal(pagina);
    auto* derecha = new QVBoxLayout();
    derecha->addWidget(lblResumen);
    derecha->addWidget(tablaMatriculas, 1);
    derecha->addWidget(btnRetirar, 0, Qt::AlignRight);
    derecha->addWidget(new QLabel("Horario semanal del estudiante:", pagina));
    derecha->addWidget(grillaEstudiante, 2);
    auto* layout = new QHBoxLayout(pagina);
    layout->addWidget(grupo);
    layout->addLayout(derecha, 1);

    connect(btnMatricular, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarMatricula);
    connect(btnRetirar, &QPushButton::clicked, this,
            &VistaPrincipal::solicitarRetiro);
    connect(cmbEstudiante, &QComboBox::currentIndexChanged, this,
            &VistaPrincipal::estudianteCambiado);
    connect(cmbSemestreMatricula, &QComboBox::currentIndexChanged, this,
            &VistaPrincipal::filtrarCursosMatricula);
    return pagina;
}

// Vuelve a llenar la lista conservando al estudiante que estaba elegido.
void VistaPrincipal::ponerEstudiantes(
    const QList<QPair<QString, QString>>& estudiantes)
{
    QString elegido = estudianteElegido();
    cmbEstudiante->blockSignals(true);
    cmbEstudiante->clear();
    for (const auto& e : estudiantes)
        cmbEstudiante->addItem(e.second + " (" + e.first + ")", e.first);
    elegirDato(cmbEstudiante, elegido);
    cmbEstudiante->blockSignals(false);
    emit estudianteCambiado();
}

void VistaPrincipal::ponerCursosMatricula(const QList<OpcionCurso>& cursos)
{
    opcionesCurso = cursos;
    filtrarCursosMatricula();
}

void VistaPrincipal::filtrarCursosMatricula()
{
    QString elegido = cursoElegido();
    int semestre = cmbSemestreMatricula->currentData().toInt();
    cmbCurso->clear();
    for (const OpcionCurso& c : opcionesCurso)
        if (semestre == 0 || c.semestre == semestre)
            cmbCurso->addItem(c.texto, c.codigo);
    elegirDato(cmbCurso, elegido);
}

void VistaPrincipal::mostrarMatriculas(const QString& resumen,
                                       const QList<QStringList>& filas,
                                       const QList<BloqueVista>& horario)
{
    lblResumen->setText(resumen);
    llenarTabla(tablaMatriculas, filas);
    llenarGrilla(grillaEstudiante, horario);
}

QString VistaPrincipal::cursoDeMatriculaElegida() const
{
    int fila = tablaMatriculas->currentRow();
    if (tablaMatriculas->selectedItems().isEmpty() || fila < 0)
        return "";
    return tablaMatriculas->item(fila, 1)->text();
}

void VistaPrincipal::llenarTabla(QTableWidget* tabla,
                                 const QList<QStringList>& filas)
{
    tabla->setRowCount(0);
    for (const QStringList& fila : filas)
    {
        int f = tabla->rowCount();
        tabla->insertRow(f);
        for (int c = 0; c < tabla->columnCount() && c < fila.size(); c++)
            tabla->setItem(f, c, new QTableWidgetItem(fila[c]));
    }
}

// Cada bloque ocupa una celda que abarca todas sus horas. El color de fondo
// se aclara u oscurece segun el tema para que se distinga sin ser llamativo.
void VistaPrincipal::llenarGrilla(QTableWidget* grilla,
                                  const QList<BloqueVista>& bloques)
{
    QColor fondo = palette().color(QPalette::Base);
    fondo = fondo.lightness() < 128 ? fondo.lighter(230) : fondo.darker(120);
    grilla->clearSpans();
    grilla->clearContents();
    for (const BloqueVista& b : bloques)
    {
        auto* item = new QTableWidgetItem(b.texto);
        item->setTextAlignment(Qt::AlignCenter);
        item->setBackground(fondo);
        grilla->setItem(b.inicio - 7, b.dia, item);
        if (b.fin - b.inicio > 1)
            grilla->setSpan(b.inicio - 7, b.dia, b.fin - b.inicio, 1);
    }
}

void VistaPrincipal::mostrarResultado(const QString& texto)
{
    lblResultado->setText(texto);
}

void VistaPrincipal::mostrarError(const QString& titulo, const QString& texto)
{
    QMessageBox::warning(this, titulo, texto);
}

bool VistaPrincipal::confirmar(const QString& titulo, const QString& texto)
{
    QMessageBox caja(QMessageBox::Question, titulo, texto,
                     QMessageBox::Yes | QMessageBox::No, this);
    caja.button(QMessageBox::Yes)->setText("Sí");
    return caja.exec() == QMessageBox::Yes;
}

void VistaPrincipal::mostrarDetalle(const QString& descripcion,
                                    const QString& tituloLista,
                                    const QStringList& lista)
{
    DialogoDetalle dialogo(descripcion, tituloLista, lista, this);
    dialogo.exec();
}

// Propone la carpeta datos del proyecto: la busca subiendo desde la carpeta
// del ejecutable, que puede ser salida/ o la de compilacion de Qt Creator.
QString VistaPrincipal::elegirArchivo(bool paraGuardar)
{
    QDir carpeta(QCoreApplication::applicationDirPath());
    for (int nivel = 0; nivel < 3 && !carpeta.exists("datos"); nivel++)
        carpeta.cdUp();
    QString inicio = carpeta.exists("datos") ? carpeta.filePath("datos") : "";
    const QString filtro = "Datos CSV (*.csv)";
    if (paraGuardar)
        return QFileDialog::getSaveFileName(this, "Guardar datos", inicio,
                                            filtro);
    return QFileDialog::getOpenFileName(this, "Abrir datos", inicio, filtro);
}
