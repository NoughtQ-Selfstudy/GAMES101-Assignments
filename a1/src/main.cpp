#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

constexpr double MY_PI = 3.1415926;

double deg2rad(double d) {
    return d / 180 * MY_PI;
}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    double alpha = deg2rad(rotation_angle);

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    model(0, 0) = cos(alpha);
    model(0, 1) = -sin(alpha);
    model(1, 0) = sin(alpha);
    model(1, 1) = cos(alpha);

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f M_p2o = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f M_o = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f M_t = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f M_s = Eigen::Matrix4f::Identity();

    double t, r;

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    M_p2o(0, 0) = zNear;
    M_p2o(1, 1) = zNear;
    M_p2o(2, 2) = zNear + zFar;
    M_p2o(2, 3) = -zNear * zFar;
    M_p2o(3, 2) = 1;
    M_p2o(3, 3) = 0;

    t = fabs(zNear) * tan(deg2rad(eye_fov) / 2);
    r = aspect_ratio * t;

    M_t(2, 3) = -(zNear + zFar) / 2;
    M_s(0, 0) = 1 / r;
    M_s(1, 1) = 1 / t;
    M_s(2, 2) = 2 / (zNear - zFar);

    M_o = M_s * M_t;
    projection = M_o * M_p2o;

    return projection;
}

Eigen::Matrix4f get_rotation(Vector3f axis, float angle) {
    Eigen::Matrix3f I = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f N = Eigen::Matrix3f::Zero();
    Eigen::Matrix3f R_3;
    Eigen::Matrix4f R = Eigen::Matrix4f::Identity();

    float alpha = deg2rad(angle);
    // 归一化旋转轴
    axis.normalize();

    N(0, 1) = -axis(2);
    N(0, 2) = axis(1);
    N(1, 0) = axis(2);
    N(1, 2) = -axis(0);
    N(2, 0) = -axis(1);
    N(2, 1) = axis(0);

    R_3 = cos(alpha) * I + (1 - cos(alpha)) * axis * axis.transpose() + sin(alpha) * N;
    R.block<3, 3>(0, 0) = R_3;

    return R;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    // 新增：定义旋转轴，默认为 Z 轴
    Eigen::Vector3f axis = {0, 0, 1};

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_rotation(axis, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {   // ESC
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_rotation(axis, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
        // 新增：添加用于切换轴的按键
        else if (key == 'x') {
            axis = {1, 0, 0}; // 切换到 X 轴
            std::cout << "Rotating around X-axis" << std::endl;
        }
        else if (key == 'y') {
            axis = {0, 1, 0}; // 切换到 Y 轴
            std::cout << "Rotating around Y-axis" << std::endl;
        }
        else if (key == 'z') {
            axis = {0, 0, 1}; // 切换回 Z 轴
            std::cout << "Rotating around Z-axis" << std::endl;
        }
    }

    return 0;
}
