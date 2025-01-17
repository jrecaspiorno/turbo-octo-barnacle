#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>

#include "bbb_spi.h"

uint8_t tx[BUFFERSIZE] = {0};
uint8_t rx[BUFFERSIZE] = {0};

// struct spi_ioc_transfer - describes a single SPI transfer
//
// @tx_buf:        Holds pointer to userspace buffer with transmit data, or null.
//                 If no data is provided, zeroes are shifted out.
// @rx_buf:        Holds pointer to userspace buffer for receive data, or null.
// @len:           Length of tx and rx buffers, in bytes.
// @speed_hz:      Temporary override of the device's bitrate.
// @bits_per_word: Temporary override of the device's wordsize.
// @delay_usecs:   If nonzero, how long to delay after the last bit transfer
//                 before optionally deselecting the device before the next transfer.
// @cs_change: True to deselect device before starting the next transfer.
//             NOTE Sólo para transferencias multiples SPI_IOC_MESSAGE(nMessages)
//

struct spi_ioc_transfer spi_ioc;

int fid=-1;


int init_spi(uint32_t speed) {
    uint8_t  mode = SPI_MODE_1;         // CPHA = 1, CPOL = 0, CS = cs
    uint8_t  bits = 8;

    //The “active low” default for chipselect mode can be overridden (by specifying SPI_CS_HIGH)
    //mode |= SPI_CS_HIGH;

    memset(&spi_ioc, 0, sizeof(struct spi_ioc_transfer));

    spi_ioc.tx_buf          = (unsigned long) tx;
    spi_ioc.rx_buf          = (unsigned long) rx;
    spi_ioc.len             = 0;
    spi_ioc.delay_usecs     = 0;
    spi_ioc.speed_hz        = speed;
    spi_ioc.bits_per_word   = bits;
    spi_ioc.cs_change       = 0;

    fid = open("/dev/spidev1.0", O_RDWR);
    if (fid < 0) {
        printf("SPI failed to open\n");
        return -1;
    }

    if ( ioctl(fid, SPI_IOC_WR_MODE, &mode) == -1 ) {
        perror("can't set spi write mode");
        return -1;
    }

    if ( ioctl(fid, SPI_IOC_RD_MODE, &mode) == -1 ) {
        perror("can't get spi read mode");
        return -1;
    }

    // bits per word
    if ( ioctl(fid, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1 ) {
        perror("can't set bits per word");
        return -1;
    }

    if ( ioctl(fid, SPI_IOC_RD_BITS_PER_WORD, &bits) == -1 ) {
        perror("can't get bits per word");
        return -1;
    }

    // max speed hz
    if ( ioctl(fid, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1 ) {
        perror("can't set max speed hz");
        return -1;
    }

    if ( ioctl(fid, SPI_IOC_RD_MAX_SPEED_HZ, &speed) == -1 ) {
        perror("can't get max speed hz");
        return -1;
    }

    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);

    return 0;
}


int tx_spi(uint32_t len) {

    if (fid<0) return -1;

    spi_ioc.len = len;
    if (ioctl(fid, SPI_IOC_MESSAGE(1), &spi_ioc) == -1) {
        perror("can't send spi data");
        return -1;
    }
    return 0;
}

void close_spi(void) {
    if (fid!=-1)
        close(fid);
    fid=-1;
}
