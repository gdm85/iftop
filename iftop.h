/*
 * iftop.h:
 *
 */

#ifndef __IFTOP_H_ /* include guard */
#define __IFTOP_H_

/* 5 * 60 / 3  */
#define HISTORY_LENGTH  100
#define RESOLUTION 3

typedef struct {
    long recv[HISTORY_LENGTH];
    long sent[HISTORY_LENGTH];
    int last_write;
} history_type;

void tick();

#endif /* __IFTOP_H_ */
