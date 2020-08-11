socket-recycling: socket-recycling.cpp 
	g++ -std=c++11 socket-recycling.cpp -o socket-recycling -lpthread -lboost_system -lboost_thread 
