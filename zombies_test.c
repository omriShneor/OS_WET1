#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include "syscalls_zombies.h" 

// ========================================================
// =================testing help functions=================
// ========================================================
void do_long_task(int d) {
    short i;
    int j;
    for (j = 0 ; j < d ; j++)
        for (i = 1 ; i != 0 ; i++) {
            ;
        }
}

int __makezombie;
int MAKEZOMBIE()
{
	//printf("%d trying to fork...", getpid());
	__makezombie = fork();
	if (__makezombie < 0) {/*printf("fork failed\n");*/}
	else if (__makezombie == 0) {/*printf("fork succeeded. i am the son and my id is %d\n", getpid());*/ exit(0);}
	do_long_task(2000);
	return __makezombie;
}

#define ASSERT(condition) if (!(condition)) {printf("ASSERT failed in line %d\n", __LINE__ ); fflush(stdout); /*return false;*/}

#define RUN_TEST(name) printf("TESTING %s....", #name); fflush(stdout);		\
if (name()) { printf("SUCCESS\n"); fflush(stdout);} else {printf("FAIL\n"); fflush(stdout);}	\

int __wait;
#define WAIT_FOR_SONS_TO_DIE() while (wait(&__wait) != -1);

int __doinnewprocess;
#define DO_IN_NEW_PROCESS __doinnewprocess = fork(); \
if (__doinnewprocess == 0)	\

// ========================================================
// ================/testing help functions=================
// ========================================================







//===================================================
//===================================================
//===================================================
//===================================================

bool test_functionality(){
	DO_IN_NEW_PROCESS
	{
		set_max_zombies(4, getpid());

		ASSERT(MAKEZOMBIE() > 0);
		ASSERT(MAKEZOMBIE() > 0);
		ASSERT(MAKEZOMBIE() > 0);
		ASSERT(MAKEZOMBIE() > 0);
		ASSERT(MAKEZOMBIE() > 0);

		ASSERT(fork() == -1);	// should fail (have 5 zombies)

		int status;
		wait(&status);

		int result = MAKEZOMBIE(); // should work (have 4 zombies)
		ASSERT(result > 0);

		exit(0);
	}
	WAIT_FOR_SONS_TO_DIE();


	// test with limit = 0
	DO_IN_NEW_PROCESS
	{
		set_max_zombies(0, getpid());

		int pid1 = MAKEZOMBIE(); // should work
		ASSERT(pid1 > 0);

		ASSERT(fork() == -1);	// should fail (have 1 zombies)		

		int status;
		wait(&status);

		int result = MAKEZOMBIE(); // should work (have 0 zombies)
		ASSERT(result > 0);

		exit(0);
	}
	WAIT_FOR_SONS_TO_DIE();


	return true;
}


bool test_set_max_zombies()
{
	DO_IN_NEW_PROCESS
	{
		// son code
		ASSERT(set_max_zombies(-1, getpid()) == -1);
		ASSERT(errno = EINVAL);

		ASSERT(set_max_zombies(2, -1) == -1);
		ASSERT(errno = ESRCH);

		ASSERT(set_max_zombies(2, getpid()) == 0);

		exit(0);
	}

	WAIT_FOR_SONS_TO_DIE();
	return true;
}
bool test_get_max_zombies()
{
	DO_IN_NEW_PROCESS
	{
		// no limit was defined
		ASSERT(get_max_zombies() == -1);
		ASSERT(errno = EINVAL);

		set_max_zombies(1337, getpid());

		ASSERT(get_max_zombies() == 1337);

		exit(0);
	}

	WAIT_FOR_SONS_TO_DIE();
	return true;
}
bool test_get_zombies_count()
{
	DO_IN_NEW_PROCESS
	{
		// no limit was defined
		ASSERT(get_zombies_count(getpid()) == 0);

		set_max_zombies(3, getpid());

		MAKEZOMBIE();
		ASSERT(get_zombies_count(getpid()) == 1);

		MAKEZOMBIE();
		ASSERT(get_zombies_count(getpid()) == 2);

		MAKEZOMBIE();
		ASSERT(get_zombies_count(getpid()) == 3);

		MAKEZOMBIE();
		ASSERT(get_zombies_count(getpid()) == 4);

		MAKEZOMBIE(); // should fail now
		ASSERT(get_zombies_count(getpid()) == 4);

		MAKEZOMBIE(); // should fail now
		ASSERT(get_zombies_count(getpid()) == 4);

		exit(0);
	}

	WAIT_FOR_SONS_TO_DIE();
	return true;
}


