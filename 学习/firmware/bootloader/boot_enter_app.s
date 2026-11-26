.syntax unified
.cpu cortex-m4
.thumb
.section .text.Boot_EnterApp,"ax",%progbits
.global Boot_EnterApp
.type Boot_EnterApp, %function
.thumb_func
Boot_EnterApp:
    /* r0 = MSP, r1 = Thumb Reset_Handler. No C frame after replacing MSP. */
    movs r2, #0
    msr control, r2
    isb
    msr msp, r0
    msr basepri, r2
    msr faultmask, r2
    dsb
    isb
    cpsie i
    blx r1
1:  b 1b
.size Boot_EnterApp, .-Boot_EnterApp
.section .note.GNU-stack,"",%progbits
