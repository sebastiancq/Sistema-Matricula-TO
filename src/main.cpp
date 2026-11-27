#include "controlador/ControladorMatricula.h"
#include "modelo/PersonaFactory.h"
#include <QApplication>
#include <QDir>
#include <QStandardPaths>

using namespace std;

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SistemaMatricula");
    registrarTiposDePersona();

    // Reglas de matricula, en el orden en que se revisan. Cada estudiante
    // puede llevar hasta 25 creditos por semestre.
    SistemaMatricula modelo;
    modelo.agregarRegla(make_unique<ReglaCarrera>());
    modelo.agregarRegla(make_unique<ReglaCruceHorario>());
    modelo.agregarRegla(make_unique<ReglaLimiteCreditos>(25));
    modelo.agregarRegla(make_unique<ReglaPrerrequisito>());
    VistaPrincipal vista;
    ControladorMatricula controlador(&vista, &modelo);

    // Se retoma la sesion anterior y, desde ahi, cada cambio se guarda en
    // ella. La primera vez se usan los datos de inicio de semestre incluidos
    // en el programa. El guardado empieza despues de cargar: abrir el
    // programa no reescribe la sesion, solo los cambios del usuario.
    QString carpeta =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(carpeta);
    QString sesion = carpeta + "/sesion.csv";
    if (!QFile::exists(sesion) || !controlador.cargarArchivo(sesion))
        controlador.cargarArchivo(":/datos/01_inicio_semestre.csv");
    controlador.guardarSesionEn(sesion);

    vista.showMaximized();
    return app.exec();
}
