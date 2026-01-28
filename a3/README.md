# 说明

所有评分点均完成。

- 提交格式正确
- 参数插值：`rasterizer.cpp` 文件中的 `rasterize_triangle` 函数
    - 参照作业 2 完成深度值计算
    - 调用 `interpolate()` 函数对颜色、法线等进行插值
- Blinn-Phong 反射模型：`main.cpp` 文件中的 `phong_fragment_shader` 函数
    - 根据 Blinn-Phong 反射模型公式补全代码（主要是环境光 + 漫反射光 + 高光的计算）
- 纹理映射：`main.cpp` 文件中的 `texture_fragment_shader` 函数
    - 获取当前纹理坐标的纹素值，注意值的范围在 [0, 1) 之间（1 不能取，所以我设置成了 0.9999，否则越界产生运行时错误）
    - 其余部分沿用 Blinn-Phong 的计算
- 凹凸映射和位移映射：`main.cpp` 文件中的 `bump_fragment_shader` 和 `displacement_fragment_shader` 函数
    - 根据注释内容实现这两个映射，大体思路是差不多的
    - 其中 `h` 函数就是对应纹理坐标的颜色值，而 `w`, `h` 是纹理的宽和高
    - 位移映射相比凹凸映射还多了一步对点的计算（需要仔细阅读注释），缺少这步会导致位移映射的渲染结果缺少光泽
    - 位移映射还要复用 Blinn-Phong 的计算
- 尝试更多模型
    - 只尝试了 `phong` 和 `normal` 两类渲染，其他渲染没有成功实现，因为给的文件不够完整
- 双线性纹理插值：`Texture.hpp` 文件中的 `getColorBilinear` 函数
    - 保证邻近 4 个点的坐标不要越界
    - 关于线性插值(LERP)的计算，要弄清的一点是：插值点离某个点越近，该点对插值点的影响越大，其权重也越大
    - 双线性插值结果位于 `images/spot/texture_bilinear.png`，而原版为 `images/spot/texture.png`；将两张图放大到足够比例再比对，会发现前者的锯齿感会少一些（在奶牛内部效果会更明显，外部边缘依然有很明显的锯齿感）