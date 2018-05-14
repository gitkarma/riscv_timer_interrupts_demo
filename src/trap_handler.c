#include <stdint.h>
#include "encoding.h"
#include "util.h"
#include "trap_handler.h"

void dump_tf(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
  static const char* regnames[] = {
    "z ", "ra", "sp", "gp", "tp", "t0",  "t1",  "t2",
    "s0", "s1", "a0", "a1", "a2", "a3",  "a4",  "a5",
    "a6", "a7", "s2", "s3", "s4", "s5",  "s6",  "s7",
    "s8", "s9", "sA", "sB", "t3", "t4",  "t5",  "t6"
  };

  //regs[0] = 0;

  for(int i = 0; i < 32; i+=4)
  {
    for(int j = 0; j < 4; j++)
      printf("%s %lx%c",regnames[i+j], regs[i+j],j < 3 ? '\t' : '\n');
  }
  printf("pc %lx\n", epc);
}

void handle_misaligned_fetch(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
  dump_tf(cause, epc, regs);
  printf("Misaligned instruction access!\n");
}

void handle_illegal_instruction(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
  dump_tf(cause, epc, regs);
  printf("An illegal instruction was executed!\n");
}

void handle_breakpoint(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
  dump_tf(cause, epc, regs);
  printf("Breakpoint!\n");
}

void handle_misaligned_store(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
  dump_tf(cause, epc, regs);
  printf("Misaligned AMO!\n");
}

void handle_interrupt(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
  printf("This was an interruption .. clearing SIP_S{E|S|T}IP and getting back to code!\n");
  clear_csr(mip, MIP_MSIP | MIP_MEIP);// | MIP_MTIP | MIP_MEIP);
  //tohost_exit(1337);
}

void set_timer_max(){
    MTIMECMP[0] = (uint64_t)-1ULL;
}

void handle_machine_timer_interrupt(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
  //write_csr(mip, 0);         // Clearing IP (interrupt pending)
  //clear_csr(mie, MIP_MTIP);         // Clearing IP (interrupt pending)
  //*MTIMECMP = (uint64_t)-1ULL; // Setting to 0xFFF...F because the timer fires if mtime >= mtimecmp

  printf("[ CAUSE_MACHINE_TIMER_INTERRUPT ] ... machine timer interrupt catched ... \n"); //t=%d \n", MTIME);
  
  while (read_csr(mip) & MIP_MTIP) {
      MTIMECMP[0] = MTIME + 0x300;
  }

}

//#define MCAUSE        (*(volatile long long *)(CSR_MCAUSE))
//#define SCAUSE        (*(volatile long long *)(CSR_SCAUSE))
#define MTVAL        (*(volatile long long *)(CSR_MTVAL))

void handle_trap(uintptr_t cause, uintptr_t epc, uintptr_t regs[32])
{
  uint64_t mcause_v = read_csr(mcause);
  uint64_t scause_v = read_csr(scause); // Not relevant since we are in M-mode only
  //uint64_t mtval_v = read_csr(mtval);

//#define USE_SPIKE_HERE
#ifdef USE_SPIKE_HERE
  printf("\nmcause = %llx   -   scause = %llx \n", mcause_v, scause_v);
#else
  printf("\nmcause = %llx   -   scause = %llx   -   mtval = %llx \n", mcause_v, scause_v, MTVAL);
#endif

  if (cause & 0x8000000000000000){
      uint64_t int_code = cause & ~0x8000000000000000;

      printf("[handle_trap] - Interrupt : %s .. Executing interrupt_handler\n", interrupts_txt[cause]);

      typedef void (*interrupt_handler)(uintptr_t, uintptr_t, uintptr_t*);

      const static interrupt_handler interrupt_handlers[] = {
        // [CAUSE_USER_SOFTWARE_INTERRUPT] = handle_supervisor_timer_interrupt,
        // [CAUSE_SUPERVISOR_SOFTWARE_INTERRUPT] = handle_supervisor_timer_interrupt,
        // [CAUSE_MACHINE_SOFTWARE_INTERRUPT] = handle_supervisor_timer_interrupt,
        //[CAUSE_USER_TIMER_INTERRUPT] = handle_supervisor_timer_interrupt,
        //[CAUSE_SUPERVISOR_TIMER_INTERRUPT] = handle_supervisor_timer_interrupt

        [CAUSE_MACHINE_TIMER_INTERRUPT] = handle_machine_timer_interrupt,

        //[CAUSE_USER_EXTERNAL_INTERRUPT] = handle_supervisor_timer_interrupt,
        //[CAUSE_SUPERVISOR_EXTERNAL_INTERRUPT] = handle_supervisor_timer_interrupt,
        //[CAUSE_MACHINE_EXTERNAL_INTERRUPT] = handle_supervisor_timer_interrupt,
      };

      if(int_code < ARRAY_SIZE(interrupt_handlers) && interrupt_handlers[int_code])
        interrupt_handlers[int_code](cause, epc, regs);
      else
        handle_interrupt(int_code, epc, regs);
      
      return;
  }

  //printf("[handle_trap] - Exception : %s .. Executing trap_handler\n", MTVAL, (intptr_t)cause, exceptions_txt[cause]);
  printf("[handle_trap] - Exception : %s .. Executing trap_handler\n", exceptions_txt[cause]);


  typedef void (*trap_handler)(uintptr_t, uintptr_t, uintptr_t*);

  const static trap_handler trap_handlers[] = {
    [CAUSE_MISALIGNED_FETCH] = handle_misaligned_fetch,
    //[CAUSE_FETCH_PAGE_FAULT] = handle_fault_fetch,
    [CAUSE_ILLEGAL_INSTRUCTION] = handle_illegal_instruction,
    //[CAUSE_USER_ECALL] = handle_syscall,
    [CAUSE_BREAKPOINT] = handle_breakpoint,
    [CAUSE_MISALIGNED_STORE] = handle_misaligned_store,
    //[CAUSE_LOAD_PAGE_FAULT] = handle_fault_load,
    //[CAUSE_STORE_PAGE_FAULT] = handle_fault_store
  };

  if(cause < ARRAY_SIZE(trap_handlers) && trap_handlers[cause])
    trap_handlers[cause](cause, epc, regs);
  
  tohost_exit(1337);
}




