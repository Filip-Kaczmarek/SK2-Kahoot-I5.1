OBJS = ./out/main.o ./out/DataEncoder.o ./out/HeadData.o ./out/DataProcesser.o
CXXFLAGS+= -std=c++11 -g -Wall
main: $(OBJS)
	g++ $(CXXFLAGS) $(OBJS) -o main 
	
./out/main.o: main.cpp ./ProtocolHead/HeadData.h ./Service/DataProcesser.h ./config/server_config.h
	g++ $(CXXFLAGS) -c main.cpp -o ./out/main.o
./out/DataProcesser.o: ./Service/DataProcesser.cpp ./ProtocolHead/protocolmsg.h ./ProtocolHead/HeadData.h ./ProtocolHead/DataEncoder.h 
	g++ $(CXXFLAGS) -c ./Service/DataProcesser.cpp -o ./out/DataProcesser.o
./out/HeadData.o: ./ProtocolHead/HeadData.cpp ./ProtocolHead/protocolmsg.h
	g++ $(CXXFLAGS) -c ./ProtocolHead/HeadData.cpp -o ./out/HeadData.o
./out/DataEncoder.o: ./ProtocolHead/DataEncoder.cpp ./ProtocolHead/protocolmsg.h
	g++ $(CXXFLAGS) -c ./ProtocolHead/DataEncoder.cpp -o ./out/DataEncoder.o
clean:
	rm -rf ./out/*.o main



