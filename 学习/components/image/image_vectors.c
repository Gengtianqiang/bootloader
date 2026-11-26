#include "image_vectors.h"
#include "course_layout.h"
int Image_VectorsValid(uint32_t stack_top, uint32_t reset_handler)
{
    uint32_t entry = reset_handler & ~1UL;
    return stack_top > COURSE_RAM_BASE && stack_top <= COURSE_RAM_END &&
        (stack_top & 7UL) == 0U && (reset_handler & 1UL) != 0U &&
        entry >= COURSE_APP_BASE && entry < COURSE_FLASH_END;
}
