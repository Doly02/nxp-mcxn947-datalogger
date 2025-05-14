This folder contains signal recordings that are essential for detecting power loss events and for analyzing how the firmware of the digital data logger responds to such conditions.
The log files are named using the pattern `pwrloss-x.sal`, where `x` is a number indicating the capture sequence. 

Note: 
- In capture `x = 0`, the recording started shortly after the digital data logger was powered on. Therefore, the initial voltage rise (power-up transition) is not visible in this trace.
- - In captures `x = {1,2,3,4}`, the recording was started before the digital data logger was connected to power. As a result, these traces include the full power-up sequence, including the rising edge of the `VIN` signal.


To visualize the recorded signals, you need the Logic 2  application from Saleae.

Signal Descriptions:
- `TIMER_state` signal indicates when the power loss detection is enabled or triggered (main CTIMER signal).
- `CMP_state` signal indicates the comparator's response to power loss.
- `VIN` signal is the input voltage (5V branch).


On the signal captures, it can be observed that around the 16th second, the power loss detection is enabled. This is indicated by:
- `TIMER_state` transitioning from `1` to `0`
- `CMP_state` stays at `0`

If a power loss is subsequently detected, it is indicated by:
- `CMP_state` transitioning from `0` to `1`
- `TIMER_state` transitioning from `1` to `0`

At this moment, the firmware initiates a dedicated procedure (RECORD_PowerlossFlush) that ensures no data is lost and the file system remains intact during the power-down event. This includes safely closing open files and flushing remaining data from internal buffers to the storage medium.

It is also important to monitor the `VIN` signal in the voltage range from 5 V down to 3.5 V. When the voltage approaches 3.5 V, the LDO regulator may become unstable, which can affect the operation of the digital logger.