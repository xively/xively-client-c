#include <xi_fs_bsp_to_xi_mapping.h>

/* helper function that translates the errno errors to the xi_bsp_io_fs_state_t values */
xi_state_t xi_fs_bsp_io_fs_2_xi_state( xi_bsp_io_fs_state_t bsp_state_value )
{
    xi_state_t ret = XI_STATE_OK;

    switch ( bsp_state_value )
    {
        case XI_BSP_IO_FS_STATE_OK:
            ret = XI_STATE_OK;
            break;
        case XI_BSP_IO_FS_ERROR:
            ret = XI_FS_ERROR;
            break;
        case XI_BSP_IO_FS_INVALID_PARAMETER:
            ret = XI_INVALID_PARAMETER;
            break;
        case XI_BSP_IO_FS_RESOURCE_NOT_AVAILABLE:
            ret = XI_FS_RESOURCE_NOT_AVAILABLE;
            break;
        case XI_BSP_IO_FS_OUT_OF_MEMORY:
            ret = XI_OUT_OF_MEMORY;
            break;
        default:
            /** IF we're good engineers, then this should never happen */
            ret = XI_INTERNAL_ERROR;
            break;
    }

    return ret;
}
