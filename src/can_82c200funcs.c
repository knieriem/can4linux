/* can_82c200funcs
*
* can4linux -- LINUX CAN device driver source
* 
* Copyright (c) 2001 port GmbH Halle/Saale
* (c) 2001 Heinz-J�rgen Oertel (oe@port.de)
*          Claus Schroeter (clausi@chemie.fu-berlin.de)
* derived from the the LDDK can4linux version
*     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
*------------------------------------------------------------------
* $Header: /z2/cvsroot/products/0530/software/can4linux/src/can_82c200funcs.c,v 1.8 2002/10/25 10:39:00 oe Exp $
*
*--------------------------------------------------------------------------
*
*
* modification history
* --------------------
* $Log: can_82c200funcs.c,v $
* Revision 1.8  2002/10/25 10:39:00  oe
* - vendor specific handling for Advantech board added by "R.R.Robotica" <rrrobot@tin.it>
*
* Revision 1.7  2002/10/11 16:58:06  oe
* - IOModel, Outc, VendOpt are now set at compile time
* - deleted one misleading printk()
*
* Revision 1.6  2002/08/20 05:57:22  oe
* - new write() handling, now not ovrwriting buffer content if buffer fill
* - ioctl() get status returns buffer information
*
* Revision 1.5  2002/08/08 17:55:32  oe
* - added vendor specific code for stzp 's' IXXAT board iPC03
* - received message with id=0xffffffff == error
*
* Revision 1.4  2001/09/14 14:58:09  oe
* first free release
*
*
*/
#include <can_defs.h>


/* int	CAN_Open = 0; */

/* timing values */
uint8 CanTiming[10][2]={
	{CAN_TIM0_10K,  CAN_TIM1_10K},
	{CAN_TIM0_20K,  CAN_TIM1_20K},
	{CAN_TIM0_40K,  CAN_TIM1_40K},
	{CAN_TIM0_50K,  CAN_TIM1_50K},
	{CAN_TIM0_100K, CAN_TIM1_100K},
	{CAN_TIM0_125K, CAN_TIM1_125K},
	{CAN_TIM0_250K, CAN_TIM1_250K},
	{CAN_TIM0_500K, CAN_TIM1_500K},
	{CAN_TIM0_800K, CAN_TIM1_800K},
	{CAN_TIM0_1000K,CAN_TIM1_1000K}};




/* Board reset
   means the following procedure:
  set Reset Request
  wait for Rest request is changing - used to see if board is available
  initialize board (with valuse from /proc/sys/Can)
    set output control register
    set timings
    set acceptance mask
*/


#ifdef DEBUG
int CAN_ShowStat (int board)
{
    if (dbgMask && (dbgMask & DBG_DATA)) {
#ifdef CAN_PELICANMODE
    printk(" MODE 0x%x,", CANin(board, canmode));
    printk(" STAT 0x%x,", CANin(board, canstat));
    printk(" IRQE 0x%x,", CANin(board, canirq_enable));
#else
    printk(" CTL 0x%x,", CANin(board, canctl));
    printk(" STAT 0x%x,", CANin(board, canstat));
#endif
    /* printk(" INT 0x%x\n", CANin(board, canirq)); */
    printk("\n");
    }
    return 0;
}
#endif

int can_GetStat(
	struct inode *inode,
	CanSja1000Status_par_t *stat
	)
{
unsigned int board = MINOR(inode->i_rdev);	
msg_fifo_t *Fifo;
unsigned long flags;

    stat->baud = Baud[board];
    /* printk(" STAT ST %d\n", CANin(board, canstat)); */
    stat->status = CANin(board, canstat);
#ifdef CAN_PELICANMODE
    /* printk(" STAT EWL %d\n", CANin(board, errorwarninglimit)); */
    stat->error_warning_limit = CANin(board, errorwarninglimit);
    stat->rx_errors = CANin(board, rxerror);
    stat->tx_errors = CANin(board, txerror);
    stat->error_code= CANin(board, errorcode);
#else
    stat->error_warning_limit = 0;
    stat->rx_errors = 0;
    stat->tx_errors = 0;
    stat->error_code= 0;
#endif

    /* Disable CAN Interrupts */
    /* !!!!!!!!!!!!!!!!!!!!! */
    save_flags(flags); cli();
    Fifo = &Rx_Buf[board];
    stat->rx_buffer_size = MAX_BUFSIZE;	/**< size of rx buffer  */
    /* number of messages */
    stat->rx_buffer_used =
    	(Fifo->head < Fifo->tail)
    	? (MAX_BUFSIZE - Fifo->tail + Fifo->head) : (Fifo->head - Fifo->tail);
    Fifo = &Tx_Buf[board];
    stat->tx_buffer_size = MAX_BUFSIZE;	/* size of tx buffer  */
    /* number of messages */
    stat->tx_buffer_used = 
    	(Fifo->head < Fifo->tail)
    	? (MAX_BUFSIZE - Fifo->tail + Fifo->head) : (Fifo->head - Fifo->tail);
    /* Enable CAN Interrupts */
    /* !!!!!!!!!!!!!!!!!!!!! */
    restore_flags(flags);
    return 0;
}

