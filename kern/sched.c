#include <inc/assert.h>

#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>


// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *idle;
	int i,j;

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING) and never choose an
	// idle environment (env_type == ENV_TYPE_IDLE).  If there are
	// no runnable environments, simply drop through to the code
	// below to switch to this CPU's idle environment.

	// LAB 4: Your code here.
	if(curenv != NULL)
	{
		j = ENVX(curenv->env_id);
	}
	else
	{
		j = 0;
	}

	// For debugging and testing purposes, if there are no
	// runnable environments other than the idle environments,
	// drop into the kernel monitor.
	for (i = (j + 1) % NENV; i != j ; i = (i + 1)%NENV) {
		if (envs[i].env_type != ENV_TYPE_IDLE &&
		    envs[i].env_status == ENV_RUNNABLE)
		{
			if(curenv)
			{
			//	cprintf("sched_yield yes j %08x i %08x curenvid %08x old id %08x id %08x\n",j,i,curenv->env_id,envs[j].env_id,envs[i].env_id);
			}
			else
			{
			//	cprintf("sched_yield no j %08x i %08x old id %08x id %08x\n",j,i,envs[j].env_id,envs[i].env_id);
			}
			env_run(&envs[i]);
			return;
		}
	}
	if (i == NENV) {
		cprintf("No more runnable environments!\n");
		while (1)
			monitor(NULL);
	}

	// Run this CPU's idle environment when nothing else is runnable.
	idle = &envs[cpunum()];
	if (!(idle->env_status == ENV_RUNNABLE || idle->env_status == ENV_RUNNING))
		panic("CPU %d: No idle environment!", cpunum());
	env_run(idle);
}
