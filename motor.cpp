#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <ugpio/ugpio.h>

using namespace std;
enum MOTOR_STATE {ACCELERATE, DECCELERATE, STABLE, Null};

int isGPIORequested(int gpio) {
    int rq;
    int rv;
    if ((rq = gpio_is_requested(gpio)) < 0) {
	 perror("gpio_is_requested");
	 return 0;
    }

    // export the gpio
    if (!rq) {
	printf("> exporting gpio\n");
	if ((rv = gpio_request(gpio, NULL)) < 0){
	    perror("gpio_request");
	    return 0;
	}
    }
    printf("successfully requested gpio\n");
    return 1;
}

int unexp(int gpio) {
    // get gpio status
    int rq;
    
    if ((rq = gpio_is_requested(gpio)) < 0) {
	 perror("gpio_is_requested");
	 return 0;
    }
    if (!rq) {
	printf("> unexporting gpio\n");
	if (gpio_free(gpio) < 0) {
	    perror("gpio_free");
	}
    }
} 

int gpioRead(int gpio, int* arr, int size){
    int i;
    int rv;
    //int gpio;
    int value;
    
    int rq = isGPIORequested(gpio);
    if (rq < 1) {
	return 0;
    } 
    // set to input direction
    printf("> setting to input\n");
    if ((rv = gpio_direction_input(gpio)) < 0) {
	perror("gpio_direction_input");
    }
	
    // read the gpio [size] times
    printf("> begin reading GPIO%d\n",gpio);
    for (i = 0; i < size; i++) { 
	// read the gpio
	value = gpio_get_value(gpio);
	arr[i] = value;
	printf("  > Read GPIO%d: value '%d'\n", gpio, value);
	sleep(1);	// pause between each read
    }
    // uneqxport gpio
    
    return 0;
}

MOTOR_STATE readInput(int inps[], int n){
    if (n == 0) {
	return Null;
    }
    int accel = 0;
    int decel= 0;
    for (int i = 0; i < n; ++i) {
	if (inps[i] < inps[i+1]) {
	    ++accel;
	}
	if (inps[i] > inps[i+1]) {
	    ++decel;
	}  
	//(inps[i] < inps[i+1])? (++accel):(++decel);
    
    }
    if (accel < decel) {
	return ACCELERATE;
    }
    if (decel > accel) {
	return DECCELERATE;
    }
    if (accel == decel) {
	return STABLE;
    }
    return Null;
}


bool driver(MOTOR_STATE s, int output_gpio) {
    if (s == Null) {
	return false;
    }

    int gpio_object; // if gpio in use, false
    if((gpio_object = isGPIORequested(output_gpio)) < 0) {
	return false;
    }
    // else, set the direction of the gpio to output
    if ((gpio_object = gpio_direction_output(output_gpio, 0) < 0)) {
        perror("error setting gpio direction to output");
	return false;
    }
    // if you've made it to this line, its time to turn on the motor 
    switch (s) {
	case ACCELERATE: // fucking artificially implementing pwm value :0
	    printf("accelerate\n");
	    system("fast-gpio pwm 0 50 10");
	    break;
	case DECCELERATE:
	    printf("deccelerate\n");
	    system("fast-gpio pwm 0 50 5");
	    break;
	case STABLE:
	    printf("stable\n");
	    system("fast-gpio pwm 0 50 7.5");
	    break;
    }
    printf("test\n");
    int freepin = unexp(output_gpio);
    if (freepin) {
	return true;
    }
    return false;
}

int main(int argc, char **argv) {

    int gpio_read = atoi(argv[1]); 
    int s_on = atoi(argv[2]);
    int gpio_out = atoi(argv[3]);
    int arr[s_on];

    gpioRead(gpio_read, arr, s_on); // arr should now have infor in it

    
    MOTOR_STATE currState = readInput(arr, s_on); // convert info to state
    bool driverworked = driver(currState, gpio_out);
    if (driverworked) {
	printf("success! \n");
	return 0;
    }
    return 1;
}
