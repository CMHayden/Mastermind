#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <time.h> 
#include <string.h> 


#define BLOCK_SIZE (4*1024) //set the block size
#define BUTTON 19 //constant var for the button pin number
#define TIMEOUT 4 //constant var for the time out value
#define GPIO_Base 0x3f200000 //constant variable for gpio base

#define GPFSEL_OUTPUT 0x01 //contant variable for setting pin mode (output)
#define GPFSEL_INPUT 0x00 //contant variable for setting pin mode (input)

static volatile int * gpio; //list of ints for gpio
static int redPin, greenPin; //declare static variables for the red and green pins



pinMode(volatile int * gpio, int pin, unsigned char fun){ //this fuction uses assembly to set the function of a BCM pin
	int GPFSEL = (pin/10)*4;
	
	int bitshift = (pin%10)*3;
	
	fun = fun & 0x07;
	
	
	asm(
	"	MOV R0, %[GPIO]\n"
	"	MOV R1, %[bitshift]\n"
	"	MOV R2, %[fun]\n"
	"	ADD R0, %[GPFSEL]\n"
	"	LDR R3, [R0]\n"
	"	here:MOV R4, #7 @prepare 111\n"
	"	MVN R4, R4, LSL R1 @shift and negate 111\n"
	"	AND R3, R3, R4\n"
	"	ORR R2, R3, R2, LSL R1\n"
	"	STR R2, [R0]\n"
	:[fun] "+r" (fun)
	:[GPIO] "r" (gpio), [bitshift] "r" (bitshift), [GPFSEL] "r" (GPFSEL)
	:"r0", "r1", "r2", "r3", "r4", "cc"
	);
}


digitalWrite(volatile int * gpio, int pin, int state){ //this function uses assembly, it sets a BCM pin to high or low
	
		asm(
		"	alovelylabel:\n"
		"	MOV R0, %[GPIO]\n"
		"	ADD R0, #28  @Move gpio base upto GPSET0\n"
		"	MOV R1, %[state]\n"
		"	CMP R1, #1\n"
		"	BEQ set\n"
		"	ADD R0, #12 @move GPIO up to GPCLR0\n"
		"	set: MOV R1, %[pin]\n"
		"	CMP R1, #32\n"
		"	BLT write @Pin number is offset and GPIO is correct register\n"
		"	ADD R0, #4 @Move to next register\n"
		"	SUB R1, #32 @Fix pin number to offset\n"
		"	write: MOV R2, #1\n"
		"	LSL R2, R1\n"
		"	STR R2, [R0]\n"
		:[pin] "+r" (pin)
		:[GPIO] "r" (gpio), [state] "r" (state)
		:"r0","r1", "r2", "r3", "cc"
		);
}



int readPin (volatile int * gpio, int pin) { //this assembly code reads the state of a pin
    int offset = ((pin / 32) + 13) * 4;
    
    int pinSet = pin % 32;
    
    int r;
    
    
    asm(
        "\tLDR R0, %[gpi]\n"
        "\tMOV R1, R0\n"
        "\tADD R1, %[offset]\n"
        "\tLDR R1, [R1]\n"
        "\tMOV R2, #1\n"
        "\tLSL R2, %[pinShift]\n"
        "\tAND %[r], R2, R1\n"
        : [r] "=r" (r)
        : [gpi] "m" (gpio),
          [offset] "r" (offset),
          [pinShift] "r" (pinSet)
        : "r0", "r1", "r2", "cc", "memory"
    );

    if (r != 0)
      return 1;
    return 0;
}


volatile int * getGPIO(){ //this fucntion takes care of the memory mapping of  GPIO
	
	volatile int * gpio;
	
	int f;



	if ( (f = open("/dev/mem",O_RDWR | O_SYNC | O_CLOEXEC)) < 0) {
		printf("could not open /dev/main\n");
		exit(0);
	}
	

	gpio = mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE , MAP_SHARED, f, GPIO_Base);
	

	if ((int) gpio ==-1 ){
		exit(0);
	}

	return gpio;
}




pinFlash(int pin, int flashes){ //this function is used by redFlash and greenFlash. it flashes an inputted number of times
	
	int i =0;

	for (; i<flashes; i++){
		digitalWrite(gpio, pin, 1);
		usleep(500000);
		digitalWrite(gpio, pin, 0);
		usleep(500000);
	}
	
}


redFlash(int flashes){ //this function uses pinFlash to flash the red LED
	pinFlash(redPin, flashes);
}


greenFlash(int flashes){ //this function uses pinFLash to flash the green LED
	pinFlash(greenPin, flashes);
}


ledInputRecieved(int input){ //flashes the red once then green a specified amount of times
	redFlash(1);
	greenFlash(input);
}


