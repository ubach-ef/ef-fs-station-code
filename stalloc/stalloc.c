#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdlib.h>

#include "/usr2/fs/include/params.h"
#include "/usr2/fs/include/fs_types.h"
#include "/usr2/st/include/stparams.h"
#include "/usr2/st/include/stcom.h"
#include "/usr2/st/include/stm_addr.h"

main()
{
    int size, stm_id;
    key_t key;

    key = STM_KEY;
    size = STM_SIZE;

    if( (stm_id = stm_get( key, size)) == -1) {
        fprintf( stderr, " stm_get failed \n");
        exit( -1);
    }
    stm_att( key);
}
