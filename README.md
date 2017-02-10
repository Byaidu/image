# image
控制台显示图片
Ver 2.0 By Byaidu

image [/d] [/s] [/l listfile] [bmpfile] [X] [Y] [Width] [Height] [/TRANSPARENTBLT] [/SRCAND] [/SRCCOPY]

        /d      清空当前窗口中显示的图片
        /s      清空当前窗口中指定矩形区域中显示的图片
        /l listfile     使用指定文件批量处理
        bmpfile 指定要显示的图片
        X       指定矩形区域左上角的X轴逻辑坐标
        Y       指定矩形区域左上角的Y轴逻辑坐标
        Width   为/s选项指定矩形区域的逻辑宽度。
        Height  为/s选项指定矩形区域的逻辑高度。
        /TRANSPARENTBLT 表示把图片中的RGB值为255,255,255当作透明颜色拷贝到当前窗口
        /SRCAND 通过使用与操作符来将图片和当前窗口中的颜色合并
        /SRCCOPY        将图片直接拷贝到目标矩形区域

若直接使用image abc.bmp 10 50 不加后面的参数，默认使用/SRCCOPY
注意，X,Y所构成的坐标由0起算
