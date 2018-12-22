
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

struct student {
  struct student *next;
  struct student *left;
  struct student *right;
  int studentID;
  int studentAge;
};
typedef struct student Stu;

pthread_t Th1;
pthread_t Th2;
pthread_t Th3;
int queueLength = 0;
pthread_mutex_t freeItemQueue_mutex;
pthread_mutex_t binarySearchTree_mutex;
Stu* freeItemQueue;
Stu* binarySearchTree;

void insert_into_FIQ(Stu *s){

    //if there's nothing in the free item queue...
    if(freeItemQueue == NULL){
        //...add the empty node in it now!
        freeItemQueue = s;
    } else{
        //...otherwise, set the next thing to the free item queue...
        s->next = freeItemQueue;
        //...and the free item queue equal to the free item queue plus the new node.
        freeItemQueue = s;
    }
    //increase the length because we're adding things to the free item queue
    queueLength++;
    printf(" ======= queue length added: %d =========\n", queueLength);
}

//returns an empty node from the free item queue
Stu* get_empty_node_from_FIQ(){
    Stu* s = freeItemQueue;
    //if there's nothing in the free item queue...
    if(s == NULL){
        //...return null
        return NULL;
    } else{
        //set the FIQ to not include the first node.
        freeItemQueue=(freeItemQueue)->next;
        //otherwise, decrease the length size
        queueLength--;
        printf(" ======== queue length subtracted: %d ========= \n", queueLength);
        //then, return the FIQ with the first node, so you can manipulate it.
        return s;
    }
}

Stu *create(){
  Stu *s = (Stu*)malloc(sizeof(Stu));
  s->studentID = -1;
  s->studentAge = -1;
  s->next = NULL;
  s->left = NULL;
  s->right = NULL;
}

void insert_into_binary_search_tree(Stu **tree, Stu *student){
  //if tree has nothing in it...
    if(*tree == NULL){
      //...put student in it
        *tree = student;
    } else {
      //if the tree has something in it...
        Stu *s = *tree;
        //...if the tree's id is greater than the new id, do it all again with the next left node
        if(s->studentID < student->studentID){
            insert_into_binary_search_tree(&(s->left), student);
        } else {
            //otherwise, if the tree's id is less than the new id, do it all again with the next right node
            if(s->studentID > student->studentID){
                insert_into_binary_search_tree(&(s->right), student);
            } else{
                //otherwise, set the tree's age to the passed age.
                s->studentAge = student->studentAge;
            }
        }
    }
}

void create_binary_search_tree_if_empty(){
    Stu *s = get_empty_node_from_FIQ();
    if(s!=NULL){
      int studentID = (rand() % (20000-10001)) + 10001;
      int studentAge = (rand() % (25-18)) + 18;
      s->studentID = studentID;
      s->studentAge = studentAge;
      s->left = NULL;
      s->right = NULL;
      s->next = NULL;
      insert_into_binary_search_tree(&binarySearchTree, s);
    }
}

//I got some help from Wenjin Zhang on this for the algorithm and special cases
int delete_from_binary_search_tree(Stu **tree)
{
    Stu *top;
    Stu *left;
    Stu *removeNode;

    if(NULL==*tree){
        printf("Th3: this is a null tree. doing nothing\n");
        return -1;
    }
    if ((*tree)->left==NULL && NULL==(*tree)->right){
        removeNode = (*tree);
        printf("Th3: this is a one node tree. removing node.\n");
        *tree = NULL;
  } else if (NULL==(*tree)->left){
        printf("Th3: this tree has a right node only. removing root.\n");
        removeNode = (*tree);
        *tree = (*tree)->right;
  } else if (NULL==(*tree)->right){
        printf("Th3: this tree has a left node only. removing root.\n");
        removeNode = (*tree);
        *tree = (*tree)->left;
    } else{
        printf("Th3: this tree has both left and right nodes. removing root.\n");
        top = *tree;
        left = (*tree)->left;
        //we need to find the biggest number to the on the highest left subtree (so first left, then all the way right.)
        while (left->right){
            top=left;
            left = left->right;
        }
        //once we find it, change the root
        (*tree)->studentAge= left->studentAge;
        //if binary search tree root is equal to the saved parent...
        if(*tree==top){
            //...the root's left node is equal to left's left node...
            (*tree)->left=left->left;
        } else {
            //...otherwise, top's right node is equal to L's left node.
            top->right = left->left;
        }
        removeNode = left;
    }
    removeNode->left=NULL;
    removeNode->right=NULL;
    //return the node to the Free item queue;
    insert_into_FIQ(removeNode);
    return 1;
}

