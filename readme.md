# BigInt

适用于c++的大整数类型，包括加，减，乘，除，模运算（待实现）

## 功能

- [x] BigInt + BigInt
- [x] BigInt - BigInt
- [x] BigInt * BigInt
- [x] BigInt * int
- [x] BigInt / BigInt
- [x] BigInt / int
- [ ] BigInt % BigInt
- [ ] BigInt % int

## 后续计划

使用FFT更新大整数乘法

## 文件结构

- bigInt.h：BigInt class所在类
- main.cpp：用于测试各功能
- run.py：利用python内置的大数运算来编写测试样例和c++程序交互进行验证

# 参考

主要参考了[人形魔芋](https://www.zhihu.com/people/ren-xing-mo-yu-68)的代码实现：[C++ 压位高精度](https://zhuanlan.zhihu.com/p/571239433)，并修改了其中一部分逻辑bug