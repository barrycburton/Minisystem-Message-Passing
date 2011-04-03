/*
 * Bounded buffer via message passing.
 *
 * Sample program that implements a single producer-single consumer
 * system using message passing--the MP system represents the buffer.
 * 
 * Change MAXCOUNT to vary the number of items produced by the producer.
 */

#include "defs.h"
#include "minithread.h"
#include "minimsg.h"

#define INTER_PROCESS 0

#define BUFFER_SIZE 10

#define MAXCOUNT 100

minimsg_port_t consume;
minimsg_port_t produce;
int pcount;

int consumer(arg_t arg) {
	int get_num, i, size;
	int count = 0, num;
	char rcv_mem[128];
	minimsg_port_t other;
	minimsg_port_t sys_consume;
	
	consume = minimsg_port_create();

	printf("Consumer thread started (id: %d).\n", minithread_id());

	while (count < MAXCOUNT) {
		get_num = rand() % BUFFER_SIZE + 1;
		get_num = (get_num <= MAXCOUNT - count) ? get_num : MAXCOUNT - count;

		printf("Consumer wants to get %d items out of buffer ...\n", get_num);
		
		for (i=0; i<get_num; i++) {
			size = 128;
			minimsg_receive(consume, rcv_mem, &size, NULL, NULL);
			if ( size == sizeof(int) ) {
				num = *((int*)rcv_mem);
				count++;
				printf("Consumer is taking %d out of buffer.\n", num);
				minimsg_send(consume, produce, sizeof(int), rcv_mem, 0);
			}
		}
	}
	/* send an extra message to see if it gets cleaned up */
	minimsg_send(consume, produce, sizeof(int), rcv_mem, 0);

	printf("%d items Consumed.\n", count);

	if ( INTER_PROCESS ) {
		minithread_yield();

		printf("Now Trying To Contact Another Process.\n");
		sys_consume = minimsg_system_port();
		*((int*)rcv_mem) = 1;
		minimsg_send(sys_consume, MINIMSG_SYSTEM_PORT_BCAST_ID, sizeof(int), rcv_mem, 0);
		size = 128;
		minimsg_receive(sys_consume, rcv_mem, &size, &other, NULL);

		if ( sys_consume >= other ) {
			count = 0;

			size = 128;
			minimsg_receive(consume, rcv_mem, &size, NULL, NULL);

			while (count < MAXCOUNT) {
				get_num = rand() % BUFFER_SIZE + 1;
				get_num = (get_num <= MAXCOUNT - count) ? get_num : MAXCOUNT - count;

				printf("Consumer wants to get %d items out of buffer ...\n", get_num);
				
				for (i=0; i<get_num; i++) {
					size = 128;
					minimsg_receive(sys_consume, rcv_mem, &size, NULL, NULL);
					if ( size == sizeof(int) ) {
						num = *((int*)rcv_mem);
						count++;
						printf("Consumer is taking %d out of buffer.\n", num);
						minimsg_send(sys_consume, other, sizeof(int), rcv_mem, 0);
					}
				}
			}

			printf("%d items Consumed.\n", count);
		} else {
			minimsg_send(consume, produce, sizeof(int), rcv_mem, 0);
		}
	}

	minimsg_port_destroy(consume);

	return 0;
}

int producer(arg_t arg) {
	int put_num, i, in_buff = 0;
	int size;
	char send_mem[128];
	minimsg_port_t other;
	minimsg_port_t sys_produce;

	pcount = 0;
	
	produce = minimsg_port_create();

	printf("Producer thread started (id: %d).\n", minithread_id());

	minithread_fork(consumer, NULL);

	minithread_yield();

	while (pcount < MAXCOUNT) {
		put_num = rand() % BUFFER_SIZE + 1;
		put_num = (put_num <= MAXCOUNT - pcount) ? put_num : MAXCOUNT - pcount;

		printf("Producer wants to put %d items into buffer ...\n", put_num);
		
		for (i=0; i<put_num; i++) {
			pcount++;
			*((int*)send_mem) = pcount;
			minimsg_send(produce, consume, sizeof(int), send_mem, 0);
			printf("Producer is putting %d into buffer.\n", pcount);
			in_buff++;
			if ( in_buff == BUFFER_SIZE ) {
				while ( in_buff > 0 ) {
					size = 128;
					minimsg_receive(produce, send_mem, &size, NULL, NULL);
					if ( size == sizeof(int) ) {
						in_buff--;
					}
				}
			}
		}
	}
	
	while ( in_buff > 0 ) {
		size = 128;
		minimsg_receive(produce, send_mem, &size, NULL, NULL);
		if ( size == sizeof(int) ) {
			in_buff--;
		}
	}

	printf("%d items Produced.\n", pcount);

	if ( INTER_PROCESS ) {
		minithread_yield();

		// now try with a second process
		sys_produce = minimsg_system_port();
		*((int*)send_mem) = 1;
		minimsg_send(sys_produce, MINIMSG_SYSTEM_PORT_BCAST_ID, sizeof(int), send_mem, 0);
		size = 128;
		minimsg_receive(sys_produce, send_mem, &size, &other, NULL);

		if ( sys_produce < other ) {
			pcount = 0;

			size = 128;
			minimsg_receive(produce, send_mem, &size, NULL, NULL);

			while (pcount < MAXCOUNT) {
				put_num = rand() % BUFFER_SIZE + 1;
				put_num = (put_num <= MAXCOUNT - pcount) ? put_num : MAXCOUNT - pcount;

				printf("Producer wants to put %d items into buffer ...\n", put_num);
				
				for (i=0; i<put_num; i++) {
					pcount++;
					*((int*)send_mem) = pcount;
					minimsg_send(sys_produce, other, sizeof(int), send_mem, 0);
					printf("Producer is putting %d into buffer.\n", pcount, i, put_num);
					in_buff++;
					if ( in_buff == BUFFER_SIZE ) {
						size = 128;
						minimsg_receive(sys_produce, send_mem, &size, NULL, NULL);
						if ( size == sizeof(int) ) {
							in_buff--;
						}
					}
				}
			}

			while ( in_buff > 0 ) {
				size = 128;
				minimsg_receive(sys_produce, send_mem, &size, NULL, NULL);
				if ( size == sizeof(int) ) {
					in_buff--;
				}
			}

			printf("%d items Produced.\n", pcount);
		} else {
			minimsg_send(produce, consume, sizeof(int), send_mem, 0);
		}
	}

	minimsg_port_destroy(produce);

	return 0;
}


void main(void) {
	printf("app_mp_buffer begins.\n");

	minithread_system_initialize(producer, NULL);

	dbgprintf("Memory Leaks (If Any) Follow:\n");
	_CrtDumpMemoryLeaks();
	system("pause");
}
