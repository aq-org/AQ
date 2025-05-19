<p align="center">
  <img width="100px" src="https://github.com/aq-org/AQ/blob/main/aq.png?raw=true" align="center" alt="AQ" />
  <h2 align="center">AQ</h2>
  <p align="center">AQ is a fast, small, simple and safe interpreted programming language. It may be a great work. </p>
</p>

   <p align="center">
     <a href="https://github.com/aq-org/AQ/blob/main/LICENSE">
       <img alt="License" src="https://img.shields.io/badge/license-AQ-dark" />
     </a>
     <a href="https://github.com/aq-org/AQ/commits">
       <img alt="Commits" src="https://img.shields.io/github/commit-activity/t/aq-org/AQ" />
     </a>
     <a href="https://github.com/aq-org/AQ/pulse">
       <img alt="Created At" src="https://img.shields.io/github/created-at/aq-org/AQ" />
     </a>
     <a href="https://github.com/aq-org/AQ/graphs/commit-activity">
       <img alt="Last Commits" src="https://img.shields.io/github/last-commit/aq-org/AQ" />
     </a>
     <a href="https://github.com/aq-org/AQ">
       <img alt="Languages" src="https://img.shields.io/github/languages/count/aq-org/AQ" />
     </a>
     <a href="https://github.com/aq-org/AQ">
       <img alt="Language" src="https://img.shields.io/github/languages/top/aq-org/AQ" />
     </a>
     <a href="https://github.com/aq-org/AQ/issues">
       <img alt="Issues" src="https://img.shields.io/github/issues/aq-org/AQ" />
     </a>
     <a href="https://github.com/aq-org/AQ/pulse">
       <img alt="Code Size" src="https://img.shields.io/github/languages/code-size/aq-org/AQ" />
     </a>
     <a href="https://github.com/aq-org/AQ/graphs/contributors">
       <img alt="Repo Size" src="https://img.shields.io/github/repo-size/aq-org/AQ" />
     </a>
     <a href="https://github.com/aq-org/AQ/stargazers">
       <img alt="Stars" src="https://img.shields.io/github/stars/aq-org" />
     </a>
     <a href="https://github.com/aq-org/AQ/forks">
       <img alt="Forks" src="https://img.shields.io/github/forks/aq-org/AQ" />
     </a>
     <a href="https://twitter.com/aq_organization">
       <img alt="Twitter" src="https://img.shields.io/twitter/follow/aq_organization" />
     </a>
     <br />
     <br />
   </p>

   <p align="center">
     <a href="https://www.twitter.com/aq_organization" rel="nofollow"><img src="https://img.shields.io/badge/x-%23232323.svg?&amp;style =for-the-badge&amp;logo=X&amp;logoColor=white" height="25" style="max-width: 100%;"></a>
     <a href="https://www.instagram.com/aqsorg/" rel="nofollow"><img src="https://img.shields.io/badge/instagram-%23E4405F.svg?&amp; style=for-the-badge&amp;logo=instagram&amp;logoColor=white" height="25" style="max-width: 100%;"></a>
     <a href="https://www.facebook.com/aq.organization" rel="nofollow"><img src="https://img.shields.io/badge/facebook-%231DA1F2.svg?&amp;style =for-the-badge&amp;logo=facebook&amp;logoColor=white" height="25" style="max-width: 100%;"></a>
     <a href="https://www.reddit.com/u/aqorg/" rel="nofollow"><img src="https://img.shields.io/badge/reddit-%23E4405F.svg? &amp;style=for-the-badge&amp;logo=reddit&amp;logoColor=white" height="25" style="max-width: 100%;"></a>
     <a href="https://aqorg.tumblr.com/" rel="nofollow"><img src="https://img.shields.io/badge/tumblr-%23232323.svg?&amp;style= for-the-badge&amp;logo=tumblr&amp;logoColor=white" height="25" style="max-width: 100%;"></a>
     </p>

   <p align="center">
     <a href="#Quickstart">Quickstart</a>
     ·
     <a href="https://github.com/aq-org/AQ/issues/new">Report a Bug</a>
     ·
     <a href="https://github.com/aq-org/AQ/discussions/new/choose">Request to add features</a>
   </p>

