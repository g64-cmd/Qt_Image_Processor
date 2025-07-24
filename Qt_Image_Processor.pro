QT += core gui widgets
QT += multimedia
QT += multimediawidgets
CONFIG += c++17

VCPKG_ROOT_PATH = "C:/vcpkg/vcpkg/installed/x64-windows"

QMAKE_CXXFLAGS += -I$$VCPKG_ROOT_PATH/include/opencv4
INCLUDEPATH += $$VCPKG_ROOT_PATH/include

LIBS += -L$$VCPKG_ROOT_PATH/lib

CONFIG(debug, debug|release) {
    LIBS += -lopencv_core4d
    LIBS += -lopencv_imgproc4d
    LIBS += -lopencv_highgui4d
    LIBS += -lopencv_features2d4d
    LIBS += -lopencv_calib3d4d
    LIBS += -lopencv_stitching4d
    LIBS += -lopencv_saliency4d
    LIBS += -lopencv_objdetect4d
    LIBS += -lopencv_saliency4d
    LIBS += -lopencv_imgcodecs4d
    LIBS += -lopencv_video4d
    LIBS += -lopencv_videoio4d
    LIBS += -ldlibd
    LIBS += -lopenblas
    LIBS += -llapack
    LIBS += -lavformatd
    LIBS += -lavcodecd
    LIBS += -lavutild
    LIBS += -lswresampled
    LIBS += -lswscaled
} else {
    LIBS += -lopencv_core4
    LIBS += -lopencv_imgproc4
    LIBS += -lopencv_highgui4
    LIBS += -lopencv_features2d4
    LIBS += -lopencv_calib3d4
    LIBS += -lopencv_stitching4
    LIBS += -lopencv_saliency4
    LIBS += -lopencv_objdetect4
    LIBS += -lopencv_saliency4
    LIBS += -lopencv_imgcodecs4
    LIBS += -lopencv_video4
    LIBS += -lopencv_videoio4
    LIBS += -ldlib
    LIBS += -lopenblas
    LIBS += -llapack

    LIBS += -lavformat
    LIBS += -lavcodec
    LIBS += -lavutil
    LIBS += -lswresample
    LIBS += -lswscale
}

SOURCES += \
    beautydialog.cpp \
    beautyprocessor.cpp \
    cannyprocessor.cpp \
    coloradjustprocessor.cpp \
    draggableitemmodel.cpp \
    droppablegraphicsview.cpp \
    gammaprocessor.cpp \
    grayscaleprocessor.cpp \
    histogramwidget.cpp \
    imageblenddialog.cpp \
    imageblendprocessor.cpp \
    imageconverter.cpp \
    imageprocessor.cpp \
    imagestitcherprocessor.cpp \
    imagetexturetransferdialog.cpp \
    imagetexturetransferprocessor.cpp \
    interactivepixmapitem.cpp \
    main.cpp \
    mainwindow.cpp \
    newstitcherdialog.cpp \
    processcommand.cpp \
    stagingareamanager.cpp \
    stitcherdialog.cpp \
    videoprocessor.cpp

HEADERS += \
    beautydialog.h \
    beautyprocessor.h \
    cannyprocessor.h \
    coloradjustprocessor.h \
    draggableitemmodel.h \
    droppablegraphicsview.h \
    gammaprocessor.h \
    grayscaleprocessor.h \
    histogramwidget.h \
    imageblenddialog.h \
    imageblendprocessor.h \
    imageconverter.h \
    imageprocessor.h \
    imagestitcherprocessor.h \
    imagetexturetransferdialog.h \
    imagetexturetransferprocessor.h \
    interactivepixmapitem.h \
    mainwindow.h \
    newstitcherdialog.h \
    processcommand.h \
    stagingareamanager.h \
    stitcherdialog.h \
    videoprocessor.h

FORMS += \
    beautydialog.ui \
    imageblenddialog.ui \
    imagetexturetransferdialog.ui \
    mainwindow.ui \
    newstitcherdialog.ui \
    stitcherdialog.ui

TRANSLATIONS += \
    Qt_Image_Processor_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
    RESOURCES_DIR = $$PWD/resources

    win32 {
        CONFIG(debug, debug|release) {
            DEST_DIR = $$OUT_PWD/debug
        } else {
            DEST_DIR = $$OUT_PWD/release
        }
    }

    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_path($$RESOURCES_DIR) $$shell_path($$DEST_DIR)
