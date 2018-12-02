#include "hand_tracking.hh"

#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <librealsense/rs.hpp>

HandTracking::HandTracking(bool debug) {
    std::ifstream is("/etc/pimpolho");
    int threshold_upper = 25, open_kernel_radius = 8, close_kernel_radius = 12;
    if (is) {
        is >> threshold_upper >> open_kernel_radius >> close_kernel_radius;
    }

    if (!ctx.get_device_count())
        std::exit(1);
    dev = ctx.get_device(0);
    dev->enable_stream(rs::stream::depth, 0, 0, rs::format::z16, 60);
    rs::intrinsics depth_intrinsics;
    depth_intrinsics = dev->get_stream_intrinsics(rs::stream::depth);

    cv::Size size(depth_intrinsics.width, depth_intrinsics.height);
    cv::Mat open_kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(open_kernel_radius, open_kernel_radius));
    cv::Mat close_kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(close_kernel_radius, close_kernel_radius));
    auto depth_callback = [size, threshold_upper, open_kernel, close_kernel, debug](rs::frame f) {
        static cv::VideoWriter video_converted, video_threshold, video_open, video_close;
        cv::Mat frame16(size, CV_16UC1, (uint16_t*) f.get_data());
        cv::Mat frame;
        frame16.convertTo(frame, CV_8U, 1. / 32);
        cv::Mat binary_mask(size, CV_8U);
        // cv::threshold(frame, binary_mask, 30, 255, cv::THRESH_BINARY_INV);
        cv::inRange(frame, 5, threshold_upper, binary_mask);
        if (debug) {
            std::cout << "Frame\n";
            if (!video_converted.isOpened()) {
                video_converted.open("converted.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 15, size, false);
                video_threshold.open("threshold.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 15, size, false);
            }
            video_converted << frame;
            video_threshold << binary_mask;
        }
        cv::erode(binary_mask, frame, open_kernel);
        cv::dilate(frame, binary_mask, close_kernel);
        // cv::morphologyEx(binary_mask, frame, cv::MORPH_OPEN, open_kernel);
        // cv::morphologyEx(frame, binary_mask, cv::MORPH_CLOSE, close_kernel);
        if (debug) {
            if (!video_open.isOpened()) {
                video_open.open("open.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 15, size, false);
                video_close.open("close.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 15, size, false);
            }
            video_open << frame;
            video_close << binary_mask;
        }
    };
    dev->set_frame_callback(rs::stream::depth, depth_callback);
    dev->start();
}

HandTracking::~HandTracking() {
    dev->stop();
}

bool HandTracking::serialize(int8_t *buffer) {
    // TODO
    return false;
}
