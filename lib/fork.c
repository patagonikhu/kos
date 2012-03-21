// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at vpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.

//	panic("pgfault not implemented");
	uint32_t pn;
	pn = (uint32_t)ROUNDDOWN(addr,PGSIZE) / PGSIZE;	
	pte_t pte;
	pte = ((pte_t *)vpt)[pn];
	if(((err & FEC_WR) == 0) || ((pte & PTE_COW) == 0))
	{
		panic("lib/pgfault : bad page fault at 0x%08x err 0x%08x pte %08x %08x %08x \n",addr,err,pte,pn,pn);	
	}	
	int t;
	t = sys_page_alloc(0, PFTEMP,PTE_P | PTE_U | PTE_W);
	if(t < 0)
	{
		panic("lib/pgfault sys_page_alloc panic error %08d %08x %08x\n",t,sys_getenvid());
	}
	memmove(PFTEMP,ROUNDDOWN(addr,PGSIZE),PGSIZE);
	t = sys_page_map(0,PFTEMP,0,ROUNDDOWN(addr,PGSIZE),PTE_P | PTE_U | PTE_W);
	if(t < 0)
	{
		panic("lib/pgfault sys_page_map panic\n");
	}
	sys_page_unmap(0,PFTEMP);
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;
	void * addr;

	// LAB 4: Your code here.
	pte_t pte;
	pte = vpt[pn];
	addr = (void *)(pn * PGSIZE);
	if(((pte & PTE_P) == 0) || ((pte & PTE_U) == 0))	
	{
		panic("duppage panic.addr 0x%08x  pte 0x%08x\n",addr,pte);
	}
	int t;
	if(((pte & PTE_W) != 0) || ((pte & PTE_COW) != 0))	
	{
		t = sys_page_map(0, addr, envid, addr,PTE_P | PTE_U | PTE_COW);
		//t =sys_page_map(0, addr, envid, addr,PTE_P | PTE_U | PTE_W);
		if(t < 0)
		{
			panic("duppage sys_page_map fail %08x %08x %d\n",envid,addr,t);
		}
		if((pte & PTE_COW) != 0)
		{
			t = sys_page_map(0, addr, 0, addr,PTE_P | PTE_U | PTE_COW);
			//t = sys_page_map(0, addr, 0, addr,PTE_P | PTE_U | PTE_W);
			if(t < 0)
			{
				panic("duppage sys_page_map fail\n");
			}

		}
	}
	else
	{
		t = sys_page_map(0, addr, envid, addr,PTE_P | PTE_U |PTE_W);
		if(t < 0)
		{
			panic("duppage sys_page_map fail\n");
		}
	}
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use vpd, vpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	envid_t child_envid;
	set_pgfault_handler(pgfault);
	child_envid = sys_exofork();
	if(child_envid < 0)
	{
		return -1;	
	}
	if(child_envid == 0)
	{

		thisenv = envs + ENVX(sys_getenvid());
		return 0;
	}
	int dn,pn;
	pte_t pte;
	int addr;
	for(dn = 0; dn < UTOP/PTSIZE; dn++)
	{
		if((((pte_t *)vpd)[dn] & PTE_P) !=0)
		{	
			for(pn = 0; pn < 1024;pn++)
			{
				pte = ((pte_t *)vpt)[dn * 1024 + pn];
				addr = (dn * 1024 + pn) * PGSIZE;
				if(addr == (UTOP - PGSIZE))
				{
					sys_page_map(child_envid, (void*)addr, child_envid, (void*)addr, PTE_P | PTE_U | PTE_W);
				}
				else if(pte & PTE_P)
				{
					if(pte & PTE_U)
					{
						duppage(child_envid, dn * 1024 + pn);
					}
					else
					{
					sys_page_map(child_envid, (void*)addr, child_envid, (void*)addr, PTE_P );
					
					}
				} 
			}
		}
	} 	
	int t;
	t = sys_page_alloc(child_envid, (void *)(UXSTACKTOP - PGSIZE), PTE_P | PTE_U | PTE_W);
	if(t < 0)
	{
		panic("fork sys_page_alloc fail %d\n",t);
	}	
	extern void _pgfault_upcall(void);
	t = sys_env_set_pgfault_upcall(child_envid, _pgfault_upcall);
	if(t < 0)
	{
		panic("fock:set pgfault status fail %d\n",t);
	}	
	t = sys_env_set_status(child_envid,ENV_RUNNABLE);
	if(t < 0)
	{
		panic("fock:set child status fail %d\n",t);
	}	
	
	return child_envid; 
	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
