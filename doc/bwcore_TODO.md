# bwcore1.5

## 目标

原有代码冗杂且效率低(主要代码4k+行)，可维护性差，全部重构bwcore1.3~1.4(impbwcore)代码。

bwcore1.5(strongbwcore)对目前成熟的技术进行实验和调优。主要目标为快速终局搜索、快速位运算估值、pvs搜索、置换表等。实验性的内容之后考虑。

## 辅助文件

* boardhex.html 用于产生和查看棋盘的十六进制码。

## 棋盘数据结构

位运算棋盘操作board.cpp，封装常用操作。

包含以下操作：

生成可行位置、位运算棋步、对称变换、计数。

### 可行位置生成

使用Zebra代码。使用纯位运算消除分支语句，并用分支技巧把8次位运算降低到3次。

### 位运算棋步

使用位运算或BMI2指令进行快速翻转。

把四个方向的翻转的每一条线，使用PEXT提取映射成一条线。这条线上的翻转可以构造查表。再PDEP放回。

1. 需要有pos映射后的翻转位置(0..63)=>(0..7)。该值为4*64个常数，直接打表查询。
2. 需要pos在各个位置上的mask。
3. 主查询表为在一条线上的翻转，有8*65536种状态，映射为2字节。耗内存1MB。

### 变换

棋盘有8种对称变换，可以下面方式计算全部的变换：

`id`  `flipv`   `lrotate` `rrotate`

`id+bsw(fliph)` `flipv+bsw(r180)`  `lrotate+bsw(transpose)`  `rrotate+bsw`

其中`flipv`按中央竖线对称，`rotate`旋转90度，`transpose`按主对角线对称。

在x86-64上，`fliph`即指令`bswap`，按8位分组翻转64个位。`flip_v`可通过6次`&`和`<<`+3次`|`的位运算得到；`rotate`位运算类似。注：`transpose`也可由3次`zip`(2*32矩阵转置)得到，`zip`由两次`pext`得到。

## 估值

### 模板

* e1(edga+2x),e2,e3,e4直线, k8,k7,k6,k5,k4斜线
* c52, c33 角
* wmob, wodd, wb

使用e2~e4,k8~k4时，无需对棋盘变换。c52需要全部8种变换。c33需要`flipv,fliph,r180`，e1需要`flipv,lrotate,rrotate`。

对长度9-10的模板进行三进制转换。约定默认黑色(先手方)在二进制下为高位，三进制下位权为2。二进制下低位在三进制下为高位。占内存约10MB。

### 文件格式

主要由16位整数构成。

| version | part count | pattern type count | pattern type length        | pattern coeff data | checksum | desc_str_len | desc_str | generate timestamp |
| ------- | ---------- | ------------------ | -------------------------- | ------------------ | -------- | ------------ | -------- | ------------------ |
| int16   | int16      | int16              | int16*(pattern type count) | int16*             | int16    | int16        | cstring  | int64              |

