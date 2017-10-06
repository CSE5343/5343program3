#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

/*
 * Preston Tighe
 * CSE 5343 - Operating Systems
 * Processor Evans
 * Program 3
 *
 * Command: gcc sorts.c -pthread
 *
 * Conclusions:
 * Using low amounts of array items the concurrent sorting was almost the same speed as sequential sorting.
 * Using 20,000 elements the double length array took about double the time of the sequential sorting.
 * Concurrent sorting took about 40 - 70% of the time of sequential at using different array amounts.
 */

void arrayManager(int);                                                 
void fillArray(int, int*);                                              
void copyArray(int, int*, int*);                                        
double sortArray(int, int*);                                
double mergeArrays(int, const int*, const int*, int*);

void pthreadSort(int, int*, int*, int*, double[2]);             
void sortAndMerge(int, int*, int*, int*, double[2]);                    
double sortDouble(int, int*);                                           

typedef struct thread_args {
    int numItems;
    int* array;
    double sortTime;
    int threadNum;
} thread_args;

void *threadSort(void* args) {
    thread_args* arguments = args;
    printf("\nIn thread %d\n", arguments->threadNum);
    arguments->sortTime = sortArray(arguments->numItems, arguments->array);
    printf("\n\tThread %d time: %.5f", arguments->threadNum, arguments->sortTime);
    pthread_exit(NULL);
}

int main(void) {
    int numItems = 1;                                                 

    while(numItems != 0) {
        printf("\n\nPlease enter an array size or enter '0' to exit: ");

        while (scanf("%d", &numItems) != 1)    {
            while (getchar() != '\n');
            printf("\n\nInvalid selection\n");
        }

        if(numItems == 0) {
            printf("\n\nExiting\n\n");
            exit(1);
        } else if (numItems > 0) {
            arrayManager(numItems);
        } else {
            printf("\n\nInvalid selection\n");
        }
    }
}

void arrayManager(int numItems) {
    int* arrayOneOriginal = (int*) malloc(numItems * sizeof(int));        
    int* arrayTwoOriginal = (int*) malloc(numItems * sizeof(int));        
    int* pthreadsArray = (int*) malloc(numItems * sizeof(int) * 2);
    int* arrayOneCopy = (int*) malloc(numItems * sizeof(int));            
    int* arrayTwoCopy = (int*) malloc(numItems * sizeof(int));            
    int* mergedArray = (int*) malloc(numItems * sizeof(int) * 2);
    int* doubleArray = (int*) malloc(numItems * sizeof(int) * 2);

    srand(time(NULL));

    fillArray(numItems, arrayOneOriginal);
    fillArray(numItems, arrayTwoOriginal);

    copyArray(numItems, arrayOneOriginal, arrayOneCopy);
    copyArray(numItems, arrayTwoOriginal, arrayTwoCopy);

    int i;
    for(i = 0; i < numItems; i++) {
        doubleArray[i] = arrayOneOriginal[i];
    }
    for(i = 0; i < numItems; i++) {
        doubleArray[i + numItems] = arrayTwoOriginal[i];
    }        

    double pthreadSummary[3];
    double sortAndMergeSummary[2];
    double doubleLengthSortTime = 0;

    sortAndMerge(numItems, arrayOneCopy, arrayTwoCopy, mergedArray, sortAndMergeSummary);
    pthreadSort(numItems, arrayOneOriginal, arrayTwoOriginal, pthreadsArray, pthreadSummary);
    doubleLengthSortTime = sortDouble(numItems * 2, doubleArray);

    // Summaries
    printf("\n\n\nConcurrent sorting & merge summary\n\n");
    printf("\t\t Sort time: %.5f\n\t\tMerge time: %.5f\n\tTotal time: %.5f",pthreadSummary[0], pthreadSummary[2], pthreadSummary[0] + pthreadSummary[2]);

    printf("\n\n\nSequential sorting & merge summary\n\n");
    printf("\t\t Sort time: %.5f\n\t\tMerge time: %.5f\n\tTotal time: %.5f", sortAndMergeSummary[0], sortAndMergeSummary[1], sortAndMergeSummary[0] + sortAndMergeSummary[1]);

    printf("\n\n\nDouble length array summary\n\n");
    printf("\tTotal time: %.5f", doubleLengthSortTime);

    // Free all memory used
    free(arrayOneOriginal);
    free(arrayTwoOriginal);
    free(pthreadsArray);

    free(arrayOneCopy);
    free(arrayTwoCopy);
    free(mergedArray);

    free(doubleArray);
}

