#/opt/arm9260/bin/arm-none-linux-gnueabi-gcc -I ./xml/include/libxml2  -L ./xml/lib  -lxml2 -lm -lpthread *.c -oledPro
VPATH = Common:Extern_lib:FTP:Network:RS485:Time:XML
cc = arm-none-linux-gnueabi-gcc
override CFLAGS += $(patsubst %,-I%,$(subst :, ,$(VPATH))) -I ./Extern_lib/xml/include/libxml2/ -I Extern_lib/sqlite3/include
override LDFLAGS +=  -L ./Extern_lib/xml/lib  -lxml2 -lm -lpthread


ledPro_file = main.o common.o get_file_name.o link_list.o  semaphore.o ftp.o network.o protocol_parse.o rs485.o serial.o mytime.o xml_parse.o

ledPro:$(ledPro_file)
	$(cc)  $^ -o $@ $(CFLAGS)  $(LDFLAGS) 

main.o:main.c
	$(cc)  -c $^ -o $@ $(CFLAGS)
	
common.o:common.c
	$(cc)  -c $^ -o $@ $(CFLAGS)
	
get_file_name.o:get_file_name.c
	$(cc)  -c $^ -o $@ $(CFLAGS)
	
link_list.o:link_list.c
	$(cc)  -c $^ -o $@ $(CFLAGS)
	
semaphore.o:semaphore.c
	$(cc)  -c $^ -o $@ $(CFLAGS)
	
ftp.o:ftp.c
	$(cc)  -c $^ -o $@ $(CFLAGS)
	
network.o:network.c
	$(cc)  -c $^ -o $@ $(CFLAGS)
	
protocol_parse.o:protocol_parse.c
	$(cc)  -c $^ -o $@ $(CFLAGS)
	
rs485.o:rs485.c
	$(cc)  -c $^ -o $@ $(CFLAGS)
	
serial.o:serial.c
	$(cc)  -c $^ -o $@ $(CFLAGS)
	
mytime.o:mytime.c
	$(cc)  -c $^ -o $@ $(CFLAGS)
	
xml_parse.o:xml_parse.c
	$(cc)  -c $^ -o $@ $(CFLAGS)

clean:	
	rm -f *.o