#输出目录
OUTDIR = ./out
#输出lib目录
LIBX86 = $(OUTDIR)/lib_x86
LIBARM = $(OUTDIR)/lib_arm
#执行目录
LIBSRC = ./src/lib
#测试源代码目录
TESTSRC = ./src/test
 
#for dir in $(SRCDIR);\
#do $(MAKE) -C $$dir ;\
#done	 
#for dir in $(SRCDIR);\
#do $(MAKE) -C $$dir clean||exit 1;\
#done
#1.创建输出目录
#2.复制test代码到输出目录
#3.创建lib目录
#4.运行子目录的makefile
run: prepare buildx86 build_arm


#复制测试代码，创建lib目录
prepare:
	mkdir -p $(OUTDIR);
	mkdir -p $(LIBX86);
	mkdir -p $(LIBARM);
	cp $(TESTSRC)/*.* $(OUTDIR)
	cp $(TESTSRC)/Makefile $(OUTDIR)

#编译x86的lib复制到x86的文件夹下	
buildx86:
	+$(MAKE) -C $(LIBSRC)
	cp $(LIBSRC)/lib/* $(LIBX86)
	+$(MAKE) -C $(LIBSRC) clean
	+$(MAKE) -C $(OUTDIR)

#编译x86的lib复制到x86的文件夹下	
build_arm:
	+$(MAKE) -C $(LIBSRC) ARM=1
	cp $(LIBSRC)/lib/* $(LIBARM)
	+$(MAKE) -C $(LIBSRC) clean	
	+$(MAKE) -C $(OUTDIR) ARM=1
			
#1.运行子目录make clean
#2.删除输出目录
.PHONY:clean
clean: 
	rm -rf $(OUTDIR)
