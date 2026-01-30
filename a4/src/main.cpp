#include <chrono>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    int n = control_points.size();
    
    if (n == 1) return control_points[0];

    std::vector<cv::Point2f> new_points(n - 1);

    for (int i = 1; i < n; ++i) {
        new_points[i - 1] = (1 - t) * control_points[i - 1] + t * control_points[i];
    }

    return recursive_bezier(new_points, t);

}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    const float step = 0.0001;

    for (float i = 0; i <= 1; i += step) {
        auto point = recursive_bezier(control_points, i);

        // baseline:
        // window.at<cv::Vec3b>(point.y, point.x)[2] = 255;

        // bonus: anti-aliasing
        // The closer the sampling point is to the pixel point, 
        // the closer the pixel point's color is to pure red.
        float x = point.x, y = point.y;
        int l = std::floor(x), r = std::min(699.0f, std::ceil(x));
        int t = std::floor(y), b = std::min(699.0f, std::ceil(y));
        std::vector<std::pair<int, int>> points = {{l, t}, {r, t}, {l, b}, {r, b}};
        for (auto point : points) {
            int px = point.first, py = point.second;
            float d = std::sqrt(std::pow(x - px, 2) + std::pow(y - py, 2));
            window.at<cv::Vec3b>(py, px)[2] = std::max(
                static_cast<int>(window.at<cv::Vec3b>(py, px)[2]), 
                static_cast<int>(std::max(0.0f, (1 - d)) * 255)
            );
        }
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            // naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
