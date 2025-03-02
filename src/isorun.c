#include "isorun.h"
#include "err.h"

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>


int isorun(char *const command[])
{
    assert(NULL != command[0] && "Empty command passed");
    int ret = execvp(command[0], command);
    if (ret) {
        err_msg("failed executing the command");
        return 1;
    }

    return 0;
}
