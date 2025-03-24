#include <stdio.h>
#include <stdint.h>


uint32_t sqrt_fixed_q9_23(uint32_t x) {

    // 1. lsls r0, r0, #7 : 將輸入左移 7 位 (乘以 2^7)
    uint32_t r0 = x << 7;
    // 2. mov r2, r0 : 複製 r0 到 r2
    uint32_t r2 = r0;

    // 3. 迴圈（模擬 .irp 1,2,...,13）逐步逼近平方根
    for (int shift = 1; shift <= 13; ++shift) {
        // r1 = r0 + (r0 >> shift)
        uint32_t temp = r0 + (r0 >> shift);
        // 模擬 adds 指令的溢位檢查：若溢位則 temp 會小於 r0
        if (temp < r0) {
            continue;
        }
        // 再計算：r1 = temp + (temp >> shift)
        uint32_t temp2 = temp + (temp >> shift);
        if (temp2 < temp) {
            continue;
        }
        // 若均無溢位，則更新 r0 與 r2
        r0 = temp2;                // 相當於 movcc r0, r1
        r2 = r2 + (r2 >> shift);     // 相當於 addcc r2, r2, r2, lsr #shift
    }

    // 4. negs r0, r0 : 將 r0 取負
    int32_t neg = - (int32_t)r0;

    // 5. umull r1, r3, r0, r2 : 32x32→64 位無符號乘法，低32位到 r1，高32位到 r3
    uint64_t prod = (uint64_t)(uint32_t)neg * r2;
    uint32_t r3 = (uint32_t)(prod >> 32);

    // 6. adds r0, r2, r3, lsr #1 : r0 = r2 + (r3 >> 1)
    r0 = r2 + (r3 >> 1);

    // 7. lsrs r0, r0, #8 : 將 r0 右移 8 位，調整回 Q9.23 固定點格式
    r0 >>= 8;

    return r0;
}

int main(void) {

    uint32_t mantissa32 = 2 << 23;
    uint32_t new_mantissa = sqrt_fixed_q9_23(mantissa32);

 
    int64_t msquared = (int64_t)new_mantissa * new_mantissa;
    int64_t x0 = (int64_t)mantissa32 << 23;
    int64_t residual = x0 - msquared;
    if (residual > new_mantissa) {
        new_mantissa += 1;
    }


    printf("%u\n", new_mantissa);
    return 0;
}
