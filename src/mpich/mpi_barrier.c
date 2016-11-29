#include "cortex/cortex.h"
#include "cortex/debug.h"
#include "cortex/profile.h"

#define MPICH_BARRIER_TAG -1234

/**
 * This translates MPI_Barrier calls into a series of
 * point to point calls (MPI_Sendrecv). The following
 * algorithm was found in the Mpich implementation.
 */
int cortex_mpich_translate_MPI_Barrier(const dumpi_barrier *prm, 
			uint16_t thread, 
			const dumpi_time *cpu, 
			const dumpi_time *wall,
			const dumpi_perfinfo *perf,
			void *uarg) {

	thread = ((cortex_dumpi_profile*)uarg)->rank;

	INFO("Barrier using Mpich's barrier algorithm\n");

	int rank, size, src, dst, mask;
	rank = thread;
	cortex_comm_get_size(uarg, prm->comm, &size);
	mask = 0x1;

	dumpi_sendrecv args;
	args.comm = prm->comm;
	args.sendcount = 0;
	args.sendtype = DUMPI_BYTE;
	args.sendtag = MPICH_BARRIER_TAG;
	args.recvcount = 0;
	args.recvtype = DUMPI_BYTE;
	args.recvtag = MPICH_BARRIER_TAG;
	args.status = NULL;

	while(mask < size) {
		dst = (rank + mask) % size;
		src = (rank - mask + size) % size;

		args.dest = dst;
		args.source = src;

		cortex_post_MPI_Sendrecv(&args, thread, cpu, wall, perf, uarg);

		mask <<= 1;
	}

	return 0;
}
