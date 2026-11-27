#include "vista/DialogoDetalle.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QVBoxLayout>

DialogoDetalle::DialogoDetalle(const QString& descripcion,
                               const QString& tituloLista,
                               const QStringList& elementos, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Detalle");
    resize(460, 340);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(descripcion, this));
    if (!tituloLista.isEmpty())
    {
        layout->addWidget(new QLabel(tituloLista, this));
        auto* lista = new QListWidget(this);
        lista->addItems(elementos);
        layout->addWidget(lista);
    }
    auto* botones = new QDialogButtonBox(QDialogButtonBox::Close, this);
    botones->button(QDialogButtonBox::Close)->setText("Cerrar");
    layout->addWidget(botones);
    connect(botones, &QDialogButtonBox::rejected, this, &QDialog::reject);
}
