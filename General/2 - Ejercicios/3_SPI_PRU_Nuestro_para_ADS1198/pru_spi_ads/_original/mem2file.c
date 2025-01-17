/* This mem2file.c program is a modified version of devmem2 by Jan-Derk Bakker
 * as referenced below. This program was modified by Derek Molloy for the book
 * Exploring BeagleBone. It is used in Chapter 13 to dump the DDR External Memory
 * pool to a file. See: www.exploringbeaglebone.com/chapter13/
 *
 * devmem2.c: Simple program to read/write from/to any location in memory.
 *
 *  Copyright (C) 2000, Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 *
 * This software has been developed for the LART computing board
 * (http://www.lart.tudelft.nl/). The development has been sponsored by
 * the Mobile MultiMedia Communications (http://www.mmc.tudelft.nl/)
 * and Ubiquitous Communications (http://www.ubicom.tudelft.nl/)
 * projects.
 *
 * The author can be reached at:
 *
 *  Jan-Derk Bakker
 *  Information and Communication Theory Group
 *  Faculty of Information Technology and Systems
 *  Delft University of Technology
 *  P.O. Box 5031
 *  2600 GA Delft
 *  The Netherlands
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

#define MAP_SIZE 0x0FFFFFFF
#define MAP_MASK (MAP_SIZE - 1)
#define MMAP_LOC   "/sys/class/uio/uio0/maps/map1/"

unsigned int readFileValue(char filename[]){
   FILE* fp;
   unsigned int value = 0;
   fp = fopen(filename, "rt");
   fscanf(fp, "%x", &value);
   fclose(fp);
   return value;
}

int main_mem2file(int argc, char **argv) {
    int fd;
    void *map_base, *virt_addr;
    unsigned long read_result, writeval;
    unsigned int addr = readFileValue(MMAP_LOC "addr");
    unsigned int dataSize = readFileValue(MMAP_LOC "size");
    unsigned int numberOutputSamples = dataSize * 2;
    off_t target = addr;

    if(argc>1){     // There is an argument -- lists number of samples to dump
                    // this defaults to the total DDR Memory Pool x 2 (16-bit samples) 
	numberOutputSamples = atoi(argv[1]);
    }

    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1){
	printf("Failed to open memory!");
	return -1;
    }
    fflush(stdout);

    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) {
       printf("Failed to map base address");
       return -1;
    }
    fflush(stdout);

	//----Medcape--------------------
	time_t current_time;
    struct tm *time_info;
    char time_string[30];  // space for "YYYY-MM-DD_HH-MM-SS\0"
    FILE *pf_data;
	
    time(&current_time);
    time_info=localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%F_%H-%M-%S.dat", time_info);


    if ( (pf_data=fopen(time_string, "w")) == NULL ) {
        perror("Error al fichero de datos");
       return -1;
    }
	//-------------------------------
	
    int i=0;
    for(i=0; i<numberOutputSamples; i++){
	virt_addr = map_base + (target & MAP_MASK);
        read_result = *((uint16_t *) virt_addr);
        //printf("Value at address 0x%X (%p): 0x%X\n", target, virt_addr, read_result);
        printf("%d %d\n",i, read_result);
		//---Medcape-----------------------
		fwrite(read_result, sizeof(uint16_t), 1, pf_data);
		//---------------------------------
		
        target+=2;                   // 2 bytes per sample
    }
    fflush(stdout);

    if(munmap(map_base, MAP_SIZE) == -1) {
       printf("Failed to unmap memory");
       return -1;
    }
	
	close(pf_data);
    close(fd);
    return 0;
}
