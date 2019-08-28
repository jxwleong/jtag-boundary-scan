## Summary
1.  [What is this repo about?](#repoIntro)
2.  [What is JTAG?](#jtagIntro)
3.  [JTAG Instuctions](#jtagInstruc)
    1. [BYPASS](#bypass)
    2. [IDCODE](#idcode)
    3. [SAMPLE/PRELOAD](#samPre)
    4. [EXTEST](#extest)
4.  [JTAG I/O signals](#ioSig) 
5.  [Boundary Scan Description Language(BSDL)](#bsdl)
    1. [What is BSDL?](#bsdlIntro)
    2. [How to read BSDL?](#bsdlHowTo)
        1. [Insturction opcode and length](#instructNLength)  
        2. [Device IDCODE](#deviceID) 
        3. [Boundary Scan Cells and Registers Informations](#bscinfo)
6.  [Result](#result)
    1. [Information required for JTAG Boundary Scan](#infoBoundaryScan)  
    2. [Result for System Workbench for STM32](#stm32Workbench)
        1. [IDCODE](#wbIDCODE)
        2. [BYPASS](#wbBYPASS)
        3. [SAMPLE/PRELOAD](#wbSAMPLE_PRELOAD)
        4. [EXTEST](#wbEXTEST)
    3. [Result for Command-line interface (CLI)](#cli)
        1. [IDCODE](#cliIDCODE)
        2. [BYPASS](#cliBYPASS)
        3. [SAMPLE/PRELOAD](#cliSAMPLE_PRELOAD)
        4. [EXTEST](#cliEXTEST)
7.  [References](#refer)   

## <a name="repoIntro"></a> What is this repo about?
This is an project to explore JTAG Boundary Scan by using STM32F103C8T6. In this project, JTAG instructions such as BYPASS, 
IDCODE, SAMPLE/PRELOAD will be used to test the JTAG device(s) in STM32F103C8T6.

## <a name="jtagIntro"></a> What is JTAG?
JTAG is an industrial standard for testing and verifying PCB designs after fabricate (Boundary Scan). Besides that, JTAG is often used as an debugger for hardware such as microcontroller like STM32F103C8T6.

![JTAG Register](https://trello-attachments.s3.amazonaws.com/5d00bb8cf5abae679851c1e1/663x747/aaf8d3e3d635433cda69bcc63c7b9f44/Instruction-decoder.png)  

<div align="center">
  Figure 1. JTAG Register from [2.]  
</div>  
    
Based on Figure 1, Any JTAG devices will have at least 4 I/O pin (TCK, TMS, TDI, TDO) with TRST as optional. For the purpose of this
project, only 4 I/O pins were used.  
  
With so many data lines connected to the device, a controller unit is needed to tell the JTAG device what to do. That control unit is named
Test Access Port (TAP) controller.

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

## <a name="jtagInstruc"></a>  JTAG Instructions  
### <a name="bypass"></a> 1.BYPASS  
By IEEE Standard 1149.1, the instruction code for BYPASS instruction is all 0b1 (depend length of instruction register). This instruction is used to bypass device(s) that are not tested or to perform some specific region of circuit where are prompt to failure.
By referring to Figure 1, after BYPASS instruction is loaded, `TDI` and `TDO` are connected with `Bypass Reg`. The `Bypass Reg` contain one bit of dont't care data. Thus, whenever using BYPASS instruction, there will be numbers of these dont't care data bits depend on how many bypass device(s). `bypass bits = n * 1`.

### <a name="idcode"></a> 2. IDCODE
JTAG IDCODE is a 32-bit device specific part number. Athouhg it's not a compulsory instruction specified by IEEE, but most JTAG device
have this instruction. Most of the time, the IDCODE of JTAG device is used to get Boundary Scan Information from BSDL files.
By referring to Figure 1, after IDCODE instruction is loaded, `TDI` and `TDO` are connected with `Device ID Reg`. If there's only
one device, then shift a 32-bit data from `TDI` so that the 32-bit device ID can be shifted out from `Device ID Reg`. If there're more
than one JTAG device, then the total number of bits to shift is `shift bits = n * 32`.

Besides, there's another way to get IDCODE of JTAG device that is reset the TAP controller to TEST_LOGIC_RESET state. By resetting
the Tap state machine, the IDCODE instruction will be loaded automatically into `Instruction Register`.

```
After reset, you can read the Device ID Register (default). To perform any other action, you must move 
the TAP to the Instruction Register scan cycle to select an appropriate data register. For either type
of scan cycle (data register or instruction register), the first action in the scan cycle is a capture 
operation. The Capture-DR state enables the data register indicated by the current Instruction 
Register contents. The Capture-IR state enables access to the Instruction Register.
```  
 
<div align="center">
  Snippet 1. Read IDCODE after reset TAP State Machine from [5.] pg 5 
</div>  

### <a name="samPre"></a> 3. SAMPLE/PRELOAD
This instruction is required by IEEE Standard 1149.1. This instruction connect `TDI` and `TDO` through the `Boundary scan register`.  
Thus, SAMPLE/PRELOAD instruction allows user to take a snapshot of the system I/O pins witout affecting the functionality `System logic`.

![Boundary Scan Register Sample/Preload](https://i.ibb.co/YDsHXfp/ezgif-com-gif-maker-1.gif)  

<div align="center">
  Gif 1. Boundary Scan Register from [6.] pg 9 
</div>  

By default (no shifting), the input pin (PIN_IN) data will be connect to `Capture Registers`. After the instruction is loaded then go to `CAPTURE_DR` state, the `PIN_IN` input pin, `OEJ` control pin and `OUTJ` output pin data will be capture by the `Capture Registers` .
After capturing the data (SAMPLE), then proceed to `SHIFT_DR`. At this state, the data wanted to be preload into `OEJ` and `OUTJ`
pin can be shifted in from `TDI`. After shifting the correct test pattern, then go to `UPDATE_DR` state to update the data to 
`Update Registers`. The preloaded data is now ready for `EXTEST` instruction.

### <a name="extest"></a> 4. EXTEST  
This instruction is also a compulsory instruction by IEEE Standard 1149.1. This instruction is often used to test the external
circuitry of the device. For example,
![alt text](https://i.ibb.co/hKqxy4k/Capture.png "Connection between chips")
<div align="center">
  Figure 4. Example of EXTEST from [2.]
</div>    

Based the circuitry on Figure 4, EXTEST instruction can test Stuck-at fault and short circuit between pins/ chips. To test the
Stuck-at fault defect at chip #1, a test pattern of `0bxxxx 1xxx` can be preloaded on chip #1 by using `SAMPLE/PRELOAD` instruction. Then, `EXTEST` instruction can be loaded in chip #1. For chip #2, sample the I/O pins by using `SAMPLE/PRELOAD` instruction. If the
results of sampling shows `0bxxx1 xxxx`. Then, the pin is Stuck-at fault.  
  

![Boundary Scan Register Extest](https://i.ibb.co/XLz80cc/ezgif-com-gif-maker-2.gif)  
<div align="center">
  Gif 2. Boundary Scan Register from [6.] pg 9 
</div>  

Before using `EXTEST` instruction, the test pattern must be preloaded with `SAMPLE/PRELOAD` instruction. After loading `EXTEST` instruction, go to `CAPTURE_DR` state. In this state, the preloaded data at `Update Registers` will drive `INJ`, `PIN_OE`and `PIN_OUT`.
The next test pattern can be shift in by apply `TMS` of 1 and a pulse of `TCK` to go to `SHIFT_DR`. After applying the test pattern, go
to `UPDATE_DR` state to update the `Update Registers` with latest test pattern. Repeat the process by going to `CAPTURE_DR` state if
further `EXTEST` is desired.  

> **Note, me and my [supervisor](https://github.com/chaosAD) noticed that when `EXTEST` instruction is loaded first time, it works fine. But when trying to reload the EXTEST instruction for another test. The MCU seems to regain control of the pin and cause unexpected behaviour. To solve this, if the EXTEST instruction is already loaded and directly shift the data from TDI and TDO. In short, if multiple EXTEST is needed, make sure don't reload the instruction register.**

## <a name="ioSig"></a> JTAG I/O signals
![Example of JTAG waveform](https://trello-attachments.s3.amazonaws.com/5cee3006c401286b7627b5c5/5d01a23eae083935945dcb4a/af60b1f0e726cff13dfc213a915b40b6/jtagWave.png)
<div align="center">
  Figure 5. Example of JTAG waveform from [6.] pg 25  
</div>  

When the TAP controller enter `SHIFT_IR` or `SHIFT_DR` state, the first `TCK` clock cycle does not shift the data from `TDI`. Instead, at the second `TCK` clock cycle, `TDI` and `TDO` is shifted. By referring from Figure5, when current state is `CAPTURE_DR` then apply `TMS` of 1 and a pulse of `TCK` to transition to `SHIFT_DR` state. Then, at the second `TCK` cycle of `SHIFT_DR` the first bits of `TDI` and `TDO` is write and read.
```
During the SHIFT_IR state, an instruction code is entered by shifting data
on the TDI pin on the rising edge of TCK. The last bit of the opcode must
be clocked at the same time that the next state, EXIT1_IR, is activated;
EXIT1_IR is entered by clocking a logic high on TMS. Once in the
EXIT1_IR state, TDO becomes tri-stated again. TDO is always tri-stated
except in the SHIFT_IR and SHIFT_DR states. After an instruction code
is entered correctly, the TAP controller advances to perform the serial
shifting of test data in one of three modes—SAMPLE/PRELOAD,
EXTEST, or BYPASS—that are described below.
```

<div align="center">
  Snippet 2. Documentation from [6.] pg 25  
</div>  
  
Based on Snippet 2, on the last bit of data shift, it must be on the next state which is `EXIT1_DR`. 
> Note that the same principle apply to shifting of data and instruction.  

## <a name="bsdl"></a> Boundary Scan Description Language (BSDL) 
### <a name="bsdlIntro"></a> What is BSDL?  
BSDL is a subset of VHDL which is a hardware description language which provide description on how a particular JTAG device need be
be implemented for boundary scan. There are a few important informations that need to be extract from BSDL file for performing
boundary scan such as:  

* **Device Instructions Opcode**: JTAG boudary  scan instruction opcode and length of the device in binary to perform boundary scan. 
* **Boundary Scan Description**: This description provide the information about the structure of boundary scan cell of the device. 
Most pin on a device will have three boundary scan cells, input, output and control. It also describe what state of the cells pin
are need to perform boundary scan instructions.  
* **Package Pin  Mapping**:  This is used to determine the internal connections within an integrated circuit. It also describe how the
pads of the device are wired to external pins.  
* **IDCODE Register**: This entity specified the device specific IDCODE number of the device.  

### <a name="bsdlHowTo"></a> How to read BSDL?  
#### <a name=instructNLength></a> Instruction Opcode and length
```vhdl
-- Specifies the number of bits in the instruction register.

   attribute INSTRUCTION_LENGTH of STM32F1_Low_Med_density_value_LQFP48 : entity is 5;

-- Specifies the boundary-scan instructions implemented in the design and their opcodes.
   
   attribute INSTRUCTION_OPCODE of STM32F1_Low_Med_density_value_LQFP48 : entity is 
     "BYPASS  (11111)," &
     "EXTEST  (00000)," &
     "SAMPLE  (00010)," &
     "PRELOAD (00010)," &
     "IDCODE  (00001)";
```  
<div align="center">
  Snippet 3. Boundary Scan Instruction code and length from [9.] line 149
</div> 
By referring the attributed on Snippet 3, the instruction length of this device is 5-bit. The instruction opcode was also stated clearly which is:<br/>
    
* **BYPASS** : 5b11111  
* **EXTEST** : 5b00000 
* **SAMPLE** : 5b00010  
* **PRELOAD** : 5b00010  
* **IDCODE**  : 5b00001  

#### <a name=deviceID></a> Device IDCODE
```vhdl
-- Specifies the bit pattern that is loaded into the DEVICE_ID register during the IDCODE 
-- instruction when the TAP controller passes through the Capture-DR state.
   
   attribute IDCODE_REGISTER of STM32F1_Low_Med_density_value_LQFP48 : entity is 
     "XXXX" &              -- 4-bit version number
     "0110010000100000" &  -- 16-bit part number -- 420
     "00000100000" &       -- 11-bit identity of the manufacturer
     "1";                  -- Required by IEEE Std 1149.1
```  
<div align="center">
  Snippet 4. Device IDCODE from [9.] line 168
</div>  
  
Based on Snippet 4, the IDCODE for the Boundary Scan Device is in binary `0bXXXX 0110 0100 0010 0000 0000 0100 0001` and in hex form is `0xX6420041`.  

![Boundary Scan IDCODE](https://i.ibb.co/VvTfm76/JTAGIDcode.png)
<div align="center">
  Figure 6. Booundary Scan IDCODE from [10.] pg 1086
</div>   
  
  
  
But by referring Figure 6, the actual JTAG boundary scan device IDCODE is `0x16410041` which is `0b1 0110 0100 0001 0000 0000 0100 0001` in binary.  
  
#### <a name=bscinfo></a> Boundary Scan Cells and Registers Informations  
```vhdl
-- Specifies the length of the boundary scan register.
   
   attribute BOUNDARY_LENGTH of STM32F1_Low_Med_density_value_LQFP48 : entity is 232;
```
<div align="center">
  Snippet 5. Device IDCODE from [9.] line 185
</div>  
  
Based on Snippet 5, the length of boundary scan register is 232. This means that when running `SAMPLE/PRELOAD` and `EXTEST` instructions, we need to shift in 232 bits of data. Besides, the sampling result from `SAMPLE/PRELOAD` will have to shift out 232 bits by shifting in 232 btis of data.
   
```vhdl
-- The following list specifies the characteristics of each cell in the boundary scan register from 
-- TDI to TDO. The following is a description of the label fields:
--      num     : Is the cell number.
--      cell    : Is the cell type as defined by the standard.
--      port    : Is the design port name. Control cells do not have a port name.
--      function: Is the function of the cell as defined by the standard. Is one of input, output2, 
--                output3, bidir, control or controlr.
--      safe    : Specifies the value that the BSR cell should be loaded with for safe operation 
--                when the software might otherwise choose a random value.
--      ccell   : The control cell number. Specifies the control cell that drives the output enable 
--                for this port.
--      disval  : Specifies the value that is loaded into the control cell to disable the output 
--                enable for the corresponding port.
--      rslt    : Resulting state. Shows the state of the driver when it is disabled.
   
   attribute BOUNDARY_REGISTER of STM32F1_Low_Med_density_value_LQFP48 : entity is 
--     
--    num	cell	port		function	safe  [ccell  disval  rslt]
--
                ...
      "216	(BC_1,	*,		CONTROL,	1),				" &
      "215	(BC_1,	PC13,		OUTPUT3,	X,	216,	1,	Z),	" &
      "214	(BC_4,	PC13,		INPUT,		X),				" &      
```  
<div align="center">
  Snippet 6. Boundary Scan Cell Sturture from [9.] line 189
</div>  
  
Based on Snippet 6, for  
**SAMPLE/PRELOAD**  
In this case, we want to sample the current input status of pin `PC13`. Note the `disval` of cell 215. To disable the output enable, cell 215 need to set to 1. By referring Gif 1. at [here](#samPre), when `CONTROL` register is set to HIGH (1), the tri-state output will be disable. Thus, the input data of pin PC13 can be captured.  
 
**EXTEST**  
Let say we want to set the output of pin PC13 to HIGH (1). Based on the `disval` description of control cell (215), to enable the tri-state output buffer, `CONTROL` register is set to LOW (0) so that the preloaded data at `Update Register` of `OUTPUT` can be drive to pin PC13. For more info on how `EXTEST` work, please click [here](#extest).  

## <a name="result"></a> Result  
![JTAG Architecture for STM32F103xx](https://trello-attachments.s3.amazonaws.com/5d0ad54197302875cae43986/577x380/bb87a975c30916ed276583988f1eab23/JTAG_Architecture.jpg)   
<div align="center">
  Figure 7. JTAG Architecture for STM32F103xx from [10.] pg 1084  
</div>    
There for two jtag devices in STM32F103C8T6 as shown at Figure 7. When doing JTAG instructions, both devices must be considered. Based on Figure7, the instruction length of Boundary scan TAP (5-bits) and Cortex-M3 TAP are different. Not only the instruction length of both devices are different but also the instruction opcode as well as the device's ID are different.  

### <a name="infoBoundaryScan"></a> Information required for JTAG Boundary Scan 

####1. Instruction opcode for different TAP devices  

| Instruction(binary)/ TAP devices 	| IDCODE 	| BYPASS 	| SAMPLE/PERLOAD 	| EXTEST 	|
|----------------------------------	|:------:	|--------	|----------------	|--------	|
| Boundary Scan TAP                	|  00001 	|  11111 	|      00010     	|  00000 	|
| Cortex-M3 TAP                    	|  1110  	|  1111  	|    0001/0010   	|  0000  	|  

<div align="center">
  Table 1. The JTAG instructions opcode for two TAP devices in STM32F103C8T6 from [9.] line 149 and [11.] pg75-76
</div>  

When doing boundary scan instruction such as `SAMPLE/PRELOAD` and `EXTEST`, the boundary scan instruction will be loaded into Boudary Scan TAP and Cortex-M3 TAP will be loaded with `BYPASS` instruction.  

####2. Device ID for both TAP devices  
![IDCODE for both TAPs](https://i.ibb.co/C0HnHKK/IDCODE.png)  
<div align="center">
  Figure 8. IDCODE for two TAP devices in STM32F103C8T6 from [10.] pg1086
</div>   

Based on Figure8, the IDCODE for Boundary Scan TAP is `0x16410041` and Cortex-M3 TAP is `0x3BA00477`.  

####3. Value to write at boundary scan cell  

| BoundaryScanCell/ JTAG Instruction 	| INPUT 	| OUTPUT 	| CONTROL 	|
|------------------------------------	|-------	|--------	|---------	|
| SAMPLE/PRELOAD                     	|  0/1  	|   0/1   	|    1    	|
| EXTEST                             	|   x   	|   0/1  	|    0    	|  
> For **SAMPLE/PRELOAD** instruction, INPUT and OUTPUT cell (0/1) is set by user externally and CONTROL is preloaded.  
> For **EXTEST** instruction, OUTPUT cell (0/1) and CONTROL was set by user by preloading the data using **SAMPLE/PRELOAD**.
<div align="center">
  Table 2. Data to be preloaded to boundary scan register respect with to instructions.
</div>  

By referring to [Boundary Scan Cells and Registers Informations](#bscinfo), we know that the length of boundary scan cell is `232` which mean that we need to shift in 232 bits via TDI. Basically, a I/O is connected to three boundary scan cells (INPUT, OUTPUT and CONTROL). Depend on which instruction used, the datat shift in for boundary scan cells are different.  

**1. SAMPLE/PRELOAD**
To sample the INPUT boundary scan cell state (0/1). The CONTROL cell are set to 1 to disable the output enable. At the same time, the data wanted to be preload for EXTEST can shift in at the same time.  

**2. EXTEST**
To do EXTEST operation which set the OUTPUT cell. The CONTROL cell are set to 0 to enable the output enable. Then, the desired OUTPUT state (0/1) preloaded can be drive to the I/O pin. 
> The first bit shifted out from TDO will be the LSB of result. The shifting of all JTAG operations is from LSB to MSB.  

### <a name="stm32Workbench"></a> System Workbench for STM32  
#### <a name="wbIDCODE"></a> **1. IDCODE**  
#### <a name="wbBYPASS"></a> **2. BYPASS**  
#### <a name="wbSAMPLE_PRELOAD"></a> **3. SAMPLE/PRELOAD**  
#### <a name="wbEXTEST"></a> **4. EXTEST**  

### <a name="cli"></a> Command-line interface (CLI) 
#### <a name="cliIDCODE"></a> **1. IDCODE**  
#### <a name="cliBYPASS"></a> **2. BYPASS**  
#### <a name="cliSAMPLE_PRELOAD"></a> **3. SAMPLE/PRELOAD**  
#### <a name="cliEXTEST"></a> **4. EXTEST**   

## <a name="refer"></a> References
[1.] [JTAG - Wikipedia](https://en.wikipedia.org/wiki/JTAG)  
[2.] [12 3 DFT2 JTAG Instruction](https://www.youtube.com/watch?v=XEN01h9qkC4)  
[3.] [What is JTAG and how can I make use of it?](https://www.xjtag.com/about-jtag/what-is-jtag/)  
[4.] [Technical Guide to JTAG](https://www.xjtag.com/about-jtag/jtag-a-technical-overview/)  
[5.] [DSP56300 JTAG Examples](https://www.nxp.com/docs/en/application-note/AN2074.pdf)  
[6.] [AN 39: IEEE 1149.1 (JTAG) Boundary-Scan Testing in Altera Devices
](https://www.intel.com/content/dam/www/programmable/us/en/pdfs/literature/an/an039.pdf)  
[7.] [BSDL Files](https://www.xjtag.com/about-jtag/bsdl-files/)  
[8.] [BSDL Tutorial](https://www.corelis.com/education/tutorials/bsdl-tutorial/#What_is_BSDL)  
[9.] [STM32F1_Low_Med_density_value_LQFP48.bsd](https://trello-attachments.s3.amazonaws.com/5cee3006c401286b7627b5c5/5cee300601299d2ce2c732fd/2e138503f7ce2022af8ae01e40947948/STM32F1_Low_Med_density_value_LQFP48.bsd)  
[10.] [STM32F101xx, STM32F102xx, STM32F103xx, STM32F105xx and STM32F107xx advanced Arm®-based 32-bit MCUs - Reference manual](https://www.st.com/content/ccc/resource/technical/document/reference_manual/59/b9/ba/7f/11/af/43/d5/CD00171190.pdf/files/CD00171190.pdf/jcr:content/translations/en.CD00171190.pdf)  
[11.] [ARM® Debug Interface Architecture Specification ADIv5.0 to ADIv5.2](https://static.docs.arm.com/ihi0031/c/IHI0031C_debug_interface_as.pdf)
