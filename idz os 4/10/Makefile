CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGETS = server client logger_client
PORT = 8080
LOGGER_PORT = 9000
NUM_CLIENTS = 3

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

logger_client: logger_client.c
	$(CC) $(CFLAGS) -o logger_client logger_client.c

test: all
	@./logger_client $(LOGGER_PORT) & \
	LOGGER_PID=$$!; \
	sleep 1; \
	./server $(PORT) $(NUM_CLIENTS) 127.0.0.1 $(LOGGER_PORT) & \
	SERVER_PID=$$!; \
	sleep 1; \
	./client 127.0.0.1 $(PORT) "Proposal 1" & \
	./client 127.0.0.1 $(PORT) "Proposal 2" & \
	./client 127.0.0.1 $(PORT) "Proposal 3"; \
	wait $$SERVER_PID; \
	kill $$LOGGER_PID

test_sigint: all
	@echo "Testing SIGINT handling..."
	@./logger_client $(LOGGER_PORT) & \
	LOGGER_PID=$$!; \
	sleep 1; \
	./server $(PORT) $(NUM_CLIENTS) 127.0.0.1 $(LOGGER_PORT) & \
	SERVER_PID=$$!; \
	sleep 1; \
	./client 127.0.0.1 $(PORT) "Test 1" & \
	./client 127.0.0.1 $(PORT) "Test 2" & \
	sleep 1; \
	kill -INT $$SERVER_PID; \
	wait $$SERVER_PID; \
	kill $$LOGGER_PID

clean:
	rm -f $(TARGETS)

.PHONY: all clean test test_sigint