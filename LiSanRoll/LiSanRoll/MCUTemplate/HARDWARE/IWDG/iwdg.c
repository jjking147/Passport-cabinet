#include "iwdg.h"

void IWDG_Init(u8 prer, u16 rlr)
{
    fwdgt_write_enable();
    fwdgt_prescaler_value_config(prer);
    fwdgt_reload_value_config(rlr);
    fwdgt_counter_reload();
    fwdgt_enable();
}

void IWDG_Feed(void)
{
    fwdgt_counter_reload();
}
