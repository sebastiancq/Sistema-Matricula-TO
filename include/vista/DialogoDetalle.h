#pragma once

#include <QDialog>

// Ventana modal con una descripcion y una lista debajo: los cursos de una
// persona o de un aula, o los estudiantes de un curso.
class DialogoDetalle : public QDialog
{
    Q_OBJECT

public:
    DialogoDetalle(const QString& descripcion, const QString& tituloLista,
                   const QStringList& elementos, QWidget* parent = nullptr);
};
