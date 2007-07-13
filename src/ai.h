

#ifndef AI_H
#  define AI_H


typedef enum { TYPE_NULL, TYPE_INT, TYPE_PTR } TaskData;

/* 
 * Basic task
 *  @name is the task's name (function name in Lua)
 *  @target is the target which will depend on the task itself
 */
typedef struct Task {
	struct Task* next;

	char *name;

	
	TaskData dtype;
	union {
		void *target; /* Vector2d, etc... */
		unsigned int ID; /* Pilot ID, etc... */
	};
} Task;


/*
 * init/exit
 */
int ai_init (void);
void ai_exit (void);


#endif /* AI_H */
