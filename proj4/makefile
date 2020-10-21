LIB=-lpthread
CCPP=g++
CFLAGS = -g -Wall

all : proj4 doit
proj4 : proj4.cpp
	$(CCPP) proj4.cpp -o proj4 $(LIB)
doit : doit.cpp
	$(CCPP) doit.cpp -o doit $(LIB)
clean :
	rm -f doit
	rm -f proj4