//Th1
void *allocateMemory(void *x){
  printf("this is allocate memory talking\n");

  //protect global variable
  pthread_mutex_lock(&freeItemQueue_mutex);
  int i=0;
  Stu *s;
  while(i < 100){
      //create n nodes
      s = create();
      //insert them into the free item queue
      printf("Th1: inserting %dth node into free item queue\n", i);
      insert_into_FIQ(s);
      i++;
  }
  pthread_mutex_unlock(&freeItemQueue_mutex);

  while(1){
    printf(" - free item queue length: %d - \n",queueLength);
    //protect global variable
    pthread_mutex_lock(&freeItemQueue_mutex);
    if(queueLength <= 20){
      i=0;
      Stu *s;
      while(i < 50){
          //create 50 nodes
          s = create();
          //insert them into the free item queue
          printf("Th1: inserting %dth node into free item queue\n", i);
          insert_into_FIQ(s);
          i++;
      }
    }
    pthread_mutex_unlock(&freeItemQueue_mutex);
    usleep(600);
  }
  return NULL;
}

//Th2
void *populateBinarySearchTree(void *x){
  printf("this is populate binary search tree talking\n");
  while(1){
    pthread_mutex_lock(&freeItemQueue_mutex);
    Stu *s = get_empty_node_from_FIQ();

    if(s!=NULL){
      int studentID = (rand() % (20000-10001)) + 10001;
      int studentAge = (rand() % (25-18)) + 18;
      s->studentID = studentID;
      s->studentAge = studentAge;

      //protect global variable here
      pthread_mutex_lock(&binarySearchTree_mutex);
      printf("Th2: inserting new student %d into binary search tree\n", studentID);
      insert_into_binary_search_tree(&binarySearchTree, s);
      pthread_mutex_unlock(&binarySearchTree_mutex);
    }

    pthread_mutex_unlock(&freeItemQueue_mutex);
    create_binary_search_tree_if_empty();



    usleep(200);
  }
  return NULL;
}

void *removeStudentRecords(void *x){
  printf("this is remove student records talking\n");
  while(1){
    //protect global variable here
    pthread_mutex_lock(&binarySearchTree_mutex);
    delete_from_binary_search_tree(&binarySearchTree);
    pthread_mutex_unlock(&binarySearchTree_mutex);
    usleep(800);
  }
  return NULL;
}

int main()
{

  /* initialize the mutex lock */
  if (pthread_mutex_init(&binarySearchTree_mutex, NULL) != 0){
    printf("\n mutex init failed\n");
    return 1;
  }

  if (pthread_mutex_init(&freeItemQueue_mutex, NULL) != 0){
    printf("\n mutex init failed\n");
    return 1;
  }

  /* create the threads */
  if(pthread_create(&Th1, NULL, allocateMemory, NULL)) {
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }

  if(pthread_create(&Th2, NULL, populateBinarySearchTree, NULL)) {
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }

  if(pthread_create(&Th3, NULL, removeStudentRecords, NULL)) {
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }

  /* wait for them to finish. */
  if(pthread_join(Th1, NULL)){
    printf("err");
  }

  if(pthread_join(Th2, NULL)){
    printf("err");
  }

  if(pthread_join(Th3, NULL)){
    printf("err");
  }

  return 0;

}
