TARGET=bin/spikte_chat

SRCS=$(shell find src -name "*.cpp") # I am, indeed, a bit lazy
OBJS=$(patsubst src/%.cpp,obj/%.o,$(SRCS))

LIBS=lib/raygui.c lib/socket_utils.c
OBJS_LIBS=$(patsubst lib/%.c,obj/%.o,$(LIBS))

INCLUDES= -Ilib -Iinclude
EXLIBS= -lraylib -lm -ldl -lpthread -lGL -lrt -lX11 -lsqlite3 -lssl -lcrypto
FLAGS= -Wall -Wpedantic -Wshadow -Wsign-conversion -Wnull-dereference -Wformat=2 -Wundef


$(TARGET): build $(OBJS) $(OBJS_LIBS)
	g++ $(OBJS) $(OBJS_LIBS) -o $(TARGET) $(EXLIBS) $(INCLUDES) 


$(OBJS): obj/%.o: src/%.cpp
	g++ -c $< -o $@ $(FLAGS) -g

$(OBJS_LIBS): obj/%.o: lib/%.c
	g++ -c $< -o $@ $(FLAGS) -g

build:
	mkdir -p obj/utils
	mkdir -p obj/gui/chat_theme
	mkdir -p obj/core
	mkdir -p bin

debug: $(SRCS) $(LIBS)
	g++ $(SRCS) $(LIBS) -o bin/debug -g $(EXLIBS) $(INCLUDES) $(FLAGS)

clean:
	rm -vf ./bin/*

fclean: clean
	rm -vf ./obj/*.o
	rm -vf ./obj/utils/*.o
	rm -vf ./obj/gui/chat_theme/*.o
	rm -vf ./obj/gui/*.o
	rm -vf ./obj/core/*.o

re: fclean $(TARGET)
.PHONY: clean fclean re debug build


pkey.pem:
	openssl genpkey -algorithm rsa -out pkey.pem -pkeyopt rsa_keygen_bits:2048
chain.pem: pkey.pem
	openssl req -x509 -new -key pkey.pem -days 36500 -subj '/CN=localhost' -out chain.pem
