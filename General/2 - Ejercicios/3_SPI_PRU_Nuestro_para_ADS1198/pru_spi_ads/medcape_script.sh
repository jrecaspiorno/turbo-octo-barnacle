#!/bin/bash 
rmmod uio_pruss
modprobe uio_pruss extram_pool_sz=0x0005DC

/home/debian/workspace2016/pru_spi_ads/medcape > zfinal9.dat
