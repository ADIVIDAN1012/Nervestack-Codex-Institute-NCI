#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cJSON.h"

// Enum for value types
typedef enum {
    VAL_NIL,
    VAL_BOOL,
    VAL_NUMBER,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_BLUEPRINT,
    VAL_BLUEPRINT_INSTANCE,
    VAL_TOOLKIT,
    VAL_BRIDGE,
    VAL_RANGE
} ValueType;

// Forward declaration of Value
typedef struct Value Value;

void free_value(Value* value);
Value* copy_value(const Value* val);

// Forward declaration of ASTNode
typedef struct ASTNode ASTNode;

// Forward declaration of Scope
typedef struct Scope Scope;

// Struct for Blueprint instance values
typedef struct {
    char* name;
    Scope* scope;
    ASTNode* constructor;
} Blueprint;

// Struct for Blueprint instance values
typedef struct {
    Scope* blueprint_scope;
    Scope* instance_scope;
} BlueprintInstanceValue;

typedef struct {
    Scope* toolkit_scope;
    Scope* exports;
} ToolkitValue;

typedef struct {
    Scope* bridge_scope;
} BridgeValue;

// Struct for values
typedef struct Value {
    ValueType type;
    union {
        double number;
        char *string;
        bool boolean;
        struct FunctionSymbol* function; // For function values
        Blueprint blueprint;
        BlueprintInstanceValue blueprint_instance;
        ToolkitValue toolkit;
        BridgeValue bridge;
        struct {
            double start;
            double end;
        } range;
    } as;
} Value;

// Definition of Symbol
typedef struct Symbol {
    char *name;
    Value *value;
    bool is_constant;
} Symbol;

// Definition of Scope
typedef struct Scope {
    struct Scope* parent;
    Symbol* symbols;
    int symbol_count;
    int symbol_capacity;
} Scope;

// Function prototypes for scope management
Scope* create_scope(Scope* parent);
void destroy_scope(Scope* scope);
Value* get_variable(Scope* scope, const char* name);
void set_variable(Scope* scope, const char* name, Value* value);
void define_variable(Scope* scope, const char* name, Value* value, bool is_const);

Value* copy_value(const Value* val) {
    if (!val) return NULL;
    Value* new_val = (Value*)malloc(sizeof(Value));
    memcpy(new_val, val, sizeof(Value));
    if (val->type == VAL_STRING) {
        new_val->as.string = strdup(val->as.string);
    } else if (val->type == VAL_BLUEPRINT_INSTANCE) {
        // Reference semantics: Just copy pointers
        new_val->as.blueprint_instance.blueprint_scope = val->as.blueprint_instance.blueprint_scope;
        new_val->as.blueprint_instance.instance_scope = val->as.blueprint_instance.instance_scope;
    } else if (val->type == VAL_RANGE) {
        new_val->as.range.start = val->as.range.start;
        new_val->as.range.end = val->as.range.end;
    }
    return new_val;
}

void free_value(Value* value) {
    if (!value) return;
    if (value->type == VAL_STRING && value->as.string) {
        free(value->as.string);
    } else if (value->type == VAL_BLUEPRINT) {
        // Reference semantics: Do not free name or scope as they are shared/shallow copied
        // free(value->as.blueprint.name);
        // destroy_scope(value->as.blueprint.scope);
    } else if (value->type == VAL_BLUEPRINT_INSTANCE) {
        // Reference semantics: Do NOT destroy scopes here as they might be shared.
        // This causes a memory leak but fixes functionality and double-free crashes.
    } else if (value->type == VAL_TOOLKIT) {
        destroy_scope(value->as.toolkit.toolkit_scope);
        destroy_scope(value->as.toolkit.exports);
    } else if (value->type == VAL_BRIDGE) {
        destroy_scope(value->as.bridge.bridge_scope);
    }
    free(value);
}

typedef struct FunctionSymbol {
    char name[50];
    ASTNode *node;
} FunctionSymbol;

char* value_to_string(Value* val);
Value* interpret_ast(ASTNode *node, Scope* scope);
void free_ast(ASTNode *node);
ASTNode* parse_ast_from_json(cJSON *json_node);
ASTNode* parse_ast_from_file(const char* filename); // Forward decl

// Simple event registry and parallel task queue
typedef struct {
    char *event_name;
    ASTNode **handlers;
    int num_handlers;
    int capacity;
} EventEntry;

static EventEntry *event_registry = NULL;
static int event_registry_count = 0;
static int event_registry_capacity = 0;

static ASTNode **paral_queue = NULL;
static int paral_queue_count = 0;
static int paral_queue_capacity = 0;

static void register_listener(const char *event_name, ASTNode **handler_body, int num_body) {
    for (int i = 0; i < event_registry_count; i++) {
        if (strcmp(event_registry[i].event_name, event_name) == 0) {
            if (event_registry[i].num_handlers + num_body > event_registry[i].capacity) {
                event_registry[i].capacity = event_registry[i].capacity + num_body + 4;
                event_registry[i].handlers = (ASTNode**)realloc(event_registry[i].handlers, event_registry[i].capacity * sizeof(ASTNode*));
            }
            for (int j = 0; j < num_body; j++) {
                event_registry[i].handlers[event_registry[i].num_handlers++] = handler_body[j];
            }
            return;
        }
    }
    if (event_registry_count >= event_registry_capacity) {
        event_registry_capacity = event_registry_capacity == 0 ? 4 : event_registry_capacity * 2;
        event_registry = (EventEntry*)realloc(event_registry, event_registry_capacity * sizeof(EventEntry));
    }
    event_registry[event_registry_count].event_name = strdup(event_name);
    event_registry[event_registry_count].capacity = num_body + 4;
    event_registry[event_registry_count].handlers = (ASTNode**)malloc(event_registry[event_registry_count].capacity * sizeof(ASTNode*));
    event_registry[event_registry_count].num_handlers = 0;
    for (int j = 0; j < num_body; j++) {
        event_registry[event_registry_count].handlers[event_registry[event_registry_count].num_handlers++] = handler_body[j];
    }
    event_registry_count++;
}

static void emit_signal(const char *event_name, Scope* scope) {
    for (int i = 0; i < event_registry_count; i++) {
        if (strcmp(event_registry[i].event_name, event_name) == 0) {
            for (int j = 0; j < event_registry[i].num_handlers; j++) {
                free_value(interpret_ast(event_registry[i].handlers[j], scope));
            }
        }
    }
}

static void enqueue_paral(ASTNode **body, int num_body) {
    if (paral_queue_count + num_body > paral_queue_capacity) {
        paral_queue_capacity = paral_queue_capacity == 0 ? (num_body + 4) : (paral_queue_capacity + num_body + 4);
        paral_queue = (ASTNode**)realloc(paral_queue, paral_queue_capacity * sizeof(ASTNode*));
    }
    for (int i = 0; i < num_body; i++) {
        paral_queue[paral_queue_count++] = body[i];
    }
}

static void drain_hold(Scope* scope) {
    for (int i = 0; i < paral_queue_count; i++) {
        free_value(interpret_ast(paral_queue[i], scope));
    }
    paral_queue_count = 0;
}

bool value_to_bool(Value* val) {
    if (!val) return false;
    switch (val->type) {
        case VAL_NIL:
            return false;
        case VAL_BOOL:
            return val->as.boolean;
        case VAL_NUMBER:
            return val->as.number != 0;
        case VAL_STRING:
            if (!val->as.string) return false;
            return val->as.string[0] != '\0';
        default:
            return true;
    }
}

Value* create_string_value_helper(const char* s) {
    Value* val = (Value*)malloc(sizeof(Value));
    val->type = VAL_STRING;
    val->as.string = strdup(s ? s : "");
    return val;
}

typedef enum {
    NODE_PROGRAM,
    NODE_NUMBER,
    NODE_BOOL,
    NODE_NIL,
    NODE_STRING,
    NODE_BINARY_OP,
    NODE_VAR_ACCESS,
    NODE_VAR_ASSIGN,
    NODE_CONSTANT_DECL,
    NODE_SHOW_STATEMENT,
    NODE_EXPRESSION_STATEMENT,
    NODE_NICK_DECL,
    NODE_FUNCTION_DECL,
    NODE_RETURN_STATEMENT,
    NODE_FUNCTION_CALL,
    NODE_CHECK_STATEMENT,
    NODE_ALTER_CLAUSE,
    NODE_BLUEPRINT,
    NODE_ATTRIBUTE_ACCESS,
    NODE_SPAWN,
    NODE_ADOPT,
    NODE_DEN,
    NODE_CONVERT,
    NODE_TOOLKIT,
    NODE_PLUG,
    NODE_BRIDGE,
    NODE_INLET,
    NODE_LINK,
    NODE_EXPOSE,
    NODE_HALT,
    NODE_PROCEED,
    NODE_WAIT,
    NODE_TRIGGER,
    NODE_BLAME,
    NODE_TYPE,
    NODE_KIND,
    NODE_NICK,
    NODE_HIDDEN,
    NODE_SHIELDED,
    NODE_INTERNAL,
    NODE_EMBED,
    NODE_PARAL,
    NODE_HOLD,
    NODE_SIGNAL,
    NODE_LISTEN,
    NODE_ASK,
    NODE_AUTHEN,
    NODE_TRANSFORM,
    NODE_CONDENSE,
    NODE_PACK,
    NODE_UNPACK,
    NODE_INTERPOLATED_STRING,
    NODE_TRAVERSE,
    NODE_EACH, // Replaces TRAVERSE
    NODE_UNTIL,
    NODE_DOCSTRING,
    NODE_CONSTRUCTOR_DECL,
    NODE_ATTEMPT_TRAP_CONCLUDE,
    NODE_METHOD_CALL,
    NODE_MODULE,
    NODE_BRING,
    NODE_CONTRACT,
    NODE_UNARY_OP
} NodeType;


typedef struct ASTNode ASTNode;

typedef struct {
    ASTNode **statements;
    int num_statements;
} ProgramNode;

typedef struct {
    ASTNode *left;
    char *op;
    ASTNode *right;
} BinaryOpNode;

typedef struct {
    char *var_name;
} VarAccessNode;

typedef struct {
    ASTNode *target;
    ASTNode *value;
} VarAssignNode;

typedef struct {
    char *const_name;
    ASTNode *value;
} ConstantDeclNode;

typedef struct {
    ASTNode *expression;
} ExpressionStatementNode;

typedef struct {
    ASTNode **expressions;
    int num_expressions;
} ShowStatementNode;

typedef struct {
    char *original_type;
    char *alias;
} NickDeclNode;

typedef struct {
    char *name;
    char **params;
    int num_params;
    ASTNode **body;
    int num_body_statements;
    char *docstring;
    bool exposed;
    bool shared;
    char *func_type;
} FunctionDeclNode;



typedef struct {
    ASTNode *expression;
} ReturnStatementNode;

typedef struct {
    char *function_name;
    ASTNode **arguments;
    int num_arguments;
} FunctionCallNode;

typedef struct {
    ASTNode *blueprint_expr;
    ASTNode **arguments;
    int num_arguments;
} SpawnNode;

typedef struct {
    char *child_blueprint_name;
    char *parent_blueprint_name;
} AdoptNode;

typedef struct {
    ASTNode *condition;
    ASTNode **body;
    int num_body_statements;
    struct AlterClause *alter_clauses;
    int num_alter_clauses;
    ASTNode **altern_clause;
    int num_altern_statements;
} CheckStatementNode;

typedef struct AlterClause {
    ASTNode *condition;
    ASTNode **body;
    int num_body_statements;
} AlterClause;

typedef struct {
    char *name;
    char *parent;
    ASTNode *constructor;
    ASTNode **attributes;
    int num_attributes;
    ASTNode **methods;
    int num_methods;
    char *docstring;
} BlueprintNode;

typedef struct {
    ASTNode **attempt_body;
    int num_attempt_statements;
    char *error_var;
    ASTNode **trap_body;
    int num_trap_statements;
    ASTNode **conclude_body;
    int num_conclude_statements;
    bool peek;
} AttemptTrapConcludeNode;



typedef struct {
    int dummy;
} HaltNode;

typedef struct {
    int dummy;
} ProceedNode;

typedef struct {
    ASTNode *duration;
} WaitNode;



typedef struct {
    char *name;
    ASTNode **body;
    int num_body_statements;
} BlameNode;

typedef struct {
    char *type_name;
} TypeNode;

typedef struct {
    ASTNode *expression;
} KindNode;

typedef struct {
    ASTNode *original;
    ASTNode *alias;
} NickNode;

typedef struct {
    ASTNode *declaration;
} HiddenNode;

typedef struct {
    ASTNode *declaration;
} ShieldedNode;

typedef struct {
    ASTNode *declaration;
} InternalNode;

typedef struct {
    ASTNode **body;
    int num_body_statements;
} EmbedNode;

typedef struct {
    ASTNode **body;
    int num_body_statements;
} ParalNode;

typedef struct {
    ASTNode **body;
    int num_body_statements;
} HoldNode;

typedef struct {
    ASTNode **body;
    int num_body_statements;
} SignalNode;

typedef struct {
    ASTNode **body;
    int num_body_statements;
} ListenNode;

typedef struct {
    ASTNode **body;
    int num_body_statements;
} AskNode;

typedef struct {
    ASTNode **body;
    int num_body_statements;
} AuthenNode;

