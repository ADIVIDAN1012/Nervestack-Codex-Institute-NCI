void dump_scope(Scope* scope) {
    printf("--- Dump Scope %p ---\n", scope);
    for (int i = 0; i < scope->symbol_count; i++) {
        printf("  [%d] '%s': Type %d\n", i, scope->symbols[i].name, scope->symbols[i].value ? scope->symbols[i].value->type : -1);
    }
    if (scope->parent) {
        printf("  Parent: %p\n", scope->parent);
        dump_scope(scope->parent);
    }
    printf("--- End Dump ---\n");
}
