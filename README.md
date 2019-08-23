## What is this repo about?
This is an project to explore JTAG Boundary Scan by using STM32F103C8T6. In this project, JTAG instructions such as BYPASS, 
IDCODE, SAMPLE/PRELOAD will be used to test the JTAG device(s) in STM32F103C8T6.

## What is JTAG?
JTAG is an industrial standard for testing and verifying PCB designs after fabricate. Besides that, JTAG is often used as an debugger
for hardware such as microcontroller like STM32F103C8T6.

![JTAG Register](https://trello-attachments.s3.amazonaws.com/5d00bb8cf5abae679851c1e1/663x747/aaf8d3e3d635433cda69bcc63c7b9f44/Instruction-decoder.png)  

<div align="center">
  Figure 1. JTAG Register from [2.]  
</div>  
    
Based on Figure 1. , Any JTAG devices will have at least 4 I/O pin (TCK, TMS, TDI, TDO) with TRST as optional. For the purpose of this
project, only 4 I/O pins were used.  
  
With so many data lines connected to the device, a controller unit is needed to tell the JTAG device what to do. That control unit is named
Test Access Port(TAP) controller.

![
Machine](https://www.xjtag.com/wp-content/uploads/tap_state_machine.gif)  
<div align="center">
  Figure 2. JTAG TAP Controller (State Machine) from [3.]  
</div>  
  
TAP Controller basiccaly is a 16 states state machine that control the behaviour of JTAG devices. This state machine is controlled by TMS 
and TCK signals. The TMS singals will be captured on the rising edge of TCK. Depend on the current state of the TAP state machine, the JTAG
device can transition to another state to do different operations. For exaple, if current state is `CAPTURE_DR` which capture the data from 
`External Connections` into `Boundary Scan Register` then a pulse of high `TMS` and `TCK` was send to JTAG device. After that, current state 
will be `SHIFT_DR` which will shift data from `TDI` to `TDO`.

## JTAG Instructions  
### 1.BYPASS  
By IEEE Standard 1149.1, the instruction code for BYPASS instruction is all 0b1 (depend length of instruction register). This instruction is used to bypass device(s) that are not tested or to perform some specific region of circuit where are prompt to failure.
By referring to Figure 1. , after BYPASS instruction is loaded, `TDI` and `TDO` are connected with `Bypass Reg`. The `Bypass Reg` contain one bit of dont't care data. Thus, whenever using BYPASS instruction, there will be numbers of these dont't care data bits depend on how many bypass device(s). `bypass bits = n * 1`.

### 2. IDCODE
JTAG IDCODE is a 32-bit device specific part number. Athouhg it's not a compulsory instruction specified by IEEE, but most JTAG device
have this instruction. Most of the time, the IDCODE of JTAG device is used to get Boundary Scan Information from BSDL files.
By referring to Figure 1. , after IDCODE instruction is loaded, `TDI` and `TDO` are connected with `Device ID Reg`. If there's only
one device, then shift a 32-bit data from `TDI` so that the 32-bit device ID can be shifted out from `Device ID Reg`. If there're more
than one JTAG device, then the total number of bits to shift is `shift bits = n * 32`.

Besides, there's another way to get IDCODE of JTAG device that is reset the TAP controller to TEST_LOGIC_RESET state. By resetting
the Tap state machine, the IDCODE instruction will be loaded automatically into `Instruction Register`.

![read IDCODE after reset](https://i.ibb.co/FVH7ggz/reset-Read-IDCOde.png)  
<div align="center">
  Figure 3. Read IDCODE after reset TAP State Machine from [5.] pg5 
</div>  

### 3. SAMPLE/PRELOAD
This instruction is required by IEEE Standard 1149.1. This instruction connect `TDI` and `TDO` through the `Boundary scan register`.  
Thus, SAMPLE/PRELOAD instruction allows user to take a snapshot of the system I/O pins witout affecting the functionality `System logic`.

![Boundary Scan Register Sample/Preload](https://i.ibb.co/YDsHXfp/ezgif-com-gif-maker-1.gif)  

<div align="center">
  Gif 1. Boundary Scan Register from [6.] pg9 
</div>  

By default(no shifting), the input pin (PIN_IN) data will be connect to `Capture Registers`. After the instruction is loaded then go to `CAPTURE_DR` state, the `PIN_IN` input pin, `OEJ` control pin and `OUTJ` output pin data will be capture by the `Capture Registers` .
After capturing the data(SAMPLE), then proceed to `SHIFT_DR`. At this state, the data wanted to be preload into `OEJ` and `OUTJ`
pin can be shifted in from `TDI`. After shifting the correct test pattern, then go to `UPDATE_DR` state to update the data to 
`Update Registers`. The preloaded data is now ready for `EXTEST` instruction.

### 4. EXTEST  
This instruction is also a compulsory instruction by IEEE Standard 1149.1. This instruction is often used to test the external
circuitry of the device. For example,
![alt text](https://i.ibb.co/hKqxy4k/Capture.png "Connection between chips")
<div align="center">
  Figure 4. Example of EXTEST from [2.]
</div>    

Based the circuitry on Figure 4. , EXTEST instruction can test Stuck-at fault and short circuit between pins/ chips. To test the
Stuck-at fault defect at chip #1, a test pattern of `0bxxxx 1xxx` can be preloaded on chip #1 by using `SAMPLE/PRELOAD` instruction. Then, `EXTEST` instruction can be loaded in chip #1. For chip #2, sample the I/O pins by using `SAMPLE/ PRELOAD` instruction. If the
results of sampling shows `0bxxx1 xxxx`. Then, the pin is Stuck-at fault.  
  

![Boundary Scan Register Extest](https://i.ibb.co/XLz80cc/ezgif-com-gif-maker-2.gif)  
<div align="center">
  Gif 2. Boundary Scan Register from [6.] pg9 
</div>  

Before using `EXTEST` instruction, the test pattern must be preloaded with `SAMPLE/PRELOAD` instruction. After loading `EXTEST` instruction, go to `CAPTURE_DR` state. In this state, the preloaded data at `Update Registers` will drive `INJ`, `PIN_OE`and `PIN_OUT`.
The next test pattern can be shift in by apply `TMS` of 1 and a pulse of `TCK` to go to `SHIFT_DR`. After applying the test pattern, go
to `UPDATE_DR` state to update the `Update Registers` with latest test pattern. Repeat the process by going to `CAPTURE_DR` state if
further `EXTEST` is desired.  

## JTAG I/O signals
![Example of JTAG waveform](https://trello-attachments.s3.amazonaws.com/5cee3006c401286b7627b5c5/5d01a23eae083935945dcb4a/af60b1f0e726cff13dfc213a915b40b6/jtagWave.png)
<div align="center">
  Figure 5. Example of JTAG waveform from [6.] pg25  
</div>  

When the TAP controller enter `SHIFT_IR` or `SHIFT_DR` state, the first `TCK` clock cycle does not shift the data from `TDI`. Instead, at the second `TCK` clock cycle, `TDI` and `TDO` is shifted. By refering from Figure5. , when current state is `CAPTURE_DR` then apply `TMS` of 1 and a pulse of `TCK` to transition to `SHIFT_DR` state. Then, at the second `TCK` cycle of `SHIFT_DR` the first bits of `TDI` and `TDO` is write and read.
![Last shift on next TAP state](https://i.ibb.co/pd5mFcy/jtag-io-signals.png)
<div align="center">
  Figure 5. Documentation from [6.] pg25  
</div>  
## References
[1.] [JTAG - Wikipedia](https://en.wikipedia.org/wiki/JTAG)  
[2.] [12 3 DFT2 JTAG Instruction](https://www.youtube.com/watch?v=XEN01h9qkC4)  
[3.] [What is JTAG and how can I make use of it?](https://www.xjtag.com/about-jtag/what-is-jtag/)  
[4.] [Technical Guide to JTAG](https://www.xjtag.com/about-jtag/jtag-a-technical-overview/)  
[5.] [DSP56300 JTAG Examples](https://www.nxp.com/docs/en/application-note/AN2074.pdf)  
[6.] [AN 39: IEEE 1149.1 (JTAG) Boundary-Scan Testing in Altera Devices
](https://www.intel.com/content/dam/www/programmable/us/en/pdfs/literature/an/an039.pdf)  
