/*
 * syscalls_zombies.h
 *
 *  Created on: 17 באפר׳ 2017
 *      Author: Admin
 */

#ifndef SYSCALLS_ZOMBIES_H_
#define SYSCALLS_ZOMBIES_H_

#include <errno.h>
#include <termios.h>

int set_max_zombies(int max_z, pid_t pid){
	long int res;
	__asm__(
		"int $0x80;"
		: "=a"(res)
		: "0"(243), "b"(max_z), "c"(pid)
		: "memory"
	);
	if(res < 0){
		errno = -res;
		res = -1;
	}
	return res;
}

int get_max_zombies(void){
	long int res;
	__asm__(
		"int $0x80;"
		: "=a"(res)
		: "0"(244)
		: "memory"
	);
	if(res < 0){
		errno = -res;
		res = -1;
	}
	return res;
}

int get_zombies_count(pid_t pid){
	long int res;
	__asm__(
		"int $0x80;"
		: "=a"(res)
		: "0"(245), "b"(pid)
		: "memory"
	);
	if(res < 0){
		errno = -res;
		res = -1;
	}
	return res;
}

pid_t get_zombie_pid(int n){
	long int res;
	__asm__(
		"int $0x80;"
		: "=a"(res)
		: "0"(246), "b"(n)
		: "memory"
	);
	if(res < 0){
		errno = -res;
		res = -1;
	}
	return (pid_t)res;
}

int give_up_zombie(int n, pid_t adopter_pid){
	long int res;
	__asm__(
		"int $0x80;"
		: "=a"(res)
		: "0"(247), "b"(n), "c"(adopter_pid)
		: "memory"
	);
	if(res < 0){
		errno = -res;
		res = -1;
	}
	return res;
}


#endif /* SYSCALLS_ZOMBIES_H_ */
