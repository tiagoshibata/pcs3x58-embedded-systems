#include "hand_tracking.hh"

#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <librealsense/rs.hpp>

HandTracking::HandTracking(bool debug) {
    std::ifstream is("/etc/pimpolho");
    int threshold_upper = 25, open_kernel_radius = 8, close_kernel_radius = 16;
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
    cv::Mat erode_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(open_kernel_radius, open_kernel_radius));
    cv::Mat dilate_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(close_kernel_radius, close_kernel_radius));
    auto depth_callback = [size, threshold_upper, erode_kernel, dilate_kernel, debug](rs::frame f) {
        static cv::VideoWriter video_converted, video_threshold, video_open, video_close;
        cv::Mat frame16(size, CV_16UC1, (uint16_t*) f.get_data());
        cv::Mat frame;
        frame16.convertTo(frame, CV_8U, 1. / 32);
        if (debug) {
            if (!video_converted.isOpened()) {
                video_converted.open("converted.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 15, size, false);
                video_threshold.open("threshold.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 15, size, false);
                video_open.open("open.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 15, size, false);
                video_close.open("close.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 15, size, false);
            }
            video_converted << frame;
        }
        // cv::morphologyEx(binary_mask, frame, cv::MORPH_OPEN, open_kernel);
        // cv::morphologyEx(frame, binary_mask, cv::MORPH_CLOSE, close_kernel);
        cv::inRange(frame, 5, threshold_upper, frame);
        if (debug)
            video_threshold << frame;
        cv::erode(frame, frame, erode_kernel);
        if (debug)
            video_open << frame;
        cv::dilate(frame, frame, dilate_kernel);
        if (debug)
            video_close << frame;

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
