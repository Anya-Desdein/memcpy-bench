#define _GNU_SOURCE 	// GNU-specific extensions like
			// sched_setaffinity() sched_getaffinity() 
			// these are not part of standard POSIX API

#include <sched.h>      // Set thread's CPU affinity
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <stdint.h>
#include <inttypes.h>

#include <dlfcn.h> 	// dynamic linking library 

#include "perf_utils.h"
#include "memcpy.h"

#include "assert.h"

#define TEXT_MAX_SIZE (1 << 8)
#define MALLOC_SIZE   (1 << 12)

size_t clock_frequency = 0;

int main(void) {

	// PURPOSE OF THIS SECTION: Pin this process to only one core in order to measure performance

	
	/*
		I think that I've finally understood how cpu_set mask works
		Each bit corresponds to a certain CPU and can be 1 or 0
		
		It's arranged by their indexes (logical CPU ID)
		
		CPU 3 	CPU 2 	CPU 1 	CPU 0
		 0        1      1       0 

		In this example only CPU 2 and 1 can run the process
		
	*/	
	cpu_set_t cpu_set; // this is a mask
	size_t cpuset_size = sizeof(cpu_set);
	
	int all_cpus    = sysconf(_SC_NPROCESSORS_CONF);
	int online_cpus = sysconf(_SC_NPROCESSORS_ONLN);

	CPU_ZERO( &cpu_set);		      // clear set and inits struct
	CPU_SET ( (online_cpus-1), &cpu_set); // assign last

	pid_t pid = getpid();
	
	printf("CPU_SET MASK SIZE: %zu BYTES\n",    cpuset_size);
	printf("TOTAL CPU COUNT: %d, ONLINE: %d\n", all_cpus, online_cpus);
	printf("CURRENT PROCESS ID: %d\n",	    (int)pid);
	

	void *pu = dlopen("./perf_utils.so",  RTLD_NOW);
	if (!pu) {
		printf("dlopen error: %s\n", dlerror());
		return 1;
	}
	
	Cpustat cpustat;
	cpuid_gcc_t cpuid_gcc = (cpuid_gcc_t) dlsym(pu, "cpuid_gcc");

	if (!cpuid_gcc) {
		printf("dlsym error: %s\n", dlerror());
		return 1;
	}

	// Check if rdtsc and cpuid are available
	cpustat = cpuid_gcc();
	if (cpustat.has_rdtsc == 0) {
		printf("No rdtsc\n");
		return 1;
	}

	if (cpustat.has_invariant_tsc != 0) {
		printf("Invariant TSC available\n");
		if (cpustat.clock_rate != 0) {
			clock_frequency = cpustat.clock_rate;
			printf("Clock frequency: %zu\n", clock_frequency);
		} else {
			printf("Clock frequency equal 0\n");
		}
		
	} else {	
		printf("Invariant TSC unavailable\n");
	} 

	if (sched_setaffinity(pid, cpuset_size, &cpu_set) == -1) {
		perror("sched_setaffinity");
		return 1;
	}	
	
	if (sched_getaffinity(pid, cpuset_size, &cpu_set) == -1) {
		perror("sched_setaffinity");
		return 1;
	} printf("Making sure that cpu affinity was set properly:\n");
	
	for (size_t i=0; i<cpuset_size; i++) {
		if ( (CPU_ISSET(i, &cpu_set)) ) { 
			printf("1");
		} else { 
			printf("0");
		}
		
		if (!((i+1)%4))
			printf(" ");

		if (!((i+1)%48))
			printf("\n");
	} printf("\n");

	typedef struct {
		char   text[TEXT_MAX_SIZE];
		size_t size;
	} Testdata;

	union {
		Testdata longtxt;
		Testdata shorttxt;
		Testdata lol;
		Testdata orang;
	} Tests;

	int mcount = 4;
	void *f[mcount];

	f[0] = dlopen("./cmemcpy.so",  RTLD_NOW);
	f[1] = dlopen("./cmemcpy2.so", RTLD_NOW);
	f[2] = dlopen("./cmemcpy3.so", RTLD_NOW);
	f[3] = dlopen("./cmemcpy4.so", RTLD_NOW);
	for (int i=0; i < mcount; i++) {
		if (!f[i]) {
			perror("dlopen");
			return 1;
		}
	}

	char *err[4];
	memcpy_t cmemcpy  = (memcpy_t)dlsym(f[0], "cmemcpy" );
	err[0] = dlerror();
	memcpy_t cmemcpy2 = (memcpy_t)dlsym(f[1], "cmemcpy2");
	err[1] = dlerror();
	memcpy_t cmemcpy3 = (memcpy_t)dlsym(f[2], "cmemcpy3");
	err[2] = dlerror();
	memcpy_t cmemcpy4 = (memcpy_t)dlsym(f[3], "cmemcpy4");
	err[3] = dlerror();
	for (int i=0; i < mcount; i++) {
		if (err[i]) {
			printf("dlsym error: %s\n", err[i]);
			return 1;
		}

	}

	char *dest = (char *)malloc(MALLOC_SIZE);
	assert(dest && "Malloc failed");
	
	strcpy(
		Tests.longtxt.text,
		"declare p as pointer to function (pointer to function (double, float) "
		"returning pointer to void) returning pointer to const pointer to pointer "
		"to function() returning int =\n"
	);

	Tests.longtxt.size = strlen(Tests.longtxt.text);
	cmemcpy(dest, Tests.longtxt.text, Tests.longtxt.size); 
	size_t addr=Tests.longtxt.size;
	
	strcpy(
		Tests.shorttxt.text, 
		"int(** const *(*p)(void*(*)(double, float)))())\n");

	Tests.shorttxt.size = strlen(Tests.shorttxt.text);
	cmemcpy2(dest+addr, Tests.shorttxt.text, Tests.shorttxt.size);
	addr+=Tests.shorttxt.size;

	strcpy(
		Tests.lol.text,
		">_<\n");
	Tests.lol.size = strlen(Tests.lol.text);

	cmemcpy3(dest +
		 addr, 

		 Tests.lol.text,  
		 
		 Tests.lol.size);

	addr+=Tests.lol.size;
	
	strcpy(
		Tests.orang.text,
		"Did you know that orangutans use medicine?\n");
	Tests.orang.size = strlen(Tests.orang.text);

	cmemcpy4(dest +
		 addr, 

		 Tests.orang.text,  
		 
		 Tests.orang.size);

	addr+=Tests.orang.size;

	*(dest + addr) = '\0';
	printf("%s", dest);
	free(dest);
	return 0;
}
