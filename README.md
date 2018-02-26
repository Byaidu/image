# image
控制台显示图片 Ver 3.0 by Byaidu

help			显示帮助

load file \[tag\]		申请一块画布tag，并加载图片到画布tag

unload tag		删除画布tag

save file tag		将画布tag的内容存储到file中

target tag		切换当前绘图目标为画布tag

buffer tag		申请一块画布tag

stretch tag w h		将画布tag缩放到w*h的大小

cls			清空画布cmd的内容

rotate tag degree	将画布tag顺时针旋转degree度

draw tag x y \[trans|and\]将画布tag绘制到当前绘图目标的x,y位置上

info tag		将画布tag的宽和高存储到变量image

export			将画布cmd的句柄存储到变量image

import handle tag	通过句柄将另一个控制台的画布cmd映射到此控制台的画布tag

getpix tag x y		将画布tag上x,y位置的rgb值存储到变量image

setpix tag x y r g b	设置画布tag上x,y位置的rgb值

![](https://images2018.cnblogs.com/blog/1123683/201802/1123683-20180223134856925-1663767861.jpg)
