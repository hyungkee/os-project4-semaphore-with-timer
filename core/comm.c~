/********************************************************
 * Filename: core/comm.c
 *  
 * Author: jtlim, RTOSLab. SNU.
 * 
 * Description: message queue management. 
 ********************************************************/
#include <core/eos.h>

void eos_init_mqueue(eos_mqueue_t *mq, void *queue_start, int16u_t queue_size, int8u_t msg_size, int8u_t queue_type) {
	/* initialization */
	mq->queue_start = queue_start;
	mq->front = 0;
	mq->rear = 0;
	mq->queue_size = queue_size;
	mq->msg_size = msg_size;
	mq->queue_type = queue_type;

	// set semaphore count of putsem, getsem.
	eos_init_semaphore(&(mq->putsem), queue_size, queue_type); // can write (queue_size)-repeat
	eos_init_semaphore(&(mq->getsem), 0, queue_type); // can't read in first
}

void * my_memcpy(void * dst, const void * src, size_t len)
{
	char * pDst = (char *) dst;
	char const * pSrc = (char const *) src;

	while (len--)
	{
		*pDst++ = *pSrc++;
	} 
	return (dst);
}

int8u_t eos_send_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
	// need put semaphore
	if(eos_acquire_semaphore(&(mq->putsem), timeout)){
		// copy message to queue element
		my_memcpy(mq->queue_start + mq->msg_size * mq->rear, message, mq->msg_size);
		mq->rear++; // it means enqueue

		// circular position revision
		if(mq->rear == mq->queue_size)
			mq->rear = 0;

		// provide get semaphore. (someone can read messages from queue.)
		eos_release_semaphore(&(mq->getsem));
		return 1;
	}
	return 0;
}

int8u_t eos_receive_message(eos_mqueue_t *mq, void *message, int32s_t timeout) {
	// need get semaphore
	if(eos_acquire_semaphore(&(mq->getsem), timeout)){
		// copy queue element value to message space
		my_memcpy(message, mq->queue_start + mq->msg_size * mq->front, mq->msg_size);
		mq->front++; // it means dequeue

		// circular position revision
		if(mq->front == mq->queue_size)
			mq->front = 0;

		// provide put semaphore. (someone can write messages to queue.)
		eos_release_semaphore(&(mq->putsem));
		return 1;
	}
	return 0;
}


