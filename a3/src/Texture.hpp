//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <algorithm>

class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    Eigen::Vector3f getColorBilinear(float u, float v) {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;

        int u_l = std::max(0, static_cast<int>(std::floor(u_img)));
        int u_r = std::min(width - 1, static_cast<int>(std::ceil(u_img)));
        int v_b = std::max(0, static_cast<int>(std::floor(v_img)));
        int v_t = std::min(height - 1, static_cast<int>(std::ceil(v_img)));

        auto c_lb = image_data.at<cv::Vec3b>(v_b, u_l);
        auto c_rb = image_data.at<cv::Vec3b>(v_b, u_r);
        auto c_lt = image_data.at<cv::Vec3b>(v_t, u_l);
        auto c_rt = image_data.at<cv::Vec3b>(v_t, u_r);

        Eigen::Vector3f color_lb(c_lb[0], c_lb[1], c_lb[2]);
        Eigen::Vector3f color_rb(c_rb[0], c_rb[1], c_rb[2]);
        Eigen::Vector3f color_lt(c_lt[0], c_lt[1], c_lt[2]);
        Eigen::Vector3f color_rt(c_rt[0], c_rt[1], c_rt[2]);

        auto color_lerp1 = lerp(u_l, u_r, color_lb, color_rb, u_img);
        auto color_lerp2 = lerp(u_l, u_r, color_lt, color_rt, u_img);
        auto color = lerp(v_b, v_t, color_lerp1, color_lerp2, v_img);
        
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    Eigen::Vector3f lerp(float a, float b, Eigen::Vector3f val_a, Eigen::Vector3f val_b, float x) {
        return ((b - x) / (b - a)) * val_a + ((x - a) / (b - a)) * val_b;
    }

};
#endif //RASTERIZER_TEXTURE_H
