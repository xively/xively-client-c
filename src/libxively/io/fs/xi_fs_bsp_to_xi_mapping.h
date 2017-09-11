#include "xively_error.h"
#include "xi_bsp_io_fs.h"

/* helper function that translates the errno errors to the xi_bsp_io_fs_state_t values */
xi_state_t xi_fs_bsp_io_fs_2_xi_state( xi_bsp_io_fs_state_t bsp_state_value );
