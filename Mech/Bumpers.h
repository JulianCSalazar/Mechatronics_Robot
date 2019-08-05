/* 
 * File:   Bumpers.h
 * Author: rkgrant
 *
 * Created on November 26, 2017, 8:11 PM
 */

#ifndef BUMPERS_H
#define	BUMPERS_H

#define LEFT_BUMPER 1
#define RIGHT_BUMPER 2
#define BOTH_BUMPERS 3

void Bumpers_Init(void);

unsigned int Bumpers_Read(void);

#endif	/* BUMPERS_H */

