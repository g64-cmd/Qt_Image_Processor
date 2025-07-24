# 基础配置
QT += core gui widgets
CONFIG += c++17

# =================== vcpkg 终极集成方案 (最终确认版) ===================

# 1. 定义 vcpkg 的安装路径 (请确保这个路径是您 vcpkg 的真实安装路径)
VCPKG_ROOT_PATH = "C:/vcpkg/vcpkg/installed/x64-windows"

# 2. 关键修正：强制将正确的头文件路径添加到编译命令中
# 我们需要添加 .../include/opencv4 这个路径
QMAKE_CXXFLAGS += -I$$VCPKG_ROOT_PATH/include/opencv4
INCLUDEPATH += $$VCPKG_ROOT_PATH/include

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
    LIBS += -lopencv_objdetect4d  # <-- 新增此行，解决 CascadeClassifier 链接错误
    LIBS += -lopencv_saliency4d   # <-- 这是您之前需要的 saliency 模块
    LIBS += -lopencv_stitching4d
    LIBS += -lopencv_calib3d4d
    LIBS += -lopencv_imgcodecs4d
    # ... 其他您需要的带 'd' 的模块
    LIBS += -ldlibd               # <-- 新增此行，解决 dlib 链接错误
    LIBS += -lopenblas
    LIBS += -llapack
} else {
    # Release 模式
    LIBS += -lopencv_core4
    LIBS += -lopencv_imgproc4
    LIBS += -lopencv_highgui4
    LIBS += -lopencv_features2d4
    LIBS += -lopencv_calib3d4
    LIBS += -lopencv_stitching4
    LIBS += -lopencv_saliency4
    LIBS += -lopencv_objdetect4  # <-- 新增此行，解决 CascadeClassifier 链接错误
    LIBS += -lopencv_saliency4   # <-- 这是您之前需要的 saliency 模块
    LIBS += -lopencv_stitching4
    LIBS += -lopencv_calib3d4
    LIBS += -lopencv_imgcodecs4
    # ... 其他您需要的不带 'd' 的模块
    LIBS += -ldlib               # <-- 新增此行，解决 dlib 链接错误
    LIBS += -lopenblas
    LIBS += -llapack
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
    imagestitcherprocessor.cpp \
    imagetexturetransferdialog.cpp \
    imagetexturetransferprocessor.cpp \
    interactivepixmapitem.cpp \
    main.cpp \
    mainwindow.cpp \
    newstitcherdialog.cpp \
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
    imagestitcherprocessor.h \
    imagetexturetransferdialog.h \
    imagetexturetransferprocessor.h \
    interactivepixmapitem.h \
    mainwindow.h \
    newstitcherdialog.h \
    processcommand.h \
    stagingareamanager.h \
    stitcherdialog.h

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

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
    # 定义资源文件夹的源路径 (假设它和 .pro 文件在同一目录下)
    RESOURCES_DIR = $$PWD/resources

    # 定义目标路径 (根据构建模式是 Debug 还是 Release)
    # $$OUT_PWD 是指向构建目录 (build-...) 的变量
    win32 {
        CONFIG(debug, debug|release) {
            DEST_DIR = $$OUT_PWD/debug
        } else {
            DEST_DIR = $$OUT_PWD/release
        }
    }

    # 定义一个在链接步骤完成后执行的命令
    # 1. 先删除旧的 resources 文件夹 (如果存在)，确保是全新复制
    # 2. 再将源 resources 文件夹完整复制到目标目录
    QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_path($$RESOURCES_DIR) $$shell_path($$DEST_DIR)
