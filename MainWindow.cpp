#include "MainWindow.h"
#include <QFileDialog>
#include <QtWidgets/qmessagebox.h>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    _ui.setupUi(this);

    connect(_ui.openImageButton, &QPushButton::pressed, [&] {
        QString imageFile = QFileDialog::getOpenFileName(this, tr("Open image"), QString(), tr("Images (*.png *.jpg *.xpm);;All files (*.*)"));
        if (!imageFile.isEmpty())
        {
            QPixmap pixmap;
            if (pixmap.load(imageFile))
                _ui.imageLabel->setPixmap(pixmap);
            else
                QMessageBox::warning(this, tr("Cannot open image"), imageFile);
        }
    });
}
