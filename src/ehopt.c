/*
 * @Author: hxy
 * @Date: 2023-10-02 18:18:57
 * @LastEditTime: 2023-11-14 16:04:21
 * @Description: 处理异常帧
 */
#include "as.h"
enum frame_state
{
	state_idle,
	state_saw_size,
	state_saw_cie_offset,
	state_saw_pc_begin,
	state_seeing_aug_size,
	state_skipping_aug,
	state_wait_loc4,
	state_saw_loc4,
	state_error,
};

struct cie_info
{
	unsigned code_alignment;
	int z_augmentation;
};

struct frame_data
{
	enum frame_state state;

	int cie_info_ok;
	struct cie_info cie_info;

	symbolS *size_end_sym;
	fragS *loc4_frag;
	int loc4_fix;

	int aug_size;
	int aug_shift;
};

static struct eh_state
{
	struct frame_data eh_data;
	struct frame_data debug_data;
} frame;

void
eh_begin (void) {
	memset (&frame, 0, sizeof (frame));
}


int
check_eh_frame (expressionS *exp, unsigned int *pnbytes) {
    struct frame_data *d;
    /* Don't optimize.  */
    if (flag_traditional_format)
        return 0;
    return 0;
}
