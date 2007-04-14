

#ifndef AI_H
#  define AI_H


/* 
 * Basic task
 *  @name is the task's name (function name in Lua)
 *  @target is the target which will depend on the task itself
 */
struct Task {
	struct Task* next;

	char *name;

	union {
		void *target; /* Vector2d, etc... */
		unsigned int ID; /* Pilot ID, etc... */
	};
};
typedef struct Task Task;


int ai_init (void);
void ai_exit (void);


#endif /* AI_H */
