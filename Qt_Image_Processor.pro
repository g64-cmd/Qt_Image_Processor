# =============================================================================
# Qt_Image_Processor.pro
#
# Qt项目配置文件 (.pro)。
# 该文件定义了项目所需的Qt模块、C++标准、第三方库依赖、源文件、
# 头文件、UI界面文件以及构建和部署规则。
#
# 项目维护者：g64
# 最后更新日期：2025-07-25
# =============================================================================


#------------------------------------------------------------------------------
# 1. 项目基本配置 (Project Basic Configuration)
#------------------------------------------------------------------------------

# 添加项目所需的Qt模块。
# - core: 提供核心的非GUI功能，如信号与槽、文件I/O、数据结构等。
# - gui: 提供GUI功能的基础类，包括窗口系统集成、事件处理、2D图形等。
# - widgets: 提供一套经典的桌面UI控件 (QPushButton, QLabel, etc.)。
# - multimedia: 提供音视频播放、录制和处理的功能。
# - multimediawidgets: 提供与多媒体功能集成的UI控件 (e.g., QVideoWidget)。
QT += core gui widgets multimedia multimediawidgets

# 设置项目使用的C++标准。
CONFIG += c++17

# 定义最终生成的可执行文件的名称。
# 默认为项目文件名 (Qt_Image_Processor)。
TARGET = Qt_Image_Processor


#------------------------------------------------------------------------------
# 2. 依赖项管理 (Dependency Management)
#------------------------------------------------------------------------------
# 此部分负责配置第三方库，如OpenCV, dlib, FFmpeg等。
#------------------------------------------------------------------------------

# 定义 vcpkg 的安装路径。
# 注意：这是一个固定的硬编码路径。如果项目需要在不同的计算机上编译，
# 请确保每台计算机上的 vcpkg 都安装在此路径下，否则需要手动修改此行。
VCPKG_ROOT_PATH = "C:/vcpkg/vcpkg/installed/x64-windows"

# 将第三方库的头文件目录添加到项目的包含路径中。
# QMAKE_CXXFLAGS 是传递给C++编译器的标志。
# INCLUDEPATH 是qmake专门用来管理包含路径的变量。
QMAKE_CXXFLAGS += -I$$VCPKG_ROOT_PATH/include/opencv4
INCLUDEPATH += $$VCPKG_ROOT_PATH/include

# 将第三方库的库文件目录添加到项目的链接路径中。
LIBS += -L$$VCPKG_ROOT_PATH/lib


#------------------------------------------------------------------------------
# 3. 库链接 (Library Linking)
#------------------------------------------------------------------------------
# 根据构建模式（Debug或Release）链接不同版本的第三方库。
# - Debug模式下，库文件名通常以 'd' 结尾 (e.g., opencv_core4d.lib)。
# - Release模式下，使用性能优化过的正式版库。
#------------------------------------------------------------------------------

# CONFIG(debug, debug|release) 是一个作用域，当处于Debug模式时生效。
CONFIG(debug, debug|release) {
    # --- Debug Libraries ---
    message("Linking with DEBUG libraries.")

    # OpenCV Libraries
    LIBS += -lopencv_core4d
    LIBS += -lopencv_imgproc4d
    LIBS += -lopencv_highgui4d
    LIBS += -lopencv_features2d4d
    LIBS += -lopencv_calib3d4d
    LIBS += -lopencv_stitching4d
    LIBS += -lopencv_saliency4d
    LIBS += -lopencv_objdetect4d
    LIBS += -lopencv_imgcodecs4d
    LIBS += -lopencv_video4d
    LIBS += -lopencv_videoio4d

    # dlib and its dependencies
    LIBS += -ldlibd
    LIBS += -lopenblas
    LIBS += -llapack

    # FFmpeg Libraries (for video processing)
    LIBS += -lavformatd
    LIBS += -lavcodecd
    LIBS += -lavutild
    LIBS += -lswresampled
    LIBS += -lswscaled

} else {
    # --- Release Libraries ---
    message("Linking with RELEASE libraries.")

    # OpenCV Libraries
    LIBS += -lopencv_core4
    LIBS += -lopencv_imgproc4
    LIBS += -lopencv_highgui4
    LIBS += -lopencv_features2d4
    LIBS += -lopencv_calib3d4
    LIBS += -lopencv_stitching4
    LIBS += -lopencv_saliency4
    LIBS += -lopencv_objdetect4
    LIBS += -lopencv_imgcodecs4
    LIBS += -lopencv_video4
    LIBS += -lopencv_videoio4

    # dlib and its dependencies
    LIBS += -ldlib
    LIBS += -lopenblas
    LIBS += -llapack

    # FFmpeg Libraries (for video processing)
    LIBS += -lavformat
    LIBS += -lavcodec
    LIBS += -lavutil
    LIBS += -lswresample
    LIBS += -lswscale
}


