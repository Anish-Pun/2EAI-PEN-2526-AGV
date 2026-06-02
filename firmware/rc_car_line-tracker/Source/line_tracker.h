#ifndef LINE_TRACKER_H
#define LINE_TRACKER_H

#include <stdbool.h>

void line_tracker_init(void);

bool line_left(void);
bool line_middle(void);
bool line_right(void);

#endif