#include "my_task.h"


#include <stdlib.h>
#include <util/atomic.h>

#define RING_BUFFER_MAX_SIZE 16

/**
 * Ring buffer used for storing pending tasks.
 */
static struct
{
  volatile task_t data[RING_BUFFER_MAX_SIZE];
  volatile uint8_t head, size;
}
_ring_buffer =
{
  .head = 0,
  .size = 0
};

return_status my_task__queue_new(task_t task)
{
  return_status status = return_status__ok;
  // If no function was provided, the task is invalid.
  if (task.func == NULL)
  {
    status = return_status__my_task__bad_parameter;
  }
  if (return_status__ok == status)
  {
    /*
     * Since both the main program and the ISRs may queue new tasks, we need to
     * run the entire operation inside an atomic block.
     */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
      uint8_t ring_buffer_head = _ring_buffer.head;
      uint8_t ring_buffer_previous_size = _ring_buffer.size;
      // If there's no room for a new task, return an error.
      if (ring_buffer_previous_size == RING_BUFFER_MAX_SIZE)
      {
        status = return_status__my_task__overflow;
      }
      if (return_status__ok == status)
      {
        /*
         * Compute the ring buffer's new tail.
         * 
         * Note that no overflow can occur from this expression: both head and
         * size cannot exceed 15 (RING_BUFFER_MAX_SIZE - 1), so
         * ring_buffer_new_tail cannot exceed 31 (2 * RING_BUFFER_MAX_SIZE - 1).
         */
        uint8_t ring_buffer_new_tail =
          ring_buffer_head + ring_buffer_previous_size + 1;
        /*
         * If ring_buffer_new_tail exceeds the ring buffer's max size, adjust
         * its value accordingly.
         * 
         * Note that subtracting RING_BUFFER_MAX_SIZE once is enough to adjust
         * ring_buffer_new_tail: its initial maximum value was 31
         * (2 * RING_BUFFER_MAX_SIZE - 1), thus subtracting RING_BUFFER_MAX_SIZE
         * would lead to a maximum value of 15 (RING_BUFFER_MAX_SIZE - 1).
         */
        if (ring_buffer_new_tail >= RING_BUFFER_MAX_SIZE)
        {
          ring_buffer_new_tail -= RING_BUFFER_MAX_SIZE;
        }
        // Add the new task to the ring buffer.
        _ring_buffer.data[ring_buffer_new_tail] = task;
        // Update the ring buffer's size.
        _ring_buffer.size = ring_buffer_previous_size + 1;
      }
    }
  }
  return status;
}

task_t my_task__try_to_read_next()
{
  /*
   * Reserve some space for a copy of the next task.
   * 
   * We also need a "flag" in order to know whether we did find a new task to
   * execute. Since a well formed task_t should never have func set to NULL,
   * we're using func just like a flag: we set it to NULL at first, and then, if
   * there actually is a new task, func will no longer be NULL.
   */
  task_t task =
  {
    .func = NULL
  };
  // Keep in mind, this method is expected to run within an outer atomic block.
  uint8_t ring_buffer_previous_size = _ring_buffer.size;
  /*
   * If the buffer is empty, there's no new task to run, and task.func stays
   * NULL. Otherwise, read and remove its head.
   */
  if (ring_buffer_previous_size > 0)
  {
    uint8_t ring_buffer_previous_head = _ring_buffer.head;
    /*
     * Once again, this computation will never cause an overflow: both head
     * and size cannot exceed 15 (RING_BUFFER_MAX_SIZE - 1), so
     * ring_buffer_new_tail cannot exceed 30 (2 * RING_BUFFER_MAX_SIZE - 2).
     */
    uint8_t ring_buffer_tail =
      ring_buffer_previous_head + ring_buffer_previous_size;
    /*
     * If ring_buffer_tail exceeds the ring buffer's max size, adjust its
     * value accordingly.
     * 
     * Note that subtracting RING_BUFFER_MAX_SIZE once is enough to adjust
     * ring_buffer_tail: its initial maximum value was 30
     * (2 * RING_BUFFER_MAX_SIZE - 2), thus subtracting RING_BUFFER_MAX_SIZE
     * would lead to a maximum value of 14 (RING_BUFFER_MAX_SIZE - 1).
     */
    if (ring_buffer_tail >= RING_BUFFER_MAX_SIZE)
    {
      ring_buffer_tail -= RING_BUFFER_MAX_SIZE;
    }
    task = _ring_buffer.data[ring_buffer_tail];
    // Compute the new value for _ring_buffer.head .
    uint8_t ring_buffer_new_head = ring_buffer_previous_head + 1;
    /*
     * If ring_buffer_new_head exceeds the ring buffer's max size, adjust its
     * value accordingly.
     */
    if (ring_buffer_new_head >= RING_BUFFER_MAX_SIZE)
    {
      ring_buffer_new_head -= RING_BUFFER_MAX_SIZE;
    }
    // Update both head and size of the ring buffer.
    _ring_buffer.head = ring_buffer_new_head;
    _ring_buffer.size = ring_buffer_previous_size - 1;
  }
  return task;
}