bool test_get_zombie_pid()
{
	DO_IN_NEW_PROCESS
	{
		ASSERT(get_zombie_pid(-1) == -1 && errno == EINVAL);	
		ASSERT(get_zombie_pid(0) == -1 && errno == EINVAL);	

		// reverse order zombies
		set_max_zombies(3, getpid());

		ASSERT(get_zombie_pid(0) == -1 && errno == EINVAL);	

		int pid1 = MAKEZOMBIE();
		ASSERT(get_zombie_pid(0) == pid1);	
		ASSERT(get_zombie_pid(1) == -1 && errno == EINVAL);	

		int pid2 = MAKEZOMBIE();
		int pid3 = MAKEZOMBIE();


		ASSERT(get_zombie_pid(0) == pid1);
		ASSERT(get_zombie_pid(1) == pid2);
		ASSERT(get_zombie_pid(2) == pid3);	
		ASSERT(get_zombie_pid(3) == -1 && errno == EINVAL);	

		exit(0);
	}
	WAIT_FOR_SONS_TO_DIE();

	return true;
}
bool test_get_zombie_pid_reverse()
{
	DO_IN_NEW_PROCESS
	{
		// reverse order zombies
		set_max_zombies(3, getpid());

		pid_t pid1 = fork();
		if (pid1 == 0)
		{
			do_long_task(3000);
			exit(0);
		}


		pid_t pid2 = fork();
		if (pid2 == 0)
		{
			do_long_task(1500);
			exit(0);
		}

		pid_t pid3 = fork();
		if (pid3 == 0)
		{
			exit(0);
		}

		do_long_task(4500);

		ASSERT(get_zombie_pid(0) == pid3);
		ASSERT(get_zombie_pid(1) == pid2);
		ASSERT(get_zombie_pid(2) == pid1);

		exit(0);
	}

	WAIT_FOR_SONS_TO_DIE();
	return true;
}



