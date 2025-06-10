CC = gcc
CFLAGS = -Wall -O2
TARGETS = server client logger_client
PORT = 8080
LOGGER_PORTS = 9000 9001 9002
NUM_CLIENTS = 3
SERVER_IP = 127.0.0.1

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

logger_client: logger_client.c
	$(CC) $(CFLAGS) -o logger_client logger_client.c

test: all
	@echo "Starting logger clients..."
	@for port in $(LOGGER_PORTS); do \
		./logger_client $$port $(SERVER_IP) $(PORT) & \
		echo "Logger started on port $$port (PID $$!)"; \
		LOGGER_PIDS="$$LOGGER_PIDS $$!"; \
	done; \
	sleep 1; \
	echo "Starting server with $(words $(LOGGER_PORTS)) loggers..."; \
	./server $(PORT) $(NUM_CLIENTS) $(foreach port,$(LOGGER_PORTS),$(SERVER_IP) $(port)) & \
	SERVER_PID=$$!; \
	sleep 1; \
	echo "Launching $(NUM_CLIENTS) clients..."; \
	./client $(SERVER_IP) $(PORT) "Romantic dinner" & \
	./client $(SERVER_IP) $(PORT) "Weekend getaway" & \
	./client $(SERVER_IP) $(PORT) "Movie night"; \
	wait $$SERVER_PID; \
	echo "Stopping loggers..."; \
	kill $$LOGGER_PIDS

clean:
	rm -f $(TARGETS)

.PHONY: all clean test_multi