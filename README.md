# R-Compiler

R-Compiler 能够编译Rust语言的子集，并生成llvm IR代码。

## 安装

```bash
# clone 仓库
git clone
cd R-Compiler
```

### 使用方法1
```bash

# 创建构建目录
mkdir build
cd build

# 生成构建文件
cmake ..

# 编译项目
make

# 运行编译器
./R-Compiler
```
程序会从标准输入读取源代码，并将生成的 LLVM IR 代码输出到标准输出。

### 使用方法2
```bash
# 编译项目
make build

# 运行项目
make run
```

## Reference

[Rust语言子集](https://github.com/peterzheng98/RCompiler-Spec/)

[Parser参考](https://zhuanlan.zhihu.com/p/471075848)

[Semantic参考](https://notes.sjtu.edu.cn/s/Rkii5SClH)

[IR&Judging](https://notes.sjtu.edu.cn/s/Ocf1RnyaR)

[测试点](https://github.com/peterzheng98/RCompiler-Testcases)