int __a, __b;
bool test_give_up_zombies()
{
	DO_IN_NEW_PROCESS {
		set_max_zombies(10, getpid());

		int son_pid = fork();
		if (son_pid == 0)
		{
			set_max_zombies(5, getpid());
			int pid0 = MAKEZOMBIE();
			int pid1 = MAKEZOMBIE();

			ASSERT(get_zombie_pid(0) == pid0);
			ASSERT(get_zombie_pid(1) == pid1);
			ASSERT(get_zombies_count(getpid()) == 2);

			do_long_task(10000); // wait for someone to move 2 more zombies to me

			ASSERT(get_zombie_pid(0) == pid0);
			ASSERT(get_zombie_pid(1) == pid1);
			ASSERT(get_zombie_pid(2) != -1);
			ASSERT(get_zombie_pid(3) != -1);
			ASSERT(get_zombie_pid(4) == -1);
			ASSERT(get_zombies_count(getpid()) == 4);

			exit(0);
		}

		do_long_task(5000); // wait for son to create his 2 zombies

		// create 2 zombies
		__a = MAKEZOMBIE();
		__b = MAKEZOMBIE();
		ASSERT(get_zombie_pid(0) == __a);
		ASSERT(get_zombie_pid(1) == __b);
		ASSERT(get_zombies_count(getpid()) == 2);
		ASSERT(give_up_zombie(0, son_pid) == 0);

		// move these 2 zombies to son
		ASSERT(give_up_zombie(2, son_pid) == 0);
		ASSERT(get_zombies_count(getpid()) == 0);

		WAIT_FOR_SONS_TO_DIE();
		exit(0);
	}
	WAIT_FOR_SONS_TO_DIE();
}
bool test_give_up_zombies_self1(){
	DO_IN_NEW_PROCESS
	{
		// have 4 zombies. move 1 to myself
		set_max_zombies(4, getpid());

		int pid1 = MAKEZOMBIE();
		int pid2 = MAKEZOMBIE();
		int pid3 = MAKEZOMBIE();
		int pid4 = MAKEZOMBIE();

		ASSERT(pid1 == get_zombie_pid(0));
		ASSERT(pid2 == get_zombie_pid(1));
		ASSERT(pid3 == get_zombie_pid(2));
		ASSERT(pid4 == get_zombie_pid(3));

		ASSERT(give_up_zombie(1, getpid()) == 0);

		ASSERT(get_zombie_pid(0) == pid2);
		ASSERT(get_zombie_pid(1) == pid3);
		ASSERT(get_zombie_pid(2) == pid4);
		ASSERT(get_zombie_pid(3) == pid1);

		exit(0);
	}
	WAIT_FOR_SONS_TO_DIE();
	return true;
}
bool test_give_up_zombies_self2(){
	DO_IN_NEW_PROCESS
	{
		// have 4 zombies. move 2 to myself
		set_max_zombies(4, getpid());

		int pid1 = MAKEZOMBIE();
		int pid2 = MAKEZOMBIE();
		int pid3 = MAKEZOMBIE();
		int pid4 = MAKEZOMBIE();

		ASSERT(pid1 == get_zombie_pid(0));
		ASSERT(pid2 == get_zombie_pid(1));
		ASSERT(pid3 == get_zombie_pid(2));
		ASSERT(pid4 == get_zombie_pid(3));

		ASSERT(give_up_zombie(2, getpid()) == 0);

		ASSERT(get_zombie_pid(0) == pid3);
		ASSERT(get_zombie_pid(1) == pid4);
		ASSERT(get_zombie_pid(2) == pid1);
		ASSERT(get_zombie_pid(3) == pid2);

		exit(0);
	}
	WAIT_FOR_SONS_TO_DIE();
	return true;
}
bool test_give_up_zombies_self3(){
	DO_IN_NEW_PROCESS
	{
		// have 4 zombies. move all to myself
		set_max_zombies(4, getpid());

		int pid1 = MAKEZOMBIE();
		int pid2 = MAKEZOMBIE();
		int pid3 = MAKEZOMBIE();
		int pid4 = MAKEZOMBIE();

		ASSERT(pid1 == get_zombie_pid(0));
		ASSERT(pid2 == get_zombie_pid(1));
		ASSERT(pid3 == get_zombie_pid(2));
		ASSERT(pid4 == get_zombie_pid(3));

		ASSERT(give_up_zombie(4, getpid()) == 0);

		ASSERT(get_zombie_pid(0) == pid1);
		ASSERT(get_zombie_pid(1) == pid2);
		ASSERT(get_zombie_pid(2) == pid3);
		ASSERT(get_zombie_pid(3) == pid4);

		exit(0);
	}
	WAIT_FOR_SONS_TO_DIE();
	return true;
}
bool test_give_up_zombies_general(){
	DO_IN_NEW_PROCESS
	{
		// undefined
		ASSERT(give_up_zombie(1, getpid()) == -1 && errno == EINVAL); // n is greater than current zombies count (0)
		ASSERT(give_up_zombie(0, getpid()) == -1 && errno == EINVAL); // adopter is undefined
		ASSERT(give_up_zombie(0, -1) == -1 && errno == ESRCH); // pid < 0
		
		ASSERT(set_max_zombies(2, getpid()) == 0);

		ASSERT(give_up_zombie(1, getpid()) == -1 && errno == EINVAL);
		ASSERT(give_up_zombie(0, getpid()) == 0);
		ASSERT(give_up_zombie(-1, getpid()) == -1 && errno == EINVAL);
		ASSERT(give_up_zombie(0, -1) == -1 && errno == ESRCH);

		MAKEZOMBIE();

		ASSERT(give_up_zombie(0, getpid()) == 0);
		ASSERT(give_up_zombie(1, getpid()) == 0);
		ASSERT(give_up_zombie(2, getpid()) == -1 && errno == EINVAL);
		ASSERT(give_up_zombie(-1, getpid()) == -1 && errno == EINVAL);
		ASSERT(give_up_zombie(0, -1) == -1 && errno == ESRCH);

		exit(0);
	}
	WAIT_FOR_SONS_TO_DIE();
	return true;
}


