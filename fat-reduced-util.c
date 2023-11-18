#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>
#include<sys/time.h>
#include<assert.h>
#include<time.h>
#include<math.h>

#include "fat-reduced.h"

/*
 * name: fat-reduced-util.c
 * last update:
 *    13 nov 2022, bjr
 *     5 nov 2023, bjr
 *
 */

static struct DirEnt root_dir[DIR_N] ;
static unsigned int fat_table[FAT_N] ; //FAT_/N = CLUSTER_N
static struct Cluster cluster_table[CLUSTER_N] ;

void init_fat() {
	 int i ;
	 if (is_verbose_g>1) {
		printf("(%s,%d):\n\t%ld fat entries (%ld bytes),"
			"\n\t%ld dir entries (%ld bytes),"
			"\n\tdirent size is %ld bytes.\n", 
			__FILE__,__LINE__,
			(unsigned long) FAT_N, sizeof(fat_table), 
			(unsigned long) DIR_N, sizeof(root_dir), 
			 sizeof(struct DirEnt)
			) ;
	}

	for (i=0;i<FAT_N;i++) fat_table[i] = 0 ;
	for (i=0;i<sizeof(root_dir);i++) ((char *)root_dir)[i] = 0 ;
	return ;
}

int qu_action(int actc, char * actv[]) {
	if (is_verbose_g>1) {
		printf("(%s,%d):qu_action\n",__FILE__,__LINE__) ;
	}
	return ERR_ABORT ;
}

int ls_action(int actc, char * actv[]) {

	// ...

	if (is_verbose_g>1) {
		printf("(%s,%d):action: %s, %d\n",__FILE__,__LINE__,(char *)actv[0], actc) ;
	}
	
	
	// ...
	
	// format to use
	// "%3d %6d %s\n"
	
	
	return 0 ;
}

int wr_action(int actc, char * actv[]) {

	// ...

	if (is_verbose_g>1) {
		printf("(%s,%d):action: %s, %d\n",__FILE__,__LINE__,(char *)actv[0], actc) ;
	}
	
	if (strlen(actv[1])>FILENAME_LEN) actv[1][FILENAME_LEN] = '\0' ;


	// ...
	
	
	return 0 ;
}

int rd_action(int actc, char * actv[]) {

	// ...
	
	if (is_verbose_g>1) {
		printf("(%s,%d):action: %s, %d\n",__FILE__,__LINE__,(char *)actv[0], actc) ;
	}


	// ...
    
    /*
      1. get number of clusters from length of content
     IF FILE NAME ALREADY EXISTS
      2. define variable cluster_reference to save space where a free cluster is found
      3. walk through cluster array
            if there are still clusters in the content to fill and find cluster that is free,
                save the index to cluster_reference and pass to fat table
                save portion of data that fits in that cluster*/
    
    char* FILENAME = actv[1];
    int found_file_index = -1;
    int where_to_start_following_clusters = -1;
    
    for (int i=0; i<DIR_N; i++)
    {
        if (strcmp(root_dir[i].name, FILENAME)) 
        {
            found_file_index = i;
            break;
        }
    }
    
    if (found_file_index == -1)
    {
        for (int i=0; i<DIR_N; i++) //I feel repetitive doing this, but I want to make sure the loop above goes through all root_dir before making sure the file DNE
        {
            if (strlen(root_dir[i].name) == 0) {
                strncpy(root_dir[i].name, FILENAME, strlen(FILENAME));
                root_dir[i].name[strlen(FILENAME)] = '\0';
            }
        }
    }
    else //if file exists, I need to get the last cluster that contains its contents thus far
    {
        where_to_start_following_clusters = root_dir[found_file_index].starting_cluster;
    }
    
    char* content = actv[2];
    long byte_length_of_content = strlen(content);
    int number_of_clusters = ceil(byte_length_of_content/CLUSTER_SIZE);
    
    
    int cluster_reference = 0;
    
    for (int i=0; i<CLUSTER_N; i++)
    {
        if(i<number_of_clusters && strlen(cluster_table[i].data) == 0)
        {
            cluster_reference = i;
            strncpy(cluster_table[i].data, content, CLUSTER_SIZE); //how to pass the correct portion of content to the cluster?
        }
        //once i equals the number of cluster, cap the last cluster of data with \0
        cluster_table[i].data[strlen(content)] = '\0';
    }
	
	// format to use
	// "%s\n"
	
	
	return 0 ;
}

int rm_action(int actc, char * actv[]) {

	// ...
	
	if (is_verbose_g>1) {
		printf("(%s,%d):action: %s, %d\n",__FILE__,__LINE__,(char *)actv[0], actc) ;
	}


	// ...
	

	return 0 ;
}

int dd_action(int actc, char * actv[]) {

	// ....
	
	// formats to use
	// "%3d %s\t]->%d"
	// "->%d"
	// "\n"

	return 0 ;
}

/* end of file */
