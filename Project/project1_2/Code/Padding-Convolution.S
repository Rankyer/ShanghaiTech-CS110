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

    addi t4, t3, -1      # t4 = kernel_rows - 1
    srai t4, t4, 1       # t4 = pad_size = (kernel_rows - 1) / 2
    add t5, t0, t4       # t5 = image_column + pad_size
    add t5, t5, t4       # t5 = padded_column = image_column + 2 * pad_size
    add t6, t1, t4       # t6 = img_rows + pad_size
    add t6, t6, t4       # t6 = padded_rows = img_rows + 2 * pad_size

    # 为padded_image分配空间
    li a0, 9             # sbrk
    mul a1, t5, t6       # a1 = padded_column * padded_rows
    slli a1, a1, 2       # a1 *= sizeof(int)
    ecall
    mv s9, a0            # s9保存padded_image的地址

    li s1, 0              # i = 0
padded_image_loop:
    bge s1, t1, padded_image_loop_end  

    li s2, 0              # j = 0
padded_image_inner_loop:
    bge s2, t0, padded_image_inner_loop_end  

    # padded_image[(i + pad_size) * padded_column + (j + pad_size)] = image[i * image_column + j]
    add a1, s1, t4       # a1 = i + pad_size
    mul a1, a1, t5       # a1 *= padded_column
    add a1, a1, s2       # a1 += j
    add a1, a1, t4       # a1 += pad_size
    slli a1, a1, 2       # a1 *= 4 (int大小为4字节)

    mul a2, s1, t0       # a2 = i * image_column
    add a2, a2, s2       # a2 += j
    slli a2, a2, 2       # a2 *= 4 (int大小为4字节)

    add a1, s9, a1       # a1 = &padded_image[a1]
    add a2, s10, a2      # a2 = &image[a2]
    lw a2, 0(a2)         # a2 = image[a2]
    sw a2, 0(a1)         # padded_image[a1] = image[a2]

    addi s2, s2, 1       # j++
    j padded_image_inner_loop

padded_image_inner_loop_end:
    addi s1, s1, 1       # i++
    j padded_image_loop

padded_image_loop_end:
    # 开始卷积运算
    li s1, 0              # i = 0
result_row_loop:
    bge s1, t1, result_row_loop_end  

    li s2, 0              # j = 0
result_column_loop:
    bge s2, t0, result_column_loop_end  

    li s0, 0              # sum = 0
    li s3, 0              # ki = 0
kernel_row_loop:
    bge s3, t3, kernel_row_loop_end  

    li s4, 0              # kj = 0
kernel_column_loop:
    bge s4, t2, kernel_inner_end 

    # 计算 sum += padded_image[(i + ki) * padded_column + (j + kj)] * kernel[ki * kernel_column + kj]
    add t6, s1, s3       # t6 = i + ki
    mul t6, t6, t5       # t6 *= padded_column
    add t6, t6, s2       # t6 += j
    add t6, t6, s4       # t6 += kj
    slli t6, t6, 2       # 索引转换为字节偏移量

    add a1, s9, t6       # a1 = &padded_image[t6]
    lw a1, 0(a1)         # a1 = padded_image[t6]

    mul s8, s3, t2       # s8 = ki * kernel_column
    add s8, s8, s4       # s8 += kj
    slli s8, s8, 2       # 索引转换为字节偏移量

    add a2, s11, s8      # a2 = &kernel[s8]
    lw a2, 0(a2)         # a2 = kernel[s8]

    mul a1, a1, a2       # a1 *= a2
    add s0, s0, a1       # sum += a1

    addi s4, s4, 1       # kj++
    j kernel_column_loop

kernel_inner_end:
    addi s3, s3, 1       # ki++
    j kernel_row_loop

kernel_row_loop_end:
    # 输出结果
    mv a1, s0          
    li a0, 1            
    ecall
    # 空格
    li a1, 32            
    li a0, 11          
    ecall

    addi s2, s2, 1       # j++
    j result_column_loop

result_column_loop_end:
    # 换行
    li a1, 10          
    li a0, 11          
    ecall

    addi s1, s1, 1       # i++
    j result_row_loop

result_row_loop_end:
    li a0, 10          
    ecall


