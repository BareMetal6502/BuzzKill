# Example Sound Commands
You can use the following examples to test your board. You will need to send the byte sequences as shown, using either an SPI or I2C connection (determined by the J1 jumper). Please see the Hardware Guide for exact connection instructions.

In the examples, the bytes strings are broken up onto separate lines to more clearly illustrate the distinct commands that make up the sequence. Each line also has a comment added to help explain what each command achieves. These are for documentation purposes only; only the numeric values themselves should actually be sent.

## Example 1: Simple 1kHz Sine Wave
```
00 00                Reset all registers to initial values (stops any current sounds)
42 80 3e             Set voice osc0 to 1kHz frequency (sine waveform is default)
8a ff 80             Begin new note on voice osc0 at volume level 8
c1 f1                Set master volume to 15 and enable voice osc0 output
```

## Example 2: Siren Sound
```
00 00                Reset all registers to initial values (stops any current sounds)
42 80 3e             Set voice osc0 to 1kHz frequency (sine waveform is default)
02 08 00             Set mod osc0 to 1/2 Hz
ca 01 40             Connect mod osc0 to voice osc0, using frequency scale patch
8a ff 80             Begin new note on voice osc0 at volume level 8
c1 f1                Set master volume to 15 and enable voice osc0 output
```

## Example 3: Alarm Sound
```
00 00                Reset all registers to initial values (stops any current sounds)
40 04 40 1f 80 60    Set voice osc0 to 500Hz frequency and pulse waveform
02 80 00             Set mod osc0 to 8Hz
10 04 10 00 80 e0    Set mod osc1 to 1Hz and hilltop waveform
c8 04 01 20 14 40    Connect mod osc0 using frequency scale patch, and mod osc1 using amplitude scale patch
8a ff 80             Begin new note on voice osc0 at volume level 8
c1 f1                Set master volume to 15 and enable voice osc0 output
```
