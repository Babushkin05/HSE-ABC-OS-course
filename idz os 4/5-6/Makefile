CC = gcc
CFLAGS = -Wall -O2
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
	@echo "Starting logger client on port $(LOGGER_PORT)..."
	./logger_client $(LOGGER_PORT) & \
	LOGGER_PID=$$! ; \
	sleep 1 ; \
	echo "Starting server on port $(PORT) (expecting $(NUM_CLIENTS) proposals) with logger..." ; \
	./server $(PORT) $(NUM_CLIENTS) 127.0.0.1 $(LOGGER_PORT) & \
	SERVER_PID=$$! ; \
	sleep 1 ; \
	echo "Launching clients..." ; \
	./client 127.0.0.1 $(PORT) "Dinner on the Eiffel Tower" & \
	./client 127.0.0.1 $(PORT) "Walk under the stars" & \
	./client 127.0.0.1 $(PORT) "Weekend in Venice" ; \
	wait $$SERVER_PID ; \
	kill $$LOGGER_PID


clean:
	rm -f $(TARGETS)
