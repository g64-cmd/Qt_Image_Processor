# Qt_Image_Processor

## 项目简介

Qt_Image_Processor 是一个基于 Qt 6 开发的桌面应用程序，集成了丰富的图像和视频处理功能，适用于多种视觉处理场景。

## 依赖项

- **框架**: Qt 6.9.1
- **计算机视觉**: OpenCV 4.x, dlib
- **音视频处理**: FFmpeg 7.1.1
- **编译器**: MSVC 2022 (Windows)
- **包管理器**: vcpkg

## 软件功能

### 图像处理
- 亮度、对比度、饱和度、色相、伽马校正
- 图像锐化、灰度化、边缘检测 (Canny)
- 图像拼接、组合、融合、人脸美颜、纹理迁移
- 实时直方图显示、像素点颜色拾取器
- 暂存区管理，支持拖放、撤销/重做

### 视频处理
- 基于 FFmpeg 的多格式视频播放，含音频轨道
- 实时滤镜（灰度）
- 基于 dlib 的实时人脸检测
- 视频参数实时调节
- 播放控制（播放/暂停、进度条）
- 导出当前帧为图片

## 软件结构

- **mainwindow.\***: 主窗口类，负责整体 UI 布局及各模块集成
- **videoprocessor.\***: 视频处理核心控制器，管理播放列表和 UI 控件，创建并管理 VideoDecoder 线程
- **VideoDecoder** (videoprocessor.\*): 后台线程，负责 FFmpeg 解码、dlib 人脸检测、OpenCV 特效处理
- **imageprocessor.\***: 图像处理工具类，包含各类 OpenCV 算法
- **stagingareamanager.\***: 图像暂存区管理器，负责图片的添加、删除、更新及显示
- **\*dialog.\***: 各类高级功能弹窗（如 beautydialog.\*, imageblenddialog.\*）
- **resources.qrc**: Qt 资源文件，包含图标、字体、样式表等

## 待升级项

- 图像画笔、图像文字框、图像裁切
- 视频打开逻辑整合
- 视频播放跳转、倍速、音量调整，视频缩放
- 视频滤镜（锐化、模糊）
- 保存处理后视频
- 用户登录机制
- UI美化

## 许可证

本项目遵循 GPLv3 许可证，详情见 [LICENSE](LICENSE)。

## 联系方式

如有问题或建议，请通过本仓库的 Issues 页面提交。