void fillArray(int numItems, int* array) {
    for(int i = 0; i < numItems; i++) {
        array[i] = rand() % 1000;
    }
}

void copyArray(int numItems, int* source, int* dest) {
    for(int i = 0; i < numItems; i++) {
        dest[i] = source[i];
    }
}

double sortArray(int numItems, int* array) {
    clock_t elapsedTime;
    elapsedTime = clock();

    // Bubble sort array
    for(int i = 0; i < numItems; i++) {
        for(int j = 0; j < numItems - 1; j++) {
            if(array[j] > array[j + 1]) {
                int temp = array[j + 1];
                array[j + 1] = array[j];
                array[j] = temp;
            }
        }
    }

    elapsedTime = clock() - elapsedTime;
    return (float) elapsedTime / CLOCKS_PER_SEC;
}

double mergeArrays(int numItems, const int* arrayOne, const int* arrayTwo, int* mergedArray) {
    clock_t elapsedTime;
    elapsedTime = clock();

    int locationOne = 0;
    int locationTwo = 0;

    for(int i = 0; i < numItems * 2; i++) {
        if(locationOne < numItems && locationTwo < numItems) {
            if(arrayOne[locationOne] < arrayTwo[locationTwo]) {
                mergedArray[i] = arrayOne[locationOne];
                locationOne++;
            } else {
                mergedArray[i] = arrayTwo[locationTwo];
                locationTwo++;
            }
        } else if(locationOne < numItems) {
            while(i < numItems * 2) {
                mergedArray[i] = arrayOne[locationOne];
                locationOne++;
                i++;
            }
        } else {
            while(i < numItems * 2)  {
                mergedArray[i] = arrayTwo[locationTwo];
                locationTwo++;
                i++;
            }
        }
    }

    elapsedTime = clock() - elapsedTime;
    return (float) elapsedTime / CLOCKS_PER_SEC;
}

void pthreadSort(int numItems, int* arrayOne, int* arrayTwo, int* mergedArray, double summary[2]) {
    int numThreads = 2;
    double mergeTime = 0;

    thread_args threadOneArgs = {numItems, arrayOne, 0, 1};
    thread_args threadTwoArgs = {numItems, arrayTwo, 0, 2};

    thread_args threadArgs[numThreads];
    threadArgs[0] = threadOneArgs;
    threadArgs[1] = threadTwoArgs;

    pthread_t threads[numThreads];

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    clock_t begin;
    begin = clock();

    printf("\nClock started... %.5f\n", ((float)(clock()) - begin)/CLOCKS_PER_SEC);

    int i;
    for(i = 0; i < numThreads; i++) {
        pthread_create(&threads[i], &attr, threadSort, &threadArgs[i]);
    }

    printf("\nThreads Created... %.5f\n", ((float)(clock()) - begin) / CLOCKS_PER_SEC);

    pthread_attr_destroy(&attr);

    for(i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nThreads Joined... %.5f\n", ((float)(clock()) - begin) / CLOCKS_PER_SEC);

    mergeTime = mergeArrays(numItems, arrayOne, arrayTwo, mergedArray);

    printf("\nArrays Merged... %.5f\n", ((float)(clock()) - begin) / CLOCKS_PER_SEC);

    summary[0] = (float)(clock() - begin) / CLOCKS_PER_SEC;
    summary[1] = threadArgs[0].sortTime + threadArgs[1].sortTime;
    summary[2] = mergeTime;
}

void sortAndMerge(int numItems, int* arrayOne, int* arrayTwo, int* mergedArray, double summary[2]) {
    double sortTime = 0;
    double mergeTime = 0;

    printf("\nSorting array 1\n");
    sortTime = sortArray(numItems, arrayOne);
    printf("\nSorting array 2\n");
    sortTime += sortArray(numItems, arrayTwo);

    mergeTime = mergeArrays(numItems, arrayOne, arrayTwo, mergedArray);

    summary[0] = sortTime;
    summary[1] = mergeTime;
}

double sortDouble(int numItems, int* doubleArray) {
    printf("Sorting double length array");
    double elapsedTime = sortArray(numItems, doubleArray);
    return elapsedTime;
}