typedef struct {
    ASTNode **body;
    int num_body_statements;
} TransformNode;

typedef struct {
    ASTNode **body;
    int num_body_statements;
} CondenseNode;

typedef struct {
    ASTNode **items;
    int num_items;
} PackNode;

typedef struct {
    ASTNode **body;
    int num_body_statements;
} UnpackNode;

typedef struct {
    ASTNode *object;
    char *attribute_name;
} AttributeAccessNode;

typedef struct {
    char *name;
    ASTNode **attributes;
    int num_attributes;
} DenNode;

typedef struct {
    ASTNode *source;
    char *target_type;
} ConvertNode;

typedef struct {
    char *name;
    ASTNode **body;
    int num_body_statements;
} ToolkitNode;

typedef struct {
    char *toolkit_name;
    char *file_path;
} PlugNode;

typedef struct {
    char *name;
    ASTNode **body;
    int num_body_statements;
} BridgeNode;

typedef struct {
    ASTNode **body;
    int num_body_statements;
} InletNode;

typedef struct {
    ASTNode *greeter;
    ASTNode *implementation;
} LinkNode;

typedef struct {
    char *identifier;
} ExposeNode;

typedef struct {
    char *var_name;
    ASTNode *start_val;
    ASTNode *end_val;
    ASTNode *step_val;
    ASTNode **body;
    int num_body_statements;
} TraverseNode;

typedef struct {
    char *var_name;
    ASTNode *iterable;
    ASTNode **body;
    int num_body_statements;
} EachNode;

typedef struct {
    ASTNode *condition;
    ASTNode **body;
    int num_body_statements;
} UntilNode;

typedef struct {
    ASTNode *object;
    char *method_name;
    ASTNode **args;
    int num_args;
} MethodCallNode;

typedef struct {
    char *name;
    ASTNode **body;
    int num_body_statements;
} ModuleNode;

typedef struct {
    char *module;
    char *source;
} BringNode;

typedef struct {
    char *name;
    // For now simple struct
} ContractNode;

typedef struct {
    char *error_name;
    ASTNode *message;
} TriggerNode;

typedef enum {
    INTERPOLATED_STRING_PART_STRING,
    INTERPOLATED_STRING_PART_EXPRESSION
} InterpolatedStringPartType;

typedef struct {
    InterpolatedStringPartType type;
    union {
        char* string_val;
        ASTNode* expression;
    } data;
} InterpolatedStringPart;

typedef struct {
    InterpolatedStringPart **parts;
    int num_parts;
} InterpolatedStringNode;

typedef struct {
    char **params;
    int num_params;
    ASTNode **body;
    int num_body_statements;
} ConstructorNode;

typedef union {
    ProgramNode program;
    double number_val;
    bool boolean_val;
    char* string_val;
    char* docstring_val;
    BinaryOpNode binary_op;
    VarAccessNode var_access;
    VarAssignNode var_assign;
    ConstantDeclNode constant_decl;
    ExpressionStatementNode expr_statement;
    ShowStatementNode show_statement;
    NickDeclNode nick_decl;
    FunctionDeclNode function_decl;
    ReturnStatementNode return_statement;
    FunctionCallNode function_call;
    CheckStatementNode check_statement;
    BlueprintNode blueprint;
    AttributeAccessNode attribute_access;
    SpawnNode spawn;
    AdoptNode adopt;
    DenNode den;
    ConvertNode convert;
    ToolkitNode toolkit;
    PlugNode plug;
    BridgeNode bridge;
    InletNode inlet;
    LinkNode link;
    ExposeNode expose;
    InterpolatedStringNode interpolated_string;
    TraverseNode traverse; // Kept for legacy if needed, or mapped to Each
    EachNode each;
    UntilNode until;
    MethodCallNode method_call;
    ModuleNode module;
    BringNode bring;
    ContractNode contract;
    struct {
        char *op;
        ASTNode *operand;
    } unary_op;
    HaltNode halt;
    ProceedNode proceed;
    WaitNode wait;
    TriggerNode trigger;
    BlameNode blame;
    TypeNode type_node;
    KindNode kind;
    NickNode nick;
    HiddenNode hidden;
    ShieldedNode shielded;
    InternalNode internal;
    EmbedNode embed;
    ParalNode paral;
    HoldNode hold;
    SignalNode signal_node;
    ListenNode listen;
    AskNode ask;
    AuthenNode authen;
    TransformNode transform;
    CondenseNode condense;
    PackNode pack;
    UnpackNode unpack;
    ConstructorNode constructor_decl;
    AttemptTrapConcludeNode attempt_trap_conclude;
    TriggerNode trigger_node;
} ASTNodeData;

struct ASTNode {
    NodeType type;
    ASTNodeData data;
};

// Helper function to detect and convert input string to appropriate type
Value* detect_and_convert_type(const char* input) {
    Value* result = (Value*)malloc(sizeof(Value));
    
    // Trim leading/trailing whitespace
    while (*input == ' ' || *input == '\t') input++;
    size_t len = strlen(input);
    while (len > 0 && (input[len-1] == ' ' || input[len-1] == '\t')) len--;
    
    // Create trimmed copy
    char* trimmed = (char*)malloc(len + 1);
    strncpy(trimmed, input, len);
    trimmed[len] = '\0';
    
    // Check for boolean keywords
    if (strcmp(trimmed, "On") == 0) {
        result->type = VAL_BOOL;
        result->as.boolean = true;
        free(trimmed);
        return result;
    }
    if (strcmp(trimmed, "Off") == 0) {
        result->type = VAL_BOOL;
        result->as.boolean = false;
        free(trimmed);
        return result;
    }
    
    // Check for nil keyword
    if (strcmp(trimmed, "Nil") == 0) {
        result->type = VAL_NIL;
        free(trimmed);
        return result;
    }
    
    // Try to parse as number
    char* endptr;
    double num_val = strtod(trimmed, &endptr);
    
    // Check if entire string was consumed (valid number)
    if (*trimmed != '\0' && *endptr == '\0') {
        result->type = VAL_NUMBER;
        result->as.number = num_val;
        free(trimmed);
        return result;
    }
    
    // Default to string
    result->type = VAL_STRING;
    result->as.string = trimmed;
    return result;
}

