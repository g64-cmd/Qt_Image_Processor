#include "processcommand.h"
#include "mainwindow.h"
#include "imageprocessor.h"

ProcessCommand::ProcessCommand(MainWindow *window, Operation op, QUndoCommand *parent)
    : QUndoCommand(parent), mainWindow(window), operation(op)
{
    imageId = mainWindow->getCurrentImageId();
    beforePixmap = mainWindow->getCurrentImagePixmap();
    switch (operation) {
    case Sharpen:
        setText("图像锐化");
        break;
    case Grayscale:
        setText("灰度化");
        break;
    case Canny:
        setText("Canny 边缘检测");
        break;
    }
}

void ProcessCommand::undo()
{
    mainWindow->updateImageFromCommand(imageId, beforePixmap);
}

void ProcessCommand::redo()
{
    if (afterPixmap.isNull()) {
        QImage resultImage;
        switch (operation) {
        case Sharpen:
            resultImage = ImageProcessor::sharpen(beforePixmap.toImage());
            break;
        case Grayscale:
            resultImage = ImageProcessor::grayscale(beforePixmap.toImage());
            break;
        case Canny:
            resultImage = ImageProcessor::canny(beforePixmap.toImage());
            break;
        }
        if (!resultImage.isNull()) {
            afterPixmap = QPixmap::fromImage(resultImage);
        } else {
            setObsolete(true);
            return;
        }
    }

    mainWindow->updateImageFromCommand(imageId, afterPixmap);
}
