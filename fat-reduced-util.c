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
static unsigned int fat_table[FAT_N] ; //FAT_N = CLUSTER_N
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
	
    for (int i=0; i<DIR_N; i++)
    {
        if(strlen(root_dir[i].name) == 0)
            continue;
        
//        printf("DirEnt number: %d\n", i);
//        printf("File length: %u\n", root_dir[i].len-1);
//        printf("File name: %s\n", root_dir[i].name);
        
        printf("%3d %6d %s\n", i, root_dir[i].len, root_dir[i].name) ;
    }
    
	return 0 ;
}

struct DirEnt* file_name_to_DirEnt(char* FILENAME)
{
    for(int i=0; i<DIR_N; i++)
    {
        if(!strcmp(FILENAME, root_dir[i].name))
        {
            //printf("DirEnt index: %d\n", i);
            //printf("Current length: %d\n", root_dir[i].len);
            return &root_dir[i];
        }
    }
    return NULL;
}

int find_last_cluster_of_file(int starting_cluster)
{
    int current_cluster = starting_cluster;
    int fat_table_value = fat_table[current_cluster];
    while (fat_table_value != UINT32_MAX) {
        current_cluster = fat_table[current_cluster];
        fat_table_value = fat_table[current_cluster];
    }
    return current_cluster;
}

int wr_action(int actc, char * actv[]) {
    
	// ...

	if (is_verbose_g>1) {
		printf("(%s,%d):action: %s, %d\n",__FILE__,__LINE__,(char *)actv[0], actc) ;
	}
	
	if (strlen(actv[1])>FILENAME_LEN) actv[1][FILENAME_LEN] = '\0' ;


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
    int fat_table_location = -1;
    long content_offset = 0;
    char* content = actv[2];
    long byte_length_of_content = strlen(content);
    int cluster_reference = 0;
    struct DirEnt* file_to_DirEnt = file_name_to_DirEnt(FILENAME);
    int indexOfDirEnt = -1;
//    if (file_to_DirEnt == NULL)
//    {
//        printf("Command %s", actv[0]);
//        printf(" returned error %d.\n", ERR_NOEXIST);
//    }
//    
    
    for (int i=0; i<DIR_N; i++)
    {
        if (!strcmp(root_dir[i].name, FILENAME))
        {
            found_file_index = i;
            break;
        }
    }
    
    if (found_file_index == -1) //if file new
    {
        for (int i=0; i<DIR_N; i++) //I feel repetitive doing this, but I want to make sure the loop above goes through all root_dir before making sure the file DNE
        {
            if (strlen(root_dir[i].name) == 0) {
                indexOfDirEnt = i;
                strncpy(root_dir[i].name, FILENAME, strlen(FILENAME));
                root_dir[i].name[strlen(FILENAME)] = '\0';
                root_dir[i].len = (unsigned int) byte_length_of_content;
            
                break;
            }
        }
    }
    else //if file exists, I need to get the last cluster that contains its contents thus far
    {
        where_to_start_following_clusters = find_last_cluster_of_file(root_dir[found_file_index].starting_cluster);
        fat_table_location = where_to_start_following_clusters;
        int content_length_non_slack = file_to_DirEnt->len%(CLUSTER_SIZE+1);
        //root_dir[found_file_index].len += byte_length_of_content;
        
        
        int slack_size = (int)(CLUSTER_SIZE - content_length_non_slack);
        //printf("slack size %d\n", slack_size);
        root_dir[found_file_index].len += (unsigned int) byte_length_of_content;
        if (slack_size > 0)
        {
            long write_byte_2 = slack_size > byte_length_of_content ? byte_length_of_content : slack_size;
           
            strncpy(cluster_table[where_to_start_following_clusters].data+content_length_non_slack, content, write_byte_2); //ask about what to do with null terminator
           
            byte_length_of_content -= write_byte_2;
            content_offset=(long)write_byte_2;
        }
    }
    
    int number_of_clusters = (int)ceil(((double)byte_length_of_content)/CLUSTER_SIZE);
    //printf("number of clusters: %d\n", number_of_clusters);
    //printf("byte length content: %ld\n", byte_length_of_content);
    for (int i=0; i<CLUSTER_N; i++)
    {
        if(cluster_reference<number_of_clusters && strlen(cluster_table[i].data) == 0)
        {
            if (indexOfDirEnt != -1) {
                root_dir[indexOfDirEnt].starting_cluster = i;
                indexOfDirEnt = -1;
            }
                
            long write_byte = CLUSTER_SIZE > byte_length_of_content ? byte_length_of_content : CLUSTER_SIZE;
            strncpy(cluster_table[i].data, content + content_offset, write_byte);
            //printf("Cluster table i data: %s\n", cluster_table[i].data);
            //printf("fat table loc: %d\n", fat_table_location);
            content_offset += write_byte;
            byte_length_of_content -= write_byte;
            cluster_reference++;
            //fat_table_location = i;
            if (fat_table_location != -1)
            {
                fat_table[fat_table_location] = i;
            }
            fat_table_location = i;
        }
        //once i equals the number of cluster, cap the last cluster of data with \0
        //cluster_table[i].data[strlen(content)] = '\0';
    }
    fat_table[fat_table_location] = UINT32_MAX;
    
	return 0 ;
}