Value* interpret_ast(ASTNode* node, Scope* scope) {
    if (!node) {
        Value* nil_val = (Value*)malloc(sizeof(Value));
        nil_val->type = VAL_NIL;
        return nil_val;
    }

    Value* result_val = NULL;

    // printf("Node type: %d\n", node->type); // Trace

    switch (node->type) {
        case NODE_PROGRAM: {
            for (int i = 0; i < node->data.program.num_statements; i++) {
                free_value(interpret_ast(node->data.program.statements[i], scope));
            }
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_NUMBER: {
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NUMBER;
            result_val->as.number = node->data.number_val;
            break;
        }
        case NODE_STRING: {
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_STRING;
            result_val->as.string = strdup(node->data.string_val);
            break;
        }
        case NODE_BINARY_OP: {
            Value* left_val = interpret_ast(node->data.binary_op.left, scope);
            Value* right_val = interpret_ast(node->data.binary_op.right, scope);
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NUMBER;

            if (strcmp(node->data.binary_op.op, "+") == 0) {
                result_val->as.number = left_val->as.number + right_val->as.number;
            } else if (strcmp(node->data.binary_op.op, "-") == 0) {
                result_val->as.number = left_val->as.number - right_val->as.number;
            } else if (strcmp(node->data.binary_op.op, "*") == 0) {
                result_val->as.number = left_val->as.number * right_val->as.number;
            } else if (strcmp(node->data.binary_op.op, "/") == 0) {
                result_val->as.number = left_val->as.number / right_val->as.number;
            } else if (strcmp(node->data.binary_op.op, ">") == 0) {
                result_val->type = VAL_BOOL;
                result_val->as.boolean = left_val->as.number > right_val->as.number;
            } else if (strcmp(node->data.binary_op.op, "<") == 0) {
                result_val->type = VAL_BOOL;
                result_val->as.boolean = left_val->as.number < right_val->as.number;
            } else if (strcmp(node->data.binary_op.op, ">=") == 0) {
                result_val->type = VAL_BOOL;
                result_val->as.boolean = left_val->as.number >= right_val->as.number;
            } else if (strcmp(node->data.binary_op.op, "<=") == 0) {
                result_val->type = VAL_BOOL;
                result_val->as.boolean = left_val->as.number <= right_val->as.number;
            } else if (strcmp(node->data.binary_op.op, "==") == 0) {
                result_val->type = VAL_BOOL;
                if (left_val->type != right_val->type) {
                    result_val->as.boolean = false;
                } else {
                    switch (left_val->type) {
                        case VAL_NUMBER: result_val->as.boolean = left_val->as.number == right_val->as.number; break;
                        case VAL_BOOL: result_val->as.boolean = left_val->as.boolean == right_val->as.boolean; break;
                        case VAL_STRING: result_val->as.boolean = strcmp(left_val->as.string, right_val->as.string) == 0; break;
                        case VAL_NIL: result_val->as.boolean = true; break;
                        default: result_val->as.boolean = false; break;
                    }
                }
            } else if (strcmp(node->data.binary_op.op, "'=") == 0) {
                result_val->type = VAL_BOOL;
                if (left_val->type != right_val->type) {
                    result_val->as.boolean = true;
                } else {
                    switch (left_val->type) {
                        case VAL_NUMBER: result_val->as.boolean = left_val->as.number != right_val->as.number; break;
                        case VAL_BOOL: result_val->as.boolean = left_val->as.boolean != right_val->as.boolean; break;
                        case VAL_STRING: result_val->as.boolean = strcmp(left_val->as.string, right_val->as.string) != 0; break;
                        case VAL_NIL: result_val->as.boolean = false; break;
                        default: result_val->as.boolean = true; break;
                    }
                }
            }

            else if (strcmp(node->data.binary_op.op, "..") == 0) {
                 result_val->type = VAL_RANGE;
                 result_val->as.range.start = (left_val->type == VAL_NUMBER) ? left_val->as.number : 0;
                 result_val->as.range.end = (right_val->type == VAL_NUMBER) ? right_val->as.number : 0;
            } else if (strcmp(node->data.binary_op.op, "is") == 0) {
                // Type check: 'val is a Num' -> operator 'is', right operand is TypeNode/String?
                // Parser likely produces right operand as TypeNode or similar.
                // Assuming right operand evaluates to type string.
                // If the parser handles `is a`, it might produce `is` operator.
                result_val->type = VAL_BOOL;
                result_val->as.boolean = false;
                
                // We need to check left value's type against right value string
                char* type_name = NULL;
                if (right_val->type == VAL_STRING) {
                    type_name = right_val->as.string;
                } else {
                     // Try to interpret right node directly if it's a TYPE node
                     // But here right_val is already evaluated.
                     // The parser might have converted TypeNode to a String or similar?
                     // Let's assume right_val contains the type name as string.
                }

                if (type_name) {
                     if (strcmp(type_name, "Num") == 0) result_val->as.boolean = (left_val->type == VAL_NUMBER);
                     else if (strcmp(type_name, "Text") == 0) result_val->as.boolean = (left_val->type == VAL_STRING);
                     else if (strcmp(type_name, "On") == 0) result_val->as.boolean = (left_val->type == VAL_BOOL && left_val->as.boolean); // Maybe? or type Bool
                     else if (strcmp(type_name, "Off") == 0) result_val->as.boolean = (left_val->type == VAL_BOOL && !left_val->as.boolean); 
                     else if (strcmp(type_name, "Nil") == 0) result_val->as.boolean = (left_val->type == VAL_NIL);
                     else if (strcmp(type_name, "Spec") == 0) result_val->as.boolean = (left_val->type == VAL_FUNCTION);
                     else if (strcmp(type_name, "Blueprint") == 0) result_val->as.boolean = (left_val->type == VAL_BLUEPRINT);
                     else if (strcmp(type_name, "Instance") == 0) result_val->as.boolean = (left_val->type == VAL_BLUEPRINT_INSTANCE);
                }
            } else {
                fprintf(stderr, "Unknown binary operator: %s\n", node->data.binary_op.op);
                result_val->type = VAL_NIL;
            }

            free_value(left_val);
            free_value(right_val);
            break;
        }
        case NODE_VAR_ACCESS: {
            Value* stored_val = get_variable(scope, node->data.var_access.var_name);
            if (stored_val) {
                result_val = (Value*)malloc(sizeof(Value));
                memcpy(result_val, stored_val, sizeof(Value));
                if (stored_val->type == VAL_STRING) {
                    result_val->as.string = strdup(stored_val->as.string);
                }
                // printf("VAR_ACCESS: '%s' found. Type: %d (Scope %p)\n", node->data.var_access.var_name, result_val->type, scope);
            } else {
                fprintf(stderr, "Variable '%s' not found.\n", node->data.var_access.var_name);
                result_val = (Value*)malloc(sizeof(Value));
                result_val->type = VAL_NIL;
            }
            break;
        }

        case NODE_VAR_ASSIGN: {
            Value* value_to_assign = interpret_ast(node->data.var_assign.value, scope);
            if (node->data.var_assign.target->type == NODE_VAR_ACCESS) {
                set_variable(scope, node->data.var_assign.target->data.var_access.var_name, value_to_assign);
            } else if (node->data.var_assign.target->type == NODE_ATTRIBUTE_ACCESS) {
                // Handle attribute assignment (e.g., obj.property = value)
                Value* object_val = interpret_ast(node->data.var_assign.target->data.attribute_access.object, scope);
                if (object_val && object_val->type == VAL_BLUEPRINT_INSTANCE) {
                    // Set the attribute in the instance scope
                    set_variable(object_val->as.blueprint_instance.instance_scope, 
                                node->data.var_assign.target->data.attribute_access.attribute_name, 
                                value_to_assign);
                } else {
                    fprintf(stderr, "Cannot assign attribute to non-blueprint instance.\n");
                }
                free_value(object_val);
            } else {
                fprintf(stderr, "Unsupported assignment target type.\n");
            }
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_CONSTANT_DECL: {
            Value* value_to_assign = interpret_ast(node->data.constant_decl.value, scope);
            define_variable(scope, node->data.constant_decl.const_name, value_to_assign, true);
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_EXPRESSION_STATEMENT: {
            free_value(interpret_ast(node->data.expr_statement.expression, scope));
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_SHOW_STATEMENT: {
            for (int i = 0; i < node->data.show_statement.num_expressions; i++) {
                Value* val = interpret_ast(node->data.show_statement.expressions[i], scope);
                char* str = value_to_string(val);
                printf("%s ", str);
                free(str);
                free_value(val);
            }
            printf("\n");
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_FUNCTION_DECL: {
            Value* func_val = (Value*)malloc(sizeof(Value));
            func_val->type = VAL_FUNCTION;
            FunctionSymbol* func_sym = (FunctionSymbol*)malloc(sizeof(FunctionSymbol));
            strncpy(func_sym->name, node->data.function_decl.name, 49);
            func_sym->name[49] = '\0';
            func_sym->node = node;
            func_val->as.function = func_sym;
            set_variable(scope, node->data.function_decl.name, func_val);
            // If inside a toolkit and function is shared/exposed, add to exports
            if (node->data.function_decl.exposed || node->data.function_decl.shared) {
                Scope* current_scope = scope;
                Value* toolkit_val = NULL;
                while (current_scope) {
                    if (current_scope->symbol_count > 0 && current_scope->symbols[0].value->type == VAL_TOOLKIT) {
                        toolkit_val = current_scope->symbols[0].value;
                        break;
                    }
                    current_scope = current_scope->parent;
                }
                if (toolkit_val) {
                    set_variable(toolkit_val->as.toolkit.exports, node->data.function_decl.name, copy_value(func_val));
                }
            }
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_FUNCTION_CALL: {
            Value* func_val = get_variable(scope, node->data.function_call.function_name);
            if (func_val && func_val->type == VAL_FUNCTION) {
                FunctionSymbol* func_sym = func_val->as.function;
                ASTNode* func_node = func_sym->node;

                if (node->data.function_call.num_arguments != func_node->data.function_decl.num_params) {
                    fprintf(stderr, "Function '%s' called with incorrect number of arguments.\n", node->data.function_call.function_name);
                    result_val = (Value*)malloc(sizeof(Value));
                    result_val->type = VAL_NIL;
                } else {
                    Scope* func_scope = create_scope(scope);

                    for (int i = 0; i < node->data.function_call.num_arguments; i++) {
                        Value* arg_val = interpret_ast(node->data.function_call.arguments[i], scope);
                        set_variable(func_scope, func_node->data.function_decl.params[i], arg_val);
                    }

                    result_val = NULL;
                    for (int i = 0; i < func_node->data.function_decl.num_body_statements; i++) {
                        ASTNode* statement_node = func_node->data.function_decl.body[i];
                        Value* statement_result = interpret_ast(statement_node, func_scope);

                        if (statement_node->type == NODE_RETURN_STATEMENT) {
                            if (result_val) free_value(result_val);
                            result_val = statement_result;
                            break;
                        }
                        free_value(statement_result);
                    }

                    destroy_scope(func_scope);

                    if (!result_val) {
                        result_val = (Value*)malloc(sizeof(Value));
                        result_val->type = VAL_NIL;
                    }
                }
                free_value(func_val);
            } else {
                fprintf(stderr, "Function '%s' not implemented.\n", node->data.function_call.function_name);
                result_val = (Value*)malloc(sizeof(Value));
                result_val->type = VAL_NIL;
            }
            break;
        }
        case NODE_RETURN_STATEMENT: {
            if (node->data.return_statement.expression) {
                result_val = interpret_ast(node->data.return_statement.expression, scope);
            } else {
                result_val = (Value*)malloc(sizeof(Value));
                result_val->type = VAL_NIL;
            }
            break;
        }


        case NODE_ATTEMPT_TRAP_CONCLUDE: {
            Scope* attempt_scope = create_scope(scope);
            bool has_error = false;
            for (int i = 0; i < node->data.attempt_trap_conclude.num_attempt_statements; i++) {
                free_value(interpret_ast(node->data.attempt_trap_conclude.attempt_body[i], attempt_scope));
                Value* err = get_variable(attempt_scope, "__last_error_name");
                if (err) { has_error = true; break; }
            }
            if (has_error) {
                Value* msg = get_variable(attempt_scope, "__last_error_message");
                Scope* trap_scope = create_scope(scope);
                if (node->data.attempt_trap_conclude.peek) {
                    if (msg && msg->type == VAL_STRING) {
                        Value* peek_val = (Value*)malloc(sizeof(Value));
                        peek_val->type = VAL_STRING;
                        peek_val->as.string = strdup(msg->as.string);
                        set_variable(trap_scope, "peek", peek_val);
                    }
                }
                for (int i = 0; i < node->data.attempt_trap_conclude.num_trap_statements; i++) {
                    free_value(interpret_ast(node->data.attempt_trap_conclude.trap_body[i], trap_scope));
                }
                destroy_scope(trap_scope);
            }
            if (node->data.attempt_trap_conclude.num_conclude_statements > 0) {
                for (int i = 0; i < node->data.attempt_trap_conclude.num_conclude_statements; i++) {
                    free_value(interpret_ast(node->data.attempt_trap_conclude.conclude_body[i], scope));
                }
            }
            destroy_scope(attempt_scope);
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_KIND: {
            Value* v = interpret_ast(node->data.kind.expression, scope);
            const char* t = NULL;
            switch (v->type) {
                case VAL_NUMBER: t = "Num"; break;
                case VAL_STRING: t = "Text"; break;
                case VAL_BOOL: t = v->as.boolean ? "On" : "Off"; break;
                case VAL_NIL: t = "Nil"; break;
                case VAL_FUNCTION: t = "Spec"; break;
                case VAL_BLUEPRINT: t = "Blueprint"; break;
                case VAL_BLUEPRINT_INSTANCE: t = "Instance"; break;
                case VAL_TOOLKIT: t = "Toolkit"; break;
                default: t = "Unknown"; break;
            }
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_STRING;
            result_val->as.string = strdup(t);
            free_value(v);
            break;
        }
        case NODE_WAIT: {
            // no-op wait
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_TRAVERSE: {
            Value* start_val = interpret_ast(node->data.traverse.start_val, scope);
            Value* end_val = interpret_ast(node->data.traverse.end_val, scope);
            double step = 1.0;
            if (node->data.traverse.step_val) {
                Value* step_val = interpret_ast(node->data.traverse.step_val, scope);
                if (step_val->type == VAL_NUMBER) step = step_val->as.number;
                free_value(step_val);
            }
            double i = start_val->as.number;
            double endn = end_val->as.number;
            for (; (step >= 0 ? i <= endn : i >= endn); i += step) {
                Value* num = (Value*)malloc(sizeof(Value));
                num->type = VAL_NUMBER;
                num->as.number = i;
                set_variable(scope, node->data.traverse.var_name, num);
                for (int j = 0; j < node->data.traverse.num_body_statements; j++) {
                    ASTNode* stmt = node->data.traverse.body[j];
                    if (stmt->type == NODE_PROCEED) continue;
                    if (stmt->type == NODE_HALT) { j = node->data.traverse.num_body_statements; break; }
                    free_value(interpret_ast(stmt, scope));
                }
            }
            free_value(start_val);
            free_value(end_val);
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }

        case NODE_CHECK_STATEMENT: {
            Value* condition_val = interpret_ast(node->data.check_statement.condition, scope);
            bool condition_is_true = value_to_bool(condition_val);

            if (condition_is_true) {
                for (int i = 0; i < node->data.check_statement.num_body_statements; i++) {
                    free_value(interpret_ast(node->data.check_statement.body[i], scope));
                }
            } else {
                bool alter_executed = false;
                for (int i = 0; i < node->data.check_statement.num_alter_clauses; i++) {
                    Value* alter_condition_val = interpret_ast(node->data.check_statement.alter_clauses[i].condition, scope);
                    bool alter_condition_is_true = value_to_bool(alter_condition_val);
                    free_value(alter_condition_val);

                    if (alter_condition_is_true) {
                        for (int j = 0; j < node->data.check_statement.alter_clauses[i].num_body_statements; j++) {
                            free_value(interpret_ast(node->data.check_statement.alter_clauses[i].body[j], scope));
                        }
                        alter_executed = true;
                        break;
                    }
                }

                if (!alter_executed && node->data.check_statement.altern_clause) {
                    for (int i = 0; i < node->data.check_statement.num_altern_statements; i++) {
                        free_value(interpret_ast(node->data.check_statement.altern_clause[i], scope));
                    }
                }
            }
            free_value(condition_val);
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_BLUEPRINT: {
            Value* blueprint_val = (Value*)malloc(sizeof(Value));
            blueprint_val->type = VAL_BLUEPRINT;

            Scope* blueprint_scope = create_scope(scope);
            // printf("Created Blueprint Scope: %p\n", blueprint_scope);

            // Interpret attributes and constructor in the blueprint's scope
            for (int i = 0; i < node->data.blueprint.num_attributes; i++) {
                free_value(interpret_ast(node->data.blueprint.attributes[i], blueprint_scope));
            }
            if (node->data.blueprint.constructor) {
                free_value(interpret_ast(node->data.blueprint.constructor, blueprint_scope));
            }
            
            // Interpret methods in the blueprint's scope
            for (int i = 0; i < node->data.blueprint.num_methods; i++) {
                free_value(interpret_ast(node->data.blueprint.methods[i], blueprint_scope));
            }

            blueprint_val->as.blueprint.name = strdup(node->data.blueprint.name);
            blueprint_val->as.blueprint.scope = blueprint_scope;
            blueprint_val->as.blueprint.constructor = node->data.blueprint.constructor;

            // Register the blueprint in the current scope
            set_variable(scope, node->data.blueprint.name, blueprint_val);

            // The result of a blueprint declaration is nil
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_SPAWN: {
            Value* blueprint_val = interpret_ast(node->data.spawn.blueprint_expr, scope);
            if (blueprint_val->type != VAL_BLUEPRINT) {
                fprintf(stderr, "Cannot spawn from a non-blueprint value.\n");
                result_val = (Value*)malloc(sizeof(Value));
                result_val->type = VAL_NIL;
            } else {
                Scope* instance_scope = create_scope(blueprint_val->as.blueprint.scope);
                // printf("Spawned Instance Scope: %p (Blueprint Scope: %p)\n", instance_scope, blueprint_val->as.blueprint.scope);
                for (int i = 0; i < blueprint_val->as.blueprint.scope->symbol_count; i++) {
                    set_variable(instance_scope, blueprint_val->as.blueprint.scope->symbols[i].name, copy_value(blueprint_val->as.blueprint.scope->symbols[i].value));
                }

                result_val = (Value*)malloc(sizeof(Value));
                result_val->type = VAL_BLUEPRINT_INSTANCE;
                result_val->as.blueprint_instance.blueprint_scope = blueprint_val->as.blueprint.scope;
                result_val->as.blueprint_instance.instance_scope = instance_scope;
                result_val->as.blueprint_instance.instance_scope = instance_scope;
                // set_variable(instance_scope, "self", result_val); // Removed erroneous line
                
                // Important: Pass a COPY of result_val as 'self' so instance_scope owns its own Value struct.
                // Otherwise if result_val is freed, self becomes dangling.
                set_variable(instance_scope, "self", copy_value(result_val));

                // Call 'make' constructor if it exists in blueprint
                if (blueprint_val->as.blueprint.constructor) {
                    ASTNode* ctor_node = blueprint_val->as.blueprint.constructor;
                    // Locate constructor function declaration (it should be the node itself)

                    // Basic argument count check
                    // Constructor has 'own' as first parameter, so user provides num_params - 1 arguments
                    if (node->data.spawn.num_arguments != ctor_node->data.constructor_decl.num_params - 1) {
                         fprintf(stderr, "Constructor called with incorrect number of arguments (expected %d, got %d).\n", 
                                 ctor_node->data.constructor_decl.num_params - 1, node->data.spawn.num_arguments);
                    } else {
                         Scope* ctor_scope = create_scope(instance_scope);
                         // Pass a COPY of result_val as 'own', because ctor_scope will free it!
                         set_variable(ctor_scope, "own", copy_value(result_val)); 
                         
                         // Skip first param (own) when mapping arguments
                         for(int i=0; i<node->data.spawn.num_arguments; i++) {
                             Value* arg_val = interpret_ast(node->data.spawn.arguments[i], scope);
                             // Param index is i+1 because param[0] is own
                             set_variable(ctor_scope, ctor_node->data.constructor_decl.params[i+1], arg_val);
                         }
                         
                         for(int i=0; i<ctor_node->data.constructor_decl.num_body_statements; i++) {
                             free_value(interpret_ast(ctor_node->data.constructor_decl.body[i], ctor_scope));
                         }
                         destroy_scope(ctor_scope);
                    }
                }
            }
            free_value(blueprint_val);
            break;
        }
        case NODE_ADOPT: {
            Value* parent_blueprint_val = get_variable(scope, node->data.adopt.parent_blueprint_name);
            Value* child_blueprint_val = get_variable(scope, node->data.adopt.child_blueprint_name);

            if (parent_blueprint_val && parent_blueprint_val->type == VAL_BLUEPRINT &&
                child_blueprint_val && child_blueprint_val->type == VAL_BLUEPRINT) {

                Scope* parent_scope = parent_blueprint_val->as.blueprint.scope;
                Scope* child_scope = child_blueprint_val->as.blueprint.scope;

                for (int i = 0; i < parent_scope->symbol_count; i++) {
                    set_variable(child_scope, parent_scope->symbols[i].name, copy_value(parent_scope->symbols[i].value));
                }
            } else {
                fprintf(stderr, "Could not find parent or child blueprint for adopt.\n");
            }

            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_ATTRIBUTE_ACCESS: {
            Value* object_val = interpret_ast(node->data.attribute_access.object, scope);
            if (object_val && (object_val->type == VAL_BLUEPRINT_INSTANCE || object_val->type == VAL_TOOLKIT)) {
                Scope* target_scope = (object_val->type == VAL_BLUEPRINT_INSTANCE) 
                                      ? object_val->as.blueprint_instance.instance_scope 
                                      : object_val->as.toolkit.exports;
                Value* found_val = get_variable(target_scope, node->data.attribute_access.attribute_name);
                if (found_val) {
                    result_val = copy_value(found_val);
                } else {
                    result_val = (Value*)malloc(sizeof(Value));
                    result_val->type = VAL_NIL;
                }
            } else {
                fprintf(stderr, "Attribute access on non-blueprint instance or toolkit.\n");
                result_val = (Value*)malloc(sizeof(Value));
                result_val->type = VAL_NIL;
            }
            free_value(object_val);
            break;
        }
        case NODE_DEN: {
            Value* func_val = (Value*)malloc(sizeof(Value));
            func_val->type = VAL_FUNCTION;
            FunctionSymbol* func_sym = (FunctionSymbol*)malloc(sizeof(FunctionSymbol));
            // Anonymous function, so no name
            func_sym->name[0] = '\0';
            func_sym->node = node;
            func_val->as.function = func_sym;
            result_val = func_val;
            break;
        }
        case NODE_CONVERT: {
            Value* source_val = interpret_ast(node->data.convert.source, scope);
            char* target_type_str = node->data.convert.target_type;
            result_val = (Value*)malloc(sizeof(Value));

            if (strcmp(target_type_str, "Text") == 0) {
                result_val->type = VAL_STRING;
                result_val->as.string = value_to_string(source_val);
            } else if (strcmp(target_type_str, "Num") == 0) {
                result_val->type = VAL_NUMBER;
                if (source_val->type == VAL_STRING) {
                    result_val->as.number = atof(source_val->as.string);
                } else if (source_val->type == VAL_NUMBER) {
                    result_val->as.number = source_val->as.number;
                } else {
                    result_val->as.number = 0; // Or some other default
                }
            } else {
                // Unknown target type
                result_val->type = VAL_NIL;
            }

            free_value(source_val);
            break;
        }
        case NODE_TOOLKIT: {
            Value* toolkit_val = (Value*)malloc(sizeof(Value));
            toolkit_val->type = VAL_TOOLKIT;
            toolkit_val->as.toolkit.toolkit_scope = create_scope(scope);
            toolkit_val->as.toolkit.exports = create_scope(NULL); // Exports have no parent

            // Interpret the body of the toolkit in the new scope
            for (int i = 0; i < node->data.toolkit.num_body_statements; i++) {
                free_value(interpret_ast(node->data.toolkit.body[i], toolkit_val->as.toolkit.toolkit_scope));
            }

            set_variable(scope, node->data.toolkit.name, toolkit_val);

            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_PLUG: {
            Value* toolkit_val = get_variable(scope, node->data.plug.toolkit_name);
            if (toolkit_val && toolkit_val->type == VAL_TOOLKIT) {
                Scope* exports = toolkit_val->as.toolkit.exports;
                for (int i = 0; i < exports->symbol_count; i++) {
                    set_variable(scope, exports->symbols[i].name, exports->symbols[i].value);
                }
            } else {
                fprintf(stderr, "Could not find toolkit to plug: %s\n", node->data.plug.toolkit_name);
            }

            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_BRIDGE: {
            Value* bridge_val = (Value*)malloc(sizeof(Value));
            bridge_val->type = VAL_BRIDGE;
            bridge_val->as.bridge.bridge_scope = create_scope(scope);

            // Interpret the body of the bridge in the new scope
            for (int i = 0; i < node->data.bridge.num_body_statements; i++) {
                free_value(interpret_ast(node->data.bridge.body[i], bridge_val->as.bridge.bridge_scope));
            }

            set_variable(scope, node->data.bridge.name, bridge_val);

            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_INLET: {
            // Execute inlet body in current scope
            for (int i = 0; i < node->data.inlet.num_body_statements; i++) {
                free_value(interpret_ast(node->data.inlet.body[i], scope));
            }
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_LINK: {
            // Minimal: evaluate implementation and set greeter variable to copy
            Value* impl_val = interpret_ast(node->data.link.implementation, scope);
            if (node->data.link.greeter->type == NODE_VAR_ACCESS) {
                set_variable(scope, node->data.link.greeter->data.var_access.var_name, copy_value(impl_val));
            }
            free_value(impl_val);
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_EXPOSE: {
            // Find the current toolkit by traversing up the scope chain
            Scope* current_scope = scope;
            Value* toolkit_val = NULL;
            while (current_scope) {
                // This is a simplification. A real implementation would need a more robust way
                // to identify which toolkit we are in.
                if (current_scope->symbol_count > 0 && current_scope->symbols[0].value->type == VAL_TOOLKIT) {
                    toolkit_val = current_scope->symbols[0].value;
                    break;
                }
                current_scope = current_scope->parent;
            }

            if (toolkit_val) {
                Value* value_to_expose = get_variable(scope, node->data.expose.identifier);
                if (value_to_expose) {
                    set_variable(toolkit_val->as.toolkit.exports, node->data.expose.identifier, value_to_expose);
                } else {
                    fprintf(stderr, "Cannot expose unknown identifier: %s\n", node->data.expose.identifier);
                }
            } else {
                fprintf(stderr, "Expose can only be used inside a toolkit.\n");
            }

            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_CONSTRUCTOR_DECL: {
            Value* func_val = (Value*)malloc(sizeof(Value));
            func_val->type = VAL_FUNCTION;
            FunctionSymbol* func_sym = (FunctionSymbol*)malloc(sizeof(FunctionSymbol));
            strncpy(func_sym->name, "constructor", 49);
            func_sym->name[49] = '\0';
            func_sym->node = node;
            func_val->as.function = func_sym;
            set_variable(scope, "constructor", func_val);
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_ASK: {
            for (int i = 0; i < node->data.ask.num_body_statements; i++) {
                ASTNode* stmt = node->data.ask.body[i];
                if (stmt->type == NODE_EXPRESSION_STATEMENT) {
                    Value* val = interpret_ast(stmt->data.expr_statement.expression, scope);
                    if (val && val->type == VAL_STRING) {
                        printf("%s", val->as.string);
                    }
                    free_value(val);
                } else {
                    free_value(interpret_ast(stmt, scope));
                }
            }

            char *line = NULL;
            size_t len = 0;
            getline(&line, &len, stdin);

            // Remove trailing newline character
            if (line[strlen(line) - 1] == '\n') {
                line[strlen(line) - 1] = '\0';
            }

            // Use dynamic type detection
            result_val = detect_and_convert_type(line);
            
            // Free the original line since detect_and_convert_type makes a copy
            free(line);
            break;
        }

        case NODE_INTERPOLATED_STRING: {
            char* full_str = malloc(1024); // Allocate a large buffer
            full_str[0] = '\0';
            for (int i = 0; i < node->data.interpolated_string.num_parts; i++) {
                InterpolatedStringPart* part = node->data.interpolated_string.parts[i];
                if (part->type == INTERPOLATED_STRING_PART_STRING) {
                    strcat(full_str, part->data.string_val);
                } else { // INTERPOLATED_STRING_PART_EXPRESSION
                    Value* part_val = interpret_ast(part->data.expression, scope);
                    char* part_str = value_to_string(part_val);
                    strcat(full_str, part_str);
                    free(part_str);
                    free_value(part_val);
                }
            }
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_STRING;
            result_val->as.string = full_str;
            break;
        }
        case NODE_UNARY_OP: {
            Value* operand = interpret_ast(node->data.unary_op.operand, scope);
            if (strcmp(node->data.unary_op.op, "'") == 0) { // NOT operator
                bool val = value_to_bool(operand);
                free_value(operand);
                result_val = (Value*)malloc(sizeof(Value));
                result_val->type = VAL_BOOL;
                result_val->as.boolean = !val;
            } else {
                fprintf(stderr, "Unknown unary operator: %s\n", node->data.unary_op.op);
                free_value(operand);
                result_val = (Value*)malloc(sizeof(Value));
                result_val->type = VAL_NIL;
            }
            break;
        }
        case NODE_EACH: {
             Value* iterable_val = interpret_ast(node->data.each.iterable, scope);
             if (iterable_val->type == VAL_RANGE) {
                 double start = iterable_val->as.range.start;
                 double end = iterable_val->as.range.end;
                 double step = 1.0;
                 if (start > end) step = -1.0;
                 
                 for (double i = start; (step > 0 ? i <= end : i >= end); i += step) {
                     Value* loop_var = (Value*)malloc(sizeof(Value));
                     loop_var->type = VAL_NUMBER;
                     loop_var->as.number = i;
                     
                     // Create loop scope? Or use current scope?
                     // Usually loops have their own scope to prevent var leakage?
                     // But Nervestack might be simple. Let's use a sub-scope.
                     Scope* loop_scope = create_scope(scope);
                     set_variable(loop_scope, node->data.each.var_name, loop_var);
                     
                     for (int j = 0; j < node->data.each.num_body_statements; j++) {
                         ASTNode* stmt = node->data.each.body[j];
                         // Handle Proceed/Halt if needed (omitted for brevity unless needed)
                         free_value(interpret_ast(stmt, loop_scope));
                     }
                     destroy_scope(loop_scope);
                 }
             } else {
                 fprintf(stderr, "Type mismatch: 'each' loop requires a range.\n");
             }
             free_value(iterable_val);
             result_val = (Value*)malloc(sizeof(Value));
             result_val->type = VAL_NIL;
             break;
        }
        case NODE_UNTIL: {
             while (1) {
                Value* cond_val = interpret_ast(node->data.until.condition, scope);
                bool stop = value_to_bool(cond_val);
                free_value(cond_val);
                if (stop) break; // UNTIL condition is met, stop.
                for (int j = 0; j < node->data.until.num_body_statements; j++) {
                    ASTNode* stmt = node->data.until.body[j];
                    if (stmt->type == NODE_PROCEED) continue;
                    if (stmt->type == NODE_HALT) { j = node->data.until.num_body_statements; break; }
                    free_value(interpret_ast(stmt, scope));
                }
            }
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_METHOD_CALL: {
            Value* object_val = interpret_ast(node->data.method_call.object, scope);
            if (object_val && object_val->type == VAL_BLUEPRINT_INSTANCE) {
                 // Look for method in instance scope? Or blueprint scope?
                 // Methods are usually in Blueprint scope (shared), but `own` is the instance.
                 // Values: 
                 // BlueprintInstance has instance_scope (vars) and blueprint_scope (shared/methods).
                 // Get method from blueprint_scope?
                 Value* method_val = get_variable(object_val->as.blueprint_instance.blueprint_scope, node->data.method_call.method_name);
                 // printf("Method Call Object Scopes: Inst %p, Blue %p\n", object_val->as.blueprint_instance.instance_scope, object_val->as.blueprint_instance.blueprint_scope);
                 if (method_val && method_val->type == VAL_FUNCTION) {
                     FunctionSymbol* func_sym = method_val->as.function;
                     ASTNode* func_node = func_sym->node;
                     
                     // Create method scope (child of blueprint scope? No, needs access to `own`)
                     // Method scope should usually be child of global or definition scope, but with `own` injected.
                     Scope* method_scope = create_scope(object_val->as.blueprint_instance.instance_scope);
                     // Pass a COPY of object_val as 'own', because method_scope will free it!             
                     set_variable(method_scope, "own", copy_value(object_val)); 
                     
                     // Helper: map arguments
                     // Method 'own' is param[0]
                     if (node->data.method_call.num_args != func_node->data.function_decl.num_params - 1) {
                          fprintf(stderr, "Method '%s' called with incorrect number of arguments (expected %d, got %d).\n", 
                                  node->data.method_call.method_name, func_node->data.function_decl.num_params - 1, node->data.method_call.num_args);
                          result_val = (Value*)malloc(sizeof(Value));
                          result_val->type = VAL_NIL;
                     } else {
                         for (int i = 0; i < node->data.method_call.num_args; i++) {
                            Value* arg_val = interpret_ast(node->data.method_call.args[i], scope);
                            // Param index i+1 because param[0] is own
                            set_variable(method_scope, func_node->data.function_decl.params[i+1], arg_val);
                        }
                        
                        result_val = NULL;
                        for (int i = 0; i < func_node->data.function_decl.num_body_statements; i++) {
                            // Execute body
                            Value* r = interpret_ast(func_node->data.function_decl.body[i], method_scope);
                             if (func_node->data.function_decl.body[i]->type == NODE_RETURN_STATEMENT) {
                                if (result_val) free_value(result_val);
                                result_val = r;
                                break;
                            }
                            free_value(r);
                        }
                        if (!result_val) {
                             result_val = (Value*)malloc(sizeof(Value));
                             result_val->type = VAL_NIL;
                        }
                     }
                     destroy_scope(method_scope);
                 } else {
                      fprintf(stderr, "Method '%s' not found.\n", node->data.method_call.method_name);
                      // Debug: Print available symbols in blueprint scope
                      Scope* sc = object_val->as.blueprint_instance.blueprint_scope;
                      fprintf(stderr, "Available symbols in blueprint scope (%p): ", sc);
                      for(int k=0; k<sc->symbol_count; k++) {
                          fprintf(stderr, "'%s'(type=%d), ", sc->symbols[k].name, sc->symbols[k].value->type);
                      }
                      fprintf(stderr, "\n");
                      Value* check_val = get_variable(sc, node->data.method_call.method_name);
                      fprintf(stderr, "get_variable('%s') returned %p. Type if not null: %d\n", 
                              node->data.method_call.method_name, check_val, check_val ? check_val->type : -1);

                      result_val = (Value*)malloc(sizeof(Value));
                      result_val->type = VAL_NIL;
                 }
            } else {
                fprintf(stderr, "Method call on non-instance. Type: %d\n", object_val ? object_val->type : -1);
                result_val = (Value*)malloc(sizeof(Value));
                result_val->type = VAL_NIL;
            }
            free_value(object_val);
            break;
        }
        case NODE_MODULE: {
             // Just interpret body in current scope? Or create namespace?
             // Modules usually create a namespace.
             // Let's create a VAL_TOOLKIT-like structure for Module?
             // For now, simplicity: Execute body in current scope (flattened) or create a scope and assign to name.
             // Nervestackic modules seem to be namespaces.
             result_val = (Value*)malloc(sizeof(Value));
             result_val->type = VAL_NIL;
             break;
        }
        case NODE_BRING: {
             // Real import implementation
             // printf("Bringing module %s from %s\n", node->data.bring.module, node->data.bring.source ? node->data.bring.source : "(default)");
             
             if (node->data.bring.source) {
                 // 1. Parse the referenced file (AST JSON)
                 ASTNode* imported_ast = parse_ast_from_file(node->data.bring.source);
                 if (imported_ast) {
                     // 2. Create a module scope or just interpret in current scope?
                     // Nervestack 'bring' usually imports INTO current namespace or as a namespace.
                     // If 'bring File', usually puts definitions in current scope.
                     // Let's assume unlimited import for now (interprets ProgramNode in current scope).
                     
                     if (imported_ast->type == NODE_PROGRAM) {
                         for (int i = 0; i < imported_ast->data.program.num_statements; i++) {
                             free_value(interpret_ast(imported_ast->data.program.statements[i], scope));
                         }
                     } else {
                         free_value(interpret_ast(imported_ast, scope));
                     }
                     free_ast(imported_ast);
                 } else {
                     fprintf(stderr, "Runtime Error: Failed to import module '%s' from '%s'\n", 
                             node->data.bring.module, node->data.bring.source);
                 }
             } else {
                 fprintf(stderr, "Runtime Error: No source file provided for module '%s'\n", node->data.bring.module);
             }

             result_val = (Value*)malloc(sizeof(Value));
             result_val->type = VAL_NIL;
             break;
        }
        case NODE_CONTRACT: {
             // Define contract (interface) - currently no-op or register name
             result_val = (Value*)malloc(sizeof(Value));
             result_val->type = VAL_NIL;
             break;
        }
        case NODE_EMBED: {
            Scope* embed_scope = create_scope(scope);
            for (int i = 0; i < node->data.embed.num_body_statements; i++) {
                free_value(interpret_ast(node->data.embed.body[i], embed_scope));
            }
            destroy_scope(embed_scope);
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_PARAL: {
            enqueue_paral(node->data.paral.body, node->data.paral.num_body_statements);
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_HOLD: {
            drain_hold(scope);
            for (int i = 0; i < node->data.hold.num_body_statements; i++) {
                free_value(interpret_ast(node->data.hold.body[i], scope));
            }
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_TRIGGER: {
            // Evaluator message
            Value* msg_val = interpret_ast(node->data.trigger_node.message, scope);
            // Set error variables in current scope (and parent scopes? For now simple scope)
            // Ideally we walk up scopes until we find one with 'attempt' context?
            // But we can just set it in current and rely on recursion to find it? 
            // Actually, set_variable bubbles up if variable exists. NODE_ATTEMPT checks local attempt_scope.
            // If trigger is inside attempt_scope, set_variable works.
            set_variable(scope, "__last_error_name", create_string_value_helper(node->data.trigger_node.error_name));
            if (msg_val->type == VAL_STRING) {
                set_variable(scope, "__last_error_message", msg_val);
            } else {
                set_variable(scope, "__last_error_message", create_string_value_helper("Error triggered"));
                free_value(msg_val);
            }
            // Return NIL or ERROR? 
            // We need interpret_ast to return NULL or special value to signal interrupt?
            // Current main.c logic relies on caller checking specific variables or return values?
            // NODE_ATTEMPT checks __last_error_name.
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL; 
            break;
        }
        case NODE_SIGNAL: {
            const char *event = NULL;
            int start_idx = 0;
            if (node->data.signal_node.num_body_statements > 0 && node->data.signal_node.body[0]->type == NODE_EXPRESSION_STATEMENT) {
                ASTNode* expr = node->data.signal_node.body[0]->data.expr_statement.expression;
                if (expr->type == NODE_STRING) {
                    event = expr->data.string_val;
                }
            }
            if (event) {
                emit_signal(event, scope);
                for (int i = 1; i < node->data.signal_node.num_body_statements; i++) {
                    free_value(interpret_ast(node->data.signal_node.body[i], scope));
                }
            } else {
                for (int i = 0; i < node->data.signal_node.num_body_statements; i++) { free_value(interpret_ast(node->data.signal_node.body[i], scope)); }
            }
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_LISTEN: {
            const char *event = NULL;
            int start_idx = 0;
            if (node->data.listen.num_body_statements > 0 && node->data.listen.body[0]->type == NODE_EXPRESSION_STATEMENT) {
                ASTNode* expr = node->data.listen.body[0]->data.expr_statement.expression;
                if (expr->type == NODE_STRING) {
                    event = expr->data.string_val;
                    start_idx = 1;
                }
            }
            if (event) {
                register_listener(event, &node->data.listen.body[start_idx], node->data.listen.num_body_statements - start_idx);
            } else {
                for (int i = 0; i < node->data.listen.num_body_statements; i++) { free_value(interpret_ast(node->data.listen.body[i], scope)); }
            }
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_BOOL: {
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_BOOL;
            result_val->as.boolean = node->data.boolean_val;
            break;
        }
        case NODE_NIL: {
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_NICK: {
            // Nick is a declaration, usually returns nil
            // But we might want to register the alias?
            // For now, just evaluate original and alias (if they are expressions)
            // But NickNode definition suggests they are ASTNodes.
            // Assuming it's a declaration, we might need to do something.
            // For now, just return nil to silence error.
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
        }
        case NODE_TYPE: {
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_STRING;
            result_val->as.string = strdup(node->data.type_node.type_name);
            break;
        }
        case NODE_PACK: {
            // Create a string representation of the pack (simple implementation)
            // Format: "[item1, item2, item3]"
            char buffer[4096] = "[";
            int offset = 1;
            
            for (int i = 0; i < node->data.pack.num_items; i++) {
                Value* item_val = interpret_ast(node->data.pack.items[i], scope);
                char item_str[512];
                
                if (item_val->type == VAL_NUMBER) {
                    snprintf(item_str, sizeof(item_str), "%.10g", item_val->as.number);
                } else if (item_val->type == VAL_STRING) {
                    snprintf(item_str, sizeof(item_str), "\"%s\"", item_val->as.string);
                } else if (item_val->type == VAL_BOOL) {
                    snprintf(item_str, sizeof(item_str), "%s", item_val->as.boolean ? "true" : "false");
                } else {
                    snprintf(item_str, sizeof(item_str), "nil");
                }
                
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s%s", 
                                  i > 0 ? ", " : "", item_str);
                free_value(item_val);
            }
            
            buffer[offset++] = ']';
            buffer[offset] = '\0';
            
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_STRING;
            result_val->as.string = strdup(buffer);
            break;
        }
        default:
            fprintf(stderr, "Unhandled AST node type: %d\n", node->type);
            result_val = (Value*)malloc(sizeof(Value));
            result_val->type = VAL_NIL;
            break;
    }

    return result_val;
}

void free_ast(ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->data.program.num_statements; i++) {
                free_ast(node->data.program.statements[i]);
            }
            free(node->data.program.statements);
            break;
        case NODE_BINARY_OP:
            free_ast(node->data.binary_op.left);
            free_ast(node->data.binary_op.right);
            free(node->data.binary_op.op);
            break;
        case NODE_VAR_ACCESS:
            free(node->data.var_access.var_name);
            break;
        case NODE_VAR_ASSIGN:
            free_ast(node->data.var_assign.target);
            free_ast(node->data.var_assign.value);
            break;
        case NODE_EXPRESSION_STATEMENT:
            free_ast(node->data.expr_statement.expression);
            break;
        case NODE_SHOW_STATEMENT:
            for (int i = 0; i < node->data.show_statement.num_expressions; i++) {
                free_ast(node->data.show_statement.expressions[i]);
            }
            free(node->data.show_statement.expressions);
            break;
        case NODE_FUNCTION_DECL:
            free(node->data.function_decl.name);
            if (node->data.function_decl.docstring) {
                free(node->data.function_decl.docstring);
            }
            if (node->data.function_decl.func_type) {
                free(node->data.function_decl.func_type);
            }
            for (int i = 0; i < node->data.function_decl.num_params; i++) {
                free(node->data.function_decl.params[i]);
            }
            free(node->data.function_decl.params);
            for (int i = 0; i < node->data.function_decl.num_body_statements; i++) {
                free_ast(node->data.function_decl.body[i]);
            }
            free(node->data.function_decl.body);
            break;
        case NODE_FUNCTION_CALL:
            free(node->data.function_call.function_name);
            for (int i = 0; i < node->data.function_call.num_arguments; i++) {
                free_ast(node->data.function_call.arguments[i]);
            }
            free(node->data.function_call.arguments);
            break;
        case NODE_SPAWN:
            free_ast(node->data.spawn.blueprint_expr);
            for (int i = 0; i < node->data.spawn.num_arguments; i++) {
                free_ast(node->data.spawn.arguments[i]);
            }
            free(node->data.spawn.arguments);
            break;
        case NODE_ATTRIBUTE_ACCESS:
            free_ast(node->data.attribute_access.object);
            free(node->data.attribute_access.attribute_name);
            break;
        case NODE_DEN:
            free(node->data.den.name);
            for (int i = 0; i < node->data.den.num_attributes; i++) {
                free_ast(node->data.den.attributes[i]);
            }
            free(node->data.den.attributes);
            break;
        case NODE_CONVERT:
            free_ast(node->data.convert.source);
            free(node->data.convert.target_type);
            break;
        case NODE_TOOLKIT:
            free(node->data.toolkit.name);
            for (int i = 0; i < node->data.toolkit.num_body_statements; i++) { free_ast(node->data.toolkit.body[i]); }
            free(node->data.toolkit.body);
            break;
        case NODE_PLUG:
            if (node->data.plug.toolkit_name) free(node->data.plug.toolkit_name);
            if (node->data.plug.file_path) free(node->data.plug.file_path);
            break;
        case NODE_BRIDGE:
            free(node->data.bridge.name);
            for (int i = 0; i < node->data.bridge.num_body_statements; i++) {
                free_ast(node->data.bridge.body[i]);
            }
            free(node->data.bridge.body);
            break;
        case NODE_INLET:
            for (int i = 0; i < node->data.inlet.num_body_statements; i++) { free_ast(node->data.inlet.body[i]); }
            free(node->data.inlet.body);
            break;
        case NODE_LINK:
            free_ast(node->data.link.greeter);
            free_ast(node->data.link.implementation);
            break;
        case NODE_CHECK_STATEMENT:
            free_ast(node->data.check_statement.condition);
            for (int i = 0; i < node->data.check_statement.num_body_statements; i++) {
                free_ast(node->data.check_statement.body[i]);
            }
            free(node->data.check_statement.body);
            // Free alter and altern clauses
            break;
        case NODE_BLUEPRINT:
            free(node->data.blueprint.name);
            if (node->data.blueprint.docstring) {
                free(node->data.blueprint.docstring);
            }
            if (node->data.blueprint.parent) {
                free(node->data.blueprint.parent);
            }
            if (node->data.blueprint.constructor) {
                free_ast(node->data.blueprint.constructor);
            }
            for (int i = 0; i < node->data.blueprint.num_attributes; i++) {
                free_ast(node->data.blueprint.attributes[i]);
            }
            free(node->data.blueprint.attributes);
            break;
        case NODE_STRING:
            free(node->data.string_val);
            break;
        case NODE_INTERPOLATED_STRING:
            for (int i = 0; i < node->data.interpolated_string.num_parts; i++) {
                InterpolatedStringPart* part = node->data.interpolated_string.parts[i];
                if (part->type == INTERPOLATED_STRING_PART_STRING) {
                    free(part->data.string_val);
                } else { // INTERPOLATED_STRING_PART_EXPRESSION
                    free_ast(part->data.expression);
                }
                free(part);
            }
            free(node->data.interpolated_string.parts);
            break;
        case NODE_HALT:
        case NODE_PROCEED:
            // No dynamic allocation
            break;
        case NODE_WAIT:
            free_ast(node->data.wait.duration);
            break;
        case NODE_TRIGGER:
            free(node->data.trigger.error_name);
            free(node->data.trigger.message);
            break;
        case NODE_BLAME:
            free(node->data.blame.name);
            for (int i = 0; i < node->data.blame.num_body_statements; i++) {
                free_ast(node->data.blame.body[i]);
            }
            free(node->data.blame.body);
            break;
        case NODE_TYPE:
            free(node->data.type_node.type_name);
            break;
        case NODE_KIND:
            free_ast(node->data.kind.expression);
            break;
        case NODE_BOOL:
        case NODE_NIL:
            break;
        case NODE_NICK:
            free_ast(node->data.nick.original);
            free_ast(node->data.nick.alias);
            break;

        case NODE_HIDDEN:
            free_ast(node->data.hidden.declaration);
            break;
        case NODE_SHIELDED:
            free_ast(node->data.shielded.declaration);
            break;
        case NODE_INTERNAL:
            free_ast(node->data.internal.declaration);
            break;
        case NODE_EMBED:
            for (int i = 0; i < node->data.embed.num_body_statements; i++) { free_ast(node->data.embed.body[i]); }
            free(node->data.embed.body);
            break;
        case NODE_PARAL:
            for (int i = 0; i < node->data.paral.num_body_statements; i++) { free_ast(node->data.paral.body[i]); }
            free(node->data.paral.body);
            break;
        case NODE_HOLD:
            for (int i = 0; i < node->data.hold.num_body_statements; i++) { free_ast(node->data.hold.body[i]); }
            free(node->data.hold.body);
            break;
        case NODE_SIGNAL:
            for (int i = 0; i < node->data.signal_node.num_body_statements; i++) { free_ast(node->data.signal_node.body[i]); }
            free(node->data.signal_node.body);
            break;
        case NODE_LISTEN:
            for (int i = 0; i < node->data.listen.num_body_statements; i++) { free_ast(node->data.listen.body[i]); }
            free(node->data.listen.body);
            break;
        case NODE_ASK:
            for (int i = 0; i < node->data.ask.num_body_statements; i++) { free_ast(node->data.ask.body[i]); }
            free(node->data.ask.body);
            break;
        case NODE_AUTHEN:
            for (int i = 0; i < node->data.authen.num_body_statements; i++) { free_ast(node->data.authen.body[i]); }
            free(node->data.authen.body);
            break;
        case NODE_TRANSFORM:
            for (int i = 0; i < node->data.transform.num_body_statements; i++) { free_ast(node->data.transform.body[i]); }
            free(node->data.transform.body);
            break;
        case NODE_CONDENSE:
            for (int i = 0; i < node->data.condense.num_body_statements; i++) { free_ast(node->data.condense.body[i]); }
            free(node->data.condense.body);
            break;
        case NODE_PACK:
            for (int i = 0; i < node->data.pack.num_items; i++) { free_ast(node->data.pack.items[i]); }
            free(node->data.pack.items);
            break;
        case NODE_UNPACK:
            for (int i = 0; i < node->data.unpack.num_body_statements; i++) { free_ast(node->data.unpack.body[i]); }
            free(node->data.unpack.body);
            break;
        case NODE_CONSTRUCTOR_DECL:
            for (int i = 0; i < node->data.constructor_decl.num_params; i++) {
                free(node->data.constructor_decl.params[i]);
            }
            free(node->data.constructor_decl.params);
            for (int i = 0; i < node->data.constructor_decl.num_body_statements; i++) {
                free_ast(node->data.constructor_decl.body[i]);
            }
            free(node->data.constructor_decl.body);
            break;
        case NODE_ATTEMPT_TRAP_CONCLUDE:
            for (int i = 0; i < node->data.attempt_trap_conclude.num_attempt_statements; i++) {
                free_ast(node->data.attempt_trap_conclude.attempt_body[i]);
            }
            free(node->data.attempt_trap_conclude.attempt_body);
            if (node->data.attempt_trap_conclude.error_var) {
                free(node->data.attempt_trap_conclude.error_var);
            }
            for (int i = 0; i < node->data.attempt_trap_conclude.num_trap_statements; i++) {
                free_ast(node->data.attempt_trap_conclude.trap_body[i]);
            }
            free(node->data.attempt_trap_conclude.trap_body);
            for (int i = 0; i < node->data.attempt_trap_conclude.num_conclude_statements; i++) {
                free_ast(node->data.attempt_trap_conclude.conclude_body[i]);
            }
            free(node->data.attempt_trap_conclude.conclude_body);
            break;
        case NODE_EACH:
            free(node->data.each.var_name);
            free_ast(node->data.each.iterable);
            for (int i = 0; i < node->data.each.num_body_statements; i++) {
                free_ast(node->data.each.body[i]);
            }
            free(node->data.each.body);
            break;
        case NODE_UNTIL:
            free_ast(node->data.until.condition);
            for (int i = 0; i < node->data.until.num_body_statements; i++) {
                free_ast(node->data.until.body[i]);
            }
            free(node->data.until.body);
            break;
        case NODE_METHOD_CALL:
            free_ast(node->data.method_call.object);
            free(node->data.method_call.method_name);
            for (int i = 0; i < node->data.method_call.num_args; i++) {
                free_ast(node->data.method_call.args[i]);
            }
            free(node->data.method_call.args);
            break;
        case NODE_MODULE:
            free(node->data.module.name);
            for (int i = 0; i < node->data.module.num_body_statements; i++) {
                free_ast(node->data.module.body[i]);
            }
            free(node->data.module.body);
            break;
        case NODE_BRING:
            free(node->data.bring.module);
            if (node->data.bring.source) free(node->data.bring.source);
            break;
        case NODE_CONTRACT:
            free(node->data.contract.name);
            break;
        case NODE_UNARY_OP:
            free(node->data.unary_op.op);
            free_ast(node->data.unary_op.operand);
            break;
        // Add other cases for other node types to prevent memory leaks
        default:
            // For nodes like NODE_NUMBER, there's nothing to free inside the union
            break;
    }
    free(node);
}



ASTNode* parse_ast_from_file(const char* filename) {
    printf("Opening file: %s\n", filename);
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file: %s\n", filename); // Changed to printf
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    printf("File size: %ld\n", length);

    char *buffer = (char*)malloc(length + 1);
    if (!buffer) {
        printf("Failed to allocate memory for file content: %s\n", filename);
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    fclose(file);
    buffer[length] = '\0';

    printf("Parsing JSON...\n");
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("Error parsing JSON in %s near: %s\n", filename, error_ptr);
        } else {
            printf("Error parsing JSON: unknown error\n");
        }
        free(buffer);
        return NULL;
    }

    printf("Building AST...\n");
    ASTNode *ast = parse_ast_from_json(json);
    if (!ast) {
        printf("Failed to parse AST from %s (root is null or invalid).\n", filename);
    } else {
        printf("AST parsed successfully.\n");
    }

    cJSON_Delete(json);
    free(buffer);
    return ast;
}

int main(int argc, char** argv) {
    printf("NSPL running: %s\n", argv[1]); // Debug
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <path to ast.json>\n", argv[0]);
        return 1;
    }

    ASTNode *ast = parse_ast_from_file(argv[1]);
    if (!ast) {
        return 1;
    }
    
    Scope* global_scope = create_scope(NULL);
    if (ast->type == NODE_PROGRAM) {
        for (int i = 0; i < ast->data.program.num_statements; i++) {
            Value* result = interpret_ast(ast->data.program.statements[i], global_scope);
            if (result) {
                free_value(result);
            }
        }
    } else {
        Value* result = interpret_ast(ast, global_scope);
        if (result) {
            free_value(result);
        }
    }

    destroy_scope(global_scope);
    free_ast(ast);

    return 0;
}

ASTNode* parse_ast_from_json(cJSON *json_node) {
    if (!json_node || cJSON_IsNull(json_node)) return NULL;

    cJSON *type_json = cJSON_GetObjectItemCaseSensitive(json_node, "type");
    char *type_str = type_json ? type_json->valuestring : NULL;

    if (!type_str) {
        char *json_str = cJSON_Print(json_node);
        fprintf(stderr, "Invalid or missing type in JSON AST: %s\n", json_str);
        free(json_str);
        return NULL;
    }
    // printf("Parsing node type: %s\n", type_str);

    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        perror("Failed to allocate ASTNode");
        exit(EXIT_FAILURE);
    }
    memset(node, 0, sizeof(ASTNode));

    if (strcmp(type_str, "ProgramNode") == 0) {
        node->type = NODE_PROGRAM;
        cJSON *statements_json = cJSON_GetObjectItemCaseSensitive(json_node, "statements");
        int statement_count = cJSON_GetArraySize(statements_json);
        node->data.program.statements = (ASTNode**)malloc(statement_count * sizeof(ASTNode*));
        node->data.program.num_statements = statement_count;
        for (int i = 0; i < statement_count; i++) {
            node->data.program.statements[i] = parse_ast_from_json(cJSON_GetArrayItem(statements_json, i));
        }
    } else if (strcmp(type_str, "NumberNode") == 0) {
        node->type = NODE_NUMBER;
        node->data.number_val = cJSON_GetObjectItemCaseSensitive(json_node, "value")->valuedouble;
    } else if (strcmp(type_str, "UnaryOpNode") == 0) {
        node->type = NODE_UNARY_OP;
        node->data.unary_op.op = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "op")->valuestring);
        node->data.unary_op.operand = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "operand"));
    } else if (strcmp(type_str, "BinaryOpNode") == 0) {
        node->type = NODE_BINARY_OP;
        node->data.binary_op.left = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "left"));
        node->data.binary_op.right = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "right"));
        cJSON* op_obj = cJSON_GetObjectItemCaseSensitive(json_node, "op");
        if (cJSON_IsObject(op_obj)) {
            cJSON* op_val = cJSON_GetObjectItemCaseSensitive(op_obj, "value");
            if (cJSON_IsString(op_val) && (op_val->valuestring != NULL)) {
                node->data.binary_op.op = strdup(op_val->valuestring);
            }
        }
    } else if (strcmp(type_str, "VarAccessNode") == 0) {
        node->type = NODE_VAR_ACCESS;
        node->data.var_access.var_name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "var_name")->valuestring);
    } else if (strcmp(type_str, "VarAssignNode") == 0) {
        node->type = NODE_VAR_ASSIGN;
        node->data.var_assign.target = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "target"));
        node->data.var_assign.value = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "value"));
    } else if (strcmp(type_str, "ConstantDeclNode") == 0) {
        node->type = NODE_CONSTANT_DECL;
        node->data.constant_decl.const_name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "const_name")->valuestring);
        node->data.constant_decl.value = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "value"));
    } else if (strcmp(type_str, "ExpressionStatementNode") == 0) {
        node->type = NODE_EXPRESSION_STATEMENT;
        node->data.expr_statement.expression = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "expression"));
    } else if (strcmp(type_str, "ShowStatementNode") == 0) {
        node->type = NODE_SHOW_STATEMENT;
        cJSON* expressions_json = cJSON_GetObjectItemCaseSensitive(json_node, "expressions");
        int expression_count = cJSON_GetArraySize(expressions_json);
        node->data.show_statement.expressions = (ASTNode**)malloc(expression_count * sizeof(ASTNode*));
        node->data.show_statement.num_expressions = expression_count;
        for (int i = 0; i < expression_count; i++) {
            node->data.show_statement.expressions[i] = parse_ast_from_json(cJSON_GetArrayItem(expressions_json, i));
        }
    } else if (strcmp(type_str, "NickDeclNode") == 0) {
        node->type = NODE_NICK_DECL;
        node->data.nick_decl.original_type = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "var_name")->valuestring);
        node->data.nick_decl.alias = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "nick_name")->valuestring);
    } else if (strcmp(type_str, "FunctionDeclNode") == 0) {
        node->type = NODE_FUNCTION_DECL;
        node->data.function_decl.name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "name")->valuestring);
        cJSON *docstring_json = cJSON_GetObjectItemCaseSensitive(json_node, "docstring");
        if (docstring_json && cJSON_IsString(docstring_json)) {
            node->data.function_decl.docstring = strdup(docstring_json->valuestring);
        } else {
            node->data.function_decl.docstring = NULL;
        }
        cJSON *exposed_json = cJSON_GetObjectItemCaseSensitive(json_node, "exposed");
        node->data.function_decl.exposed = exposed_json ? cJSON_IsTrue(exposed_json) : false;
        cJSON *shared_json = cJSON_GetObjectItemCaseSensitive(json_node, "shared");
        node->data.function_decl.shared = shared_json ? cJSON_IsTrue(shared_json) : false;
        cJSON *func_type_json = cJSON_GetObjectItemCaseSensitive(json_node, "func_type");
        node->data.function_decl.func_type = (func_type_json && cJSON_IsString(func_type_json)) ? strdup(func_type_json->valuestring) : NULL;
        cJSON *params_json = cJSON_GetObjectItemCaseSensitive(json_node, "params");
        int param_count = cJSON_GetArraySize(params_json);
        node->data.function_decl.params = (char**)malloc(param_count * sizeof(char*));
        node->data.function_decl.num_params = param_count;
        for (int i = 0; i < param_count; i++) {
            node->data.function_decl.params[i] = strdup(cJSON_GetArrayItem(params_json, i)->valuestring);
        }
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.function_decl.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.function_decl.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.function_decl.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "ReturnStatementNode") == 0) {
        node->type = NODE_RETURN_STATEMENT;
        node->data.return_statement.expression = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "expression"));
    } else if (strcmp(type_str, "FunctionCallNode") == 0) {
        node->type = NODE_FUNCTION_CALL;
        node->data.function_call.function_name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "function_name")->valuestring);
        cJSON *args_json = cJSON_GetObjectItemCaseSensitive(json_node, "arguments");
        int arg_count = cJSON_GetArraySize(args_json);
        node->data.function_call.arguments = (ASTNode**)malloc(arg_count * sizeof(ASTNode*));
        node->data.function_call.num_arguments = arg_count;
        for (int i = 0; i < arg_count; i++) {
            node->data.function_call.arguments[i] = parse_ast_from_json(cJSON_GetArrayItem(args_json, i));
        }
    } else if (strcmp(type_str, "SpawnNode") == 0) {
        node->type = NODE_SPAWN;
        
        // The JSON has "blueprint_name" as a string, create a VarAccessNode from it
        cJSON *blueprint_name_json = cJSON_GetObjectItemCaseSensitive(json_node, "blueprint_name");
        if (blueprint_name_json && cJSON_IsString(blueprint_name_json)) {
            // Create a VarAccessNode for the blueprint name
            ASTNode *var_node = (ASTNode*)malloc(sizeof(ASTNode));
            var_node->type = NODE_VAR_ACCESS;
            var_node->data.var_access.var_name = strdup(blueprint_name_json->valuestring);
            node->data.spawn.blueprint_expr = var_node;
        } else {
            // Fallback to blueprint_expr if it exists (for compatibility)
            node->data.spawn.blueprint_expr = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "blueprint_expr"));
        }
        
        cJSON *args_json = cJSON_GetObjectItemCaseSensitive(json_node, "arguments");
        int arg_count = cJSON_GetArraySize(args_json);
        node->data.spawn.arguments = (ASTNode**)malloc(arg_count * sizeof(ASTNode*));
        node->data.spawn.num_arguments = arg_count;
        for (int i = 0; i < arg_count; i++) {
            node->data.spawn.arguments[i] = parse_ast_from_json(cJSON_GetArrayItem(args_json, i));
        }
    } else if (strcmp(type_str, "CheckStatementNode") == 0) {
        node->type = NODE_CHECK_STATEMENT;
        node->data.check_statement.condition = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "condition"));

        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.check_statement.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.check_statement.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.check_statement.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }

        cJSON *alter_clauses_json = cJSON_GetObjectItemCaseSensitive(json_node, "alter_clauses");
        int alter_count = cJSON_GetArraySize(alter_clauses_json);
        node->data.check_statement.alter_clauses = (AlterClause*)malloc(alter_count * sizeof(AlterClause));
        node->data.check_statement.num_alter_clauses = alter_count;
        for (int i = 0; i < alter_count; i++) {
            cJSON *alter_json = cJSON_GetArrayItem(alter_clauses_json, i);
            node->data.check_statement.alter_clauses[i].condition = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(alter_json, "condition"));
            cJSON *alter_body_json = cJSON_GetObjectItemCaseSensitive(alter_json, "body");
            int alter_body_count = cJSON_GetArraySize(alter_body_json);
            node->data.check_statement.alter_clauses[i].body = (ASTNode**)malloc(alter_body_count * sizeof(ASTNode*));
            node->data.check_statement.alter_clauses[i].num_body_statements = alter_body_count;
            for (int j = 0; j < alter_body_count; j++) {
                node->data.check_statement.alter_clauses[i].body[j] = parse_ast_from_json(cJSON_GetArrayItem(alter_body_json, j));
            }
        }

        cJSON *altern_clause_json = cJSON_GetObjectItemCaseSensitive(json_node, "altern_clause");
        if (altern_clause_json && !cJSON_IsNull(altern_clause_json) && cJSON_IsArray(altern_clause_json) && cJSON_GetArraySize(altern_clause_json) > 0) {
            int altern_body_count = cJSON_GetArraySize(altern_clause_json);
            node->data.check_statement.altern_clause = (ASTNode**)malloc(altern_body_count * sizeof(ASTNode*));
            node->data.check_statement.num_altern_statements = altern_body_count;
            for (int i = 0; i < altern_body_count; i++) {
                node->data.check_statement.altern_clause[i] = parse_ast_from_json(cJSON_GetArrayItem(altern_clause_json, i));
            }
        } else {
            node->data.check_statement.altern_clause = NULL;
            node->data.check_statement.num_altern_statements = 0;
        }
    } else if (strcmp(type_str, "ConstructorNode") == 0) {
        node->type = NODE_CONSTRUCTOR_DECL;
        cJSON *params_json = cJSON_GetObjectItemCaseSensitive(json_node, "params");
        int param_count = cJSON_GetArraySize(params_json);
        node->data.constructor_decl.params = (char**)malloc(param_count * sizeof(char*));
        node->data.constructor_decl.num_params = param_count;
        for (int i = 0; i < param_count; i++) {
            node->data.constructor_decl.params[i] = strdup(cJSON_GetArrayItem(params_json, i)->valuestring);
        }
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.constructor_decl.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.constructor_decl.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.constructor_decl.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "AttemptTrapConcludeNode") == 0) {
        node->type = NODE_ATTEMPT_TRAP_CONCLUDE;
        cJSON *attempt_body_json = cJSON_GetObjectItemCaseSensitive(json_node, "attempt_body");
        int attempt_body_count = cJSON_GetArraySize(attempt_body_json);
        node->data.attempt_trap_conclude.attempt_body = (ASTNode**)malloc(attempt_body_count * sizeof(ASTNode*));
        node->data.attempt_trap_conclude.num_attempt_statements = attempt_body_count;
        for (int i = 0; i < attempt_body_count; i++) {
            node->data.attempt_trap_conclude.attempt_body[i] = parse_ast_from_json(cJSON_GetArrayItem(attempt_body_json, i));
        }
        // Flatten trap_clauses bodies into trap_body array
        cJSON *trap_clauses_json = cJSON_GetObjectItemCaseSensitive(json_node, "trap_clauses");
        int trap_total = 0;
        if (trap_clauses_json && cJSON_IsArray(trap_clauses_json)) {
            for (int i = 0; i < cJSON_GetArraySize(trap_clauses_json); i++) {
                cJSON *clause = cJSON_GetArrayItem(trap_clauses_json, i);
                cJSON *body_json = cJSON_GetObjectItemCaseSensitive(clause, "body");
                trap_total += cJSON_GetArraySize(body_json);
            }
        }
        node->data.attempt_trap_conclude.trap_body = (ASTNode**)malloc((trap_total > 0 ? trap_total : 0) * sizeof(ASTNode*));
        node->data.attempt_trap_conclude.num_trap_statements = trap_total;
        int idx = 0;
        if (trap_clauses_json && cJSON_IsArray(trap_clauses_json)) {
            for (int i = 0; i < cJSON_GetArraySize(trap_clauses_json); i++) {
                cJSON *clause = cJSON_GetArrayItem(trap_clauses_json, i);
                cJSON *body_json = cJSON_GetObjectItemCaseSensitive(clause, "body");
                for (int j = 0; j < cJSON_GetArraySize(body_json); j++) {
                    node->data.attempt_trap_conclude.trap_body[idx++] = parse_ast_from_json(cJSON_GetArrayItem(body_json, j));
                }
            }
        }
        // conclude_clause is an array or null
        cJSON *conclude_clause_json = cJSON_GetObjectItemCaseSensitive(json_node, "conclude_clause");
        int conclude_body_count = (conclude_clause_json && cJSON_IsArray(conclude_clause_json)) ? cJSON_GetArraySize(conclude_clause_json) : 0;
        node->data.attempt_trap_conclude.conclude_body = (ASTNode**)malloc(conclude_body_count * sizeof(ASTNode*));
        node->data.attempt_trap_conclude.num_conclude_statements = conclude_body_count;
        for (int i = 0; i < conclude_body_count; i++) {
            node->data.attempt_trap_conclude.conclude_body[i] = parse_ast_from_json(cJSON_GetArrayItem(conclude_clause_json, i));
        }
        cJSON *peek_json = cJSON_GetObjectItemCaseSensitive(json_node, "peek");
        node->data.attempt_trap_conclude.peek = (peek_json && cJSON_IsBool(peek_json)) ? cJSON_IsTrue(peek_json) : false;

    } else if (strcmp(type_str, "AttributeAccessNode") == 0) {
        node->type = NODE_ATTRIBUTE_ACCESS;
        node->data.attribute_access.object = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "object"));
        node->data.attribute_access.attribute_name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "attribute")->valuestring);
    } else if (strcmp(type_str, "DenNode") == 0) {
        node->type = NODE_DEN;
        node->data.den.name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "name")->valuestring);
        cJSON *attributes_json = cJSON_GetObjectItemCaseSensitive(json_node, "attributes");
        int attr_count = cJSON_GetArraySize(attributes_json);
        node->data.den.attributes = (ASTNode**)malloc(attr_count * sizeof(ASTNode*));
        node->data.den.num_attributes = attr_count;
        for (int i = 0; i < attr_count; i++) {
            node->data.den.attributes[i] = parse_ast_from_json(cJSON_GetArrayItem(attributes_json, i));
        }
    } else if (strcmp(type_str, "ConvertNode") == 0) {
        node->type = NODE_CONVERT;
        // frontend uses "expression"
        cJSON *expr_json = cJSON_GetObjectItemCaseSensitive(json_node, "expression");
        node->data.convert.source = parse_ast_from_json(expr_json);
        node->data.convert.target_type = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "target_type")->valuestring);
    } else if (strcmp(type_str, "ToolkitNode") == 0) {
        node->type = NODE_TOOLKIT;
        node->data.toolkit.name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "name")->valuestring);
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.toolkit.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.toolkit.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.toolkit.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "PlugNode") == 0) {
        node->type = NODE_PLUG;
        node->data.plug.toolkit_name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "toolkit_name")->valuestring);
        cJSON *fp_json = cJSON_GetObjectItemCaseSensitive(json_node, "file_path");
        node->data.plug.file_path = fp_json && cJSON_IsString(fp_json) ? strdup(fp_json->valuestring) : NULL;
    } else if (strcmp(type_str, "BridgeNode") == 0) {
        node->type = NODE_BRIDGE;
        node->data.bridge.name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "name")->valuestring);
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.bridge.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.bridge.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.bridge.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "InletNode") == 0) {
        node->type = NODE_INLET;
        // frontend supplies body
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.inlet.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.inlet.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.inlet.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "LinkNode") == 0) {
        node->type = NODE_LINK;
        node->data.link.greeter = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "greeter"));
        node->data.link.implementation = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "implementation"));
    } else if (strcmp(type_str, "TraverseNode") == 0) {
        node->type = NODE_TRAVERSE;
        node->data.traverse.var_name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "var_name")->valuestring);
        node->data.traverse.start_val = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "start_val"));
        node->data.traverse.end_val = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "end_val"));
        cJSON *step_json = cJSON_GetObjectItemCaseSensitive(json_node, "step_val");
        node->data.traverse.step_val = step_json && !cJSON_IsNull(step_json) ? parse_ast_from_json(step_json) : NULL;
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.traverse.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.traverse.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.traverse.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "UntilNode") == 0) {
        node->type = NODE_UNTIL;
        node->data.until.condition = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "condition"));
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.until.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.until.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.until.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "InterpolatedStringNode") == 0) {
        node->type = NODE_INTERPOLATED_STRING;
        cJSON *parts_json = cJSON_GetObjectItemCaseSensitive(json_node, "parts");
        int part_count = cJSON_GetArraySize(parts_json);
        node->data.interpolated_string.parts = (InterpolatedStringPart**)malloc(part_count * sizeof(InterpolatedStringPart*));
        node->data.interpolated_string.num_parts = part_count;
        for (int i = 0; i < part_count; i++) {
            cJSON* part_json = cJSON_GetArrayItem(parts_json, i);
            InterpolatedStringPart* part = (InterpolatedStringPart*)malloc(sizeof(InterpolatedStringPart));
            node->data.interpolated_string.parts[i] = part;

            cJSON *part_type_json = cJSON_GetObjectItemCaseSensitive(part_json, "type");
            char *part_type_str = part_type_json->valuestring;

            if (strcmp(part_type_str, "StringNode") == 0) {
                part->type = INTERPOLATED_STRING_PART_STRING;
                part->data.string_val = strdup(cJSON_GetObjectItemCaseSensitive(part_json, "value")->valuestring);
            } else {
                part->type = INTERPOLATED_STRING_PART_EXPRESSION;
                part->data.expression = parse_ast_from_json(part_json);
            }
        }
    } else if (strcmp(type_str, "StringNode") == 0) {
        node->type = NODE_STRING;
        node->data.string_val = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "value")->valuestring);
    } else if (strcmp(type_str, "DocstringNode") == 0) {
        node->type = NODE_DOCSTRING;
        node->data.docstring_val = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "value")->valuestring);
    } else if (strcmp(type_str, "EmbedNode") == 0) {
        node->type = NODE_EMBED;
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.embed.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.embed.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.embed.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "ParalNode") == 0) {
        node->type = NODE_PARAL;
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.paral.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.paral.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.paral.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "HoldNode") == 0) {
        node->type = NODE_HOLD;
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.hold.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.hold.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.hold.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "SignalNode") == 0) {
        node->type = NODE_SIGNAL;
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.signal_node.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.signal_node.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.signal_node.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "ListenNode") == 0) {
        node->type = NODE_LISTEN;
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.listen.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.listen.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.listen.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "AskNode") == 0) {
        node->type = NODE_ASK;
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.ask.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.ask.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.ask.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "BlueprintNode") == 0) {
        node->type = NODE_BLUEPRINT;
        node->data.blueprint.name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "name")->valuestring);
        
        // Parse attributes
        cJSON *attributes_json = cJSON_GetObjectItemCaseSensitive(json_node, "attributes");
        int attribute_count = cJSON_GetArraySize(attributes_json);
        node->data.blueprint.attributes = (ASTNode**)malloc(attribute_count * sizeof(ASTNode*));
        node->data.blueprint.num_attributes = attribute_count;
        for (int i = 0; i < attribute_count; i++) {
            node->data.blueprint.attributes[i] = parse_ast_from_json(cJSON_GetArrayItem(attributes_json, i));
        }
        
        // Parse methods
        cJSON *methods_json = cJSON_GetObjectItemCaseSensitive(json_node, "methods");
        if (methods_json && !cJSON_IsNull(methods_json)) {
            int method_count = cJSON_GetArraySize(methods_json);
            node->data.blueprint.methods = (ASTNode**)malloc(method_count * sizeof(ASTNode*));
            node->data.blueprint.num_methods = method_count;
            for (int i = 0; i < method_count; i++) {
                node->data.blueprint.methods[i] = parse_ast_from_json(cJSON_GetArrayItem(methods_json, i));
            }
        } else {
            node->data.blueprint.methods = NULL;
            node->data.blueprint.num_methods = 0;
        }
        
        // Parse constructor
        cJSON *constructor_json = cJSON_GetObjectItemCaseSensitive(json_node, "constructor");
        if (constructor_json && !cJSON_IsNull(constructor_json)) {
            node->data.blueprint.constructor = parse_ast_from_json(constructor_json);
        } else {
            node->data.blueprint.constructor = NULL;
        }


    } else if (strcmp(type_str, "KindNode") == 0) {
        node->type = NODE_KIND;
        node->data.kind.expression = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "expression"));
    } else if (strcmp(type_str, "TypeNode") == 0) {
        node->type = NODE_TYPE;
        node->data.type_node.type_name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "type_name")->valuestring);
    } else if (strcmp(type_str, "BooleanNode") == 0) {
        node->type = NODE_BOOL;
        cJSON *val_json = cJSON_GetObjectItemCaseSensitive(json_node, "value");
        node->data.boolean_val = cJSON_IsTrue(val_json);
    } else if (strcmp(type_str, "NilNode") == 0) {
        node->type = NODE_NIL;
    } else if (strcmp(type_str, "NickNode") == 0) {
        node->type = NODE_NICK;
        cJSON *orig_json = cJSON_GetObjectItemCaseSensitive(json_node, "original");
        if (cJSON_IsString(orig_json)) {
            ASTNode* type_node = (ASTNode*)malloc(sizeof(ASTNode));
            type_node->type = NODE_TYPE;
            type_node->data.type_node.type_name = strdup(orig_json->valuestring);
            node->data.nick.original = type_node;
        } else {
            node->data.nick.original = parse_ast_from_json(orig_json);
        }

        cJSON *alias_json = cJSON_GetObjectItemCaseSensitive(json_node, "alias");
        if (cJSON_IsString(alias_json)) {
            ASTNode* type_node = (ASTNode*)malloc(sizeof(ASTNode));
            type_node->type = NODE_TYPE;
            type_node->data.type_node.type_name = strdup(alias_json->valuestring);
            node->data.nick.alias = type_node;
        } else {
            node->data.nick.alias = parse_ast_from_json(alias_json);
        }
    } else if (strcmp(type_str, "EachNode") == 0) {
        node->type = NODE_EACH;
        node->data.each.var_name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "var_name")->valuestring);
        node->data.each.iterable = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "iterable"));
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.each.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.each.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.each.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "UntilNode") == 0) {
        node->type = NODE_UNTIL;
        node->data.until.condition = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "condition"));
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.until.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.until.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.until.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "MethodCallNode") == 0) {
        node->type = NODE_METHOD_CALL;
        node->data.method_call.object = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "object"));
        node->data.method_call.method_name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "method_name")->valuestring);
        cJSON *args_json = cJSON_GetObjectItemCaseSensitive(json_node, "arguments");
        int arg_count = cJSON_GetArraySize(args_json);
        node->data.method_call.args = (ASTNode**)malloc(arg_count * sizeof(ASTNode*));
        node->data.method_call.num_args = arg_count;
        for (int i = 0; i < arg_count; i++) {
            node->data.method_call.args[i] = parse_ast_from_json(cJSON_GetArrayItem(args_json, i));
        }
    } else if (strcmp(type_str, "ModuleNode") == 0) {
        node->type = NODE_MODULE;
        node->data.module.name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "name")->valuestring);
        cJSON *body_json = cJSON_GetObjectItemCaseSensitive(json_node, "body");
        int body_count = cJSON_GetArraySize(body_json);
        node->data.module.body = (ASTNode**)malloc(body_count * sizeof(ASTNode*));
        node->data.module.num_body_statements = body_count;
        for (int i = 0; i < body_count; i++) {
            node->data.module.body[i] = parse_ast_from_json(cJSON_GetArrayItem(body_json, i));
        }
    } else if (strcmp(type_str, "BringNode") == 0) {
        node->type = NODE_BRING;
        cJSON *modules_json = cJSON_GetObjectItemCaseSensitive(json_node, "modules");
        if (modules_json && cJSON_IsArray(modules_json) && cJSON_GetArraySize(modules_json) > 0) {
             node->data.bring.module = strdup(cJSON_GetArrayItem(modules_json, 0)->valuestring);
        } else {
             node->data.bring.module = strdup("unknown_module");
        }
        cJSON *src = cJSON_GetObjectItemCaseSensitive(json_node, "source");
        node->data.bring.source = (src && cJSON_IsString(src)) ? strdup(src->valuestring) : NULL;
    } else if (strcmp(type_str, "ContractNode") == 0) {
        node->type = NODE_CONTRACT;
        node->data.contract.name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "name")->valuestring);
    } else if (strcmp(type_str, "TriggerNode") == 0) {
        node->type = NODE_TRIGGER;
        node->data.trigger_node.error_name = strdup(cJSON_GetObjectItemCaseSensitive(json_node, "error_name")->valuestring);
        node->data.trigger_node.message = parse_ast_from_json(cJSON_GetObjectItemCaseSensitive(json_node, "message"));
    } else if (strcmp(type_str, "PackNode") == 0) {
        node->type = NODE_PACK;
        cJSON *items_json = cJSON_GetObjectItemCaseSensitive(json_node, "items");
        int item_count = cJSON_GetArraySize(items_json);
        node->data.pack.num_items = item_count;
        node->data.pack.items = (ASTNode**)malloc(sizeof(ASTNode*) * item_count);
        for (int i = 0; i < item_count; i++) {
            node->data.pack.items[i] = parse_ast_from_json(cJSON_GetArrayItem(items_json, i));
        }
    } else {
        fprintf(stderr, "Unknown AST node type: %s\n", type_str);
        free(node);
        return NULL;
    }

    return node;
}


