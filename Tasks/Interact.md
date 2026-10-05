OVERVIEW

Scripting interaction support when robotizing emulation control for automated testing purpose should meet the following objectives:

- The interface should be straightforward to use without making the code using it overcomplicated
- The interface should be to a degree, self documenting
  - What would be on a display as human readable should be human readable in the interface. So for example, if I am checking if the screen is showing the word "Ok" the string passed in the interface should be "Ok" not a array of hex or decimal
- Test code should not be overcomplicated with precise screen coordinates. So those need to be abtracted or implied.
- Observed expected response to stimulus: When testing a system under test, we relentlessly perform operations that are expected to deterministically mutate the state of the system under test in some way, and then deliberately confirm the new state by some operation. If we do an operation without confirming the desired effect, it's not a valid test. It's reasonable to write helpers that combine the stimulus with the deliberate observation of the response (usually combined with a timeout) but we should always be mindful that the expected state mutation is confirmed.

Design

For the stateful interaction layer, create a function for setting the model on VirtualT and in local state. This model setting is retained so we can encapsulate the fact that we will need to use different strategies for confirming state mutation in the device depending on the model.
In LCD display mutation, an 8201A has a different LCD shadow buffer location than the Model 100/102 or 200. They all have different memory addresses for reading the current cursor position.
Create a routine that takes an array of Virtual T socket compatible keyboard input strings, a timeout in milliseconds, an exact match string of expected display output, and a flag indicating whether the operation resets the clears the screen/cursor during the operation as a side-effect.
If the reset flag is true, set the cursor origin to 0th psition. If the reset flag is false, this routine should first use memory reads (rm) to read the cursor position depending on model (for now we only support the NEC PC-8201A model, but the architecture should support future addition). Then it should send the characters. Then it should read the new cursor position via a separate memory read and do a separate memory read to read the LCD memory from the origin cursor location (before the keyboard input) to the current cursor location.
The interaction layer should be designed to be stateful. So it should have a Reset. The Reset should re-command the Radix setting to hex.
If a timeout is it the keyboard command the routine should return a failure indication.
If there is a communication error at any point the routine should return a failure indication.
If there is exact-match output string doesn't match, return an error indication. Note that null and backspace are not expected in the argument string, and will be skipped over in the LCD processing and not impact that match.
Use different error return codes to allow distinguishing success versus specific failures.

Tasks:
- Determine in advance any subroutines that you think you will need to support this logic and add those as preliminary tasks to those which follow.
- Do some legwork using local documentation, web searches and existing library infrastructure to resolve all obvious questions to perform the implementation to support the NEC PC-8201A. For example, confirming ability to read the display memory, detect cursor position/advance, etc. We don't want big-bang debugging trying 
to debug a whole bunch of things at once. This routine is fairly complicated as it is.
- Iterate on creating tests, executing the tests and making corrections until all tests are created and pass.
  - The set of tests for the code must provide several happy path positive tests as well as negative tests cases for important equivalence classes of input parameters.
- If you have a really big question stop and wait for an answer. Otherwise you are free to iterate on the task until done.



