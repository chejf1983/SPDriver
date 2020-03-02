#原文件wildcard是扩展通配符
SOURCES = $(wildcard ./*.cpp)
#输出目录
BINDIR = ./bin

#编译标识 make ARM=1就可以触发
ifdef ARM  
CC = arm-linux-g++  
TARGET = GSCDemo_arm
LIB_DIR = ./lib_arm
LIBUSBDIR = /usr/local/arm/arm/lib/
else
CC = g++	
TARGET = GSCDemo
LIB_DIR = ./lib_x86
LIBUSBDIR = /usr/local/lib/
endif

#头文件
INCLUDE = -I $(LIB_DIR)
#需要的lib库 
LIB = -L. $(LIB_DIR)/SpectraArsenal.a \
      -L $(LIB_DIR) -lusb-1.0
 
#1. 创建输出目录
#2. 运行编译
$(TARGET) : $(SOURCES)
	mkdir -p $(BINDIR);
	$(CC) -o $(TARGET) $(SOURCES) $(INCLUDE) $(LIB)
	cp $@ $(BINDIR)
	rm $(TARGET)
	
.PHONY:clean
clean: 
	rm -rf $(BINDIR)