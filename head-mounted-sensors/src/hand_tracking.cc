#include "hand_tracking.hh"

#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <librealsense/rs.hpp>

HandTracking::HandTracking(bool debug) {
    if (!ctx.get_device_count())
        std::exit(1);
    dev = ctx.get_device(0);
    dev->enable_stream(rs::stream::depth, 0, 0, rs::format::z16, 60);
    rs::intrinsics depth_intrinsics;
    depth_intrinsics = dev->get_stream_intrinsics(rs::stream::depth);
    cv::Size size(depth_intrinsics.width, depth_intrinsics.height);

    cv::VideoWriter converted;
    if (debug) {
        converted.open("converted.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 30, size, false);
    }
    auto depth_callback = [size, debug, &converted](rs::frame f) {
        cv::Mat frame16(size, CV_16UC1, (uint16_t*) f.get_data());
        cv::Mat frame;
        frame16.convertTo(frame, CV_8U, 1. / 64);
        if (debug)
            converted << frame;
        cv::Mat binary_mask;
        cv::threshold(frame, binary_mask, 50, 255, cv::THRESH_BINARY_INV);
        // cv::inRange(frame, 0, 200, binary_mask);
        cv::morphologyEx(binary_mask, frame, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(8, 8)));
        cv::morphologyEx(frame, binary_mask, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(16, 16)));
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
