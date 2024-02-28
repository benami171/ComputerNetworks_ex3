CC = gcc
CFLAGS = -g -Wall -Wextra


all: TCP_Receiver TCP_Sender

TCP_Receiver: TCP_Receiver.c
	$(CC) $(CFLAGS) -o TCP_Receiver TCP_Receiver.c

TCP_Sender: TCP_Sender.c
	$(CC) $(CFLAGS) -o TCP_Sender TCP_Sender.c

clean:
	rm -f TCP_Receiver TCP_Sender