int CAN_ChipReset (int board)
{
uint8 status;
int i;

    DBGin("CAN_ChipReset");
    DBGprint(DBG_DATA,(" INT 0x%x\n", CANin(board, canirq)));

#ifdef CAN_PELICANMODE
    CANout(board, canmode, CAN_RESET_REQUEST);
#else
    CANout(board, canctl, CAN_RESET_REQUEST );
#endif



    for(i = 0; i < 100; i++) SLOW_DOWN_IO;

    status = CANin(board, canstat);

#ifdef CAN_PELICANMODE
    DBGprint(DBG_DATA,("status=0x%x mode=0x%x", status,
	    CANin(board, canmode) ));
    if( ! (CANin(board, canmode) & CAN_RESET_REQUEST ) ){
#else
    DBGprint(DBG_DATA,("status=0x%x ctl=0x%x", status,
	    CANin(board, canctl) ));
    if( ! (CANin(board, canctl) & CAN_RESET_REQUEST ) ){
#endif
	    printk("ERROR: no board present!");
	    MOD_DEC_USE_COUNT;
	    DBGout();return -1;
    }

#ifndef CAN_PELICANMODE
# define canmode canctl
#endif

    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x\n", board, CANin(board, canmode)));
    /* select mode: Basic or PeliCAN */
#ifdef CAN_PELICANMODE
    CANout(board, canclk, CAN_MODE_PELICAN + CAN_MODE_CLK);
    CANout(board, canmode, CAN_RESET_REQUEST + CAN_MODE_DEF);
#else
    /* set 82c200 mode for SJA 1000 */
    CANout(board, canclk, CAN_MODE_BASICCAN + CAN_MODE_CLK);
#endif
    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x\n", board, CANin(board, canmode)));
    
    /* Board specific output control */
    CANout(board, canoutc, Outc[board]);
    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x\n", board, CANin(board, canmode)));

    CAN_SetTiming(board, Baud[board]    );
    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x\n", board, CANin(board, canmode)));
    CAN_SetMask  (board, AccCode[board], AccMask[board] );
    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x\n", board, CANin(board, canmode)));

    /* Can_dump(board); */
    DBGout();
    return 0;
}


int CAN_SetTiming (int board, int baud)
{
int i = 5;
int custom=0;
    DBGin("CAN_SetTiming");
    DBGprint(DBG_DATA, ("baud[%d]=%d", board, baud));
    switch(baud)
    {
	case   10: i = 0; break;
	case   20: i = 1; break;
	case   40: i = 2; break;
	case   50: i = 3; break;
	case  100: i = 4; break;
	case  125: i = 5; break;
	case  250: i = 6; break;
	case  500: i = 7; break;
	case  800: i = 8; break;
	case 1000: i = 9; break;
	default  : 
		custom=1;
    }
    /* select mode: Basic or PeliCAN */
#ifdef CAN_PELICANMODE
    CANout(board, canclk, CAN_MODE_PELICAN + CAN_MODE_CLK);
#else
    /* set 82c200 mode for SJA 1000 */
    CANout(board, canclk, CAN_MODE_BASICCAN + CAN_MODE_CLK);
#endif
    if( custom ) {
       CANout(board, cantim0, (uint8) (baud >> 8) & 0xff);
       CANout(board, cantim1, (uint8) (baud & 0xff ));
       printk(" custom bit timing BT0=0x%x BT1=0x%x ",
       		CANin(board, cantim0), CANin(board, cantim1));
    } else {
       CANout(board,cantim0, (uint8) CanTiming[i][0]);
       CANout(board,cantim1, (uint8) CanTiming[i][1]);
    }
    DBGprint(DBG_DATA,("tim0=0x%x tim1=0x%x",
    		CANin(board, cantim0), CANin(board, cantim1)) );

    DBGout();
    return 0;
}


int CAN_StartChip (int board)
{
int i;
    RxErr[board] = TxErr[board] = 0L;
    DBGin("CAN_StartChip");
    DBGprint(DBG_DATA, ("[%d] CAN_mode 0x%x\n", board, CANin(board, canmode)));
/*
    CANout( board,cancmd, (CAN_RELEASE_RECEIVE_BUFFER 
			  | CAN_CLEAR_OVERRUN_STATUS) ); 
*/
    for(i=0;i<100;i++) SLOW_DOWN_IO;
    /* clear interrupts */
    CANin(board, canirq);

#ifdef CAN_PELICANMODE
    /* Interrupts on Rx, TX, any Status change and data overrun */
    CANset(board, canirq_enable, (
		  CAN_OVERRUN_INT_ENABLE
		+ CAN_ERROR_INT_ENABLE
		+ CAN_TRANSMIT_INT_ENABLE
		+ CAN_RECEIVE_INT_ENABLE ));

    CANreset( board, canmode, CAN_RESET_REQUEST );
    DBGprint(DBG_DATA,("start mode=0x%x", CANin(board, canmode)));
#else
    CANset(board, canctl, CAN_RECEIVE_INT_ENABLE );
    CANreset( board, canctl, CAN_RESET_REQUEST );
    DBGprint(DBG_DATA,("start ctl=0x%x", CANin(board, canctl)));
#endif

    DBGout();
    return 0;
}


int CAN_StopChip (int board)
{
    DBGin("CAN_StopChip");
#ifdef CAN_PELICANMODE
    CANset(board, canmode, CAN_RESET_REQUEST );
#else
    CANset(board, canctl, CAN_RESET_REQUEST );
#endif
    DBGout();
    return 0;
}

/* set value of the output control register */
int CAN_SetOMode (int board, int arg)
{

    DBGin("CAN_SetOMode");
	DBGprint(DBG_DATA,("[%d] outc=0x%x", board, arg));
	CANout(board, canoutc, arg);

    DBGout();
    return 0;
}


int CAN_SetMask (int board, unsigned int code, unsigned int mask)
{
#ifdef CPC_PCI
# define R_OFF 4 /* offset 4 for the EMS CPC-PCI card */
#else
# define R_OFF 1
#endif

    DBGin("CAN_SetMask");
    DBGprint(DBG_DATA,("[%d] acc=0x%x mask=0x%x",  board, code, mask));
#ifdef CAN_PELICANMODE
    CANout(board, frameinfo,
			(unsigned char)((unsigned int)(code >> 24) & 0xff));	
    CANout(board, frame.extframe.canid1,
			(unsigned char)((unsigned int)(code >> 16) & 0xff));	
    CANout(board, frame.extframe.canid2,
			(unsigned char)((unsigned int)(code >>  8) & 0xff));	
    CANout(board, frame.extframe.canid3,
    			(unsigned char)((unsigned int)(code >>  0 ) & 0xff));	

    CANout(board, frame.extframe.canid4,
			(unsigned char)((unsigned int)(mask >> 24) & 0xff));
    CANout(board, frame.extframe.canxdata[0 * R_OFF],
			(unsigned char)((unsigned int)(mask >> 16) & 0xff));
    CANout(board, frame.extframe.canxdata[1 * R_OFF],
			(unsigned char)((unsigned int)(mask >>  8) & 0xff));
    CANout(board, frame.extframe.canxdata[2 * R_OFF],
			(unsigned char)((unsigned int)(mask >>  0) & 0xff));

#else
    CANout(board, canacc,  (unsigned char)(code & 0xff));	
    CANout(board, canmask, (unsigned char)(mask & 0xff));
#endif
    DBGout();
    return 0;
}


int CAN_SendMessage (int board, canmsg_t *tx, int wait)
{
int i = 0, cycle = 0;
int ext;			/* message format to send */
uint8 tx2reg, stat;

    DBGin("CAN_SendMessage");

  if(wait) {
      LDDK_START_TIMER(Timeout[board]);
  }
  while ( ! (stat=CANin(board, canstat))
  	& CAN_TRANSMIT_BUFFER_ACCESS 
  	&& ! LDDK_TIMEDOUT ) {
	    #if LINUX_VERSION_CODE >= 131587
	    if( current->need_resched ) schedule();
	    #else
	    if( need_resched ) schedule();
	    #endif
  }

  if( ! LDDK_TIMEDOUT  ) {
    DBGprint(DBG_DATA,(
    		"CAN[%d]: tx.flags=%d tx.id=0x%lx tx.length=%d stat=0x%x",
		board, tx->flags, tx->id, tx->length, stat));

    tx->length %= 9;			/* limit CAN message length to 8 */
    ext = (tx->flags & MSG_EXT);	/* read message format */

    /* fill the frame info and identifier fields */
#ifdef CAN_PELICANMODE
    tx2reg = tx->length;
    if( tx->flags & MSG_RTR) {
	    tx2reg |= CAN_RTR;
    }
    if(ext) {
    DBGprint(DBG_DATA, ("---> send ext message \n"));
	CANout(board, frameinfo, CAN_EFF + tx2reg);
	CANout(board, frame.extframe.canid1, (uint8)(tx->id >> 21));
	CANout(board, frame.extframe.canid2, (uint8)(tx->id >> 13));
	CANout(board, frame.extframe.canid3, (uint8)(tx->id >> 5));
	CANout(board, frame.extframe.canid4, (uint8)(tx->id << 3) & 0xff);

    } else {
	DBGprint(DBG_DATA, ("---> send std message \n"));
	CANout(board, frameinfo, CAN_SFF + tx2reg);
	CANout(board, frame.stdframe.canid1, (uint8)((tx->id) >> 3) );
	CANout(board, frame.stdframe.canid2, (uint8)(tx->id << 5 ) & 0xe0);
    }
#else
    CANout(board, cantxdes1, (uint8)((tx->id) >> 3) );
    tx2reg =  ((tx->id << 5 ) & 0xe0) | ((uint8)(tx->length)) ;
    if( tx->flags & MSG_RTR ) 
	    tx2reg |= CAN_RTR;
    CANout(board, cantxdes2, tx2reg );
#endif

    /* - fill data ---------------------------------------------------- */
#ifdef CAN_PELICANMODE
    if(ext) {
	for( i=0; i < tx->length ; i++) {
#ifdef CAN4LINUX_PCI
	    CANout( board, frame.extframe.canxdata[4*i], tx->data[i]);
#else
	    CANout( board, frame.extframe.canxdata[i], tx->data[i]);
#endif
	    	SLOW_DOWN_IO; SLOW_DOWN_IO;
	}
    } else {
	for( i=0; i < tx->length ; i++) {
#ifdef CAN4LINUX_PCI
	    CANout( board, frame.stdframe.candata[4*i], tx->data[i]);
#else
	    CANout( board, frame.stdframe.candata[i], tx->data[i]);
#endif
	    	SLOW_DOWN_IO; SLOW_DOWN_IO;
	}
    }
#else
    for( i=0; i < tx->length ; i++) {
	CANout( board, cantxdata[i], tx->data[i]); SLOW_DOWN_IO; SLOW_DOWN_IO;
    }
#endif
    /* - /end --------------------------------------------------------- */
    CANout(board, cancmd, CAN_TRANSMISSION_REQUEST );
    if( wait ) {
       while( ! (CANin(board, canstat) & CAN_TRANSMISSION_COMPLETE_STATUS) 
           && ! LDDK_TIMEDOUT ) cycle++;
       if( LDDK_TIMEDOUT ) {
       DBGprint(DBG_DATA,("Timeout! stat=0x%x",CANin(board, canstat  )));	
	/*CANout(board, cancmd, CAN_ABORT_TRANSMISSION );*/
    

/*CANout( board,cancmd, (CAN_RELEASE_RECEIVE_BUFFER 
	                              | CAN_CLEAR_OVERRUN_STATUS) );*/
       }
    } else if( TxSpeed[board] == 'f' ) {
      unsigned long flags;
      /* enter critical section */
      save_flags(flags); cli();
      /* leave critical section */
      restore_flags(flags);
    }
  }  else {
    printk("CAN[%d] Tx: Timeout! stat=0x%x\n", board, stat);
    i= -1;
  }

  if(wait) LDDK_STOP_TIMER();
  DBGout();return i;

    DBGout();
}


int CAN_GetMessage (int board, canmsg_t *rx )
{
uint8 dummy = 0, stat;
int i = 0;
  
    DBGin("CAN_GetMessage");
    LDDK_START_TIMER(Timeout[board]);
    stat = CANin(board, canstat);
#ifdef CAN_PELICANMODE
#else
    DBGprint(DBG_DATA,("0x%x:stat=0x%x ctl=0x%x",
    			Base[board], stat, CANin(board, canctl)));
#endif
    rx->flags  = 0;
    rx->length = 0;

    if( stat & CAN_DATA_OVERRUN ) {
    DBGprint(DBG_DATA,("Rx: overrun!\n"));
    Overrun[board]++;
    }

    if( stat & CAN_RECEIVE_BUFFER_STATUS ) {
#ifdef CAN_PELICANMODE
#else
      dummy  = CANin(board, canrxdes2);
      rx->id = CANin(board, canrxdes1) << 3 | (dummy & 0xe0) >> 5 ;
#endif
      dummy  &= 0x0f; /* strip length code */ 
      rx->length = dummy;
      DBGprint(DBG_DATA,("rx.id=0x%lx rx.length=0x%x", rx->id, dummy));

      dummy %= 9;
      for( i = 0; i < dummy; i++) {
#ifdef CAN_PELICANMODE
#else
	rx->data[i] = CANin(board, canrxdata[i]);
#endif
	DBGprint(DBG_DATA,("rx[%d]: 0x%x ('%c')",i,rx->data[i],rx->data[i] ));
      }
      CANout(board, cancmd, CAN_RELEASE_RECEIVE_BUFFER | CAN_CLEAR_OVERRUN_STATUS );
    } else {
      i=0;
    }
    LDDK_STOP_TIMER();
    DBGout();
    return i;

    DBGout();
}


/* SSV ADNP1486 specific code */
#ifdef ADNP1486

#  define AMD_ELAN_INDEX_REG     0x22  /* (A)DNP Index-Reg. */
#  define AMD_ELAN_DATA_REG      0x23  /* (A)DNP Daten-Reg. */

/* write a value to index register on AMD Elan */
static inline void
write_port(unsigned char wi,unsigned char data)
{
   outb (wi, AMD_ELAN_INDEX_REG);
   outb (data, AMD_ELAN_DATA_REG);
}

/* read a value from index register on AMD Elan */
static inline char
read_port(unsigned char ri)
{
   outb (ri, AMD_ELAN_INDEX_REG);
   return (inb (AMD_ELAN_DATA_REG));
}

/* Initialize SJA1000 on ADNP1486 */
static void
adnp_map_sja1000 (int base, int irq)
{
   /* if IRQ6 is'nt mapped, map now ... */
   if ((read_port (0xD7) & 0x0F) == 0)
     {
#ifdef DEBUG
       printk ("Map INT4 PIRQS6 IRQ%d...\n", irq);
#endif
       write_port (0xD7, (read_port (0xD7) & 0xF0) | irq);
     }

   /* Init (A)DNP for CAN SJA1000  */
   /* GPIO_CS1 disable             */
   write_port (0xA6, read_port (0xA6) | 0x02);

   /* GPIO_CS1 to output           */
   write_port (0xA0, read_port (0xA0) | 0x0C);

   /* Lo-Byte of Base Address (0x280) */
   write_port (0xB6, base & 0x00FF);

   /* CS1: Hi-Byte of Base Address        */
   /* mask (0x280) and SA0 (280, 281)     */
   write_port (0xB7, (base >> 8) | 0x38);

   /* CS1: qualifyed with IOR & IOW */
   write_port (0xB8, (read_port (0xB8) & 0x0F) | 0x30);

   /* GP_CSPIO = GPIO_CS1               */
   write_port (0xB2, (read_port (0xB2) & 0x0F) | 0x10);

   /* GPIO_CS1 enable */
   write_port (0xA6, 0x00);
}
#endif /* ADNP1486 */

int CAN_VendorInit (int minor)
{
    DBGin("CAN_VendorInit");

/* 1. Vendor specific part ------------------------------------------------ */
    can_range[minor] = CAN_RANGE;
    if( (VendOpt[minor] == 's') || (VendOpt[minor] == 'a')) {   
	can_range[minor] = 0x200; /*stpz board or Advantech Pcm-3680 */
    }
/* End: 1. Vendor specific part ------------------------------------------- */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,11)

    /* can also be a compile time decision CAN_PORT_IO */
    if(IOModel[minor] == 'p' || IOModel[minor] == 'i') {
	/* It's port I/O */
	if(NULL == request_region(Base[minor], can_range[minor], "CAN-IO" )) {
	    return -EBUSY;
	}
    } else
      {
	/* It's Memory I/O */
	if(NULL == request_mem_region(Base[minor], can_range[minor], "CAN-IO" )) {
	    return -EBUSY;
	}

#if CAN4LINUX_PCI
	/* PCI scan has already remapped the address */
	can_base[minor] = (unsigned char *)Base[minor];
#else
	can_base[minor] = ioremap(Base[minor], 0x200);
#endif
    }
#else 	  /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,11) */
    
/* both drivers use high memory area */
#if !defined(CONFIG_PPC) && !defined(CAN4LINUX_PCI)
    if( check_region(Base[minor], CAN_RANGE ) ) {
		     /* MOD_DEC_USE_COUNT; */
		     DBGout();
		     return -EBUSY;
    } else {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,0,0)
          request_region(Base[minor], can_range[minor], "CAN-IO" );
#else
          request_region(Base[minor], can_range[minor] );
#endif  /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,0,0) */
    }