#------------------------------------------------------------------------------
# 4. 项目文件列表 (Project Files)
#------------------------------------------------------------------------------
# 将项目的所有源文件、头文件和UI文件分组列出，以提高可读性。
# 这种分组仅为了在.pro文件中看起来清晰，不影响实际文件目录结构。
#------------------------------------------------------------------------------

# --- 应用程序入口与主窗口 ---
SOURCES += main.cpp \
           mainwindow.cpp
HEADERS += mainwindow.h
FORMS   += mainwindow.ui

# --- 对话框 (Dialogs) ---
SOURCES += beautydialog.cpp \
           imageblenddialog.cpp \
           imagetexturetransferdialog.cpp \
           newstitcherdialog.cpp \
           stitcherdialog.cpp
HEADERS += beautydialog.h \
           imageblenddialog.h \
           imagetexturetransferdialog.h \
           newstitcherdialog.h \
           stitcherdialog.h
FORMS   += beautydialog.ui \
           imageblenddialog.ui \
           imagetexturetransferdialog.ui \
           newstitcherdialog.ui \
           stitcherdialog.ui

# --- 核心图像/视频处理逻辑 (Processors) ---
SOURCES += beautyprocessor.cpp \
           cannyprocessor.cpp \
           coloradjustprocessor.cpp \
           gammaprocessor.cpp \
           grayscaleprocessor.cpp \
           imageblendprocessor.cpp \
           imagestitcherprocessor.cpp \
           imagetexturetransferprocessor.cpp \
           imageprocessor.cpp \
           videoprocessor.cpp
HEADERS += beautyprocessor.h \
           cannyprocessor.h \
           coloradjustprocessor.h \
           gammaprocessor.h \
           grayscaleprocessor.h \
           imageblendprocessor.h \
           imagestitcherprocessor.h \
           imagetexturetransferprocessor.h \
           imageprocessor.h \
           videoprocessor.h

# --- 自定义UI控件与模型 (Custom UI & Models) ---
SOURCES += draggableitemmodel.cpp \
           droppablegraphicsview.cpp \
           histogramwidget.cpp \
           interactivepixmapitem.cpp
HEADERS += draggableitemmodel.h \
           droppablegraphicsview.h \
           histogramwidget.h \
           interactivepixmapitem.h

# --- 工具与管理器 (Utilities & Managers) ---
SOURCES += imageconverter.cpp \
           processcommand.cpp \
           stagingareamanager.cpp
HEADERS += imageconverter.h \
           processcommand.h \
           stagingareamanager.h


#------------------------------------------------------------------------------
# 5. 国际化 (Internationalization)
#------------------------------------------------------------------------------
# 配置翻译文件，以支持多语言。
#------------------------------------------------------------------------------

TRANSLATIONS += Qt_Image_Processor_zh_CN.ts

# lrelease: 告诉qmake在构建过程中自动调用lrelease工具，将.ts文件编译成.qm文件。
# embed_translations: 将生成的.qm文件嵌入到可执行文件中。
CONFIG += lrelease embed_translations


#------------------------------------------------------------------------------
# 6. 资源管理 (Resource Management)
#------------------------------------------------------------------------------
# 管理项目的资源文件，如图标、样式表等。
#------------------------------------------------------------------------------

# 指定资源集合文件 (.qrc)。
RESOURCES += resources.qrc

# 定义资源文件夹的路径，用于后续的部署操作。
RESOURCES_DIR = $$PWD/resources


#------------------------------------------------------------------------------
# 7. 构建与部署 (Build & Deployment)
#------------------------------------------------------------------------------

# 为不同平台设置安装路径（主要用于 'make install' 命令）。
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 为Windows平台设置输出目录。
# debug和release版本将分别生成到构建目录下的 'debug' 和 'release' 子文件夹中。
win32 {
    CONFIG(debug, debug|release) {
        DEST_DIR = $$OUT_PWD/debug
    } else {
        DEST_DIR = $$OUT_PWD/release
    }
}

# 定义一个构建后执行的命令。
# 此命令会将 'resources' 文件夹下的所有内容复制到可执行文件所在的目录。
# 这对于需要动态加载的资源（如模型文件、配置文件等）非常有用。
# $$shell_path() 用于确保路径中的空格被正确处理。
QMAKE_POST_LINK += $$QMAKE_COPY_DIR $$shell_path($$RESOURCES_DIR) $$shell_path($$DEST_DIR)