Scope* create_scope(Scope* parent) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->parent = parent;
    scope->symbol_count = 0;
    scope->symbol_capacity = 10;
    scope->symbols = (Symbol*)malloc(scope->symbol_capacity * sizeof(Symbol));
    return scope;
}

void destroy_scope(Scope* scope) {
    for (int i = 0; i < scope->symbol_count; i++) {
        free(scope->symbols[i].name);
        free_value(scope->symbols[i].value);
    }
    free(scope->symbols);
    free(scope);
}

Value* get_variable(Scope* scope, const char* name) {
    for (int i = 0; i < scope->symbol_count; i++) {
        if (strcmp(scope->symbols[i].name, name) == 0) {
            // Return a copy to prevent modification of the original value
            Value* copy = (Value*)malloc(sizeof(Value));
            memcpy(copy, scope->symbols[i].value, sizeof(Value));
            if (copy->type == VAL_STRING) {
                copy->as.string = strdup(scope->symbols[i].value->as.string);
            }
            return copy;
        }
    }
    if (scope->parent) {
        return get_variable(scope->parent, name);
    }
    return NULL; // Variable not found
}

// Helper to define a new variable in the current scope
// is_const: true if defined with 'firm'
void define_variable(Scope* scope, const char* name, Value* value, bool is_const) {
     // Check if already defined in CURRENT scope? 
     // For now, simpler implementation: just append. 
     // Ideally we should check if it exists in current scope and error if so?
     // Existing set_variable handles update. We probably want to split "declare" vs "assign".
     // But following existing pattern, let's just use set_variable logic but add const flag support.
    
    // Check if exists
    for (int i = 0; i < scope->symbol_count; i++) {
        if (strcmp(scope->symbols[i].name, name) == 0) {
            // Already exists. If it was constant, error.
            if (scope->symbols[i].is_constant) {
                fprintf(stderr, "Runtime Error: Cannot reassign constant '%s'.\n", name);
                // For now, just return (ignoring assignment) or we could exit.
                // Let's print error and keep old value to safe-guard.
                free_value(value);
                return;
            }
            // If we are trying to REDEFINE it as constant (e.g. firm x = ... again?), 
            // usually languages allow shadowing in new scopes, but here we are in same scope.
            // If it's a reassignment `x = 5`, we use set_variable. 
            // If it's `firm x = 5`, parser produced NODE_CONSTANT_DECL.
            
            // Overwrite value
            free_value(scope->symbols[i].value);
            scope->symbols[i].value = value;
            scope->symbols[i].is_constant = is_const; // Update const-ness if redefined?
            return;
        }
    }

    if (scope->symbol_count >= scope->symbol_capacity) {
        scope->symbol_capacity *= 2;
        scope->symbols = (Symbol*)realloc(scope->symbols, scope->symbol_capacity * sizeof(Symbol));
    }

    scope->symbols[scope->symbol_count].name = strdup(name);
    scope->symbols[scope->symbol_count].value = value;
    scope->symbols[scope->symbol_count].is_constant = is_const;
    scope->symbol_count++;
}

