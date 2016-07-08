# Author: CurryChen (qgchenjianzi@foxmail.com)
# 请修改以下llvm参数到您对应的正确目录
LLVM_SRC_PATH := /Users/currychen/llvm
LLVM_BUILD_PATH := /Users/currychen/llvm/llvm3.8-binaries
LLVM_BIN_PATH := $(LLVM_BUILD_PATH)/bin
#LLVM_BUILD_PATH := /Users/currychen/Documents/clang-3.1
#LLVM_BIN_PATH := $(LLVM_BUILD_PATH)/bin
# 以下编译参数中 -I /usr/include 是指定了系统放置头文件的地方,否则会报找不到头文件的错误。
# -L /usr/lib 是指定g++查找的静态库位置
# -v 是输出编译器查找哪些库文件的信息，便于出错的时候进行调试如果输出信息过多，可以去掉
# CXXFLAGS 请参照g++ options指南
CXX := g++ -I /usr/include -v -L /usr/lib
CXXFLAGS := -fno-rtti -O0  -g

LLVM_CXXFLAGS := `$(LLVM_BIN_PATH)/llvm-config --cxxflags`
LLVM_LDFLAGS := `$(LLVM_BIN_PATH)/llvm-config --ldflags --libs --system-libs`
LLVM_LDFLAGS_NOLIBS := `$(LLVM_BIN_PATH)/llvm-config --ldflags`


CLANG_INCLUDES := \
	-I$(LLVM_SRC_PATH)/tools/clang/include \
	-I$(LLVM_BUILD_PATH)/tools/clang/include

CLANG_LIBS := \
	-lclangAST \
        -lclangAnalysis \
        -lclangBasic \
        -lclangDriver \
        -lclangEdit \
        -lclangFrontend \
        -lclangFrontendTool \
        -lclangLex \
        -lclangParse \
        -lclangSema \
        -lclangEdit \
        -lclangASTMatchers \
        -lclangRewrite \
        -lclangRewriteFrontend \
        -lclangStaticAnalyzerFrontend \
        -lclangStaticAnalyzerCheckers \
        -lclangStaticAnalyzerCore \
        -lclangSerialization \
        -lclangToolingCore \
        -lclangTooling 

SRC_CLANG_DIR := src_clang
BUILDDIR := build


$(BUILDDIR)/rewritersample: $(SRC_CLANG_DIR)/rewritersample.cpp
	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $^ $(CLANG_LIBS) $(LLVM_LDFLAGS) -o $@

#$(BUILDDIR)/RewriteSource: $(SRC_CLANG_DIR)/RewriteSource.cpp
#	$(CXX) $(CXXFLAGS) $(LLVM_CXXFLAGS) $(CLANG_INCLUDES) $^ $(CLANG_LIBS) $(LLVM_LDFLAGS) -o $@


.PHONY: clean
clean:
	-rm -rf $(BUILDDIR)/*
