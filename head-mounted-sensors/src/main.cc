#include <chrono>
#include <iostream>
#include <thread>
#include <unistd.h>

#include <librealsense/rs.hpp>
#include <opencv2/opencv.hpp>

#include "9dof.hh"
#include "realsense.h"
#include "serial.h"

typedef struct {
    uint8_t signature;
    int8_t x1;
    int8_t y1;
    int8_t x2;
    uint8_t buttons;
} __attribute__((packed)) gamepad_report_t;

namespace {
    gamepad_report_t report = {};

    void delay_write(int fd, int8_t value) {
        std::this_thread::sleep_for(std::chrono::milliseconds(12));
        if (write(fd, (void *)(&value), 1) != 1) {
            perror("write");
        }
    }

    void send_gamepad_report(int fd) {
        gamepad_report_t report_copy;
        int8_t *report_bytes = (int8_t *)&report_copy;
        for (;;) {
            report_copy = report;
            int8_t sum = 0;
            for (unsigned i = 0; i < sizeof(report); i++) {
                sum ^= report_bytes[i];
                delay_write(fd, report_bytes[i]);
            }
            delay_write(fd, sum);
            delay_write(fd, ~sum);
        }
    }
}


int main(int argc, char **argv) {
    int fd = serial_open("/dev/ttySAC0");
    int verbose = argc > 1 && std::string(argv[1]) == "-v";
    report.signature = 0x55;
    Serialize9Dof s;
    std::thread sender(send_gamepad_report, fd);

    // Cam data prepare
    rs::context ctx;
    if(ctx.get_device_count() == 0) return EXIT_FAILURE;
    rs::device * dev = ctx.get_device(0);
    dev->enable_stream(rs::stream::depth, 0, 0, rs::format::z16, 60);
    rs::intrinsics depth_intrin;
    rs::format depth_format;
    depth_intrin = dev->get_stream_intrinsics(rs::stream::depth);
    depth_format = dev->get_stream_format(rs::stream::depth);
    cv::Mat frameGray;
    cv::Mat frameGrayNrm;
    auto depth_callback = [depth_intrin, depth_format, &frameGray, &frameGrayNrm](rs::frame f)
    {
        cv::Mat frame(cv::Size(depth_intrin.width,depth_intrin.height), CV_16UC1, filterData((uint16_t*)f.get_data(),depth_intrin.width,depth_intrin.height,150));
        frame.convertTo(frameGray, CV_8U, 1. / 64);
    };
    dev->set_frame_callback(rs::stream::depth, depth_callback);
    dev->start();
    int frames = 0;
    int axisHistory[5] = {0,0,0,0,0};
    int buffer;

    for (;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(4));

        if (!frameGray.empty()) {
            cv::Mat rgb;
            cv::morphologyEx(frameGray, rgb, cv::MORPH_OPEN, cv::Mat());
            cv::morphologyEx(rgb, rgb, cv::MORPH_CLOSE, cv::Mat());
            if (frames == 5) {
                int i, soma = 0;
                double value;
                for (i=0;i<frames;i++){
                    soma += axisHistory[i];
                }
                value = soma/frames;

                report.x2 = value;
            } else {
                buffer = getAxis(rgb, 30);
                if (buffer == 0) {
                    for (int jota = 0; jota < 5; jota++) {
                        axisHistory[jota] = 0;
                    }
                } else {
                    axisHistory[frames] = buffer;
                }
                frames++;
            }
        }

        bool has_data = false;
        has_data |= s.serialize(&report.x1);
        // has_data |= realsense_has_data;
        if (!has_data)
            continue;
        if (verbose)
            printf("%x %d %d %d %d\n", report.signature, report.x1, report.y1, report.x2, report.buttons);
    }
    dev->stop();
    return 0;
}