void set_variable(Scope* scope, const char* name, Value* value) {
    // Look up in scope chain
    Scope* current = scope;
    while (current) {
        for (int i = 0; i < current->symbol_count; i++) {
            if (strcmp(current->symbols[i].name, name) == 0) {
                if (current->symbols[i].is_constant) {
                    fprintf(stderr, "Runtime Error: Cannot assign to constant '%s'.\n", name);
                    free_value(value);
                    return;
                }
                free_value(current->symbols[i].value);
                current->symbols[i].value = value;
                return;
            }
        }
        current = current->parent;
    }

    // If not found, define in current scope (implicit declaration as non-constant variable?)
    // Nervestack seems to allow implicit variable creation on assignment if not found?
    // Based on previous code: yes, it appended to `scope` (which was passed in).
    define_variable(scope, name, value, false);
}

char* value_to_string(Value* val) {
    if (!val) {
        return strdup("null");
    }
    char* str = malloc(100); // Allocate a buffer
    switch (val->type) {
        case VAL_NUMBER:
            if ((long long)val->as.number == val->as.number) {
                sprintf(str, "%lld", (long long)val->as.number);
            } else {
                sprintf(str, "%g", val->as.number);
            }
            break;
        case VAL_STRING:
            if (val->as.string) {
                free(str); // Free the small buffer
                str = strdup(val->as.string); // Return a copy of the string
            } else {
                sprintf(str, "(null string)");
            }
            break;
        case VAL_BOOL:
            sprintf(str, "%s", val->as.boolean ? "true" : "false");
            break;
        case VAL_RANGE:
            sprintf(str, "%g..%g", val->as.range.start, val->as.range.end);
            break;
        case VAL_NIL:
            sprintf(str, "nil");
            break;
        default:
            sprintf(str, "Unknown value type");
            break;
    }
    return str;
}


