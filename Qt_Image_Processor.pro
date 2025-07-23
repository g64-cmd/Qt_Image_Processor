# 基础配置
QT += core gui widgets
CONFIG += c++17

# =================== vcpkg 终极集成方案 (最终确认版) ===================

# 1. 定义 vcpkg 的安装路径 (请确保这个路径是您 vcpkg 的真实安装路径)
VCPKG_ROOT_PATH = "C:/vcpkg/vcpkg/installed/x64-windows"

# 2. 关键修正：强制将正确的头文件路径添加到编译命令中
# 我们需要添加 .../include/opencv4 这个路径
QMAKE_CXXFLAGS += -I$$VCPKG_ROOT_PATH/include/opencv4

# 3. 添加正确的库文件搜索路径
LIBS += -L$$VCPKG_ROOT_PATH/lib

# 4. 根据构建模式链接您的项目需要的所有 OpenCV 模块
# (请根据您项目的实际需要，在这里添加或删除模块)
CONFIG(debug, debug|release) {
    # Debug 模式
    LIBS += -lopencv_core4d
    LIBS += -lopencv_imgproc4d
    LIBS += -lopencv_highgui4d
    LIBS += -lopencv_features2d4d
    LIBS += -lopencv_calib3d4d
    LIBS += -lopencv_stitching4d
    LIBS += -lopencv_saliency4d
    # ... 其他您需要的带 'd' 的模块
} else {
    # Release 模式
    LIBS += -lopencv_core4
    LIBS += -lopencv_imgproc4
    LIBS += -lopencv_highgui4
    LIBS += -lopencv_features2d4
    LIBS += -lopencv_calib3d4
    LIBS += -lopencv_stitching4
    LIBS += -lopencv_saliency4
    # ... 其他您需要的不带 'd' 的模块
}
# =====================================================================

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

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
    imagetexturetransferdialog.cpp \
    imagetexturetransferprocessor.cpp \
    interactivepixmapitem.cpp \
    main.cpp \
    mainwindow.cpp \
    processcommand.cpp \
    stagingareamanager.cpp \
    stitcherdialog.cpp

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
    imagetexturetransferdialog.h \
    imagetexturetransferprocessor.h \
    interactivepixmapitem.h \
    mainwindow.h \
    processcommand.h \
    stagingareamanager.h \
    stitcherdialog.h

FORMS += \
    beautydialog.ui \
    imageblenddialog.ui \
    imagetexturetransferdialog.ui \
    mainwindow.ui \
    stitcherdialog.ui

TRANSLATIONS += \
    Qt_Image_Processor_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
