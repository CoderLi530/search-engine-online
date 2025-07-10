.PHONY:all
all:parser debug http_server

parser:parser.cpp
	g++ -o $@ $^ -lboost_system -lboost_filesystem -std=c++11

debug:debug.cpp
	g++ -o $@ $^ -ljsoncpp -std=c++11

# 使用http服务要链接线程库
http_server:http_server.cpp
	g++ -o $@ $^ -ljsoncpp -lpthread -std=c++11
.PHONY:clean
clean:
	rm -f parser debug http_server