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

![TAP State Machine](https://www.xjtag.com/wp-content/uploads/tap_state_machine.gif)  
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
By IEEE standard, the instruction code for BYPASS instruction is all 0b1 (depend length of instruction register). This instruction
is used to bypass device(s) that are not tested or to perform some specific region of circuit where are prompt to failure.
By referring to Figure 1. , after BYPASS instruction is loaded, `TDI` and `TDO` are connected with `Bypass Reg`. The `Bypass Reg` contain one bit of dont't care data. Thus, whenever using BYPASS instruction, there will be numbers of these dont't care data bits depend on how
many bypass device(s).

### 2. IDCODE
JTAG IDCODE is a 32-bit device specific part number. Athouhg it's not a compulsory instruction specified by IEEE, but most JTAG device
have this instruction. Most of the time, the IDCODE of JTAG device is used to get Boundary Scan Information from BSDL files.
By referring to Figure 1. , after IDCODE instruction is loaded, `TDI` and `TDO` are connected with `Device ID Reg`. If there's only
one device, then shift a 32-bit data from `TDI` so that the 32-bit device ID can be shifted out from `Device ID Reg`. If there're more
than one JTAG device, then the total number of bits to shift is `shift bits = n * 32`.

## References
[1.] [JTAG - Wikipedia](https://en.wikipedia.org/wiki/JTAG)  
[2.] [JTAG Instructions video](https://www.youtube.com/watch?v=XEN01h9qkC4)  
[3.] [What is JTAG and how can I make use of it?](https://www.xjtag.com/about-jtag/what-is-jtag/)  
[4.] [Technical Guide to JTAG](https://www.xjtag.com/about-jtag/jtag-a-technical-overview/)  
