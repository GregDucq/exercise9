#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <string.h>

#define MAX_LENGTH 100
#define DEF_MODE 0666

/* result stores either the max value or the sum of all values in the array */
/* current_element is the current index of the array a given thread should be looking at */
/* nums is the array of numbers the program should process */
static int result = 0, current_element = 0, nums[MAX_LENGTH];
/* lock mechanism(?) */
static pthread_mutex_t mutex;

/* add_elements is the function threads will use to add a random number to the array of numbers */
void *add_elements(void *target){
  /* stores the maxiumum number of values the array of numbers will store */
	int *end_target = (int *)target;
	
	while(1){
	  pthread_mutex_lock(&mutex);
	  /* allows the thread to add a value to the array if possible */
	  if(current_element != *end_target){
		int num = rand();
		nums[current_element] = num % 1000000;
		/* Move to the next available index in the array */
		current_element++;
	  }

	  /* Otherwise, exit the function */
	  else{
		pthread_mutex_unlock(&mutex);

		return NULL;
	  }
	  
	  pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

/* Finds the maximum number in the array or the sum of all values in the array */
void *action(void *choice_pt){
  /* Tracks whether maxiumum or the sum should be found */
	int *choice = (int *)choice_pt;

   	while(1){
	  pthread_mutex_lock(&mutex);
	  /* If there is a number in the array to process */
	  if(nums[current_element]){
		/* Determine if the value is the largest in the array if the maximum should be found */
	    if(*choice == 1 && nums[current_element] > result){
		  result = nums[current_element];
		}

		/* Calculate the sum if the sum should be found */
		if(*choice == 2){
		  result = (result + nums[current_element]) % 1000000;
		}

		/* Move to the next value in the array */
		current_element++;
	  }

	  else{
		pthread_mutex_unlock(&mutex);

		return NULL;
	  }
	  
	  pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

struct timeval tv_delta(struct timeval start, struct timeval end){
	struct timeval delta = end;

	delta.tv_sec -= start.tv_sec;
	delta.tv_usec -= start.tv_usec;
	if(delta.tv_usec < 0){
	  delta.tv_usec += 1000000;
	  delta.tv_sec--;
	}

	return delta;
}

int main(int argc, char *argv[]){
	int i, target = atoi(argv[1]), choice = atoi(argv[4]);
	pthread_t tids[MAX_LENGTH + 1];
	struct rusage start_ru, end_ru;
	struct timeval start_wall, end_wall;
	struct timeval diff_ru_utime, diff_wall, diff_ru_stime;

	/* Start timing */
	getrusage(RUSAGE_SELF, &start_ru);
    gettimeofday(&start_wall, NULL);
	
	printf("Elements: %d Threads: %d, Seed: %d\n", atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
	
	pthread_mutex_init(&mutex, NULL);
	printf("Assigning values\n");
	srand(atoi(argv[3]));

	/* Create threads that will generate the random values for the array */
	for(i = 0; i < atoi(argv[2]); i++){
	  /* printf("Creating thread %d\n", i);*/
	  pthread_create(&tids[i], NULL, add_elements, &target);
	}

	/* Reap the threads */
	for(i = 0; i < atoi(argv[2]); i++){
	  /*printf("killing thread %d\n", i);*/
	  pthread_join(tids[i], NULL);
	}

	/* Reset current index value */
	current_element = 0;
	
	printf("Performing action\n");
	
	/* Create threads that will perform function (max or sum) */
	for(i = 0; i < atoi(argv[2]); i++){
	  /*  printf("Creating thread %d\n", i);*/
	  pthread_create(&tids[i], NULL, action, &choice);
	}

	/* Reap the threads */
	for(i = 0; i < atoi(argv[2]); i++){
	  /* printf("killing thread %d\n", i); */
	  pthread_join(tids[i], NULL);
	}

	
	/* Print the numbers the threads generated */
	/*for(i = 0; i < atoi(argv[1]); i++){
	  printf("%d ", nums[i]);
	  }*/

	/* Get end time */
	gettimeofday(&end_wall, NULL);
	getrusage(RUSAGE_SELF, &end_ru);

	/* Computing difference */
	diff_ru_utime = tv_delta(start_ru.ru_utime, end_ru.ru_utime);
	diff_ru_stime = tv_delta(start_ru.ru_stime, end_ru.ru_stime);
	diff_wall = tv_delta(start_wall, end_wall);

	/* Print time */
	printf("User time: %ld.%06ld\n", diff_ru_utime.tv_sec, diff_ru_utime.tv_usec);
	printf("System time: %ld.%06ld\n", diff_ru_stime.tv_sec, diff_ru_stime.tv_usec);
	printf("Wall time: %ld.%06ld\n", diff_wall.tv_sec, diff_wall.tv_usec);

	/* Print results if user specifies */
	if(strcmp(argv[5], "Y") == 0){
	  /* Print max number */
	  if(choice == 1){
		printf("Max: %d\n", result);
	  }

	  /* Print sum */
	  else{
		printf("Sum: %d\n", result);
	  }
	}
	
	return 0;
}


