#include <stdio.h>
#include <stdlib.h>

#include <codegen/start.h>
#include <codegen/exceptions.h>

void
emit_try_block(Statement_t *stmt)
{

    int lbl = new_label();
    int lbl_after = new_label();

    emit(".global jmp_struct\n");
    emit("lea rdi, jmp_struct\n");
    emit("call setjmp\n");
    emit("cmp rax, 0\n"
         "jne lbl%d\n",
         lbl
        );

    emit("lea rdi, jmp_struct\n");
    emit("call add_exception\n");

    emit_statements(&stmt->if_block);

    emit("call leave_exception\n");

    emit("jmp lbl%d\n", lbl_after); // Jump after try block if no exception is raised

    emit("lbl%d:\n", lbl);

    emit_statements(&stmt->else_block); // except block

    emit("lbl%d:\n", lbl_after);
}

void
emit_raise(Statement_t *stmt)
{
    int lbl = new_label();

    emit("call get_exception_env\n");
    emit("push rax\n");
    emit("call leave_exception\n");
    emit("pop rax\n");
    emit("cmp rax, 0\n");
    emit("jne lbl%d\n", lbl);
    emit("call exception\n");
    emit("lbl%d:\n", lbl);
    emit("mov rdi, rax\n");
    emit("mov rsi, 1\n");
    emit("call longjmp\n");
}