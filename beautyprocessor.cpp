// beautyprocessor.cpp
#include "beautyprocessor.h"
#include "imageconverter.h"
#include <dlib/opencv.h>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <vector>

static std::unique_ptr<QTemporaryFile> extractResource(const QString& resourcePath)
{
    QFile resourceFile(resourcePath);
    if (!resourceFile.exists()) {
        qWarning() << "Resource not found:" << resourcePath;
        return nullptr;
    }

    auto tempFile = std::make_unique<QTemporaryFile>();
    if (tempFile->open()) {
        if (resourceFile.open(QIODevice::ReadOnly)) {
            tempFile->write(resourceFile.readAll());
            resourceFile.close();
            tempFile->close();
            return tempFile;
        }
    }
    qWarning() << "Failed to open or write to temporary file for resource:" << resourcePath;
    return nullptr;
}

BeautyProcessor::BeautyProcessor()
{
    // --- UPDATED: Initialize dlib's face detector and landmark model ---
    face_detector = dlib::get_frontal_face_detector();

    QString landmarkModelResourcePath = ":/resources/models/shape_predictor_68_face_landmarks.dat";

    tempModelFile = extractResource(landmarkModelResourcePath);
    if (tempModelFile) {
        try {
            dlib::deserialize(tempModelFile->fileName().toStdString()) >> landmark_predictor;
            qDebug() << "Successfully loaded landmark model from resource.";
        } catch (const dlib::serialization_error& e) {
            qWarning() << "Error: Could not load landmark model from resource" << landmarkModelResourcePath << e.what();
        }
    }
}

QImage BeautyProcessor::process(const QImage &sourceImage, int smoothLevel, int thinLevel)
{
    if (sourceImage.isNull() || landmark_predictor.num_parts() == 0) {
        qWarning() << "Beauty processor not initialized or models failed to load, skipping.";
        return sourceImage;
    }

    cv::Mat originalMat = ImageConverter::qImageToMat(sourceImage);
    if (originalMat.empty()) return sourceImage;

    if (originalMat.channels() == 4) {
        cv::cvtColor(originalMat, originalMat, cv::COLOR_BGRA2BGR);
    }

    // --- UPDATED: Use dlib's cv_image for detection ---
    dlib::cv_image<dlib::bgr_pixel> dlib_img(originalMat);

    // Smart upscaling for low-resolution images
    dlib::cv_image<dlib::bgr_pixel> detection_img = dlib_img;
    float scale = 1.0f;
    int min_size_for_detection = 250;
    if (originalMat.cols < min_size_for_detection || originalMat.rows < min_size_for_detection) {
        scale = std::max(2.0f, min_size_for_detection / static_cast<float>(std::min(originalMat.cols, originalMat.rows)));
        cv::Mat upscaled_mat;
        cv::resize(originalMat, upscaled_mat, cv::Size(), scale, scale, cv::INTER_CUBIC);
        detection_img = dlib::cv_image<dlib::bgr_pixel>(upscaled_mat);
        qDebug() << "Image is small, upscaling by" << scale << "for detection.";
    }

    // --- UPDATED: Perform face detection using dlib's detector ---
    std::vector<dlib::rectangle> faces = face_detector(detection_img);

    qDebug() << "Detected" << faces.size() << "face(s).";

    if (faces.empty()) {
        return sourceImage;
    }

    cv::Mat processedMat = originalMat.clone();

    for (const auto& face_upscaled : faces) {
        // Scale face rectangle back to original image coordinates
        dlib::rectangle face;
        if (scale > 1.0f) {
            face = dlib::rectangle(
                (long)(face_upscaled.left() / scale),
                (long)(face_upscaled.top() / scale),
                (long)(face_upscaled.right() / scale),
                (long)(face_upscaled.bottom() / scale)
                );
        } else {
            face = face_upscaled;
        }

        dlib::full_object_detection landmarks = landmark_predictor(dlib_img, face);

        if (thinLevel > 0) {
            qDebug() << "--- Applying face thinning with level:" << thinLevel;
            applyFaceThinning(processedMat, landmarks, thinLevel);
        }
        if (smoothLevel > 0) {
            qDebug() << "--- Applying skin smoothing with level:" << smoothLevel;
            applySkinSmoothing(processedMat, landmarks, smoothLevel);
        }
    }

    return ImageConverter::matToQImage(processedMat);
}

