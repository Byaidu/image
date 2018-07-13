关于本文档
----------

作者：bbaa -Te Studio

文档修改时间：2018-7-7

image基础文件说明
-----------------

### 2.1 cmd.exe

修改版Cmd 用于加载Image.dll

### 2.2 init.dll

加载同目录下的所有Dll

### 2.3 image.dll

image3.0模块核心

### Github项目地址

[[https://github.com/Byaidu/image/]{.underline}](https://github.com/Byaidu/image/)

基础语法
--------

3.1 调用方法
------------

### 3.1.1 基础调用

通过命令 Set image-\<Command\> \[argv1 argv2\...\...

### 3.1.2 复古调用

新建image.bat

内容

即可通过普通的调用第三方的方式来调用:

image \[argv1 \[argv2\...\...

此方法会影响效率

3.2 基础命令
------------

### 3.2.1 help

显示帮助

### 3.2.2 load

**语法:load \<tag\> file**

\@param tag Tag名称

\@param file 文件路径(也许不支持中文)

作用:新建图元[^1]并将file文件绘制到名为\[tag\]的图元并将图元绘制到名为\[tag\]的画布上

### 3.2.3 unload

**语法:unload \<tag\>**

\@param tag Tag名称

作用：删除画布\[tag\]并释放内存

### 3.2.4 save

**语法: save \<tag\> \<file\>**

\@param tag Tag名称

\@param file 文件路径

作用保存画布tag的内容到file

### 3.2.5 target

**语法·:target \<tag\>**

\@param tag Tag名称

作用:切换当前画图目标到tag

### 3.2.6 buffer

**语法:buffer \<tag\> \[width\] \[height\] \[r\] \[g\] \[b\]**

\@param tag Tag名称

\@param width 宽

\@param height 高

\@param r/g/b RGB颜色的R值G值B值

作用:
新建画布tag，并新建一个颜色为rgb（默认为白色）、大小为w\*h（默认为当前绘图目标的大小）图元tag，再将图元tag绘制到画布tag

### 3.2.7 resize

**语法:** resize \<tag\> \<width\> \<height\>

\@param tag Tag名称

\@param width 宽

\@param height 高

作用:
将画布tag缩放到width\*height的大小，如果绘图目标为cmd，则会同时将控制台设置为最适合绘图的状态

### 3.2.8 draw

**语法: draw \<tag\> \<x\> \<y\> \[trans \[r\] \[g\] \[b\] \| alpha
\<a\>\] **

\@param tag Tag名称

\@param x/y 绘图位置XY

\@param trans 将颜色为rgb的像素变为透明

\@param alpha

\*a 透明度

作用: 将画布tag绘制到当前绘图目标的x,y位置上\
若指定了trans，则以rgb为透明色（默认为白色）\
若指定了alpha，则以a为透明度

### 3.2.9 font

**语法:font \[r\] \[g\] \[b\] \[width\] \[height\] 0 0 \[字体粗细(400)\]
\[斜体(0)\] \[下划线(0)\] \[删除线(0)\] \[字体名\]**

作用:设置当前绘图目标所用字体的rgb值和大小

注:括号内容为不设置时的占位

如果只设置高度，宽度设为0时即按照字体比例自动调整宽度

### 3.2.10 text

**语法:text \<string\> \<x\> \<y\>**

\@param string 内容

在当前绘图目标的x,y的位置上输出字符串string

### 3.2.11 getpix

**语法：getpix \<tag\> \<x\> \<y\>**

作用: 将画布tag上x,y位置的rgb值存储到变量image

### 3.2.12 setpix

**语法: setpix tag \<x\> \<y\> \<r\> \<g\> \<b\>**

作用: 设置画布tag上x,y位置的rgb值

### 3.2.13 cls

作用:清空画布cmd的内容

### 3.2.14 export

作用: 将画布cmd的句柄存储到变量image

### 3.2.15 import

**语法:** import \<tag\> \<handle\>

\@param handle export出来的cmd句柄

作用: 通过句柄将另一个控制台的画布cmd映射到此控制台的画布tag

### 3.2.16 sleep

语法:sleep \<time\>

\@param time 毫秒

作用: 延时time毫秒

### 3.2.17 list

语法:list \<file\> \[labal\]

\@param label 标签名称

作用:
执行image指令脚本file，若指定了label则会直接跳转到脚本中的标签label

### 3.2.18 exit

作用: 退出当前image指令脚本

### 3.2.19 union

语法:union \<tag\>

作用: 合并图层tag中的所有图元成一个与图层tag同名的图元tag

### 3.2.20 debug

作用: 以图形形式输出图元索引树，用于查看画布cmd上的各个图元

### 3.2.22 mouse

语法:mouse time \[region1\] \[region2\] ..

\@param time 等待时间

作用:  

捕获鼠标坐标及事件，坐标以像素为单位，时间以毫秒为单位

若time\>-0，当发生点击事件或时间超过限制时会将鼠标坐标x,y以及坐标在画布cmd上所在图元tag的tag存储到变量image，并将图元tag的tag单独再存储到变量errorlevel\
若time\<0，不设置时间限制

若指定了region，那么返回的的就不是图元tag的tag而是region的序号，如果鼠标坐标不在任何一个指定的region中，则返回序号0\
region应以如下的形式给出：x1,y1,x2,y2

### 3.2.23 show

用法:show \<tag\>

将画布tag显示在一个窗口，当画布更新时窗口内容也会自动更新

并将窗口事件数据设置到tag.wm(tag为画布名称)

Tag.wm的值是一个以空格开头的字符串，以点为参数分割符，当同时有多个事件触发时将会以空格为分割添加第二个事件数据

事件列表


| 事件名称        | 事件格式        | 例子            | 意思            |

| WM\_KEYDOWN     | 事件名.按键名.虚拟键码 | WM\_KEYDOWN.VK\ | 键盘按键按下 |
|                 |                 | _A.65           |                 |

| WM\_KEYUP       |                 | WM\_KEYUP.VK\_A | 键盘按键松开    |
|                 |                 | .65             |                 |

| WM\_LBUTTONDOWN | 事件名.按键名.按键时的X轴位 | WM\_LBUTTONDOWN | 鼠标左键按下 |
|                 | 置.按键时的Y轴位置.图元名称 | .MK\_LBUTTON.20 |     |
|                 |                 | .20.doc         |                 |

| WM\_LBUTTONUP   |                 | WM\_LBUTTONUP.M | 鼠标左键松开    |
|                 |                 | K\_LBUTTON.20.2 |                 |
|                 |                 | 0.doc           |                 |

| WM\_RBUTTONDOWN |                 | WM\_RBUTTONDOWN | 鼠标右键按下    |
|                 |                 | .MK\_RBUTTON.20 |                 |
|                 |                 | .20.doc         |                 |

| WM\_RBUTTONUP   |                 | WM\_RBUTTONUP.M | 鼠标右键松开    |
|                 |                 | K\_RBUTTON.20.2 |                 |
|                 |                 | 0.doc           |                 |

| WM\_MBUTTONDOWN |                 | WM\_MBUTTONDOWN | 鼠标中键按下    |
|                 |                 | .MK\_MBUTTON.20 |                 |
|                 |                 | .20.doc         |                 |

| WM\_MBUTTONUP   |                 | WM\_MBUTTONUP.M | 鼠标中键松开    |
|                 |                 | K\_MBUTTON.20.2 |                 |
|                 |                 | 0.doc           |                 |

| WM\_MOUSEWHEEL  | 事件名.虚拟键名.位移.鼠标相 | WM\_MOUSEWHEEL. | 鼠标滚轮滚动 |
|                 | 对屏幕的X轴位置. | MK\_SHIF       |                 |
|                 |                 |                 |                 |
|                 | 鼠标相对屏幕的Y轴位置 | T.120.300.231 |             |

| WM\_MOUSEMOVE   | 事件名.X.Y.图元名称 | WM\_MOUSEMOVE.2 | 鼠标移动    |
|                 |                 | .2.doc          |                 |

| WM\_DESTROY     | 无              | WM\_DESTROY     | 窗口销毁        |


### 3.2.24 hide

用法:hide \<tag\>

隐藏名为tag的窗口

### 3.2.25 pen

用法:pen \<R\> \<G\> \<B\> \<WIDTH\>

设置画笔

### 3.2.26 brush

用法: brush \<R\> \<G\> \<B\>

设置刷子颜色

### 3.2.27 line

用法:line \<起点x\> \<起点y\> \<终点X\> \<终点Y\>

画线(画笔用pen命令设置)

### 3.2.28 ellipse

用法: ellipse \<左上角X\> \<左上角Y\> \<右下角X\> \<右下角Y\>

画椭圆

### 3.2.29 polygon

用法: polygon \<x1,y1\> \<x2,y2\> \[x3,y3.............

画多边形

### 3.2.30 picatom

用法: picatom \<tag\> \<x\> \<y\>

获取画布tag 点x y处的图元名称

### 3.2.31 cmd

用法:cmd \<command\>

执行cmd命令(感觉是个鸡肋)

### 3.2.32 thread

用法: thread \<file\>

在一个进程内执行多个脚本(多线程)

3.3 特性
--------

在命令前加\[Tag\]可以修改本条命令的绘图目标

如

\[cmd\] draw bin.bmp 0 0

3.4 特殊画布
------------

  cmd       CMD窗口
  --------- ---------
  desktop   桌面

4.  基本教程
    --------

    1.  基础
        ----

**Image的命令由修改版cmd.exe和image.dll解释，所以在bat的第一行添加代码，把bat交给修改版cmd.exe解释。代码的内容为:**

4.2 显示一张图片
----------------

[**[显示一张图片]{.underline}**](Demo/显示图片/显示图片.bat)

4.3 绘制文字
------------

[**[绘制文字]{.underline}**](Demo/绘制文字/绘制文字.bat)

[^1]: 图元：方便判断碰撞箱的的一种介质
