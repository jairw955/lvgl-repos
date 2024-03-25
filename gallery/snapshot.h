#ifndef __SNAPSHOT_H__
#define __SNAPSHOT_H__


#define take_shot       _take_shot(__func__)
#define take_shot_now   _take_shot_now(__func__)
void _take_shot(const char *func);
void _take_shot_now(const char *func);
void take_shot_mark(void);

#endif

