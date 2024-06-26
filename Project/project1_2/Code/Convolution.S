.import input.S

.text
main:
    la a1, image_length
    lw t0, 0(a1)         # t0为 image 长度，大矩阵的列数
    la a1, image_width
    lw t1, 0(a1)         # t1为 image 宽度，大矩阵的行数

    la a1, kernel_length
    lw t2, 0(a1)         # t2为 kernel 长度，小矩阵的列数
    la a1, kernel_width
    lw t3, 0(a1)         # t3为 kernel 宽度，小矩阵的行数

    la s10, image        # s10 保存 image 地址
    la s11, kernel       # s11 保存 kernel 地址

    # 输出矩阵的大小
    sub t4, t0, t2       # t4 = 输出列数 = image 列数 - kernel 列数 + 1  
    addi t4, t4, 1
    sub t5, t1, t3       # t5 = 输出行数 = image 行数 - kernel 行数 + 1
    addi t5, t5, 1


    li s1, 0          # i = 0
result_row_loop:
    bge s1, t5, result_row_loop_end  

    li s2, 0          # j = 0
result_column_loop:
    bge s2, t4, result_column_loop_end  

    li s0, 0          # sum = 0
    li s3, 0          # ki = 0
kernel_row_loop:
    bge s3, t3, kernel_row_loop_end  

    li s4, 0          # kj = 0
kernel_column_loop:
    bge s4, t2, kernel_inner_end 

    # sum += image[(i + ki) * image_column + (j + kj)] * kernel[ki * kernel_column + kj]
    add t6, s1, s3    # t6 = i + ki
    mul t6, t6, t0    # t6 *= image_column
    add t6, t6, s2    # t6 += j
    add t6, t6, s4    # t6 += kj
    slli t6, t6, 2    # 计算偏移量（整数为4字节）

    add a1, s10, t6   # a1 = &image[t6]
    lw a1, 0(a1)      # a1 = image[t6]

    mul s9, s3, t2    # s9 = ki * kernel_column
    add s9, s9, s4    # s9 += kj
    slli s9, s9, 2    # 计算偏移量

    add a2, s11, s9   # a2 = &kernel[s9]
    lw a2, 0(a2)      # a2 = kernel[s9]

    mul a1, a1, a2    # a1 *= a2
    add s0, s0, a1    # sum += a1

    addi s4, s4, 1    # kj++
    j kernel_column_loop

kernel_inner_end:
    addi s3, s3, 1    # ki++
    j kernel_row_loop

kernel_row_loop_end:
    # 打印结果
    mv a1, s0
    li a0, 1
    ecall
    # 空格
    li a1, 32
    li a0, 11
    ecall

    addi s2, s2, 1    # j++
    j result_column_loop

result_column_loop_end:
    # 换行
    li a1, 10
    li a0, 11
    ecall
    addi s1, s1, 1    # i++
    j result_row_loop

result_row_loop_end:
    li a0, 10
    ecall