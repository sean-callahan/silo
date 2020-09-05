#include <unistd.h>
#include <stdint.h>

#include "obj.h"

 long write_uint64(const long handle, const uint64_t n) {
    unsigned char b[8];
    b[0] = (unsigned char)n;
    b[1] = (unsigned char)(n >> 8);
    b[2] = (unsigned char)(n >> 16);
    b[3] = (unsigned char)(n >> 24);
    b[4] = (unsigned char)(n >> 32);
    b[5] = (unsigned char)(n >> 40);
    b[6] = (unsigned char)(n >> 48);
    b[7] = (unsigned char)(n >> 56);
    return write(handle, b, 8);
}

 long write_uint8(const long handle, const uint8_t n) {
    return write(handle, &n, 1);
}

long write_operand(const long handle, const Operand op) {
    long n = 0;
    n += write_uint8(handle, (uint8_t)op.type);
    switch (op.type) {
        case REGISTER:
            n += write_uint8(handle, (uint8_t)op.reg);
            break;
        case IMMEDIATE:
            n += write_uint64(handle, (uint64_t)op.imm);
            break;
        case OFFSET:
            n += write_uint64(handle, (uint64_t)op.offset);
            break;
        default:
            break;
    }
    return n;
}

long read_uint8(const long handle, const uint8_t *n) {
    return read(handle, n, 1);
}

long read_uint64(const long handle, uint64_t *n) {
    unsigned char b[8];
    long r = read(handle, b, 8);
    if (r != 8) {
        return r;
    }
    *n = ((uint64_t)b[0]) |
        ((uint64_t)b[1])<<8 |
        ((uint64_t)b[2])<<16 |
        ((uint64_t)b[3])<<24 |
        ((uint64_t)b[4])<<32 |
        ((uint64_t)b[5])<<40 |
        ((uint64_t)b[6])<<48 |
        ((uint64_t)b[7])<<56;
    return r;
}

long read_operand(const long handle, const Operand *op) {
    long n = 0;
    n += read_uint8(handle, &op->type);
    switch (op->type) {
        case REGISTER:
            n += read_uint8(handle, &op->reg);
            break;
        case IMMEDIATE:
            n += read_uint64(handle, &op->imm);
            break;
        case OFFSET:
            n += read_uint64(handle, &op->offset);
            break;
        default:
            break;
    }
    return n;
}

long code_read(const long handle, const Bytecode *dst) {
    if (!dst) {
        return 0;
    }
    long n = 0;
    return 0;
}

long code_write(const long handle, Bytecode *src) {
    if (!src) {
        return 0;
    }
    long n = 0;
    Bytecode *b = src;
    while (b) {
        n += write_uint8(handle, (uint8_t)b->op);
        n += write_operand(handle, b->from);
        n += write_operand(handle, b->to);
        b = b->next;
    }
    return n;
}