void BeautyProcessor::applySkinSmoothing(cv::Mat &image, const dlib::full_object_detection& landmarks, int level)
{
    if (level <= 0 || landmarks.num_parts() != 68) return;

    // 1. Create a precise skin mask from landmarks
    std::vector<cv::Point> face_hull;
    for (unsigned long i = 0; i <= 16; ++i) face_hull.push_back(cv::Point(landmarks.part(i).x(), landmarks.part(i).y()));
    for (unsigned long i = 26; i >= 17; --i) face_hull.push_back(cv::Point(landmarks.part(i).x(), landmarks.part(i).y()));

    cv::Mat skin_mask = cv::Mat::zeros(image.size(), CV_8UC1);
    cv::fillConvexPoly(skin_mask, face_hull, 255);

    std::vector<cv::Point> left_eye, right_eye, mouth;
    for (unsigned long i = 36; i <= 41; ++i) left_eye.push_back(cv::Point(landmarks.part(i).x(), landmarks.part(i).y()));
    for (unsigned long i = 42; i <= 47; ++i) right_eye.push_back(cv::Point(landmarks.part(i).x(), landmarks.part(i).y()));
    for (unsigned long i = 48; i <= 59; ++i) mouth.push_back(cv::Point(landmarks.part(i).x(), landmarks.part(i).y()));
    cv::fillConvexPoly(skin_mask, left_eye, 0);
    cv::fillConvexPoly(skin_mask, right_eye, 0);
    cv::fillConvexPoly(skin_mask, mouth, 0);

    cv::GaussianBlur(skin_mask, skin_mask, cv::Size(15, 15), 0, 0);

    // --- Advanced Surface Blur Algorithm ---
    qDebug() << "Applying advanced surface blur with level:" << level;

    // 2. Separate color (low frequency) from texture (high frequency)
    cv::Mat low_freq;
    int kernel_size = (level / 10) * 2 + 1;
    cv::GaussianBlur(image, low_freq, cv::Size(kernel_size, kernel_size), 0, 0);

    cv::Mat high_freq = image - low_freq;

    // 3. Enhance the low frequency layer (color/tone)
    cv::Mat enhanced_low_freq;
    int d = level / 10 + 5;
    cv::bilateralFilter(low_freq, enhanced_low_freq, d, 150, 150);

    // 4. Combine enhanced color with original texture
    cv::Mat result = enhanced_low_freq + high_freq;

    // 5. Blend the result back into the original image using the skin mask
    result.copyTo(image, skin_mask);
}

void BeautyProcessor::applyFaceThinning(cv::Mat &image, const dlib::full_object_detection& landmarks, int level)
{
    if (level <= 0 || landmarks.num_parts() != 68) return;

    cv::Mat tempImage = image.clone();
    cv::Mat map_x = cv::Mat(image.size(), CV_32FC1);
    cv::Mat map_y = cv::Mat(image.size(), CV_32FC1);

    dlib::point p_left_jaw = landmarks.part(3);
    dlib::point p_right_jaw = landmarks.part(13);
    dlib::point p_chin = landmarks.part(8);
    dlib::point p_nose_bridge = landmarks.part(27);

    cv::Point2f chin_pt(p_chin.x(), p_chin.y());
    cv::Point2f nose_pt(p_nose_bridge.x(), p_nose_bridge.y());
    cv::Point2f central_axis_vec = chin_pt - nose_pt;

    double radius = dlib::length(p_chin - landmarks.part(27));
    double strength = level / 100.0 * 0.15;

    for (int y = 0; y < tempImage.rows; y++) {
        for (int x = 0; x < tempImage.cols; x++) {
            map_x.at<float>(y, x) = static_cast<float>(x);
            map_y.at<float>(y, x) = static_cast<float>(y);

            dlib::point current_pt(x, y);
            dlib::point jaw_pt = (x < nose_pt.x) ? p_left_jaw : p_right_jaw;
            double dist_to_jaw = dlib::length(current_pt - jaw_pt);

            if (dist_to_jaw < radius) {
                cv::Point2f current_pt_f(x, y);
                cv::Point2f vec_to_pt = current_pt_f - nose_pt;
                float proj = vec_to_pt.dot(central_axis_vec) / (central_axis_vec.x*central_axis_vec.x + central_axis_vec.y*central_axis_vec.y);
                cv::Point2f pt_on_axis = nose_pt + proj * central_axis_vec;

                float dx = x - pt_on_axis.x;
                float dy = y - pt_on_axis.y;

                float scale = strength * std::pow(1.0 - dist_to_jaw / radius, 2);

                map_x.at<float>(y, x) = static_cast<float>(x + dx * scale);
                map_y.at<float>(y, x) = static_cast<float>(y + dy * scale);
            }
        }
    }
    cv::remap(tempImage, image, map_x, map_y, cv::INTER_LINEAR, cv::BORDER_REPLICATE);
}
