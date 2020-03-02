#编译目标
TARGET = SpectraArsenal.a
#编译目标输出头文件
TARGETH = $(patsubst %.a,%.h,$(TARGET))
#原文件wildcard是扩展通配符
SOURCES = $(wildcard ./*.cpp)
#OBJ文件，patsubst替换通配符，就是将SOURCES里的.cpp名称替换成.o
OBJ = $(patsubst %.cpp,%.o,$(SOURCES))
#fpic标志，告诉编译器产生与位置无关代码(Position-Independent Code)，产生的代码中，
#没有绝对地址，全部使用相对地址，故而代码可以被加载到内存的任意位置执行。
CFLAGS = -fPIC
#输出目录
OUTPUT = ./lib

#编译标识 make ARM=1就可以触发
ifdef ARM  
#ARM g++编译工具  
CC = arm-linux-g++  
#libusb 头文件
INCLUDE = -I /usr/local/arm/arm/include/libusb-1.0/ 
#libusb 库头文件（arm格式下的库文件）		  
LIBUSBDIR = /usr/local/arm/arm/lib/
else#x86 g++编译工具
CC = g++	
INCLUDE = -I /usr/local/include/libusb-1.0/   
LIBUSBDIR = /usr/local/lib/
endif

#将所有的obj压缩到静态链接库 $@--目标文件，$^--所有的依赖文件，$<--第一个依赖文件
#创建输出lib目录
#生成.a文件
#复制.a和头文件到输出lib目录
#复制头文件到输出lib目录
#复制libusb.a文件到输出lib目录
$(TARGET) : $(OBJ)
	mkdir -p $(OUTPUT)
	ar crv $@ $^ 
	cp $@ $(OUTPUT)
	cp $(TARGETH) $(OUTPUT)
	cp $(LIBUSBDIR)*.* $(OUTPUT)

#将当前目录下所有的.cpp文件生成对应的.o文件
$(OBJ) :%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE)

#make clean 命令.PHONY是一个虚项目名称
.PHONY:clean
clean: 
	rm $(TARGET) $(OBJ) 
	rm -rf $(OUTPUT)