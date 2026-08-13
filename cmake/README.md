/*
Title: 使用说明
Sort: 1
*/
# CMake 工程管理模板

基于cmake，构建的一套源代码工程管理模板。模板中规定了源代码目录结构、命名规范，编译后的目标的目录结构、命名规范等。

模板中提供了统一的Qt支持的国际化翻译功能；支持桌面应用程序、控制台应用程序、动态库工程、静态库工程、插件工程等。

当前模板适用于使用visual studio开发工具。

<!-- TOC -->

- [CMake 工程管理模板](#cmake-工程管理模板)
- [使用的前提条件](#使用的前提条件)
- [基于CMake的项目目录结构](#基于cmake的项目目录结构)
- [Qt工程的语言翻译](#qt工程的语言翻译)
- [支持的Qt工程类型](#支持的qt工程类型)
    - [自定义变量与函数](#自定义变量与函数)
    - [Qt 桌面应用程序](#qt-桌面应用程序)
        - [引用NCQFC中的Boost类库](#引用NCQFC中的Boost类库)
        - [引用NCQFC中的其他类库](#引用NCQFC中的其他类库)
        - [引用Qt类库](#引用Qt类库)
        - [引用项目内部的私有类库](#引用项目内部的私有类库)
        - [创建项目私有的类库](#创建项目私有的类库)
- [编译后的目录结构](#编译后的目录结构)
    - [Windows](#windows)
    - [Mac OS](#mac-os)
    - [Linux](#linux)

<!-- /TOC -->

# 使用的前提条件

使用CMake工程模板，需满足以下条件。

* 可用的编译器
  * Windows:
    * _Recommended_: [Visual Studio 2015](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx). 
    * _Minimum_: Visual Studio 2015 is the minimum compatible compiler.
  * Linux:
    * _Minimum_: gcc 4.9.3.

# 基于CMake的项目目录结构

在磁盘中创建如下项目目录结构：**当前例子所在目录%NCQFC_ROOT/sample/cmakeapp**

    cmakeapp                          (1) 根目录
      |--3rdparty                     (2) 第三方库根目录
      |--ncqfc                        (3) C++/Qt基础类库，来源于仓库 
      |    |--cmake                   (4) CMake工程模板
      |    |--nqutils            
      |--src                          (5) 当前项目的源代码根目录
      |    |--app                     (6) 当前项目的主工程，即可执行程序
      |    |    |--main.cpp
      |    |    |--CMakeLists.txt
      |    |--libs                    (7) 当前项目私有的动态、静态链接库目录
      |    |    |--samplestatic       (8) 当前项目私有的静态库
      |    |    |    |--CMakeLists.txt
      |    |    |    |--samplestatic_dependencies.cmake   
      |    |    |--sampleshared       (9) 当前项目私有的动态库
      |    |    |    |--sampleshared_dependencies.cmake
      |    |    |    |--CMakeLists.txt
      |    |    |--CMakeLists.txt    
      |    |--tools                   (10) 当前项目的辅助性功可自行程序
      |    |--CMakeLists.txt
      |--tests                        (11) 当前项目单元测试目录
      |    |--CMakeLists.txt
      |--CMakeLists.txt               (12) 当前项目公共的配置信息

主要目录解释：
* (1) 项目的根目录。
* (2) 项目依赖的第三方的库文件所在的目录。
* (3) 引用来自C++/Qt基础类库的源码。
  * 推荐使用SVN的[`外部定义`](http://svndoc.iusesvn.com/svnbook/1.4/svn.advanced.externals.html)属性把NCQFC的源码引入到当前的项目中。
* (4) CMake工程模板，当前项目可选择使用CMake维护源代码的工程结构。
  * 统一限定了源代码的目录结构以及命名规则。
  * 统一限定了编译后的输出目录结构以及命名规则。
* (5) 当前项目的源代码所在目录。
  * 当前项目可执行程序、动态库、静态库、工具等源码统一放置在此目录下。
* (6) 当前项目主工程，每个项目有且只能有一个主工程，其他可执行程序都属于辅助可执行程序。
* (7) 当前项目的动态、静态链接库统一的放置的目录。
* (8) 当前项目内部开发的动态链接库。
* (9) 当前项目内部开发的静态链接库。
* (10) 当前项目的辅助可执行程序统一放置的目录。
* (11) 当前项目单元测试统一放置的目录。
* (12) 当前项目统一的cmake配置信息。

# Qt工程的语言翻译

> CMake工程模板中提供了统一的语言翻译处理策略，代码中只需要使用tr宏包裹后即可成ts文件。在提供翻译功能中需要关注变量。
>> `RUN_TS`：为用户提供了一个在ON和OFF中做出选择的选项。当RUN_TS=true时，模板自动从源码中提取tr包裹的资源生成ts文件。ts文件按照一个项目一套翻译ts文件。模板执行时，会在每个项目下自动创建一个ts目录。当前项目的ts文件都统一放置在这个ts目录下。
>>
>> `RUN_QM`: 为用户提供了一个在ON和OFF中做出选择的选项。当RUN_QM=true时，模板自动从编辑好的ts文件中生成项目自身的qm文件。qm文件统一放置在编译后的根目录下的translations目录下。同时模板中桌面应用程序模板，插件类型的模板自动对qm进行合并操作。递归找到模块依赖的所有动态库，并且将动态库的qm文件合并至目标项目的qm文件。同时生成目标qm文件会统一放置在项目编译后的根目录下的share/translations目录下。
>>
>> `LANGUAGES`：定义为项目专用的目标语言的列表。当前变量需要在wisFindQt.cmake文件中指定。

# 支持的工程类型

现阶段支持基于桌面应用程序、控制台应用程序、动态链接库、静态链接库、插件库。

## 自定义变量与函数

### 自定义变量

```
# 头文件列表
HEADERS
# 源文件列表
SOURCES
# 窗体文件列表
FORMS
# 资源文件列表
RESOURCES
# 其他文件列表
OTHER_FILES
# 依赖的本项目内部开发的库
WIS_MODULE_DEPS
# 依赖的Qt库
QT_MODULES
# 依赖的boost库
BOOST_DEPENDS
# 依赖的其他第三方库
DEPENDENCIES
# 苹果专用的framework库
FRAMEWORKS
```

### 自定义MACRO和函数

```
# 控制生成动态库
WIS_QT_INIT_LIBRARY_MODULE
# 控制生成可执行程序
WIS_QT_INIT_EXE_MODULE
# 控制生成静态库
WIS_QT_INIT_STATIC_MODULE
# 控制台应用程序
WIS_QT_INIT_CONSOLE_EXE_MODULE
# 依赖于Qt的可执行程序(非主应用程序）
WIS_QT_INIT_TOOL_EXE_MODULE
# 控制台应用程序
WIS_QT_INIT_TOOL_CONSOLE_MODULE
# 信息输出
colormsg
```

## 桌面应用程序

通过CMake构建桌面应用程序时，在该桌面应用程序根目录下我们需要编写CMakeLists.txt作为构建入口文件，其次其源码目录下各级子目录下均需要编写对应的CMakeLists.txt文件来作为各级子目录的构建入口文件。下面对根目录下CMakeLists.txt文件作简要介绍：
```
# 工程名称
project(${IDE_TARGET_NAME})
# 依赖的相关头文件的路径
INCLUDE_DIRECTORIES(
    ${PROJECT_BINARY_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../libs/sampleshared
    ${CMAKE_CURRENT_LIST_DIR}/../libs/samplestatic
    ${THIRDPARTY_ROOT}/boost/include
    ${THIRDPARTY_ROOT}/boost/Windows/MSVC/x86/14.0
)
# 源文件列表
SET(SOURCES
    ${SOURCES}
    main.cpp
)
# 头文件列表.
SET(HEADERS
)
# UI文件列表
FILE(GLOB_RECURSE FORMS "${CMAKE_CURRENT_SOURCE_DIR}/*.ui")
#Qt模块
SET(QT_MODULES
    Core
)
#boost模块
SET(BOOST_COMPONENTS
    thread
)
 INCLUDE(FindWISBoost)
# 依赖的非Qt库
set(DEPENDENCIES
     
)
##lib搜索路径
LINK_DIRECTORIES(
    ${THIRDPARTY_ROOT}/boost/Windows/MSVC/x86/14.0
)
# 依赖的Boost库
SET(BOOST_DEPENDS
    thread
)
# 生成主应用程序
WIS_QT_INIT_EXE_MODULE(TRUE TRUE)
```

### 引用NCQFC中的Boost类库

在当前项目的CMakeLists.txt文件中添加如下内容：
```
##lib搜索路径
LINK_DIRECTORIES(
    ${THIRDPARTY_ROOT}/boost/Windows/MSVC/x86/14.0
)
# 依赖的Boost库
SET(BOOST_DEPENDS
    thread
)
```

### 引用NCQFC中的其他类库

在当前项目的CMakeLists.txt文件中添加如下内容：
```
set(DEPENDENCIES
     sampleshared
)
```

### 引用Qt类库

在当前项目的CMakeLists.txt文件中添加如下内容：
```
SET(QT_MODULES
    Core
)
```

### 引用项目内部的私有类库

在当前项目的CMakeLists.txt文件中添加如下内容：
```
SET(WIS_MODULE_DEPS
	sampleshared
)
```

### 创建项目私有的类库

通过CMake构建动态链接库时，在该动态库源码的根目录下我们一般需要编写两个文件CMakeLists.txt（动态库工程构建入口文件）和xxx_dependencies.cmake，其次其源码目录下各级子目录下均需要编写对应的CMakeLists.txt文件作为该目录的构建入口文件，下面对根目录下两个文件作简要介绍(完整内容请参考源码)：

### CMakeLists.txt

```
# 导入项目依赖项的文件
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/sampleshared_dependencies.cmake)
# 项目名称
PROJECT(${WIS_PROJECT_NAME})
# 源文件列表
SET(SOURCES
        ${SOURCES}
        helloworld.cpp
)
# 头文件列表
  SET (HEADERS
        ${HEADERS}
        helloworld.h
)
# 其他文件列表
SET(OTHER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/sampleshared_dependencies.cmake
)
# 生成动态库
WIS_QT_INIT_LIBRARY_MODULE(FALSE FALSE)
```

### xxx_dependencies.cmake

```
#xxx_dependencies.cmake文件描述了该动态库依赖的其他库
# 项目名称
SET(WIS_PROJECT_NAME sampleshared)
# 依赖非Qt库
set(DEPENDENCIES)
# 依赖的Qt库
SET(QT_MODULES)
```

# 编译后的目录结构

根据各大操作系统的特性，当前模板统一指定了编译后的目录结构。如下所示。

## Windows

    XXX
      |--bin
      |--lib
      |--share
      |    |--doc
      |--plugins
      |--pdb

## Mac OS

    XXX
      |--xxx.app
      |    |--Frameworks
      |    |--PlugIns
      |    |--Resources
      |        |--doc
      |    |--MacOS

## Linux

    XXX
        |--