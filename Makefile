BUILD=g++ -std=c++11 -pipe -O2 -Wall $(INCLUDE)
CC=g++ -std=c++11 -pipe -O2 -c -Wall $(INCLUDE)
LD = -pthread
TARGET = -o $(BIN)example

TEMP=./temp_file/
BIN=./bin/

INCLUDE= -I./SUSU_ThreadPool

susu_httpd=susu_httpd.cpp
susu_thread=./SUSU_ThreadPool/*.cpp
susu_log=./SUSU_LogPrinter/*.cpp
susu_epoll_object=./SUSU_EpollObject/*.cpp
susu_http_object=./SUSU_HttpObject/*.cpp
susu_init_param=./SUSU_InitParam/*.cpp
susu_timer=./SUSU_Timer/*.cpp

susu_test=./test/*.cpp

files=$(TEMP)susu_httpd.o $(susu_thread) $(TEMP)log_printer.o $(TEMP)epoll_object.o $(TEMP)http_object.o $(TEMP)init_param.o $(TEMP)timer.o

example:susu_httpd.o log_printer.o epoll_object.o http_object.o init_param.o timer.o
	$(BUILD) $(TARGET) $(LD) $(files)

susu_httpd.o:susu_httpd.cpp
	$(CC) $(susu_httpd) -o $(TEMP)susu_httpd.o

log_printer.o:$(susu_log)
	$(CC) $(susu_log) -o $(TEMP)log_printer.o

epoll_object.o:$(susu_epoll_object)
	$(CC) $(susu_epoll_object) -o $(TEMP)epoll_object.o

http_object.o:$(susu_http_object)
	$(CC) $(susu_http_object) -o $(TEMP)http_object.o

init_param.o:$(susu_init_param)
	$(CC) $(susu_init_param) -o $(TEMP)init_param.o

timer.o:$(susu_timer)
	$(CC) $(susu_timer) -o $(TEMP)timer.o

test:$(susu_test) init_param.o log_printer.o
	$(BUILD) -g ./test/test.cpp $(TEMP)init_param.o $(TEMP)log_printer.o $(TEMP)timer.o -o ./test/test.out  $(LD) $(susu_thread)

.PHONY:clean
clean:
	rm ./bin/*
	rm *temp_file/*
