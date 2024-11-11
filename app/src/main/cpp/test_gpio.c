#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <jni.h>


#define GPIO_PIN 138

typedef enum {
    GPIO_OUTPUT = 1,
    GPIO_OUTPUT_HIGH,
    GPIO_OUTPUT_LOW,
    GPIO_INPUT,
} GPIO_DIRECT;

int gpio_direction(int gpio, int dir) {
    int ret = 0;
    char buf[128];
    sprintf(buf, "/sys/class/gpio/gpio%d/direction", gpio);
    int gpiofd = open(buf, O_WRONLY);
    if (gpiofd < 0) {
        perror("Couldn't open IRQ file");
        ret = -1;
    }

    if (dir == GPIO_OUTPUT && gpiofd) {
        if (3 != write(gpiofd, "out", 3)) {
            perror("Couldn't set GPIO direction to out");
            ret = -2;
        }
    } else if (dir == GPIO_OUTPUT_HIGH && gpiofd) {
        if (4 != write(gpiofd, "high", 4)) {
            perror("Couldn't set GPIO direction to out high");
            ret = -3;
        }
    } else if (dir == GPIO_OUTPUT_LOW && gpiofd) {
        if (3 != write(gpiofd, "low", 3)) {
            perror("Couldn't set GPIO direction to out low");
            ret = -4;
        }
    } else if (gpiofd) {
        if (2 != write(gpiofd, "in", 2)) {
            perror("Couldn't set GPIO directio to in");
            ret = -5;
        }
    }

    close(gpiofd);
    return ret;
}

int gpio_set_irq(int gpio, int rising, int falling) {
    int ret = 0;
    char buf[128];
    sprintf(buf, "/sys/class/gpio/gpio%d/edge", gpio);
    int gpiofd = open(buf, O_WRONLY);
    if (gpiofd < 0) {
        perror("Couldn't open IRQ file");
        ret = -1;
    }

    if (gpiofd && rising && falling) {
        if (4 != write(gpiofd, "both", 4)) {
            perror("Failed to set IRQ to both falling & rising");
            ret = -2;
        }
    } else {
        if (rising && gpiofd) {
            if (6 != write(gpiofd, "rising", 6)) {
                perror("Failed to set IRQ to rising");
                ret = -2;
            }
        } else if (falling && gpiofd) {
            if (7 != write(gpiofd, "falling", 7)) {
                perror("Failed to set IRQ to falling");
                ret = -3;
            }
        }
    }

    close(gpiofd);

    return ret;
}

int gpio_export(int gpio) {
    char buf[50];
    int gpiofd, ret;

    /* test if it has already been exported */
    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
    gpiofd = open(buf, O_WRONLY);
    if (gpiofd != -1) {
        close(gpiofd);
        perror("cant export gpio");
        return 0;
    }

    gpiofd = open("/sys/class/gpio/export", O_WRONLY);

    if (gpiofd != -1) {
        sprintf(buf, "%d", gpio);
        ret = write(gpiofd, buf, strlen(buf));
        if (ret < 0) {
            perror("Export failed");
            return -2;
        }
        close(gpiofd);
    } else {
        perror("dont have any gpio permissions");
        return -1;
    }
    return 0;
}

void gpio_unexport(int gpio) {
    int gpiofd, ret;
    char buf[50];
    gpiofd = open("/sys/class/gpio/unexport", O_WRONLY);
    sprintf(buf, "%d", gpio);
    ret = write(gpiofd, buf, strlen(buf));
    close(gpiofd);
}

int gpio_getfd(int gpio) {
    char in[3] = {0, 0, 0};
    char buf[50];
    int gpiofd;
    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
    gpiofd = open(buf, O_RDWR);
    if (gpiofd < 0) {
        fprintf(stderr, "Failed to open gpio %d value\n", gpio);
        perror("gpio failed");
    }

    return gpiofd;
}

int gpio_read(int gpio) {
    char in[3] = {0, 0, 0};
    char buf[50];
    int nread, gpiofd;
    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
    gpiofd = open(buf, O_RDWR);
    if (gpiofd < 0) {
        fprintf(stderr, "Failed to open gpio %d value\n", gpio);
        perror("gpio failed");
    }

    do {
        nread = read(gpiofd, in, 1);
    } while (nread == 0);
    if (nread == -1) {
        perror("GPIO Read failed");
        return -1;
    }

    close(gpiofd);
    return atoi(in);
}

int gpio_write(int gpio, int val) {
    char buf[50];
    int nread, ret, gpiofd;
    sprintf(buf, "/sys/class/gpio/gpio%d/value", gpio);
    gpiofd = open(buf, O_RDWR);
    if (gpiofd > 0) {
        snprintf(buf, 2, "%d", val);
        ret = write(gpiofd, buf, 2);
        if (ret < 0) {
            perror("failed to set gpio");
            return 1;
        }

        close(gpiofd);
        if (ret == 2) return 0;
    }
    return 1;
}


int gpio_select(int gpio) {
    char gpio_irq[64];
    int ret = 0, buf, irqfd;
    fd_set fds;
    FD_ZERO(&fds);

    snprintf(gpio_irq, sizeof(gpio_irq), "/sys/class/gpio/gpio%d/value", gpio);
    irqfd = open(gpio_irq, O_RDONLY);
    if (irqfd < 1) {
        perror("Couldn't open the value file");
        return -13;
    }

    // Read first since there is always an initial status
    ret = read(irqfd, &buf, sizeof(buf));

    while (1) {
        FD_SET(irqfd, &fds);
        ret = select(irqfd + 1, NULL, NULL, &fds, NULL);
        if (FD_ISSET(irqfd, &fds)) {
            FD_CLR(irqfd, &fds);  //Remove the filedes from set
            // Clear the junk data in the IRQ file
            ret = read(irqfd, &buf, sizeof(buf));
            return 1;
        }
    }
}


JNIEXPORT jint JNICALL
Java_com_example_gpiotest_MainActivity_invokeGPIO( JNIEnv* env,
                                                   jobject mainActivity)
{
    int gpio_pin = GPIO_PIN;



    gpio_export(gpio_pin);
    gpio_direction(gpio_pin, 1);

    for (int i = 0; i < 3; i++) {
        printf(">> GPIO %d ON\n", gpio_pin);
        gpio_write(gpio_pin, 0);

        sleep(3);

        printf(">> GPIO %d OFF\n", gpio_pin);
        gpio_write(gpio_pin, 1);

        sleep(1);
    }

//    gpio_write(gpio_pin, 0);
//    printf(">> GPIO %d ON\n", gpio_pin);

    return 0;
}