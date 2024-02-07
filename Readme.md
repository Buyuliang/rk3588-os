ROC-rk3588-PC 官网主页：https://www.t-firefly.com/doc/download/107.html

先从官网下载以下文件：
AndroidTool烧写工具	RKDevTool_Release_v2.84.zip
RK驱动助手		DriverAssitant_v5.1.1.zip
MiniLoaderAll.bin	MiniLoaderAll.bin




一). win10 wsl 安装(win10 里面的 linux 子系统)
===================================================================================
参考ms官方文档：https://docs.microsoft.com/zh-cn/windows/wsl/install

安装好以后， 开始 -> 运行: bash
windows里面的磁盘，都被wsl挂载在 /mnt/ 目录下

二). GCC安装(官方推荐的 gcc 版本):
===================================================================================
https://releases.linaro.org/components/toolchain/binaries/6.3-2017.05/aarch64-linux-gnu/
https://releases.linaro.org/components/toolchain/binaries/6.3-2017.05/aarch64-linux-gnu/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu.tar.xz
https://download.csdn.net/download/u012587637/12845947?utm_source=iteye_new        官网页面下载太慢了，自己百度下载一个。

tar -zxvf gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu.tar.gz

三). 编译裸机代码
===================================================================================
./make.sh

编译成功会得到：
rk3588_loader.bin

rk3588_loader.bin 是RKDevTool能识别到loader，是可以烧入到板子上的最终文件
是 make.sh 里面通过 ./RKLoaderTools/boot_merger RKLoader.ini 生成的
由 tpl.bin 裸机代码和两个临时生成的fake*.bin文件混合而成。

四). 烧入并启动裸机代码
===================================================================================
1). 将板子启动到MASKROM模式，让 RKDevTool 识别到。

2). 切换目标存储：
	RKDevTool 切换到高级功能页， 在此页面下载 MiniLoaderAll.bin 到内存运行，
	下载完不需要重启，->
	点击此页面的 "读取存储列表" ->
	在右边列表中选择 Emmc，->
	再点击 "切换存储"，->
	就可以切换要烧写的目标存储为eMMC,

3). 下载裸机代码到板子上
	RKDevTool 切换到下载镜像页， 将loader项设置为：
	地址：0x00000000
	路径：E:\Dev\EE\Rockchip\rk3588tpl_1Mini\rk3588_loader.bin
	再打上 loader 项前面的勾，其它的都不用勾

	最后点击 "执行"，即可以将 rk3588_loader.bin 烧入板子上的 eMMC 存储内。

烧完重启板子，裸机代码就会被运行了。




五). 代码说明
===================================================================================
rk3588tpl_1Mini
	不设置运行栈，没用 bootrom 环境的栈
	运行后只等待一会， 然后又退回bootrom运行，即让板子又重新回到MASKROM模式

rk3588tpl_2Uart
	不设置运行栈，沿用 bootrom 环境的栈
	运行后会先初始化 uart2 串口，然后运行一个 mini shell 供交互用
	支持 help、exit、hexDump 三条指令