<p align="center">Like this project? Please consider <a href="https://github.com/aq-org/AQ">Sponsor</a>, <a href="https://github.com/aq-org/AQ">join development </a> or <a href="https://github.com/aq-org/AQ">Stars</a> to help it improve! </p>

<p align="center">Translations may be inaccurate or delayed, please read the English version if available. If you find any bugs, please <a href="https://github.com/aq-org/AQ/issues/new">report</a> to us. </p>

### 软件架构文档

#### 一、架构概述
AQ 是一款快速、轻量、简单且安全的解释型编程语言，其核心架构由四大模块组成：标准库支持（aqstl.h）、虚拟机核心（vm.c）、跨平台初始化模块（aqvm_init.h）及编译器实现（compiler.cc）。各模块通过模块化设计解耦，支持跨平台运行（Windows/POSIX），遵循最小依赖与安全优先原则。

#### 二、模块分解

##### 1. 跨平台初始化模块（<mcfile name="aqvm_init.h" path="f:\aq\prototype\aqvm_init.h"></mcfile>）
- **职责**：处理运行环境的初始化，确保UTF-8编码支持与区域设置兼容。
- **关键实现**：
  - Windows平台：通过`SetConsoleCP(CP_UTF8)`和`SetConsoleOutputCP(CP_UTF8)`配置控制台编码（`aqvm_win32_init`函数）。
  - POSIX平台：尝试`C.UTF-8`等多种区域设置（`aqvm_init`函数循环检测）。
- **输出**：初始化成功返回0，失败返回-1。

##### 2. 标准库支持（<mcfile name="aqstl.h" path="f:\aq\prototype\aqstl.h"></mcfile>）
- **职责**：定义基础数据结构（如`Object`、`Data`联合体）、全局对象表（`object_table`）及异常终止接口（`EXIT_VM`）。
- **关键设计**：
  - `Data`联合体支持多类型数据存储（字节、长整型、双精度浮点、字符串、对象指针等）。
  - `Object`结构包含类型标识（`type`）、常量标记（`const_type`）及数据体（`data`）。
  - 注释中保留了函数调用栈追踪的原型设计（`Trace`结构体及`PushStack`/`PopStack`函数），当前通过`#define TRACE_FUNCTION`禁用。

##### 3. 虚拟机核心（<mcfile name="vm.c" path="f:\aq\prototype\vm.c"></mcfile>）
- **职责**：实现虚拟机运行时环境，管理常量对象表（`const_object_table`），负责字节码指令的解析与执行。
- **关键接口**：
  - 包含`aqstl.h`和`aqvm_init.h`头文件，依赖标准库与初始化模块。
  - 注释中定义了对象存储相关的原型结构（如`InternalObject`），当前未启用。
  - 通过`#define TRACE_FUNCTION`禁用了函数调用栈追踪功能（与`aqstl.h`设计一致）。
- **内部结构**：
  - **常量对象表管理**：通过`const_object_table`（类型为`struct Object*`）和`const_object_table_size`（类型为`size_t`）维护常量对象的存储，确保运行时快速访问常量值。
  - **运行时环境维护**：虚拟机通过`struct Memory`（当前注释未启用）设计支持对象表管理，但当前主要依赖`const_object_table`实现轻量运行时环境。
  - **指令执行流程**：基于`enum Operator`定义的27种操作码（如`OPERATOR_LOAD`加载数据、`OPERATOR_ADD`执行加法），通过`struct Bytecode`结构体解析操作符及参数，逐指令执行完成程序逻辑。

  - **数据类型定义**：虚拟机通过类型标识符（1字节）定义数据类型，具体如下：
    - `0x00`：动态类型（无固定类型标识，运行时动态确定）。
    - `0x01`：byte（1字节无符号整数）。
    - `0x02`：int（64位有符号整数）。
    - `0x03`：float（64位双精度浮点数）。
    - `0x04`：uint64t（64位无符号整数）。
    - `0x05`：字符串（以空字符结尾的字符数组）。
    - `0x06`：指针（通常指向动态数组，首元素为数组长度（uint64t），后续元素为数组内容（索引从0开始））。
    - `0x07`：引用（指向其他对象的弱引用，不影响对象生命周期）。
    - `0x08`：常量（运行时不可修改的对象）。
    - `0x09`：类（定义对象的结构和方法的模板）。
    - **const_type验证逻辑**：当`const_type`为`true`时，`0x06`（指针）、`0x07`（引用）、`0x08`（常量）类型会额外增加验证位（连续字节），用于校验类型一致性。验证位格式为：具体类型标识符（`0x06`）后跟随类型（动态则为`0x00`），直到遇到非验证类型标识符为止（例如：`0x06 0x00`表示动态类型的指针）。

