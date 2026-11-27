#pragma once

#include "modelo/Excepciones.h"
#include "modelo/SistemaMatricula.h"
#include "vista/VistaPrincipal.h"
#include <QObject>

// Controlador: reacciona a las senales de la vista, pide al modelo cada
// operacion, convierte sus datos en texto para la vista y decide que mensaje
// mostrar. Tambien es el observador del modelo.
class ControladorMatricula : public QObject
{
    Q_OBJECT

public:
    ControladorMatricula(VistaPrincipal* vista, SistemaMatricula* modelo,
                         QObject* parent = nullptr);

    bool cargarArchivo(const QString& ruta);
    bool guardarArchivo(const QString& ruta);
    // Desde aqui, cada cambio del modelo se guarda en la ruta sin avisar.
    // Solo muestra un mensaje si no se pudo escribir.
    void guardarSesionEn(const QString& ruta);

private slots:
    void onRegistrarPersona();
    void onEditarPersona();
    void onEliminarPersona();
    void onDetallePersona(const QString& codigo);
    void onNuevaCarrera(const QString& nombre);
    void onRegistrarAula();
    void onEditarAula();
    void onEliminarAula();
    void onDetalleAula(const QString& codigo);
    void onCrearCurso();
    void onCargarCurso(const QString& codigo);
    void onEditarCurso();
    void onEliminarCurso();
    void onDetalleCurso(const QString& codigo);
    void onMatricular();
    void onRetirar();
    void onAbrir();
    void onGuardar();
    void actualizarMatriculas();

private:
    void actualizarPersonas();
    void actualizarAulas();
    void actualizarCursos();
    // Arma los datos del curso con lo que tiene el formulario. Lanza
    // DatosInvalidos si un bloque del horario se superpone con otro.
    DatosCurso datosCursoDeVista() const;
    QString nombres(const std::vector<const Estudiante*>& estudiantes) const;
    QString listaDeCursos(const std::vector<std::string>& codigos) const;
    // Escribe todos los datos en el archivo. QSaveFile escribe primero en
    // un archivo temporal, asi un cierre a la fuerza no deja el archivo a
    // medias.
    bool escribir(const QString& ruta) const;

    // Ejecuta la accion; si el modelo rechaza la operacion, muestra el motivo
    // con el titulo dado y devuelve false.
    template <typename Accion> bool intentar(const QString& titulo,
                                             Accion accion)
    {
        try
        {
            accion();
            return true;
        }
        catch (const ErrorMatricula& e)
        {
            vista->mostrarError(titulo, QString::fromStdString(e.what()));
            return false;
        }
    }

    VistaPrincipal* vista;
    SistemaMatricula* modelo;
};
