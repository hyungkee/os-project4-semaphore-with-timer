/********************************************************
 * Filename: core/sync.c
 * 
 * Author: wsyoo, RTOSLab. SNU.
 * 
 * Description: semaphore, condition variable management.
 ********************************************************/
#include <core/eos.h>

#define READY		1
#define RUNNING		2
#define WAITING		3
#define SEMAPHORE	4

void eos_init_semaphore(eos_semaphore_t *sem, int32u_t initial_count, int8u_t queue_type) {
	/* initialization */
	sem->count = initial_count;
	sem->queue_type = queue_type;
	sem->wait_queue = NULL;
}

int32u_t eos_acquire_semaphore(eos_semaphore_t *sem, int32s_t timeout) {
	while(1){
		if(sem->count > 0){
			// provide one semaphore
			sem->count--;
			return 1;
		}else{
			// can't provide semaphore
			if(timeout == -1){
				// it's non blocking method.
				return 0;

			}else if(timeout > 0){
				// it's blocking method. wait (timeout)-tick.
				eos_sleep(timeout);
				// recheck sem count. (implemented in while loop)

			}else if(timeout == 0){
				// get current task.
				eos_tcb_t* task = eos_get_current_task();

				// insert current task to wait queue in semaphore
				if(sem->queue_type == FIFO)
					_os_add_node_tail(&(sem->wait_queue), &(task->rq_node));
				if(sem->queue_type == PRIORITY){
					task->rq_node.priority = task->priority;
					_os_add_node_priority(&(sem->wait_queue), &(task->rq_node));
				}

				// change current task state for notifying what don't have to insert current task to ready queue.
				task->state = SEMAPHORE;

				// run other task
				eos_schedule();

				// when it come back, retry check sem->count
			}
		}
	}
}


void eos_release_semaphore(eos_semaphore_t *sem) {
	// return one semaphore
	sem->count++;
	{
		_os_node_t *wait_node = sem->wait_queue;
		if(wait_node != NULL){
			// if waiting task exist in semaphore, move this to ready queue.
			_os_remove_node(&(sem->wait_queue), wait_node);

			eos_tcb_t *task = (eos_tcb_t*)wait_node->ptr_data;
			task->state = READY;

			// insert waiting task to ready queue.
			reposition_task(task);
	//		eos_schedule();
		}
	}
}

void eos_init_condition(eos_condition_t *cond, int32u_t queue_type) {
	/* initialization */
	cond->wait_queue = NULL;
	cond->queue_type = queue_type;
}

void eos_wait_condition(eos_condition_t *cond, eos_semaphore_t *mutex) {
	/* release acquired semaphore */
	eos_release_semaphore(mutex);
	/* wait on condition's wait_queue */
	_os_wait(&cond->wait_queue);
	/* acquire semaphore before return */
	eos_acquire_semaphore(mutex, 0);
}

void eos_notify_condition(eos_condition_t *cond) {
	/* select a task that is waiting on this wait_queue */
	_os_wakeup_single(&cond->wait_queue, cond->queue_type);
}
