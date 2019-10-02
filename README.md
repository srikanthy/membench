## membench - Memory Microbenchmark

 A simple memory benchmark to understand the implications of cache memory on strided access

 membench:
	+ for array A of length L from 4KB to 64MB by 2x
		+ for stride s from 4 bytes (1 word) to L/2 by 2x
			+ time following loop
				+ for i from 1 to niterations
					+ for index 0 to L by s
						+ load A[i] from memory
