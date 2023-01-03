CC=gcc
CPP=g++
CFLAGS=-g
CPPFLAGS=-g -std=c++11
DEPS =
SERVEROBJ = maledict.o simplerand.o
SERVER = maledict

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.cpp $(DEPS)
	$(CPP) -c -o $@ $< $(CPPFLAGS)

default: $(SERVER)

$(SERVER): $(SERVEROBJ)
	$(CC) -o $(SERVER) $(SERVEROBJ) $(CFLAGS)

clean:
	rm -f *.o $(TARGET) test/out

run: default
	./$(TARGET)

test-connect-1: $(SERVER)
	@echo "Testing connecting to 1313..."
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 1313 1"
	@echo "Success!"

test-connect-all: $(SERVER)
	@echo "Testing connections to all ports..."
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 1313 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 1344 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 4413 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 4444 1"
	@echo "Success!"

test-ping: $(SERVER)
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 1313 0"

test-single: $(SERVER)
	@echo "Testing GET_SINGLE..."
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 1313 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 1344 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 4413 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 4444 1"
	@echo "Success!"

test-multi: $(SERVER)
	@echo "Testing GET_MULTI..."
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 1313 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 1344 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 4413 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 4444 1"
	@echo "Success!"

test-seed: $(SERVER)
	@echo "Testing a different seed..."
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 1313 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 1344 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 4413 1"
	@python3 test/runserverclient.py "./maledict > /dev/null" "python3 test/imprecate.py localhost 4444 1"
	@echo "Success!"

test-error: $(SERVER)
	@echo "Testing error code..."
	@python3 test/runserverclient.py "./maledict -s 5 > /dev/null" "python3 test/imprecate.py localhost 4413 13" > test/out
	@echo "Comparing outputs..."
	@cmp test/out test/output-error
	@echo "Success!" 

test-memory: $(SERVER)
	@echo "Testing memory leaks..."
	@python3 test/runserverclient.py "valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 --errors-for-leak-kinds=definite,indirect,possible,reachable ./maledict" "python3 test/imprecate.py localhost 1313 1"
	@echo "Success!"

test-style: $(SERVER)
	@echo "Testing style.."
	@cpplint maledict.c
	@echo "Success!"

test-functional: test-connect-1 test-connect-all test-ping \
	test-single test-multi test-seed test-error

test:  test-functional test-memory test-style
