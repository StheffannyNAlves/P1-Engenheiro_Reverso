@ Assembly program inicialization uart

.cpu cortex-m0plus
.thumb


.section .vectors, "ax"
.global __vectors
__vectors:
    .long 0x20042000  /*topo da RAM */
    .long _reset_handler /* onde o codigo começa */


.section .text
.global _reset_handler
.type _reset_handler, %function
_reset_handler:
    
    bl      main

hang:
    b       hang