int rd_action(int actc, char * actv[]) {
	// ...
	
	if (is_verbose_g>1) {
		printf("(%s,%d):action: %s, %d\n",__FILE__,__LINE__,(char *)actv[0], actc) ;
	}


	// ...
    char* FILENAME = actv[1];
    
    
    
//    int c = root_dir[file_index].starting_cluster;
//    printf("%s\n", cluster_table[c].data);
    
    struct DirEnt* dirEntForReading = file_name_to_DirEnt(FILENAME);
    
    if (dirEntForReading == NULL)
    {
        //printf("Command %s", actv[0]);
        //printf(" returned error %d.\n", ERR_NOEXIST);
        //exit(1);
        return ERR_NOEXIST;
    }
    
    
    int current_cluster = dirEntForReading->starting_cluster;
    int dataLength = dirEntForReading->len;
    //printf("data length: %d\n", dataLength);
    
    char buf[dataLength+1];
    int bytesRead = 0;
    char* buf2 = buf;
    buf[dataLength] = '\0';
    
    while (fat_table[current_cluster] != -1) {
        strncpy(buf2+bytesRead, cluster_table[current_cluster].data, CLUSTER_SIZE);
        current_cluster = fat_table[current_cluster];
        dataLength -= CLUSTER_SIZE;
        bytesRead += CLUSTER_SIZE;

    }
    
    strncpy(buf2+bytesRead, cluster_table[current_cluster].data, dataLength);
    
    printf("%s\n", buf2);

	return 0 ;
}

int rm_action(int actc, char * actv[]) {
   
	// ...
	
	if (is_verbose_g>1) {
		printf("(%s,%d):action: %s, %d\n",__FILE__,__LINE__,(char *)actv[0], actc) ;
	}


	// ...
    
    /*reminders:
     1. make FAT entries 0 --> ALL FAT table LL nodes made to 0 up to the one w -1
     2. remove file by setting file name to empty string
     
     1. search through root_dir until find correct file for elimination, using file_name_to_DirEnt method
        a. set name to empty string
        b. set len to 0
        c. use staring_cluster into find_last_cluster_of_file to find where to begin eliminating from fat and cluster table
     2. cluster and fat table mirror each other, so also clear cluster table of the bytes*/
    
    char* FILENAME = actv[1];
    struct DirEnt* dirEntForElimination = file_name_to_DirEnt(FILENAME);
   
    if (dirEntForElimination == NULL)
    {
        //printf("Command %s", actv[0]);
        //printf(" returned error %d.\n", ERR_NOEXIST);
        //exit(1);
        return ERR_NOEXIST;
    }
    dirEntForElimination->name[0] = '\0';
    dirEntForElimination->len = 0;
    
    int where_to_start_rm = dirEntForElimination->starting_cluster;
    //int where_to_end_rm = find_last_cluster_of_file(where_to_start_rm);
    int current_cluster = where_to_start_rm;
    
    while (fat_table[current_cluster] != -1)
    {
        int temp = fat_table[current_cluster];
        fat_table[current_cluster] = 0;
        cluster_table[current_cluster].data[0] = '\0';
        current_cluster = temp;
    }
    fat_table[current_cluster] = 0;
    cluster_table[current_cluster].data[0] = '\0';
    
	return 0 ;
}

int dd_action(int actc, char * actv[]) {

	// ....
	
	// formats to use
	// "%3d %s\t]->%d"
	// "->%d"
	// "\n"
    
    char* FILENAME = actv[1];
    struct DirEnt* dirEntForDumping = file_name_to_DirEnt(FILENAME);
    if (dirEntForDumping == NULL)
    {
        //printf("Command %s", actv[0]);
        //printf(" returned error %d.\n", ERR_NOEXIST);
        //exit(1);
        return ERR_NOEXIST;
    }
    int current_cluster = dirEntForDumping->starting_cluster;
    
    printf("%3d %s\t]->%d", dirEntForDumping->starting_cluster, dirEntForDumping->name, current_cluster);
  
    while(fat_table[current_cluster] != -1)
    {
        current_cluster = fat_table[current_cluster];
        printf("->%d", current_cluster);
    }
    printf("\n");
    
    
    return 0 ;
}

/* end of file */