ledShowResult(int exact, int approximate){ //shows the results of the guess using LEDs
	greenFlash(exact);
	redFlash(1);
	greenFlash(approximate);
	redFlash(3); 
}


ledSuccess(){ //this displays that it's the end of the game
	digitalWrite(gpio, redPin, 1);
	greenFlash(3);
	digitalWrite(gpio, redPin, 0);
}


int getButtonInput() { //recieves the user's input via the button
    int input = 0;
    time_t stime;
    
    time(&stime);
    
    while ((time(NULL) - stime) < TIMEOUT) {
        if(readPin(gpio, BUTTON)){
			usleep(300000);
            input++;
		}
	}
	
    return input;
}



initialiseMastermindIO(){ //initialises pins etc (IO) for the game
	int dataPins[4] = {23, 17, 27, 22};

	redPin = 5;
	greenPin = 6;
	gpio = getGPIO();


	pinMode(gpio, redPin, GPFSEL_OUTPUT);
	digitalWrite(gpio, redPin, 0);

	pinMode(gpio, greenPin, GPFSEL_OUTPUT);
	digitalWrite(gpio, greenPin, 0);
}



struct { //structure that holds 2 ints: the exact guesses and approx guesses 
	int exact;
	int approximate;
} typedef Result;


int debug, codeLength;

Result checkGuess(int * guess, int * answer){ //this funtion takes in a guess and the answer and desplays a result structure
	int index =0, approx=0, exact=0, inner;
	Result res= {0, 0};
	
	//must make a copy because the guess and answer arrays are changed
	int * answercpy = malloc(sizeof(*answercpy) * codeLength);
	memcpy(answercpy, answer, sizeof(*answercpy) * codeLength);

	for (;index<codeLength;index++){
		if (guess[index] == answercpy[index]){
			exact++;
			guess[index]= -1;
			answercpy[index]=-1;
		}
	}
	for (index=0;index<codeLength;index++){
		for (inner=0;inner<codeLength; inner++){
			if (inner!= index && guess[index] == answercpy[inner] && guess[index] != -1){
				approx++;
				guess[index]=-1;
				answercpy[inner]=-1;
			}
		}
	}

	free(answercpy); //free up allocated memory
	free(guess);

	res.exact=exact;
	res.approximate = approx;
	return res;
}

int * getGuess(){ //returns the user's guess as int list
	int * guess;
	int i;

	guess = malloc(sizeof(*guess) * codeLength);
	for (i = 0; i<codeLength; i++){
		guess[i] = getButtonInput();
		if (debug) printf("Input: %d\n", guess[i]);
		ledInputRecieved(guess[i]);
	}

	redFlash(2);

	return guess;
}

void showResult(Result res){ //displays the result via the LEDs

	ledShowResult(res .exact, res.approximate);

	if(debug) printf("Exact: %d\nApproximate: %d\n", res.exact, res.approximate);
}

int * generateAnswer(int colourCount){ //this function is used during debug
	int * answer;
	answer = malloc(sizeof(*answer) * codeLength);
	int i;

	if (debug) printf("Secret code: ");

	for (i=0; i<codeLength; i++){
		answer[i] = (rand() % colourCount) +1; 
		if (debug) printf("%d  ", answer[i]);
	}

	if (debug) printf("\n");

	return answer;
}




void main(int argc, char ** argv){ //main method
	int * answer, * guess;
	Result res;
	int opt, cCount, attempts;

	debug = 0; 
	codeLength = 3;
	cCount=3;
	attempts=0;
	srand(time(NULL)); 
	

	answer=malloc(sizeof(*answer) * codeLength);

	initialiseMastermindIO();

	opterr=0;
	while((opt=getopt(argc, argv, "dc:n:")) != -1){
		switch(opt){
		case 'd':
			debug = 1;
			break;
		case 'c':
			cCount = (*optarg -48); 
			break;
		case 'n':
			codeLength = (*optarg - 48); 
			break;
		case'?':
			switch (optopt){
			case 'n' :
				printf("option argument missing: \n-n \e[4mcode_length\e[0m = Specify the code length to guess\n");
				break;
			case 'c':
				printf("option argument missing: \n-c \e[4mno_of_colours\e[0m = Specify the number of colours used\n");
				break;
			default:
				printf("Unidentified option: %c\n", optopt);
				break;
			}
			exit(0);
		}
	}

	if (cCount<=0 || codeLength<=0) {
		printf("Argument options must be more than 0");
		exit(0);
	}


	answer = generateAnswer(cCount);

	do {
		guess = getGuess();
		attempts++;
		res = checkGuess(guess, answer);
		if (res.exact == codeLength) break;
		showResult(res);
		usleep(100000);
	} while (1);

	ledSuccess();
}


