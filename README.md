# GAMES101 Projects

本仓库记录 GAMES101 Projects 的完成进度。应闫令琪老师要求，仓库设为私有。

- 往期作业汇总帖：<http://games-cn.org/forums/topic/allhw/>
- BBS 讨论区（如果做作业遇到问题，翻一翻会有不少收获）：<https://games-cn.org/forums/forum/graphics-intro/>
    >注意，访问前两个网站时不要翻墙，否则大概率无法正常访问。
- 课程网站：<https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html>
- 个人笔记：<https://note.noughtq.top/software/cg/>


## Prerequisites

- GAMES101 上课内容
- 掌握基本的 C++ 语法，对 OOP 有一定理解


## Progress

- [x] Assignment0：虚拟机的使用
    - 在 Lecture 04 上课当天开放，但学过 Lecture 03 后就可以完成了
    - 虚拟机在我的电脑（MacBook Air, Apple M4, macOS 26）上未能安装成功，故只完成了其中的编程部分
    - 初步学习了 C++ [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_page) 库（配环境又配了半天...），并巩固齐次坐标的概念
- [x] Assignment1：旋转与投影
    - 在 Lecture 05 上课当天开放，但学过 Lecture 04 后就可以完成了
    - 通过本项目，对 3D 旋转变换（z轴，以及任意过原点的轴（罗德里格斯旋转公式）（属于进阶部分））和透视变换（包括正交变换）更加熟悉
    - 同时也对 opencv 库有了大致的了解
    - 其实可以再补充一段用命令行指定任意轴的代码，但因时间问题就不写了（~~其实是懒了~~）
- [x] Assignment2：三角形和深度缓存
    - 在 Lecture 07 上课当天开放，学到该讲才能继续完成后面的任务
    - 本项目有助于熟悉光栅化、深度缓存的实现，以及通过 MSAA 实现反走样（提高部分）
    - 代码框架和老师上课讲的有些出入——代码框架是左手系的，我（包括很多人）都是按右手系做的（Assignment1）。由于 Assignment1 的投影变换代码会复用到 Assignment2 上，所以最后得到的三角形是上下颠倒的。老师在 Lecture 08 开头说过这没关系
    - 感觉自己的超采样代码还是有问题，得到的结果和没有超采样的差不多，只是运行时间变得更久了。但由于时间原因就不再 debug 了
- [ ] Assignment3
- [ ] Assignment4
- [ ] Assignment5
- [ ] Assignment6
- [ ] Assignment7
- [ ] Assignment8
