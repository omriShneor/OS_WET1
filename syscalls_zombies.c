/*
 * zombies.c
 *
 *  Created on: 16 באפר׳ 2017
 *      Author: Admin
 */
#include <linux/sched.h>

int sys_set_max_zombies(int max_z, pid_t pid){
	if(max_z < 0){
		return -EINVAL;
	}if(pid < 0){
		return -ESRCH;
	}
	task_t* p = find_task_by_pid(pid);
	p->max_z = max_z;
	return 0;
}

int sys_get_max_zombies(void){
	if(current->max_z == -1){
		return -EINVAL;
	}
	return current->max_z;
}

int sys_get_zombies_count(pid_t pid){
	if(pid < 0){
		return -ESRCH;
	}
	task_t* p = find_task_by_pid(pid);
	if(p->max_z == -1) return 0;
	return p->num_zombies;
}

pid_t sys_get_zombie_pid(int n){
	if(n<0){
		return -EINVAL;
	}
	if(current->max_z == -1){
		return -EINVAL;
	}
	if(n>=current->num_zombies){
		return -EINVAL;
	}
	list_t* node;
	task_t* task;
	int i = current->num_zombies;
	list_for_each(node,&current->zombies){
		if(i==n+1){
			task = list_entry(node,task_t,zombies);
			return task->pid;
		}
		i--;
	}
}

int sys_give_up_zombie(int n, pid_t adopter_pid){
	if(n < 0 || (n > current->num_zombies)){
		return -EINVAL;
	}
	if(adopter_pid < 0){
		return -ESRCH;
	}
	task_t* adopter = find_task_by_pid(adopter_pid);
	if(adopter->max_z == -1){
		return -EINVAL;
	}
	int i;
	adopter->num_zombies+=n;
	for(i = 0; i < n; i++){
		int z_pid = sys_get_zombie_pid(i);
		task_t* tsk = find_task_by_pid(z_pid);
		list_del(&tsk->zombies);
		list_add(&tsk->zombies,&adopter->zombies);
		REMOVE_LINKS(tsk);
		tsk->p_pptr = adopter;
		tsk->p_opptr = adopter;
		SET_LINKS(tsk);
	}
	current->num_zombies-=n;
	return 0;
}



