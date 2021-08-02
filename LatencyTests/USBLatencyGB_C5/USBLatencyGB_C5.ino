/**
   Arduino latency testing code -- target device

   This code performs the simple function of notifying the host
   via the Serial port when a pin has gone high. There are two
   approaches that can be selected: either a message only when
   the event takes, or a continuous loop of readings.

   (c) 2015 Andrew McPherson, C4DM, QMUL
   This is licensed under a Creative Commons Attribution-ShareAlike licence.

   2021 Johnty Wang IDMIL/McGill
   updates for testing iOS devices:
   - sends C5 (for consistent triggering of the Grand Piano patch) - set to max vol
   -     (note that timing offset of when the comparator on the jig
          should be verified since the waveform is not as immediate
          as the reverse ramp used in the original Max test patch)
     sends C5 note off after 100ms
   - should use longish (1000ms) intra test interval on latency jig to avoid reverb from
     previous notes on signal level for new note. default is 150ms/250ms
*/

const int D0 = 2;
int prev = LOW;
bool armed = true;


void setup() {
  pinMode(D0, INPUT);
}

void loop() {
  // Wait for pin to go high, print trigger, then wait for it to go low again
  int curr_input = digitalRead(D0);

  if (prev == LOW && curr_input == HIGH) {
    armed = true;
  }
  if (curr_input == HIGH && armed) {
    usbMIDI.sendNoteOn(72, 127, 1);
    delay(100);
    usbMIDI.sendNoteOff(72, 0, 1);
    armed = false;
  }
  prev = curr_input;


}
