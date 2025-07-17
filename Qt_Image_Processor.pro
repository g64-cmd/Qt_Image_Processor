# 基础配置
QT += core gui widgets
CONFIG += c++17

# 设置 OpenCV 根目录（根据你的实际路径）
OPENCV_ROOT = C:/openCV/opencv/build

# 头文件路径（指向 include 目录）
INCLUDEPATH += $$OPENCV_ROOT/include
INCLUDEPATH += $$OPENCV_ROOT/include/opencv2  # 部分版本需要单独添加 [1,3](@ref)

# 库文件路径（区分 Debug/Release）
win32 {
    CONFIG(debug, debug|release) {
        # Debug 模式
        LIBS += -L$$OPENCV_ROOT/x64/vc16/lib \
                -lopencv_world4120d  # 注意版本号（490 需替换为你的实际版本）
    } else {
        # Release 模式
        LIBS += -L$$OPENCV_ROOT/x64/vc16/lib \
                -lopencv_world4120    # 无后缀
    }
}

# 视频模块预留（后续迭代需启用）
# LIBS += -lopencv_videoio4120 -lopencv_objdetect4120

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    imageconverter.cpp \
    imageprocessor.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    imageconverter.h \
    imageprocessor.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    Qt_Image_Processor_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
