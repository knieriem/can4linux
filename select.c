/* can_select
*
* can4linux -- LINUX CAN device driver source
* 
* Copyright (c) 2001 port GmbH Halle/Saale
* (c) 2001 Heinz-Jürgen Oertel (oe@port.de)
*          Claus Schroeter (clausi@chemie.fu-berlin.de)
* derived from the the LDDK can4linux version
*     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
*/

#include "defs.h"

unsigned int can_select( __LDDK_SELECT_PARAM )
{

unsigned int minor = __LDDK_MINOR;
msg_fifo_t *RxFifo = &Rx_Buf[minor];
    DBGin("can_select");
	    DBGprint(DBG_DATA,("minor = %d", minor));
#ifdef DEBUG
    CAN_ShowStat(minor);
#endif
    DBGprint(DBG_BRANCH,("POLL: fifo empty,poll waiting...\n"));

    /* every event queue that could wake up the process
     * and change the status of the poll operation
     * can be added to the poll_table structure by
     * calling the function poll_wait:  
     */
                /*     _select para, wait queue, _select para */
/* X */		poll_wait(file, &CanWait[minor] , wait);

    DBGprint(DBG_BRANCH,("POLL: wait returned \n"));
    {
      int ret_mask = 0;

      msg_fifo_t   *TxFifo = &Tx_Buf[minor];


      if( RxFifo->head != RxFifo->tail ) {
	  /* fifo has some telegrams */
	  /* Return a bit mask
	   * describing operations that could be immediately performed
	   * without blocking.
	   */
	  /*
	   * POLLIN This bit must be set
	   *        if the device can be read without blocking. 
	   * POLLRDNORM This bit must be set
	   * if "normal'' data is available for reading.
	   * A readable device returns (POLLIN | POLLRDNORM)
	   *
	   *
	   *
	   */
	  ret_mask |= POLLIN | POLLRDNORM;
      }
      if(!TxFifo->active) {
	  /* Tx-queue is empty, the user may fill it again.
	   *
	   * The condition above has been used in favor of
	   *   (TxFifo->free[TxFifo->head] != BUF_FULL)
	   * to allow the process using this driver to sleep larger
	   * amounts of time in a select call. The user will be waken
	   * up in CAN_Interrupt if a transmit interrupt has occured and
	   * the tx_queue has become empty.
	   *
	   */
	  /*
	   * POLLOUT This bit is set in the return value if the device
	   * can be written to without blocking.
	   *
	   * POLLWRNORM This bit has the same meaning as POLLOUT, and
	   * sometimes it actually is the same number. A writable
	   * device returns (POLLOUT | POLLWRNORM).
	   *
	   */
	  ret_mask |= POLLOUT | POLLWRNORM;
      }

      DBGout();
      return ret_mask;
    }
        
    DBGout();
    return 0;
}