#ifndef FSM_GH_H
#define FSM_GH_H


#define _DEFAULT_SOURCE // Feature Test Macro: expose common BSD and POSIX functions ('usleep')

/* --- FSM State Declarations --- */
int state_exit(void);
int state_start(void);
int state_check_repo(void);
int state_init(void);
int state_menu(void);


#endif /* FSM_GH_H */

