#include <stdio.h>
#include <stdlib.h>

#include <codegen/start.h>
#include <codegen/exceptions.h>


void
emit_try_block(Statement_t *stmt)
{

    int lbl = new_label();
    int lbl_after = new_label();

    emit_statements(&stmt->if_block);

    emit("jmp lbl%d\n", lbl_after); // Jump after try block if no exception is raised

    emit("lbl%d:\n", lbl);

    emit_statements(&stmt->else_block); // except block

    emit("lbl%d:\n", lbl_after);
}
