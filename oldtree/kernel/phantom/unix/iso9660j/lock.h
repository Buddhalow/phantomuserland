#ifndef _LOCK_H
#define _LOCK_H

#include <BeBuild.h>

typedef struct lock lock;
typedef struct mlock mlock;

struct lock {
	sem_id		s;
	long		c;
};

struct mlock {
	sem_id		s;
};

extern _IMPEXP_KERNEL int	new_lock(lock *l, const char *name);
extern _IMPEXP_KERNEL int	free_lock(lock *l);

#ifndef LOCK
#define	LOCK(l)		if (atomic_add(&l.c, -1) <= 0) acquire_sem(l.s);
#endif
#ifndef UNLOCK
#define	UNLOCK(l)	if (atomic_add(&l.c, 1) < 0) release_sem(l.s);
#endif

extern _IMPEXP_KERNEL int	new_mlock(mlock *l, long c, const char *name);
extern _IMPEXP_KERNEL int	free_mlock(mlock *l);

#define		LOCKM(l,cnt)	acquire_sem_etc(l.s, cnt, 0, 0)
#define		UNLOCKM(l,cnt)	release_sem_etc(l.s, cnt, 0)

#endif
