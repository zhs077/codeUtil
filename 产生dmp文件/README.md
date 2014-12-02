Release模式下需要修改一些选项才能产生dmp文件。
如果是lib或exe修改4处地方：

1. 工程 à properties à C/C++ àGeneral à Debug Information Format       选择 “Program Database for Edit & Continue (/ZI)”可使release下可调式

2. 工程 à properties à C/C++ àOptimization àOptimization                选择 “Disabled (/Od)”

3. 工程 à properties à C/C++ àOptimization àWhole Program Optimization 选择 “No”（1，2，3与生成DUMP文件有关）

4. 工程 à properties à C/C++ àOutput Files àAssembler Output            选择 “Assembly, Machine Code and Source (/FAcs)”此处生成cod文件（包含汇编的所有code）

如果是dll除了修改上面4处，还要修改Linker处

5. 工程 à properties à Linker àDebugging àGenerate Map File            选择 “Yes (/MAP)”此处生成map文件（所有函数的入口内存地址 当崩溃时会有崩溃地址可以据此查询）


