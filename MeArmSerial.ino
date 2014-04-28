#include <Servo.h>
#include <math.h>

Servo servoA;  // create a servo object 
Servo servoB;  // create a servo object 
Servo servoC;  // create a servo object 
Servo servoD;  // create a servo object 

Servo servos[4] = {servoA,servoB,servoC,servoD};
int current[4]={90,90,90,90};
int target[4]={90,90,90,90};

int min=0;
int max=135;

String inputString = "";      // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup()
{
    Serial.begin(9600);

    servoA.attach(9);
    servoB.attach(10);
    servoC.attach(11);
    servoD.attach(12);

    for(int i=0;i<4;i++)
        servos[i].write(90);
}

int del=40;

unsigned long cycle=0;

void loop()
{
    char lineBuffer[255];

    if (stringComplete) {
        inputString.trim();
        inputString.toCharArray(lineBuffer,255);
        processCommand(lineBuffer); 
        inputString="";
        stringComplete = false;
    }

    unsigned long ms=millis();
    if(ms%del==0)
    {
        for(int i=0;i<4;i++)
        {
            if(target[i]>current[i]) current[i]++;
            if(target[i]<current[i]) current[i]--;
            servos[i].write(current[i]);
        }
    }
}

int delta(int i)
{
    return abs(target[i]-current[i]);
}

void processCommand(char* line)
{
    Serial.println(line); 

    uint8_t char_counter = 0;  
    char letter;
    float value;
    int int_value;

    int i=0;
    while(next_statement(&letter, &value, line, &char_counter))
    {
        int_value = trunc(value);

        bool ok=false;
        byte index=0;

        switch(letter)
        {
        case 'a':
        case 'A':
            index=0;
            ok=true;
            break;
        case 'b':
        case 'B':
            index=1;
            ok=true;
            break;
        case 'c':
        case 'C':
            index=2;
            ok=true;
            break;
        case 'd':
        case 'D':
            index=3;
            ok=true;
            break;
        default:
            ok=false;
            break;
        }
        if(ok)
        {
            int angle=int_value;
            if(angle<min) angle=min;
            if(angle>max) angle=max;

            Serial.print("Servo");
            Serial.print(letter);
            Serial.print(" to ");
            Serial.print(angle);
            Serial.println(" degrees.");

            target[index]=angle;
        }
        else
        {
            Serial.print("Bad letter: ");
            Serial.println(letter);
        }
    }

}

/*
SerialEvent occurs whenever a new data comes in the
hardware serial RX.  This routine is run between each
time loop() runs, so using delay inside loop can delay
response.  Multiple bytes of data may be available.
*/
void serialEvent() {
    while (Serial.available()) {
        // get the new byte:
        char inChar = (char)Serial.read(); 
        // add it to the inputString:
        inputString += inChar;
        // if the incoming character is a newline, set a flag
        // so the main loop can do something about it:
        if (inChar == '\n') {
            stringComplete = true;
        } 
    }
}


/*  This following code is taken from the GRBL project
/*  https://github.com/grbl/grbl
/*  See https://github.com/grbl/grbl/blob/master/COPYING for licence
*/

#define MAX_INT_DIGITS 8 // Maximum number of digits in int32 (and float)

// Extracts a floating point value from a string. The following code is based loosely on
// the avr-libc strtod() function by Michael Stumpf and Dmitry Xmelkov and many freely
// available conversion method examples, but has been highly optimized for Grbl. For known
// CNC applications, the typical decimal value is expected to be in the range of E0 to E-4.
// Scientific notation is officially not supported by g-code, and the 'E' character may
// be a g-code word on some CNC systems. So, 'E' notation will not be recognized. 
// NOTE: Thanks to Radu-Eosif Mihailescu for identifying the issues with using strtod().
int read_float(char *line, uint8_t *char_counter, float *float_ptr)                  
{
    char *ptr = line + *char_counter;
    unsigned char c;

    // Grab first character and increment pointer. No spaces assumed in line.
    c = *ptr++;

    // Capture initial positive/minus character
    bool isnegative = false;
    if (c == '-') {
        isnegative = true;
        c = *ptr++;
    } else if (c == '+') {
        c = *ptr++;
    }

    // Extract number into fast integer. Track decimal in terms of exponent value.
    uint32_t intval = 0;
    int8_t exp = 0;
    uint8_t ndigit = 0;
    bool isdecimal = false;
    while(1) {
        c -= '0';
        if (c <= 9) {
            ndigit++;
            if (ndigit <= MAX_INT_DIGITS) {
                if (isdecimal) { exp--; }
                intval = (((intval << 2) + intval) << 1) + c; // intval*10 + c
            } else {
                if (!(isdecimal)) { exp++; }  // Drop overflow digits
            }
        } else if (c == (('.'-'0') & 0xff)  &&  !(isdecimal)) {
            isdecimal = true;
        } else {
            break;
        }
        c = *ptr++;
    }

    // Return if no digits have been read.
    if (!ndigit) { return(false); };

    // Convert integer into floating point.
    float fval;
    fval = (float)intval;

    // Apply decimal. Should perform no more than two floating point multiplications for the
    // expected range of E0 to E-4.
    if (fval != 0) {
        while (exp <= -2) {
            fval *= 0.01; 
            exp += 2;
        }
        if (exp < 0) { 
            fval *= 0.1; 
        } else if (exp > 0) {
            do {
                fval *= 10.0;
            } while (--exp > 0);
        } 
    }


    // Assign floating point value with correct sign.    
    if (isnegative) {
        *float_ptr = -fval;
    } else {
        *float_ptr = fval;
    }


    *char_counter = ptr - line - 1; // Set char_counter to next statement

    return(true);
}

const byte STATUS_EXPECTED_COMMAND_LETTER=0;
const byte STATUS_BAD_NUMBER_FORMAT=1;

void FAIL(byte code)
{
    switch (code)
    {
    case STATUS_EXPECTED_COMMAND_LETTER:
        Serial.write("\nSTATUS_EXPECTED_COMMAND_LETTER\n");
        break;
    case STATUS_BAD_NUMBER_FORMAT:
        Serial.write("\nSTATUS_BAD_NUMBER_FORMAT\n");
        break;
    default:
        Serial.write("\nUnspecified Error.\n");
        break;
    }
}

// Parses the next statement and leaves the counter on the first character following
// the statement. Returns 1 if there was a statements, 0 if end of string was reached
// or there was an error (check state.status_code).
static int next_statement(char *letter, float *float_ptr, char *line, uint8_t *char_counter) 
{
    if (line[*char_counter] == 0) {
        return(0); // No more statements
    }

    *letter = line[*char_counter];
    if(((*letter < 'A') || (*letter > 'Z')) && ((*letter < 'a') || (*letter > 'z'))) {
        FAIL(STATUS_EXPECTED_COMMAND_LETTER);
        return(0);
    }
    (*char_counter)++;
    if (!read_float(line, char_counter, float_ptr)) {
        FAIL(STATUS_BAD_NUMBER_FORMAT); 
        return(0);
    };
    return(1);
}

