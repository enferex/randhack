#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

static void print_rand_values(int n_values)
{
    int i;

    printf(">> Printing %d values from rand()...\n", n_values);
    for (i=0; i<n_values; i++) 
      printf("%d\n", rand());
}

int main(void)
{
    int32_t type, n_elts;
    uintptr_t unsafe_state_addr, rand_tbl_ptr;

    /* Call srandom, which sets the rdi register to 
     * the address of unsafe_state glibc struct
     */
    srand(time(NULL));

    /* Read the address of 'unsafe_state' state, defeating ASLR */
    __asm__ __volatile__ ("mov %%rdi, %0\n" : "=r"(unsafe_state_addr));

    /* Dereference the member (second address) in the unsafe_state struct */
    rand_tbl_ptr = *(uintptr_t *)(unsafe_state_addr + sizeof(void *));
        
    /* The second member in 'unsafe_state' is a pointer to the second element of
     * randtbl: randtbl[1].  So we backup int32_t to get to the head of randtbl.
     */
    rand_tbl_ptr = rand_tbl_ptr - sizeof(int32_t);
    printf(">> randtbl located at %p\n", (void *)rand_tbl_ptr);
    printf(">> Before clearing random table\n");
    print_rand_values(10);

    /* How large 'randtbl' can vary.
     * See glibc-2.17 source.
     *
     * Note that the first byte of 'randtbl' is a flag:
     * If the first byte of randtbl is:
     *   -- TYPE_0 (a value of 0) then the table contains 0 32bit integers
     *   -- TYPE_1 (a value of 1) then the table contains 8 32bit integers
     *   -- TYPE_2 (a value of 2) then the table contains 16 32bit integers
     *   -- TYPE_3 (a value of 3) then the table contains 32 32bit integers
     *   -- TYPE_4 (a value of 4) then the table contains 64 32bit integers
     */
    type = *(int32_t *)rand_tbl_ptr;
    n_elts = 0;
    switch (type)
    {
        case 0: n_elts = 0; break;
        case 1: n_elts = 8; break;
        case 2: n_elts = 16; break;
        case 3: n_elts = 32; break;
        case 4: n_elts = 64; break;
    }

    printf(">> Clearing contents of randtbl "
           "which is an array of %d int32 values...\n", n_elts);
    memset((void *)rand_tbl_ptr, 0, n_elts * sizeof(int32_t));
    print_rand_values(10);
    return 0;
}
