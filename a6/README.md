# 说明

所有评分点均完成。

- 提交格式正确
- 根据前一次作业和现有代码框架补全 `Render()` 和 `Triangle::getIntersection` 函数
    - `Render()`：这次作业中 `castRay` 函数实现位于 `Scene.cpp` 中而不是同一文件中，并且函数签名有所不同（需用 `eye_pos` 和 `dir` 构建 `Ray` 对象）
    - `Triangle::getIntersection`：只需根据 `t_tmp` 的值设置返回内容即可（`t_tmp > 0` 才说明相交）
- 包围盒求交（`Bounds3.hpp` 中的 `IntersectP` 函数）
    - 主要根据课件中 Bounding Volume 部分的介绍实现代码
    - 如果方向值为负，需交换 `t_enter` 和 `t_exit`（`pMax` -> `pMin`）
- BVH 查找（`BVH.cpp` 中的 `getIntersection` 函数）
	- 主要根据课件中光线遍历 BVH 的算法伪代码实现

		```cpp
		Intersect(Ray ray, BVH node) {
			if (ray misses node.bbox) return;

			if (node is a leaf node)
				test intersection with all objs;
				return closest intersection;

			hit1 = Intersect(ray, node.child1);
			hit2 = Intersect(ray, node.child2);

			return the closer of hit1, hit2;
		}
		```

	- 本作业中叶节点只有一个几何体（`object`），无需用循环遍历
	- 注意左右节点不一定都有相交的情况（根据 `Intersection` 类的 `happend` 字段判断）
		- 都相交 -> 返回 `distance` 字段值最小的那个
		- 只有一个相交 -> 返回相交的那个
		- 都不相交 -> 返回一个空的 `Intersection` 对象
- 【Bonus】SAH 查找（`BVH.cpp` 中的 `recursiveBuild` 函数）
	- 公式：`Cost = C_trav + (S_A / S_N) * N_A * C_isect + (S_B / S_N) * N_B * C_isect`
		- `S_N`, `S_A`, `S_B`：父节点、左子节点、右子节点的包围盒表面积
		- `C_trav`：遍历一个 BVH 节点的成本
		- `C_isect`：计算光线与图元是否相交的成本
		- `N_A`, `N_B`：左右子节点的图元数
	- 构建 BVH 树时选择成本最小的划分方式，因此构建 BVH 的时间比一般方法要长，但质量更高，因此渲染时间更短
	- 时间开销对比：

		|| BVH | SAH |
		|:-:|:-:|:-:|
		| BVH 构建时间（ms） | 8 | 135 |
		| 渲染时间（ms）| 1699 | 1482 |

	- 最后不要忘记把代码中用到 BVH 的部分改为采用 SAH 方法（`Scene.cpp` 的 `buildBVH` 函数，以及 `Triangle.hpp` 的 `MeshTriangle` 构造函数的末尾）
- 代码框架中的一些 bug
    - `Vector3f` 中 `[]` 运算符的 `const` 和非 `const` 版本的返回值都是 `double`，需改为 `float`；并且还要补上非 `const` 版本缺失的实现
    - 由于模型文件不大，所以 BVH 构建和渲染时间相当短，以秒为单位可能无法比对 BVH 和 SAH 方法的执行时间，所以将计时单位改为了毫秒