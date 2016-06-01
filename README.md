目的
最近基于clang在做clang-tools时，由于clang-llvm在OS X 下的开发环境略坑，消耗了我不少时间和精力，所以写下此文希望能帮助到后来的开发者，把更多的精力放在工具的制作上。

环境配置上如果有什么问题欢迎一起交流、反馈，以帮助更多开发者。

email: qgchenjianzi@foxmail.com

准备工作
获取llvm
mkdir llvm
cd llvm
svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm
获取Clang
cd llvm //进入上一步你新建的llvm文件夹下
cd tools 
svn co http://llvm.org/svn/llvm-project/cfe/trunk clang
获取Clang额外工具（可选）
cd llvm
cd tools
svn co http://llvm.org/svn/llvm-project/clang-tools-extra/trunk extra
获取Compiler-RT
cd llvm/projects
svn co http://llvm.org/svn/llvm-project/compiler-rt/trunk compiler-rt
手动编译 （推荐是去官网获取官方编译好的二进制文件，第六步提供方法）
mkdri build (为了编译的时候不污染源代码目录)
cd build
../llvm/configure
make
这将编译 LLVM 和 Clang 的 Debug 版本
获取官方编译好的二进制文件 http://llvm.org/releases/download.html
打开链接第一个看到的最新版本，目前最新版是LLVM3.8

在 Pre-Built Binaries 里面选择Mac版本的sig文件下载，下载后解压。

把解压后的文件夹重命名为llvm3.8-binaries

把该文件夹剪切到 llvm 文件夹（第一步新建的那个文件夹）中

其他
在终端下输入

ls /usr/lib/lib*curse*
如果没有含有类似文件

/usr/lib/libcurses.dylib, /usr/lib/libncurses.5.4.dylib, /usr/lib/libncurses.5.dylib, /usr/lib/libncurses.dylib

请按该教程指示下载 https://gist.github.com/cnruby/960344

注意点
由于在官网上下载的OS X二进制文件是由基于llvm的g++编译出来的，不需要在OS X上安装GNU GCC，保证工程编译器与二进制编译器的一致性，否则会报莫名其妙的错误。

HelloWorld
从github上clone一个验证工程 https://github.com/qgchenjianzi/clangToolsForMac
打开Makefile文件，按照里面的提示配置自己项目的环境
Makefile配置完成，make一下工程
在项目目录下执行
./build/rewritersample  ./input/test.c
如果成功执行，说明环境配置成功啦~~恭喜！可以快乐地撸代码了！