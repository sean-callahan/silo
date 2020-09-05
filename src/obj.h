#ifndef OBJ_H
#define OBJ_H

typedef enum Opcode      Opcode;
typedef enum Register    Register;
typedef enum OperandType OperandType;

typedef struct Operand  Operand;
typedef struct Bytecode Bytecode;

enum Opcode {
    RET,    /* return */
    PUSH,   /* push to stack */
    POP,    /* pop from stack */
    MOV32,  /* 32-bit int move */
    MOV64,  /* 64-bit int move */
    MOVF32, /* 32-bit float move */
    MOVF64, /* 64-bit float move */
    ADD32,  /* 32-bit int add */
    ADD64,  /* 64-bit int add */
    ADDF32, /* 32-bit float add */
    ADDF64, /* 64-bit float add */
    SUB32,  /* 32-bit int subtract */
    SUB64,  /* 64-bit int subtract */
    SUBF32, /* 32-bit float subtract */
    SUBF64, /* 64-bit float subtract */
};

enum Register {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
};

enum OperandType {
    OP_TYPE_NONE,
    REGISTER,
    OFFSET,
    IMMEDIATE,
};

struct Operand {
    OperandType type;
    union {
        Register reg;
        long offset;
        long imm;
    };
};

struct Bytecode {
    Bytecode *next;
    Opcode  op;
    Operand from;
    Operand to;
};

long code_read(const long handle, const Bytecode *dst);
long code_write(const long handle, Bytecode *src);

#endif