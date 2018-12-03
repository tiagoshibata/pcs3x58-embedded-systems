#include "hand_tracking.hh"

#include <cstdlib>
#include <climits>
#include <iostream>
#include <vector>
#include <utility>

#include <opencv2/opencv.hpp>
#include <librealsense/rs.hpp>

HandTracking::HandTracking(bool debug) : last_reading(0) {
    std::ifstream is("/etc/pimpolho");
    int threshold_upper = 25, open_kernel_radius = 6, close_kernel_radius = 12;
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
    auto depth_callback = [size, threshold_upper, erode_kernel, dilate_kernel, debug, this](rs::frame f) {
        static cv::VideoWriter video_converted, video_threshold, video_open, video_close;
        cv::Mat frame16(size, CV_16UC1, (uint16_t*) f.get_data());
        cv::Mat frame;
        frame16.convertTo(frame, CV_8U, 1. / 32);

        cv::Mat resized_frame;
        cv::Size resized(size.width / 2, size.height / 2);
        cv::resize(frame, resized_frame, resized, 0, 0, cv::INTER_NEAREST);

        if (debug) {
            if (!video_converted.isOpened()) {
                video_converted.open("converted.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 30, resized, false);
                video_threshold.open("threshold.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 30, resized, false);
                video_open.open("open.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 30, resized, false);
                video_close.open("close.avi", cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 30, resized, false);
            }
            video_converted << resized_frame;
        }
        // cv::morphologyEx(binary_mask, frame, cv::MORPH_OPEN, open_kernel);
        // cv::morphologyEx(frame, binary_mask, cv::MORPH_CLOSE, close_kernel);
        cv::inRange(resized_frame, 5, threshold_upper, resized_frame);
        if (debug)
            video_threshold << resized_frame;
        cv::erode(resized_frame, resized_frame, erode_kernel);
        if (debug)
            video_open << resized_frame;
        cv::dilate(resized_frame, resized_frame, dilate_kernel);
        if (debug)
            video_close << resized_frame;

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(resized_frame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        if (debug)
            std::cout << contours.size() << " contours found\n";
        if (contours.size() < 2)
            return;
        double largest_area = 0, second_largest_area = 0;
        std::vector<cv::Point> largest_area_points, second_largest_area_points;
        for (auto contour : contours) {
            double area = cv::contourArea(contour);
            if (area < second_largest_area)
                continue;
            if (area > largest_area) {
                second_largest_area = largest_area;
                second_largest_area_points = std::move(largest_area_points);
                largest_area = area;
                largest_area_points = std::move(contour);
            } else {
                second_largest_area = area;
                second_largest_area_points = std::move(contour);
            }
        }

        int largest_x = 0, largest_y = INT_MAX, second_x = 0, second_y = INT_MAX;
        for (auto point : largest_area_points) {
            // Get highest point of each blob
            if (point.y < largest_y) {
                largest_x = point.x;
                largest_y = point.y;
            }
        }
        for (auto point : second_largest_area_points) {
            if (point.y < second_y) {
                second_x = point.x;
                second_y = point.y;
            }
        }
        int diff = largest_y - second_y;
        if (largest_x > second_x)
            diff = -diff;
        diff = diff > 127 ? 127 : (diff < -127 ? -127 : diff);
        last_reading = (last_reading + diff) / 2;

        if (debug)
            std::cout << "Largest contours: " << largest_area << "(" << largest_x << ", " << largest_y << ")" << " and "
                << second_largest_area << "(" << second_x << ", " << second_y << ")" << " - " << diff << '\n';
    };
    dev->set_frame_callback(rs::stream::depth, depth_callback);
    dev->start();
}

HandTracking::~HandTracking() {
    dev->stop();
}

bool HandTracking::serialize(int8_t *buffer) {
    if (*buffer != last_reading) {
        *buffer = last_reading;
        return true;
    }
    return false;
}
