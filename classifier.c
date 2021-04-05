#include "knn.h"

// Makefile included in starter:
//    To compile:               make
//    To decompress dataset:    make datasets
//
// Example of running validation (K = 3, 8 processes):
//    ./classifier 3 datasets/training_data.bin datasets/testing_data.bin 8

/*****************************************************************************/
/* This file should only contain code for the parent process. Any code for   */
/*      the child process should go in `knn.c`. You've been warned!          */
/*****************************************************************************/

/**
 * main() takes in 4 command line arguments:
 *   - K:  The K value for kNN
 *   - training_data: A binary file containing training image / label data
 *   - testing_data: A binary file containing testing image / label data
 *   - num_procs: The number of processes to be used in validation
 * 
 * You need to do the following:
 *   - Parse the command line arguments, call `load_dataset()` appropriately.
 *   - Create the pipes to communicate to and from children
 *   - Fork and create children, close ends of pipes as needed
 *   - All child processes should call `child_handler()`, and exit after.
 *   - Parent distributes the testing set among childred by writing:
 *        (1) start_idx: The index of the image the child should start at
 *        (2)    N:      Number of images to process (starting at start_idx)
 *     Each child should gets N = ceil(test_set_size / num_procs) images
 *      (The last child might get fewer if the numbers don't divide perfectly)
 *   - Parent waits for children to exit, reads results through pipes and keeps
 *      the total sum.
 *   - Print out (only) one integer to stdout representing the number of test 
 *      images that were correctly classified by all children.
 *   - Free all the data allocated and exit.
 */
int main(int argc, char *argv[]) {

   // TODO: Handle command line arguments

  // TODO: Spawn `num_procs` children

  // TODO: Send information to children

  // TODO: Compute the total number of correct predictions from returned values


  // TODO: Handle command line arguments

  //ensure correct number of args
  if(argc != 5){
    fprintf(stderr, "incorrect number of arguments\n");
    return 1;
  }
  int total_correct = 0;
  //set up variables for command line args
  int K = atoi(argv[1]);
  Dataset* training = load_dataset(argv[2]);
  Dataset* testing = load_dataset(argv[3]);

  int num_proc = atoi(argv[4]); 
  //get size of increment
  int N = ceil((double)testing->num_items / (double)num_proc);
  int temp = 0;
  int start_idx = 0;
  
  //creating array for child processess
  int children[num_proc];
  //creating array for multiple pipes
  int in_pipes[num_proc][2];
  int out_pipe[2];
  
  //spawn outpipe
  if( pipe(out_pipe) == -1){
    perror(("pipe"));
  }
  for(int i = 0; i < num_proc; i++){
    //spawn in pipes
    if(pipe(in_pipes[i]) == -1){
      perror("pipe");
      exit(1);
    }
    //write start index and N to pipes
    
    if(write(in_pipes[i][1], &start_idx, sizeof(int)) <= 0 || write(in_pipes[i][1], &N, sizeof(int)) <= 0){
      perror("write");
    }
  //fork children
    if((children[i] = fork()) == 0){
      //call child handler then exit
      child_handler(training, testing, K, in_pipes[i][0], out_pipe[1]);
      exit(0);
    }else{
      //close reading end of in_pipe for parent
      if(close(in_pipes[i][0]) == 1){
        perror("close");
        exit(1);
      }
    }
    //increment start index
    start_idx += N;
    //making last child take less in the event that images can be divided easily
    if(start_idx+N > testing->num_items){
      N = N - (start_idx+N - testing->num_items);
    }
  }
  //close writing end of out pipe for parent
  if(close(out_pipe[1]) == -1){
    perror("close");
    exit(1);
  }
  
  //read all contents of out_pipe, adding it to sum of correct guesses
  while(read(out_pipe[0], &temp, sizeof(int)) > 0){
      total_correct += temp;
  }
  //free datasets
  free_dataset(testing);
  free_dataset(training);

  // Print out answer
  printf("%d\n", total_correct);
  return 0;
}
