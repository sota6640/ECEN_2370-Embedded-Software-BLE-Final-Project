# ECEN_2370-Embedded-Software-Projects

For this BLE Project, I developed a UART / LEUART peripheral that can simultaneously transmit and receive data resulting in two independent state machines that is required by the LEUART driver. The read state machine is interrupt driver and can utilize the same interrupt service routing (ISR) as  my write operation would. The read state machine will not receive and interrupt until the START frame is received and not receive any interrupts after the START Frame has been received. In the BLE app as well as Simplicity Studio, the START frame is the '#' sign and the STOP frame is the '!' sign, and any characters within the START and the STOP frame will be considered the command. After receiving the completed command, the read state machine will schedule an event to be serviced so the command can be evaluated and parsed.

The code was written in Simplicity Studio.