bool test_no_zombie_inheritage()
{
	DO_IN_NEW_PROCESS
	{
		pid_t father_pid = getpid();
		set_max_zombies(15, getpid());
		pid_t father_zombie1 = MAKEZOMBIE();
		pid_t father_zombie2 = MAKEZOMBIE();
		pid_t father_zombie3 = MAKEZOMBIE();

		ASSERT(get_zombie_pid(0) == father_zombie1);
		ASSERT(get_zombie_pid(1) == father_zombie2);
		ASSERT(get_zombie_pid(2) == father_zombie3);
		ASSERT(get_zombie_pid(3) == -1 && errno == EINVAL);	
		ASSERT(get_zombies_count(getpid()) == 3);

		if (fork() == 0)
		{
			ASSERT(get_max_zombies() == -1 && errno == EINVAL);
			ASSERT(get_zombies_count(getpid()) == 0);
			ASSERT(get_zombie_pid(0) == -1 && errno == EINVAL);
			ASSERT(give_up_zombie(1, father_pid) == -1 && errno == EINVAL);
			ASSERT(give_up_zombie(1, getpid()) == -1 && errno == EINVAL);

			ASSERT(set_max_zombies(5, getpid()) == 0);
			ASSERT(get_zombie_pid(0) == -1 && errno == EINVAL);
			pid_t my_zombie1 = MAKEZOMBIE();
			pid_t my_zombie2 = MAKEZOMBIE();

			ASSERT(get_zombie_pid(0) == my_zombie1);
			ASSERT(get_zombie_pid(1) == my_zombie2);
			ASSERT(get_zombie_pid(2) == -1 && errno == EINVAL);
			ASSERT(get_zombies_count(getpid()) == 2);

			exit(0);
		}
		do_long_task(3000);
		ASSERT(get_zombies_count(getpid()) == 3);
		ASSERT(get_zombie_pid(0) == father_zombie1);
		ASSERT(get_zombie_pid(1) == father_zombie2);
		ASSERT(get_zombie_pid(2) == father_zombie3);
		ASSERT(get_zombie_pid(3) == -1 && errno == EINVAL);	

		WAIT_FOR_SONS_TO_DIE();
		exit(0);
	}
	WAIT_FOR_SONS_TO_DIE();
	return true;
}



bool small_unnamed_unimportant_test()
{
	DO_IN_NEW_PROCESS 
	{
		set_max_zombies(10, getpid());
		ASSERT(get_max_zombies() == 10);
		if (fork() == 0)
		{
			ASSERT(get_max_zombies() == -1);
			ASSERT(get_zombies_count(getpid()) == 0);
			set_max_zombies(33, getpid());

			ASSERT(get_max_zombies() == 33);
			exit(0);
		}
		do_long_task(1500);

		ASSERT(get_zombies_count(getpid()) == 1);
		WAIT_FOR_SONS_TO_DIE();

		ASSERT(get_zombies_count(getpid()) == 0);
		ASSERT(get_max_zombies() == 10);
		exit(0);
	}
	WAIT_FOR_SONS_TO_DIE();
	return true;
}






