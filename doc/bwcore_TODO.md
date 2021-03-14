# bwcore1.5

## 目标

原有代码冗杂且效率低(全部代码4k+行)，可维护性差，全部重构bwcore1.3~1.4(impbwcore)代码。

bwcore1.5(strongbwcore)仍然使用较传统的搜索算法。主要为快速终局搜索、快速位运算估值、pvs搜索、置换表等。实验性的内容暂不考虑。

## 辅助文件

* boardhex.html 用于构造棋盘的十六进制码。
* match.py 用于比较两个bot
* merge.py 合并代码到单文件
* 调试时可生成搜索树debugtree.html。父节点的ret应为所有子节点ret取负中的最大值。若节点的ret>=beta，则剪枝。

## 棋盘数据结构

位运算棋盘操作board.cpp，封装常用操作。

包含以下操作：

生成可行位置、位运算棋步、对称变换、计数。

* 更新：Board现在不区分黑白颜色，board.b[0]为先手方，board.b[1]为后手方。Game类保存颜色，仅用于显示。

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

`id+bsw(fliph)` `flipv+bsw(180 rotate)`  `lrotate+bsw(transpose)`  `rrotate+bsw`

其中`flipv`按中央竖线对称，`rotate`旋转90度，`transpose`按主对角线对称。

在x86-64上，`fliph`即指令`bswap`，按8位分组翻转64个位。`flip_v`可通过6次`&`和`<<`+3次`|`的位运算得到；`rotate`位运算类似。注：`transpose`也可由3次`zip`(2*32矩阵转置)得到，`zip`由两次`pext`得到。

## 估值

### 模板

* e1(edga+2x),e2,e3,e4直线, k8,k7,k6,k5,k4斜线
* c52, c33 角
* wmob, wodd, wb

1.5.1更新：加入中心棋子数wcinner, 边棋子数wcedge，角ccor，(2,2)位置cx22

使用e2-e4,k8-k4时，无需对棋盘变换。c52需要全部8种变换。c33需要`flipv,fliph,r180`，e1需要`fliph,lrotate,rrotate`。

对长度9-10的模板进行三进制转换。约定默认黑色(先手方)在二进制下为高位，三进制下位权为2。二进制下低位在三进制下为高位。占内存约10MB。

### 文件格式

主要由16位整数构成。

| version | part count | pattern type count | pattern type length        | pattern coeff data | checksum | desc_str_len | desc_str | generate timestamp |
| ------- | ---------- | ------------------ | -------------------------- | ------------------ | -------- | ------------ | -------- | ------------------ |
| int16   | int16      | int16              | int16*(pattern type count) | int16*             | int16    | int16        | cstring  | int64              |

## 搜索

`search_normal` 执行一般的中局搜索，以Negamax为框架。

### PVS

随机选取节点(第一个)做PVS，平均情况下比普通alphabeta节点略少约10%

#### 试探搜索

[deprecated] bwcore1.3~1.4只对浅层节点使用宽窗口的试探搜索完全排序，效果较好

现在只试探搜索找到pv作为主要节点，不对所有节点排序。(未实验排序，考虑到剪枝搜索无法准确排序，而且即使不准确时次大节点对PVS空窗验证影响不大)

> 目前为递归执行。当搜索目的为空窗验证时，选取好的pv仍能有效剪枝。

### 置换表

黑白棋很少出现相同的搜索树分支。因此置换表只能用于减少迭代加深或试探搜索时出现的重复。用于pvs实验效果一般，命中率不到1%，只能减少5%左右的节点。

更新：在使用迭代加深和试探搜索时，置换表效果提高。

### 终局优化

`seach_end2` 展开搜索剩余两个空

`seach_end<N>` 简单终局搜索，不用额外剪枝，建议6个左右空时使用

### 概率剪枝

变量cnt, height。参数为pc_depth, slope, bias, sigma。

概率剪枝会对上、下界做一次浅层的零窗搜索。

> 数据生成：开gendata.cpp GENDATA_PC宏生成浅层搜索结果到pc_data.txt，再gendata.cpp remake()一次生成深层搜索结果pc_ndata.txt，然后执行回归。

## 测试程序

`make test` 生成 `test.exe`

`test.exe --` 执行检查。

#### 命令行模式

`test.exe ` 运行命令行测试模式

`p` `print` 显示当前局面

`ph` `printhex` 当前局面的十六进制编码

`m x y` `makemove x y` 在x行y列下子，坐标从0,0开始

`u` `unmakemove` 撤销一步 

`evalptn` 当前局面的模板估值

`think` 思考下一步

`play` 思考下一步并执行

`tree depth` 搜索指定步数并保存html格式的搜索树

`cnt` 统计黑白棋子数

`reset` 重置为游戏开始

`ld hex0 hex1` `board hex0 hex1` 设置当前局面为十六进制编码指定的局面

`flipv` `fliph` `rotatel` `rotater` 变换棋盘

`cswap` 交换棋盘黑白颜色

`fcol` 交换当前玩家颜色

`col 0` `col 1` 设置当前玩家颜色(0黑，1白)

`savesgf filename` 以sgf格式保存当前棋谱 

`q` `quit` `exit` 退出