#endif /* !defined  ... */

    /* we don't need ioremap in older Kernels */
    can_base[minor] = Base[minor];
#endif  /*  LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,11) */

    /* now the virtual address can be used for the register address macros */


/* 2. Vendor specific part ------------------------------------------------ */

#ifdef ADNP1486
      if( VendOpt[minor] == 't' ) {    /* TRM/816 ADNP1486 */

	if (minor == 0)
	  {
	    adnp_map_sja1000 (Base[minor], IRQ[minor]); /* make SJA visible at 
							   Port Base[minor],
							   map IRQ */
	  }
	else
	  return -EINVAL;

      }
#endif
      if( VendOpt[minor] == 's' ) {    /* stpz board */
	if( Base[minor] & 0x200 ) {
		printk("Resetting STZP PC I-03 [contr 1]\n");
		/* perform HW reset 2. contr*/
		writeb(0xff, can_base[minor] + 0x300);
	} else {
		printk("Resetting STZP PC I-03 [contr 0]\n");
		/* perform HW reset 1. contr*/
		writeb(0xff, can_base[minor] + 0x100);
	}
      }

if( VendOpt[minor] == 'a' ) {    /* Advantech Pcm-3680 */
	if( Base[minor] & 0x200 ) {
		printk("Resetting Advantech Pcm-3680 [contr 1]\n");
		/* perform HW reset 2. contr*/
		writeb(0xff, can_base[minor] + 0x300);
	} else {
		printk("Resetting Advantech Pcm-3680 [contr 0]\n");
		/* perform HW reset 1. contr*/
		writeb(0xff, can_base[minor] + 0x100);
	}
        mdelay(100);
      }