bool test_give_up_zombies1_helper(bool father_dies_before_son)
{
	// father creates zombies, son creates zombies, father moves zombies to son (and father dies before son?)
	int i;
	DO_IN_NEW_PROCESS {
		ASSERT(set_max_zombies(10, getpid()) == 0);

		pid_t father_arr[4];
		for (i = 0; i < 4; i++)
		{
			father_arr[i] = MAKEZOMBIE();
			ASSERT(father_arr[i] > 0);
		}

		int son_pid = fork();
		if (son_pid == 0)
		{
			ASSERT(set_max_zombies(100, getpid()) == 0);

			pid_t son_arr[20];
			for (i = 0; i < 20; i++)
			{
				son_arr[i] = MAKEZOMBIE();
				ASSERT(son_arr[i] > 0);
			}
			ASSERT(get_zombies_count(getpid()) == 20);

			for(i = 0; i < 20; i++)
				ASSERT(get_zombie_pid(i) == son_arr[i]);
			
			sleep(20);	// wait for someone to move 3 more zombies to me
			ASSERT(get_zombies_count(getpid()) == 23);

			for(i = 0; i < 20; i++)
				ASSERT(get_zombie_pid(i) == son_arr[i]);
			for(i = 0; i < 3; i++)
				ASSERT(get_zombie_pid(20 + i) == father_arr[i]);
			
			exit(0);
		}

		sleep(10);	// wait for son to create his 20 zombies
		// move 3 zombies to son
		ASSERT(get_zombies_count(getpid()) == 4);
		ASSERT(give_up_zombie(3, son_pid) == 0);
		ASSERT(get_zombies_count(getpid()) == 1);
		ASSERT(get_zombie_pid(0) == father_arr[3]);
		ASSERT(get_zombie_pid(1) == -1 && errno == EINVAL);
		if (father_dies_before_son == false)
			WAIT_FOR_SONS_TO_DIE(); // the 3 zombies that i moved to son are now killed (not unless your implementation is correct :) )
		exit(0);
	}
	WAIT_FOR_SONS_TO_DIE();
}

bool test_give_up_zombies1()
{
	// father creates zombies, son creates zombies, father moves zombies to son (and father dies before son?)
	return test_give_up_zombies1_helper(false) && test_give_up_zombies1_helper(true);
}




int main() {
	// test fork, wait, etc..
	RUN_TEST(test_functionality);

	// test the syscalls
	RUN_TEST(test_set_max_zombies);
	RUN_TEST(test_get_max_zombies);
	RUN_TEST(test_get_zombies_count);
	RUN_TEST(test_get_zombie_pid);
	RUN_TEST(test_get_zombie_pid_reverse);

	RUN_TEST(test_give_up_zombies);
	RUN_TEST(test_give_up_zombies_self1);
	RUN_TEST(test_give_up_zombies_self2);
	RUN_TEST(test_give_up_zombies_self3);
	RUN_TEST(test_give_up_zombies_general);

	RUN_TEST(test_no_zombie_inheritage);
	RUN_TEST(small_unnamed_unimportant_test);


	RUN_TEST(test_give_up_zombies1); // this one takes around 50 seconds so be patient

	return 0;
}






/* 		The cake is a lie


            ,:/+/-
            /M/              .,-=;//;-
       .:/= ;MH/,    ,=/+%$XH@MM#@:
      -$##@+$###@H@MMM#######H:.    -/H#
 .,H@H@ X######@ -H#####@+-     -+H###@X
  .,@##H;      +XM##M/,     =%@###@X;-
X%-  :M##########$.    .:%M###@%:
M##H,   +H@@@$/-.  ,;$M###@%,          -
M####M=,,---,.-%%H####M$:          ,+@##
@##################@/.         :%H##@$-
M###############H,         ;HM##M$=
#################.    .=$M##M$=
################H..;XM##M$=          .:+
M###################@%=           =+@MH%
@#################M/.         =+H#X%=
=+M###############M,      ,/X#H+:,
  .;XM###########H=   ,/X#H+:;
     .=+HM#######M+/+HM@+=.
         ,:/%XM####H/.
              ,.:=-.


*/