##### 4. 编译器实现（<mcfile name="compiler.cc" path="f:\aq\prototype\compiler.cc"></mcfile>）
- **职责**：处理词法分析、语法解析及字节码生成，定义操作码集（`_AQVM_OPERATOR_*`）。
- **关键组件**：
  - `LexMap`模板类：用于词法分析的哈希表（初始容量1024），支持关键字查找。
  - 异常终止接口（`EXIT_COMPILER`）：输出错误信息并终止程序。
  - 操作码覆盖内存操作（`LOAD`/`STORE`）、算术运算（`ADD`/`SUB`）、控制流（`IF`/`GOTO`）等核心指令（共27种）。

#### 三、模块依赖关系
```
compiler.cc → aqstl.h
vm.c → aqstl.h + aqvm_init.h
aqvm_init.h → 无外部依赖
aqstl.h → 无外部依赖
```
编译器直接依赖标准库头文件；虚拟机同时依赖标准库与初始化模块；初始化模块与标准库为基础层，无外部依赖。

#### 四、关键设计原则
1. **跨平台兼容**：通过`#ifdef _WIN32`条件编译区分Windows与POSIX实现，确保UTF-8编码一致性。
2. **模块化解耦**：初始化、标准库、虚拟机、编译器独立成模块，降低维护成本。
3. **安全优先**：
  - 内存操作（如`LexMap`构造）检查分配结果（`if (pair_list_ == nullptr)`），避免空指针异常。
  - 异常终止接口（`EXIT_VM`/`EXIT_COMPILER`）统一错误处理流程。
4. **轻量设计**：禁用非必要功能（如函数调用栈追踪），减少运行时开销。

#### 五、字节码结构
AQ虚拟机采用紧凑的字节码格式，支持跨平台执行。字节码结构主要包含操作码（Opcode）和操作数（Operand）两部分，具体定义如下：

##### 1. 操作码定义
操作码占1字节（0x00-0xFF），定义了虚拟机支持的核心指令集，共27种基础操作码（部分关键操作码示例）：
- `OPERATOR_NOP`（0x00）：无操作指令。
- `OPERATOR_LOAD`（0x01）：从内存加载数据到寄存器。
- `OPERATOR_STORE`（0x02）：将寄存器数据存储到内存。
- `OPERATOR_ADD`（0x06）：执行加法运算。
- `OPERATOR_IF`（0x0F）：条件跳转指令。
- `OPERATOR_GOTO`（0x16）：无条件跳转指令。
- `OPERATOR_WIDE`（0xFF）：扩展操作码，用于支持更长的操作数。

##### 2. 操作数类型及布局
操作数根据操作码不同支持以下类型：
- **立即数**：直接包含数值（如整数、浮点数），占2-8字节。
- **索引**：指向常量对象表或内存地址的索引（占2字节，最大支持65535个索引）。
- **偏移量**：用于跳转指令的相对地址偏移（占4字节，支持±2GB范围）。

##### 3. 指令编码方式
基础指令采用“1字节操作码 + 可变长度操作数”格式。例如：
- `OPERATOR_LOAD_CONST`（0x17）指令格式：`0x17 [2字节索引]`，表示从常量对象表加载索引对应的常量。
- `OPERATOR_GOTO`（0x16）指令格式：`0x16 [4字节偏移量]`，表示跳转到当前指令指针+偏移量的位置。

通过`OPERATOR_WIDE`扩展可支持更长的操作数（如4字节索引），扩展指令格式：`0xFF [1字节扩展操作码] [4字节操作数]`。

#### 六、扩展方向
- 启用`Trace`机制增强调试能力（需同步`aqstl.h`与`vm.c`的注释代码）。
- 完善`object_table`对象管理逻辑，支持动态对象生命周期控制。
- 扩展`LexMap`功能，支持更多编译器关键字类型。

> Copyright 2024 AQ author, All Rights Reserved.
> This program is licensed under the AQ License. You can find the AQ license in the root directory.