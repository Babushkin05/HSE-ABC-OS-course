CC = gcc
CFLAGS = -Wall -O2
TARGETS = server client
PORT = 8080
NUM_CLIENTS = 3

all: $(TARGETS)

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

test: all
	@echo "Starting server on port $(PORT) (expecting $(NUM_CLIENTS) proposals)..."
	./server $(PORT) $(NUM_CLIENTS) & \
	SERVER_PID=$$! ; \
	sleep 1 ; \
	echo "Launching clients..." ; \
	./client 127.0.0.1 $(PORT) "Dinner on the Eiffel Tower" & \
	./client 127.0.0.1 $(PORT) "Walk under the stars" & \
	./client 127.0.0.1 $(PORT) "Weekend in Venice" ; \
	wait $$SERVER_PID

clean:
	rm -f $(TARGETS)