/* End: 2. Vendor specific part ------------------------------------------- */

    if( IRQ[minor] > 0 ) {
        if( Can_RequestIrq( minor, IRQ[minor] , CAN_Interrupt) ) {
	     printk("Can[%d]: Can't request IRQ %d \n", minor, IRQ[minor]);
	     DBGout(); return -EBUSY;
        }
    }

    DBGout(); return 1;
}


/*
 * The plain 82c200 interrupt
 *
 *				RX ISR           TX ISR
 *                              8/0 byte
 *                               _____            _   ___
 *                         _____|     |____   ___| |_|   |__
 *---------------------------------------------------------------------------
 * 1) CPUPentium 75 - 200
 *  75 MHz, 149.91 bogomips
 *  AT-CAN-MINI                 42/27�s            50 �s
 *  CPC-PCI		        35/26�s		   
 * ---------------------------------------------------------------------------
 * 2) AMD Athlon(tm) Processor 1M
 *    2011.95 bogomips
 *  AT-CAN-MINI  std            24/12�s            ?? �s
 *               ext id           /14�s
 *
 * 
 * 1) 1Byte = (42-27)/8 = 1.87 �s
 * 2) 1Byte = (24-12)/8 = 1.5  �s
 *
 *
 *
 * RX Int with to Receive channels:
 * 1)                _____   ___
 *             _____|     |_|   |__
 *                   30    5  20  �s
 *   first ISR normal length,
 *   time between the two ISR -- short
 *   sec. ISR shorter than first, why? it's the same message
 */

