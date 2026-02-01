# 说明

所有评分点均完成。

- 提交格式正确
- 光线生成（`Renderer.cpp` 中的 `Renderer` 函数）
    - 屏幕坐标 `(i, j)` -> 相机空间坐标 `(x, y)`
    - 像素中心定位 + 归一化：`x_norm = (i + 0.5) / scene.width`, `y_norm = (j + 0.5) / scene.height`
    - 转换到屏幕空间（NDC）：`x_ndc = 2 * x_norm - 1`, `y_ndc = 1 - 2 * y_norm`（因为图像是从左上角开始存储的，`j` 向下增长，但 3D 空间中，`y` 向上为正，因此需要取相反数）
    - 根据注释，不要忘记乘上 `scale`，以及为 `x` 单独乘上 `imageAspectRatio`
    - 计算 `dir` 时需调用 `normalize()` 函数进行归一化处理
    - 原代码框架中将 `framebuffer` 保存到文件中的循环中，三个 `char`（范围：[-128, 127]）需改成 `unsigned char`（范围：[0, 255]，符合 RGB 取值）
- 光线与三角形相交（`Triangle.hpp` 中的 `rayTriangleIntersect` 函数）
    - 根据课件给出的公式对照着计算就行了（`tnear`, `u`, `v` 分别对应课件中的 $t, b_1, b_2$）
    - 最后返回的布尔值表示光线和三角形是否相交，判断条件是 `tnear`, `u`, `v` 均不小于 0 且 `u + v` 不超过 1