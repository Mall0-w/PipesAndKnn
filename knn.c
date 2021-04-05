#include "knn.h"

/****************************************************************************/
/* For all the remaining functions you may assume all the images are of the */
/*     same size, you do not need to perform checks to ensure this.         */
/****************************************************************************/

/**************************** A1 code ****************************************/

/* Same as A1, you can reuse your code if you want! */
double distance(Image *a, Image *b) {
  // TODO: Return correct distance
  double sum = 0;
  for(int i = 0; i < a->sx * a->sy; i++){
      sum += pow(a->data[i] - b->data[i],2);
  }
  return sqrt(sum);
}

/* Same as A1, you can reuse your code if you want! */
int knn_predict(Dataset *data, Image *input, int K) {
  // TODO: Replace this with predicted label (0-9)
  //take inputed image
  double lowest_distances[K];
  int distance_indexes[K];
  double highest_distance = 0;
  int highest_location = 0;
  double temp_distance = 0;
  //compare image to every image in data set
  
  for(int i = 0; i < data->num_items; i++){
    temp_distance = distance(data->images+i, input);
    if(i < K){
      //set initial k distances
      lowest_distances[i] = temp_distance;
      distance_indexes[i] = i;
      if(temp_distance > highest_distance){
        highest_distance = temp_distance;
        highest_location = i;
      }
    }
    else{
      //check if image is more similar than item in most similar set
      if(temp_distance < highest_distance){
        //if it is place in set
        lowest_distances[highest_location] = temp_distance;
        highest_distance = temp_distance;
        distance_indexes[highest_location] = i;
        //now find new least similar element location
        for(int j = 0; j < K; j++){
          if(lowest_distances[j] > highest_distance){
            highest_distance = lowest_distances[j];
            highest_location = j;
          }
        }
      }
    }
  }
  
  unsigned char toRet = 0;
  int highest = 0;
  int temphighest;
  for(int i = 0; i < K; i++){
    temphighest = 0;
    for(int j = i; j < K; j++){
      if(data->labels[distance_indexes[i]] == data->labels[distance_indexes[j]]){
        temphighest++;
      }
    }
    if(temphighest > highest){
      highest = temphighest;
      toRet = data->labels[distance_indexes[i]];
    }
  }
  
  //pick the most common label in those k distances
  return toRet;
}

/**************************** A2 code ****************************************/

/* Same as A2, you can reuse your code if you want! */
Dataset *load_dataset(const char *filename) {
  // TODO: Allocate data, read image data / labels, return
  
  //not testing different images sizes so can set sx and sy to 28
  int sx = 28;
  int sy = 28;
  //open file in binary format
  FILE* f = fopen(filename, "rb");
  if(f == NULL){
    fprintf(stderr, "File not found\n");
    return NULL;
  }
  //read first four bytes to see how many images there are
  int numImages;
  if(fread(&numImages, sizeof(int), 1, f) == 0){
    perror("fread");
    return NULL;
  }
  //begin allocating memory for dataset
  Dataset* d = calloc(sizeof(Dataset), 1);
  d->num_items = numImages;
  d->labels = calloc(sizeof(unsigned char), numImages);
  d->images = calloc(sizeof(Image), numImages);
  if(d->labels == NULL || d->images == NULL){
    fprintf(stderr, "Not enough memory\n");
    return NULL;
  }
  //go through every image and allocate memeory for them
  for(int i = 0; i < numImages; i++){
    d->images[i].sx = sx;
    d->images[i].sy = sy;
    //allocate memory for data
    d->images[i].data = calloc(sizeof(unsigned char), d->images[i].sx * d->images[i].sy);
    if(d->images[i].data == NULL){
      fprintf(stderr, "Not enough memory for data\n");
      return NULL;
    }
    //read first byte indicating label
    //note will be in unsigned char format will need to convert to proper label after
    if (0 == fread(d->labels+i, 1, 1, f)){
      fprintf(stderr, "error reading binary file to label\n");
    }
    if (0 == fread((d->images+i)->data, sizeof(unsigned char), d->images[i].sx * d->images[i].sy, f)){
      fprintf(stderr, "error reading binary file to images\n");
    }
  }
  fclose(f);
  return d;
  return NULL;
}

/* Same as A2, you can reuse your code if you want! */
void free_dataset(Dataset *data) {
  // TODO: free data
  for(int i = 0; i < data->num_items; i ++){
    free(data->images[i].data);
  }
  free(data->images);
  free(data->labels);
  free(data);
  // TODO: Free dataset (Same as A1)
  return;
}


/************************** A3 Code below *************************************/

/**
 * NOTE ON AUTOTESTING:
 *    For the purposes of testing your A3 code, the actual KNN stuff doesn't
 *    really matter. We will simply be checking if (i) the number of children
 *    are being spawned correctly, and (ii) if each child is recieving the 
 *    expected parameters / input through the pipe / sending back the correct
 *    result. If your A1 code didn't work, then this is not a problem as long
 *    as your program doesn't crash because of it
 */

/**
 * This function should be called by each child process, and is where the 
 * kNN predictions happen. Along with the training and testing datasets, the
 * function also takes in 
 *    (1) File descriptor for a pipe with input coming from the parent: p_in
 *    (2) File descriptor for a pipe with output going to the parent:  p_out
 * 
 * Once this function is called, the child should do the following:
 *    - Read an integer `start_idx` from the parent (through p_in)
 *    - Read an integer `N` from the parent (through p_in)
 *    - Call `knn_predict()` on testing images `start_idx` to `start_idx+N-1`
 *    - Write an integer representing the number of correct predictions to
 *        the parent (through p_out)
 */
void child_handler(Dataset *training, Dataset *testing, int K, 
                   int p_in, int p_out) {
  // TODO: Compute number of correct predictions from the range of data
  //read start_idx and N from pipe 
  int start, N;
  int count = 0;

  //read start position and N from pipe
  if(read(p_in, &start, sizeof(int)) <= 0 || read(p_in, &N, sizeof(int)) <=0){
    perror("read");
  }
  //get the amount of correctly guessed labels
  for(int i = start; i < start+N; i++){
      if(knn_predict(training, testing->images+i, K) == testing->labels[i]) count++;
  }
  //write number of correct guesses to pipe
  if(write(p_out, &count, sizeof(int)) <= 0){
    perror("write");
  }
  free_dataset(testing);
  free_dataset(training);
  
  return;
}