void CAN_Interrupt ( int irq, void *dev_id, struct pt_regs *ptregs )
{
int minor;
int i;
unsigned long flags;
int ext;			/* flag for extended message format */
int irqsrc, dummy;
msg_fifo_t   *RxFifo; 
msg_fifo_t   *TxFifo;
#if CAN_USE_FILTER
msg_filter_t *RxPass;
unsigned int id;
#endif 
#if 0
int first = 0;
#endif 

#if CONFIG_TIME_MEASURE
    outb(0xff, 0x378);  
#endif

    /*  minor = irq2minormap[irq]; */
    minor = *(int *)dev_id;
    RxFifo = &Rx_Buf[minor]; 
    TxFifo = &Tx_Buf[minor];
#if CAN_USE_FILTER
    RxPass = &Rx_Filter[minor];
#endif 

    /* Disable PITA Interrupt */
    /* writel(0x00000000, Can_pitapci_control[minor] + 0x0); */

    irqsrc = CANin(minor, canirq);
    if(irqsrc == 0) {
         /* first call to ISR, it's not for me ! */
	 goto IRQdone_doneNothing;
    }
    do {
    /* loop as long as the CAN controller shows interrupts */
    DBGprint(DBG_DATA, (" => got IRQ[%d]: 0x%0x\n", minor, irqsrc));
    /* Can_dump(minor); */

#if 0
    /* how often do we lop through the ISR ? */
    if(first) printk("n = %d\n", first);
    first++;
#endif

    /* preset flags */
    (RxFifo->data[RxFifo->head]).flags =
        		(RxFifo->status & BUF_OVERRUN ? MSG_BOVR : 0);

    /*========== receive interrupt */
    if( irqsrc & CAN_RECEIVE_INT ) {
        /* fill timestamp as first action */
	 do_gettimeofday(&(RxFifo->data[RxFifo->head]).timestamp);

#ifdef CAN_PELICANMODE
	dummy  = CANin(minor, frameinfo );
#else
	dummy  = CANin(minor, canrxdes2 );
#endif
#if CAN_USE_FILTER
                                          /* don't need & 0xe0 */
	id     = CANin(minor, canrxdes1 ) << 3 | (dummy & 0xe0) >> 5 ;

       	/* got RTR */
	if( dummy & CAN_RTR && RxPass->filter[id].rtr_response != NULL ) {
	    canmsg_t *RTRR = RxPass->filter[id].rtr_response;

#ifdef CAN_PELICANMODE
#else
	    CANout( minor, cantxdes1 ,(uint8) (RTRR->id >> 3));
	    CANout( minor, cantxdes2 ,(uint8) ((RTRR->id << 5) & 0xe0) | RTRR->length );
#endif
	    for(i = 0; i < RTRR->length && i < 8 ; i++ ) {
		CANout(minor, cantxdata[i], RTRR->data[i]); 
	    }
	    CANout(minor, cancmd, CAN_TRANSMISSION_REQUEST );
	} else if( dummy & CAN_RTR ) {
	    (RxFifo->data[RxFifo->head]).flags |= MSG_RTR;
        }

	(RxFifo->data[RxFifo->head]).id = id;
#else
        if( dummy & CAN_RTR ) {
	    (RxFifo->data[RxFifo->head]).flags |= MSG_RTR;
	}

#ifdef CAN_PELICANMODE
        if( dummy & CAN_EFF ) {
	    (RxFifo->data[RxFifo->head]).flags |= MSG_EXT;
	}
	ext = (dummy & CAN_EFF);
	if(ext) {
	    (RxFifo->data[RxFifo->head]).id =
	          ((unsigned int)(CANin(minor, frame.extframe.canid1)) << 21)
		+ ((unsigned int)(CANin(minor, frame.extframe.canid2)) << 13)
		+ ((unsigned int)(CANin(minor, frame.extframe.canid3)) << 5)
		+ ((unsigned int)(CANin(minor, frame.extframe.canid4)) >> 3);
	} else {
	    (RxFifo->data[RxFifo->head]).id =
        	  ((unsigned int)(CANin(minor, frame.stdframe.canid1 )) << 3) 
        	+ ((unsigned int)(CANin(minor, frame.stdframe.canid2 )) >> 5);
	}
#else
        (RxFifo->data[RxFifo->head]).id =
                                         /* don't need & 0xe0 */
        	CANin(minor, canrxdes1 ) << 3 | (dummy & 0xe0) >> 5 ;
#endif
#endif
	/* get message length */
        dummy  &= 0x0f;			/* strip length code */ 
        (RxFifo->data[RxFifo->head]).length = dummy;

        dummy %= 9;	/* limit count to 8 bytes */
        for( i = 0; i < dummy; i++) {
            SLOW_DOWN_IO;SLOW_DOWN_IO;
#ifdef CAN_PELICANMODE
	    if(ext) {
		(RxFifo->data[RxFifo->head]).data[i] =
#ifdef CAN4LINUX_PCI
			CANin(minor, frame.extframe.canxdata[4*i]);
#else
			CANin(minor, frame.extframe.canxdata[i]);
#endif
	    } else {
		(RxFifo->data[RxFifo->head]).data[i] =
#ifdef CAN4LINUX_PCI
			CANin(minor, frame.stdframe.candata[4*i]);
#else
			CANin(minor, frame.stdframe.candata[i]);
#endif
	    }
#else
	    (RxFifo->data[RxFifo->head]).data[i] = CANin(minor,canrxdata[i]);
#endif
	}
	RxFifo->status = BUF_OK;
#if CAN_USE_FILTER
	if(RxPass->use && !RxPass->filter[id].enable){
#if FDEBUG
		printk("discarded %d \n",id);
#endif
		goto DiscardRX;
        }
	else
#endif
        RxFifo->head = ++(RxFifo->head) % MAX_BUFSIZE;

	if(RxFifo->head == RxFifo->tail) {
		printk("CAN[%d] RX: FIFO overrun\n", minor);
		RxFifo->status = BUF_OVERRUN;
        } 
#if CAN_USE_FILTER
DiscardRX:
#endif
        /*---------- kick the select() call  -*/
        /* __wake_up(struct wait_queue ** p, unsigned int mode); */
        /* __wake_up(struct wait_queue_head_t *q, unsigned int mode); */
        /*
        void wake_up(struct wait_queue**condition)                                                                  
        */
        /* This function will wake up all processes
           that are waiting on this event queue,
           that are in interruptible sleep
        */
        wake_up_interruptible(  &CanWait[minor] ); 

        CANout(minor, cancmd, CAN_RELEASE_RECEIVE_BUFFER );
        if(CANin(minor, canstat) & CAN_DATA_OVERRUN ) {
		 printk("CAN[%d] Rx: Overrun Status \n", minor);
		 CANout(minor, cancmd, CAN_CLEAR_OVERRUN_STATUS );
	}

   }

    /*========== transmit interrupt */
    if( irqsrc & CAN_TRANSMIT_INT ) {
	uint8 tx2reg;
	unsigned int id;
	if( TxFifo->free[TxFifo->tail] == BUF_EMPTY ) {
	    /* printk("TXE\n"); */
	    TxFifo->status = BUF_EMPTY;
            TxFifo->active = 0;
            goto Tx_done;
	} else {
	    /* printk("TX\n"); */
	}

        /* enter critical section */
        save_flags(flags);cli();

#ifdef CAN_PELICANMODE
	tx2reg = (TxFifo->data[TxFifo->tail]).length;
	if( (TxFifo->data[TxFifo->tail]).flags & MSG_RTR) {
		tx2reg |= CAN_RTR;
	}
        ext = (TxFifo->data[TxFifo->tail]).flags & MSG_EXT;
        id = (TxFifo->data[TxFifo->tail]).id;
        if(ext) {
	    DBGprint(DBG_DATA, ("---> send ext message \n"));
	    CANout(minor, frameinfo, CAN_EFF + tx2reg);
	    CANout(minor, frame.extframe.canid1, (uint8)(id >> 21));
	    CANout(minor, frame.extframe.canid2, (uint8)(id >> 13));
	    CANout(minor, frame.extframe.canid3, (uint8)(id >> 5));
	    CANout(minor, frame.extframe.canid4, (uint8)(id << 3) & 0xff);
        } else {
	    DBGprint(DBG_DATA, ("---> send std message \n"));
	    CANout(minor, frameinfo, CAN_SFF + tx2reg);
	    CANout(minor, frame.stdframe.canid1, (uint8)((id) >> 3) );
	    CANout(minor, frame.stdframe.canid2, (uint8)(id << 5 ) & 0xe0);
        }
#else
        CANout(minor, cantxdes1,
        		(uint8)(((TxFifo->data[TxFifo->tail]).id ) >> 3));
        tx2reg = (((TxFifo->data[TxFifo->tail]).id << 5 ) & 0xe0) 
                     | ((uint8) (TxFifo->data[TxFifo->tail]).length  ) ;
        if( (TxFifo->data[TxFifo->tail]).flags & MSG_RTR ) {
	    tx2reg |= CAN_RTR;
	}
        CANout(minor, cantxdes2, tx2reg );
#endif


#ifdef CAN_PELICANMODE
	tx2reg &= 0x0f;		/* restore length only */
	if(ext) {
	    for( i=0; i < tx2reg ; i++) {
#ifdef CAN4LINUX_PCI
		CANout(minor, frame.extframe.canxdata[4*i],
		    (TxFifo->data[TxFifo->tail]).data[i]);
#else
		CANout(minor, frame.extframe.canxdata[i],
		    (TxFifo->data[TxFifo->tail]).data[i]);
#endif
	    }
        } else {
	    for( i=0; i < tx2reg ; i++) {
#ifdef CAN4LINUX_PCI
		CANout(minor, frame.stdframe.candata[4*i],
		    (TxFifo->data[TxFifo->tail]).data[i]);
#else
		CANout(minor, frame.stdframe.candata[i],
		    (TxFifo->data[TxFifo->tail]).data[i]);
#endif
	    }
        }
#else	/* standard Basic CAN mode vvv */
        for( i=0; i < (TxFifo->data[TxFifo->tail]).length && i < 8 ; i++){ 
	    CANout(minor, cantxdata[i], (TxFifo->data[TxFifo->tail]).data[i]);
            SLOW_DOWN_IO;
	}
#endif

        CANout(minor, cancmd, CAN_TRANSMISSION_REQUEST );

	TxFifo->free[TxFifo->tail] = BUF_EMPTY; /* now this entry is EMPTY */
	TxFifo->tail = ++(TxFifo->tail) % MAX_BUFSIZE;

        /* leave critical section */
	restore_flags(flags);
   }
Tx_done:
   /*========== error status */
   if( irqsrc & CAN_ERROR_INT ) {
   int s;
	printk("CAN[%d]: Tx err!\n", minor);
        TxErr[minor]++;

        /* insert error */
        s = CANin(minor,canstat);
        if(s & CAN_BUS_STATUS )
	    (RxFifo->data[RxFifo->head]).flags += MSG_BUSOFF; 
        if(s & CAN_ERROR_STATUS)
	    (RxFifo->data[RxFifo->head]).flags += MSG_PASSIVE; 

	(RxFifo->data[RxFifo->head]).id = 0xFFFFFFFF;
        /* (RxFifo->data[RxFifo->head]).length = 0; */
	/* (RxFifo->data[RxFifo->head]).data[i] = 0; */
	RxFifo->status = BUF_OK;
        RxFifo->head = ++(RxFifo->head) % MAX_BUFSIZE;
	if(RxFifo->head == RxFifo->tail) {
		printk("CAN[%d] RX: FIFO overrun\n", minor);
		RxFifo->status = BUF_OVERRUN;
        } 
	
   }
   if( irqsrc & CAN_OVERRUN_INT ) {
   int s;
	printk("CAN[%d]: overrun!\n", minor);
        Overrun[minor]++;

        /* insert error */
        s = CANin(minor,canstat);
        if(s & CAN_DATA_OVERRUN)
	    (RxFifo->data[RxFifo->head]).flags += MSG_OVR; 

	(RxFifo->data[RxFifo->head]).id = 0xFFFFFFFF;
        /* (RxFifo->data[RxFifo->head]).length = 0; */
	/* (RxFifo->data[RxFifo->head]).data[i] = 0; */
	RxFifo->status = BUF_OK;
        RxFifo->head = ++(RxFifo->head) % MAX_BUFSIZE;
	if(RxFifo->head == RxFifo->tail) {
		printk("CAN[%d] RX: FIFO overrun\n", minor);
		RxFifo->status = BUF_OVERRUN;
        } 

        CANout(minor, cancmd, CAN_CLEAR_OVERRUN_STATUS );
   } 
   } while( (irqsrc = CANin(minor, canirq)) != 0);

/* IRQdone: */
    DBGprint(DBG_DATA, (" => leave IRQ[%d]\n", minor));
#ifdef CAN4LINUX_PCI
    /* Interrupt_0_Enable (bit 17) + Int_0_Reset (bit 1) */
    /*  
     Uttenthaler:
      nur 
        writel(0x00020002, Can_pitapci_control[minor] + 0x0);
      als letzte Anweisung in der ISR
     Sch�tt:
      bei Eintritt
        writel(0x00000000, Can_pitapci_control[minor] + 0x0);
      am ende
        writel(0x00020002, Can_pitapci_control[minor] + 0x0);
    */
    writel(0x00020002, Can_pitapci_control[minor] + 0x0);
    writel(0x00020000, Can_pitapci_control[minor] + 0x0);
#endif
IRQdone_doneNothing:
#if CONFIG_TIME_MEASURE
    outb(0x00, 0x378);  
#endif
}


