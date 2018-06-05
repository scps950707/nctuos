#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/mem.h>
#include <kernel/cpu.h>
#include <kernel/syscall.h>
#include <kernel/trap.h>
#include <kernel/spinlock.h>
#include <inc/stdio.h>

struct spinlock consoleLock;
void do_puts(char *str, uint32_t len)
{
	spin_lock(&consoleLock);
	uint32_t i;
	for (i = 0; i < len; i++)
	{
		k_putch(str[i]);
	}
	spin_unlock(&consoleLock);
}

int32_t do_getc()
{
	return k_getc();
}

int32_t do_syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t retVal = -1;
	extern void sys_settextcolor(unsigned char forecolor, unsigned char backcolor);
	extern void sys_cls();
	extern void sched_yield(void);

	switch (syscallno)
	{
	case SYS_fork:
		/* TODO: Lab 5
     * You can reference kernel/task.c, kernel/task.h
     */
		retVal = sys_fork();
		break;

	case SYS_getc:
		retVal = do_getc();
		break;

	case SYS_puts:
		do_puts((char*)a1, a2);
		retVal = 0;
		break;

	case SYS_getpid:
		/* TODO: Lab 5
     * Get current task's pid
     */
		retVal = thiscpu->cpu_task->task_id;
		break;

	case SYS_getcid:
		/* Lab6: get current cpu's cid */
		retVal = thiscpu->cpu_id;
		break;

	case SYS_sleep:
		/* TODO: Lab 5
     * Yield this task
     * You can reference kernel/sched.c for yielding the task
     */
		thiscpu->cpu_task->state = TASK_SLEEP;
		thiscpu->cpu_task->remind_ticks = a1;
		sched_yield();
		break;

	case SYS_kill:
		/* TODO: Lab 5
     * Kill specific task
     * You can reference kernel/task.c, kernel/task.h
     */
		sys_kill(thiscpu->cpu_task->task_id);
		retVal = 0;
		break;

  case SYS_get_num_free_page:
		/* TODO: Lab 5
     * You can reference kernel/mem.c
     */
		retVal = sys_get_num_free_page();
    break;

  case SYS_get_num_used_page:
		/* TODO: Lab 5
     * You can reference kernel/mem.c
     */
		retVal = sys_get_num_used_page();
    break;

  case SYS_get_ticks:
		/* TODO: Lab 5
     * You can reference kernel/timer.c
     */
    retVal = sys_get_ticks();
    break;

  case SYS_settextcolor:
		/* TODO: Lab 5
     * You can reference kernel/screen.c
     */
	sys_settextcolor(a1,a2);
	retVal = 0;
    break;

  case SYS_cls:
		/* TODO: Lab 5
     * You can reference kernel/screen.c
     */
	sys_cls();
	retVal = 0;
    break;
  /* TODO: Lab7 file I/O system call */
  case SYS_open:
	retVal = sys_open((char*)a1,a2,a3);
	break;
  case SYS_read:
	retVal = sys_read(a1, (void*)a2, a3);
	break;
  case SYS_write:
	retVal = sys_write(a1, (void*)a2, a3);
	break;
  case SYS_close:
	retVal = sys_close(a1);
	break;
  case SYS_lseek:
	retVal = sys_lseek(a1, a2, a3);
	break;
  case SYS_unlink:
	retVal = sys_unlink((char*)a1);
	break;
  case SYS_ls:
	retVal = sys_ls((char*)a1);
	break;
  case SYS_rm:
	retVal = sys_rm((char*)a1);
	break;
  case SYS_touch:
	retVal = sys_touch((char*)a1);
    break;
  }
	return retVal;
}

static void syscall_handler(struct Trapframe *tf)
{
	/* TODO: Lab5
   * call do_syscall
   * Please remember to fill in the return value
   * HINT: You have to know where to put the return value
   */

	int ret = do_syscall(tf->tf_regs.reg_eax,
			tf->tf_regs.reg_edx,
			tf->tf_regs.reg_ecx,
			tf->tf_regs.reg_ebx,
			tf->tf_regs.reg_edi,
			tf->tf_regs.reg_esi);
	tf->tf_regs.reg_eax=ret;
}

void syscall_init()
{
  /* TODO: Lab5
   * Please set gate of system call into IDT
   * You can leverage the API register_handler in kernel/trap.c
   */

	extern void SYS_CALL();
	register_handler(T_SYSCALL,syscall_handler,SYS_CALL,1,3);
}

