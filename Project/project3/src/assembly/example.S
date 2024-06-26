.data

.Message_SW2:
        .string "Welcome! This is SW2!"
.Message_SW1:
        .string "Welcome! This is SW1!"

.section .text
.globl displayWelcomeScreen
.globl detectButtonPress

.type displayWelcomeScreen, @function
.type detectButtonPress, @function

entry_point:
        .zero   4

detectButtonPress:
        addi    sp, sp, -16
        sw      ra, 12(sp)
        sw      s0, 8(sp)
        addi    s0, sp, 16
button_check:
        li      a0, 5
        call    Get_Button
        mv      a5, a0
        beq     a5, zero, button_sw2_check
        li      a5, 1
        j       button_return
button_sw2_check:
        li      a0, 6
        call    Get_Button
        mv      a5, a0
        beq     a5, zero, delay_check
        li      a5, 2
        j       button_return
delay_check:
        li      a0, 10
        call    delay_1ms
        j       button_check
button_return:
        mv      a0, a5
        lw      ra, 12(sp)
        lw      s0, 8(sp)
        addi    sp, sp, 16
        jr      ra

displayWelcomeScreen:
        addi    sp, sp, -80
        sw      ra, 76(sp)
        sw      s0, 72(sp)
        addi    s0, sp, 80
        sw      a0, -68(s0)
        li      a0, 0
        call    LCD_Clear
        li      a5, 2016
        sw      a5, -48(s0)
        li      a5, 65536
        addi    a5, a5, -2017
        sw      a5, -44(s0)
        li      a5, 2047
        sw      a5, -40(s0)
        li      a5, 65536
        addi    a5, a5, -2048
        sw      a5, -60(s0)
        li      a5, 31
        sw      a5, -56(s0)
        li      a5, 65536
        addi    a5, a5, -32
        sw      a5, -52(s0)
        li      a5, 3
        sw      a5, -28(s0)
        lw      a4, -68(s0)
        li      a5, 1
        beq     a4, a5, display_sw2
        lw      a4, -68(s0)
        li      a5, 2
        beq     a4, a5, display_sw1
        j       display_finish
display_sw2:
        sw      zero, -20(s0)
        j       sw2_loop_end
sw2_loop:
        lw      a4, -20(s0)
        lw      a5, -28(s0)
        rem     a4, a4, a5
        addi    a5, s0, -48
        slli    a4, a4, 2
        add     a5, a4, a5
        lw      a5, 0(a5)
        sw      a5, -36(s0)
        lw      a4, -36(s0)
        li      a3, 160
        li      a2, 80
        li      a1, 0
        li      a0, 0
        call    LCD_Fill
        li      a0, 20
        call    delay_1ms
        lw      a5, -20(s0)
        addi    a5, a5, 1
        sw      a5, -20(s0)
sw2_loop_end:
        lw      a4, -20(s0)
        li      a5, 29
        ble     a4, a5, sw2_loop
        li      a5, 65536
        addi    a3, a5, -1
        lui     a5, %hi(.Message_SW2)
        addi    a2, a5, %lo(.Message_SW2)
        li      a1, 60
        li      a0, 10
        call    LCD_ShowString
        j       display_finish
display_sw1:
        sw      zero, -24(s0)
        j       sw1_loop_end
sw1_loop:
        li      a4, 50
        lw      a5, -24(s0)
        sub     a4, a4, a5
        lw      a5, -28(s0)
        rem     a4, a4, a5
        addi    a5, s0, -60
        slli    a4, a4, 2
        add     a5, a4, a5
        lw      a5, 0(a5)
        sw      a5, -32(s0)
        lw      a4, -32(s0)
        li      a3, 160
        li      a2, 80
        li      a1, 0
        li      a0, 0
        call    LCD_Fill
        li      a0, 20
        call    delay_1ms
        lw      a5, -24(s0)
        addi    a5, a5, 1
        sw      a5, -24(s0)
sw1_loop_end:
        lw      a4, -24(s0)
        li      a5, 29
        ble     a4, a5, sw1_loop
        li      a5, 65536
        addi    a3, a5, -1
        lui     a5, %hi(.Message_SW1)
        addi    a2, a5, %lo(.Message_SW1)
        li      a1, 60
        li      a0, 10
        call    LCD_ShowString
display_finish:
        li      a5, 4096
        addi    a0, a5, -1096
        call    delay_1ms
        lw      ra, 76(sp)
        lw      s0, 72(sp)
        addi    sp, sp, 80
        jr      ra
