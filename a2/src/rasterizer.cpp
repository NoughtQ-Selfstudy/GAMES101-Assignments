// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>
#include <limits>
#include <vector>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

// Change the types of x and y to float for MSAA
static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    Vector3f p(x, y, 1.0f);
    Vector3f side, p_side, c_vec[3];
    bool is_inside;
    
    for (int i = 0; i < 3; ++i) {
        side = _v[(i + 1) % 3] - _v[i];
        p_side = _v[(i + 1) % 3] - p;
        c_vec[i] = side.cross(p_side);
    }

    is_inside = c_vec[0].dot(c_vec[1]) >= 0 
                && c_vec[1].dot(c_vec[2]) >= 0 
                && c_vec[2].dot(c_vec[0]) >= 0;

    return is_inside;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            // 本来下面这个语句的第二个 vert.z 需要加负号，改变三角形遮盖顺序
            // 但是我改了以后，三角形的形状变得很奇怪，所以我最后还是不改了（见 result/error.png）
            // 具体见：http://games-cn.org/forums/topic/hw2%e7%9a%84%e7%96%91%e9%97%ae/
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle
    float l = std::min({v[0][0], v[1][0], v[2][0]});
    float r = std::max({v[0][0], v[1][0], v[2][0]});
    float b = std::min({v[0][1], v[1][1], v[2][1]});
    float h = std::max({v[0][1], v[1][1], v[2][1]});

    int c_bias[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    // std::vector<float> sample_buf(width * height * 4);


    for (int x = l; x <= r; ++x)
        for (int y = b; y <= h; ++y) {
            // // Baseline
            // if (insideTriangle(x + 0.5, y + 0.5, t.v)) {
            //     // If so, use the following code to get the interpolated z value.
            //     auto[alpha, beta, gamma] = computeBarycentric2D(x + 0.5, y + 0.5, t.v);
            //     float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
            //     float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
            //     z_interpolated *= w_reciprocal;

            //     // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
            //     if (z_interpolated < depth_buf[x * width + y]) {
            //         depth_buf[x * width + y] = z_interpolated;
            //         Vector3f tmp_pt(x, y, 1.0f);
            //         set_pixel(tmp_pt, t.getColor());
            //     }
            // }

            int idx = get_index(x, y);

            // MSAA
            for (int k = 0; k < 4; ++k) {
                float sample_x = x + c_bias[k][0] * 0.5 + 0.25;
                float sample_y = y + c_bias[k][1] * 0.5 + 0.25;
                if (insideTriangle(sample_x, sample_y, t.v)) {
                    auto[alpha, beta, gamma] = computeBarycentric2D(sample_x, sample_y, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;
                    
                    int sample_idx = idx * 4 + k;
                    if (z_interpolated < sample_buf[sample_idx]) {
                        sample_buf[sample_idx] = z_interpolated;
                        sample_color_buf[sample_idx] = t.getColor();     
                    }
                }
            }

            Vector3f color = Vector3f::Zero();
            // This loop cannot be merged with the previous loop,
            // otherwise it will miss those sampling points that are not inside the triangle.
            for (int k = 0; k < 4; ++k) {
                color += sample_color_buf[idx * 4 + k];
            }
            set_pixel(Vector3f(x, y, 1.0f), color / 4.0);
        }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
        std::fill(sample_color_buf.begin(), sample_color_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
        std::fill(sample_buf.begin(), sample_buf.end(), std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);

    sample_color_buf.resize(4 * w * h);
    sample_buf.resize(4 * w * h);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;
}

void rst::rasterizer::set_sample_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    sample_color_buf[ind] = color;
}

// clang-format on