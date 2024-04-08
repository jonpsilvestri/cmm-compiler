#include "ast.h"
#include "parser.h"

#ifndef __CODEGEN_H__
#define __CODEGEN_H__

typedef enum{
	// operators
	GC_PLUS,
	GC_MINUS,
	GC_MULT,
	GC_DIV,
	GC_ASSG,

	// variables
	GC_GLOBAL,
	GC_LOCAL,
	
	// jumps
	GC_GOTO,
	GC_LABEL,

	// function operations
	GC_RETRIEVE,
	GC_ENTER,
	GC_LEAVE,
	GC_PARAM,
	GC_CALL,
	GC_RETURN_VOID,
	GC_RETURN
} OpType;

typedef enum {
	OPERAND_INTCONST,
	OPERAND_ST_PTR
} OperandType;

typedef struct Operand {
	OperandType operand_type;
	union {
		int iconst;
		Symbol* st_ref;
	} val;
} Operand;

typedef struct Quad{
	OpType op;
	Operand* src1;
	Operand* src2;
	Operand* dest;
	struct Quad* next;
	int nargs;
} Quad;

void generate_println();
void generate_mips();
Quad* newinstr(OpType, Operand*, Operand*, Operand*);
Quad* gen_code_assg_stmt(ASTnode*);
Operand* make_operand(OperandType, Symbol*, int);
void create_3ac(ASTnode*);
void append_quad(Quad*);
Symbol* create_tmp();
void print_3ac();
void create_params_3ac(ASTnode*);
void print_global();
int num_st_entries();
void print_3ac_comment(Quad*);

#endif  /* __CODEGEN_H__ */