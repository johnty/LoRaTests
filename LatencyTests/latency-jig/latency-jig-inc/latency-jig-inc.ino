/**
   Latency measurement code -- measurement jig

   Simple tool to toggle a digital pin and measure the delay until
   the desired response happens. The function of the digital output,
   and the signal to be measured, are determined by whatever external
   hardware is attached.

   The resulting delay in microseconds is printed over the serial port
   so multiple trials can be made into a histogram. The granularity of
   the timer is 4us on standard 16MHz Arduino boards.

   (c) 2015 Andrew McPherson, C4DM, QMUL
   This is licensed under a Creative Commons Attribution-ShareAlike licence.
*/

const int gOutputPin = 2;    // Pin sending the trigger
const int gInputPin = 3;     // Pin reading the response
const int gLEDPin = 13;      // Pin for the LED on the board, for visual feedback

const int gDelayBetweenTrials = 95;  // Measured in milliseconds; only approximate
const int numTrails = 100;
const int maxAdd = 35;
int variableDelay_ms;
int curr_offset;

int counter = 0;
#define VAR_DELAY false

volatile uint8_t *gInputPinPortReg, *gOutputPinPortReg;
uint8_t gInputPinBit, gOutputPinBit;

void setup() {

  variableDelay_ms = gDelayBetweenTrials;
  pinMode(gOutputPin, OUTPUT);
  pinMode(gInputPin, INPUT);
  pinMode(gLEDPin, OUTPUT);

  digitalWrite(gOutputPin, LOW);

  // Do some internal machinations to pull out the register for faster access
  // compared to digitalRead(). This is something of a violation of the abstractions
  // set up in the Arduino/Wiring environment, and might change on future boards.
  gInputPinPortReg = portInputRegister(digitalPinToPort(gInputPin));
  gOutputPinPortReg = portOutputRegister(digitalPinToPort(gOutputPin));
  gInputPinBit = digitalPinToBitMask(gInputPin);
  gOutputPinBit = digitalPinToBitMask(gOutputPin);

  Serial.begin(115200);
}

void loop() {
  unsigned int overflows = 0;
  unsigned int finalCount = 0;

  digitalWrite(gLEDPin, HIGH);

  // **** This code runs with interrupts off for predictable timing ****
  cli();

  // Prepare Timer1 initially stopped
  TCCR1A = 0;  // normal mode
  TCCR1B = 0;  // no clock source
  TCCR1C = 0;  // no force compare
  TIMSK1 = 0;  // no interrupts
  TIFR1 = 1;    // clear overflow bit
  TCNT1 = 0;   // count starts at 0

  // Start timer at 16MHz (1 tick = 1/16us)
  TCCR1B = 1;

  // Toggle pin...
  // Equivalent (but slower): digitalWrite(gOutputPin, HIGH);
  *gOutputPinPortReg |= gOutputPinBit;

  // ...and wait (possibly forever) for response
  // Equivalent (but slower): while(digitalRead(gInputPin) != HIGH) { /* ,,, */ }
  while (!(*gInputPinPortReg & gInputPinBit)) {
    finalCount = TCNT1;  // Capture count
    if (TIFR1 & 1) {
      TIFR1 = 1;
      overflows++;
    }
  }

  TCCR1B = 0;          // Stop timer again

  sei();
  // **** End of interrupts-off code ****

  // Reset pin for next time
  digitalWrite(gOutputPin, LOW);
  digitalWrite(gLEDPin, LOW);

  // Print the result of the trial
  // The testing code itself results in a delay of 1us, so subtract
  // this to get the actual time delay
  unsigned long latencyInTimerTicks = (unsigned long)finalCount + ((unsigned long)overflows << 16);
  unsigned long latencyInMicros = latencyInTimerTicks / 16;
  Serial.print(variableDelay_ms);
  Serial.print(" ");
  Serial.print(variableDelay_ms + curr_offset);
  Serial.print(" ");
  Serial.println(latencyInMicros - 1);
  counter++;
  if (VAR_DELAY)
    curr_offset =  random(0, 50) - 25;
  else
    curr_offset = 0;
  if (counter >= numTrails) {
    counter = 0;
    variableDelay_ms++;
    if (variableDelay_ms > gDelayBetweenTrials + maxAdd) { //reset back to min
      variableDelay_ms = gDelayBetweenTrials;
      while (1)
      {
        //stop here
      }
    }
  }
  delay(variableDelay_ms + curr_offset);
  //todo: figure out timer reinabling so that we can use delayMicroseconds()



}
