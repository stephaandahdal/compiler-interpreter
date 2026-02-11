#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "execute.h"
#include "lexer.h"
#include "inputbuf.h"
#include <unordered_map>

using namespace std;

//global declarations
LexicalAnalyzer lexer;
extern int mem[1000];
extern int next_available;
extern std::vector<int> inputs;
extern int next_input;
unordered_map<string, int> location_table;
unordered_map<int, int> int_location_table;

//forward declarations
void syntax_error();
Token expect(TokenType expected_type);
int location(string var);

void parse_var_section();
void parse_id_list();
InstructionNode* parse_body();
InstructionNode* parse_stmt_list();
InstructionNode* parse_stmt();
InstructionNode* parse_assign_stmt();
InstructionNode* parse_output_stmt();
InstructionNode* parse_input_stmt();
InstructionNode* parse_output_stmt();
InstructionNode* parse_while_stmt();
InstructionNode* parse_if_stmt();
void parse_condition(InstructionNode* node);
InstructionNode* parse_for_stmt();
InstructionNode* parse_switch_stmt();
void parse_inputs();
struct InstructionNode *parse_Generate_Intermediate_Representation();

// HELPER FUNCTIONS
void syntax_error()
{
    printf("SYNTAX ERROR\n");
    exit(1);
}

Token expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}

//get and set location for string ids
int location(string var) {
    return location_table[var];
}

//get and set locations for constants in table and memory
int int_location(int var) {
    if (int_location_table.find(var) == int_location_table.end()) {
        int_location_table[var] = next_available++;
        mem[int_location_table[var]] = var;
    }

    return int_location_table[var];
}

// -------- PARSE FUNCTIONS --------
void parse_var_section() {
    parse_id_list();
    expect(SEMICOLON);
}

void parse_id_list() {
    Token id = expect(ID);
    //populate table and memory with ids
    if (location_table.find(id.lexeme) == location_table.end()) {
        location_table[id.lexeme] = next_available++;
        mem[location_table[id.lexeme]] = 0;
    }

    if (lexer.peek(1).token_type == COMMA) {
        expect(COMMA);
        parse_id_list();
    }
}

InstructionNode* parse_body() {
    expect(LBRACE);
    InstructionNode* stmt_list = parse_stmt_list();
    expect(RBRACE);

    return stmt_list;
}

InstructionNode* parse_stmt_list() {
    //linked lsit of statements
    InstructionNode* stmt = parse_stmt();
    InstructionNode* head = stmt;

    //traverse list of statements to parse each statement
    Token t = lexer.peek(1);
    if (t.token_type == ID || t.token_type == WHILE || t.token_type == IF || 
        t.token_type == SWITCH || t.token_type == FOR || t.token_type == OUTPUT || t.token_type == INPUT) {
        while (stmt->next != nullptr) {
            stmt = stmt->next;
        }
        stmt->next = parse_stmt_list();
    }

    return head;
}

InstructionNode* parse_stmt() {
    Token stmt_type = lexer.peek(1);
    //parse statement depending on type
    switch (stmt_type.token_type) {
        case ID: return parse_assign_stmt();
        case WHILE: return parse_while_stmt();
        case IF: return parse_if_stmt();
        case SWITCH: return parse_switch_stmt();
        case FOR: return parse_for_stmt();
        case OUTPUT: return parse_output_stmt();
        case INPUT: return parse_input_stmt();
        default:
            syntax_error();
            return nullptr;
    }
}

InstructionNode* parse_assign_stmt() {
    Token lhs = expect(ID);
    expect(EQUAL);

    //create node with lhs
    InstructionNode* node = new InstructionNode();
    node->type = ASSIGN;
    node->assign_inst.lhs_loc = location(lhs.lexeme);

    //set op1 location
    Token first_operand = lexer.GetToken();
    if (first_operand.token_type == ID) {
        node->assign_inst.op1_loc = location(first_operand.lexeme);
    }
    if (first_operand.token_type == NUM) {
        node->assign_inst.op1_loc = int_location(stoi(first_operand.lexeme));
    }

    //set op if exists
    if (lexer.peek(1).token_type == PLUS || lexer.peek(1).token_type == MINUS ||
        lexer.peek(1).token_type == MULT || lexer.peek(1).token_type == DIV) {
        Token op = lexer.GetToken();
        if (op.token_type == PLUS) {
            node->assign_inst.op = OPERATOR_PLUS;
        }
        if (op.token_type == MINUS) {
            node->assign_inst.op = OPERATOR_MINUS;
        }
        if (op.token_type == MULT) {
            node->assign_inst.op = OPERATOR_MULT;
        }
        if (op.token_type == DIV) {
            node->assign_inst.op = OPERATOR_DIV;
        }

        //set op2 if exists
        Token second_operand = lexer.GetToken();
        if (second_operand.token_type == ID) {
            node->assign_inst.op2_loc = location(second_operand.lexeme);
        }
        if (second_operand.token_type == NUM) {
            node->assign_inst.op2_loc = int_location(stoi(second_operand.lexeme));
        }
    //no op if not exist
    } else {
        node->assign_inst.op = OPERATOR_NONE;
    }

    node->next = nullptr;
    expect(SEMICOLON);

    return node;
}

InstructionNode* parse_output_stmt() {
    expect(OUTPUT);
    Token id = expect(ID);

    //create output node with location of var
    InstructionNode* node = new InstructionNode();
    node->type = OUT;
    node->output_inst.var_loc = location(id.lexeme);
    node->next = nullptr;

    expect(SEMICOLON);
    return node;
}

InstructionNode* parse_input_stmt() {
    expect(INPUT);
    Token id = expect(ID);

    //create input node with location of var
    InstructionNode* node = new InstructionNode();
    node->type = IN;
    node->input_inst.var_loc = location(id.lexeme);
    node->next = nullptr;

    expect(SEMICOLON);
    return node;
}

InstructionNode* parse_while_stmt() {
    expect(WHILE);
    //create conditional jump node
    InstructionNode* node = new InstructionNode();
    node->type = CJMP;

    //parse conditional and define it
    parse_condition(node);

    //parse body and attach it to conditional
    InstructionNode* body = parse_body();
    node->next = body;
    
    //jump node to go back to condition
    InstructionNode* jump_node = new InstructionNode();
    jump_node->type = JMP;
    jump_node->jmp_inst.target = node;
    jump_node->next = nullptr;

    //temp node to traverse after body
    InstructionNode* temp = body;
    while (temp->next != nullptr) {
        temp = temp->next;
    }
    temp->next = jump_node;

    //create no op node to exit
    //might need changing?
    InstructionNode* exit_while = new InstructionNode();
    exit_while->type = NOOP;
    exit_while->next = nullptr;
    jump_node->next = exit_while;
    node->cjmp_inst.target = exit_while;

    return node;
}

InstructionNode* parse_if_stmt() {
    expect(IF);
    //create conditional jump node
    InstructionNode* node = new InstructionNode();
    node->type = CJMP;

    //parse conditional and define it
    parse_condition(node);

    //parse body and attach it to conditional
    InstructionNode* body = parse_body();
    node->next = body;

    //no op node
    InstructionNode* exit_if = new InstructionNode();
    exit_if->type = NOOP;
    exit_if->next = nullptr;

    //traverse body and attach no op to end
    InstructionNode* temp = body;
    while (temp->next != nullptr) {
        temp = temp->next;
    }
    temp->next = exit_if;
    node->cjmp_inst.target = exit_if;

    return node;
}

void parse_condition(InstructionNode* node) {
    //op1
    Token first_operand = lexer.GetToken();
    if (first_operand.token_type == ID) {
        node->cjmp_inst.op1_loc = location(first_operand.lexeme);
    }
    if (first_operand.token_type == NUM) {
        node->cjmp_inst.op1_loc = int_location(stoi(first_operand.lexeme));
    }

    //op
    Token op = lexer.GetToken();
    if (op.token_type == GREATER) {
        node->cjmp_inst.condition_op = CONDITION_GREATER;
    }
    if (op.token_type == LESS) {
        node->cjmp_inst.condition_op = CONDITION_LESS;
    }
    if (op.token_type == NOTEQUAL) {
        node->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    }

    //op2
    Token second_operand = lexer.GetToken();
    if (second_operand.token_type == ID) {
        node->cjmp_inst.op2_loc = location(second_operand.lexeme);
    }
    if (second_operand.token_type == NUM) {
        node->cjmp_inst.op2_loc = int_location(stoi(second_operand.lexeme));
    }
}

InstructionNode* parse_for_stmt() {
    expect(FOR);
    expect(LPAREN);
    //get first assign
    InstructionNode* first_assign = parse_assign_stmt();

    //get condition
    InstructionNode* condition = new InstructionNode();
    condition->type = CJMP;
    parse_condition(condition);

    expect(SEMICOLON);

    //get second assign
    InstructionNode* second_assign = parse_assign_stmt();
    expect(RPAREN);

    //get body
    InstructionNode* body = parse_body();
    
    //follow order of assign1, condition, body, assign2, jump back
    first_assign->next = condition;
    condition->next = body;
    InstructionNode* temp = body;
    while (temp->next != nullptr) {
        temp = temp->next;
    }
    temp->next = second_assign;
    InstructionNode* jump_node = new InstructionNode();
    jump_node->type = JMP;
    jump_node->next = nullptr;
    jump_node->jmp_inst.target = condition;
    second_assign->next = jump_node;

    //noop to exit
    InstructionNode* exit_for = new InstructionNode();
    exit_for->type = NOOP;
    exit_for->next = nullptr;
    jump_node->next = exit_for;
    condition->cjmp_inst.target = exit_for;

    return first_assign;
}

InstructionNode* parse_switch_stmt() {
    expect(SWITCH);
    //s is the switch
    Token s = expect(ID);
    int s_loc = location(s.lexeme);
    expect(LBRACE);

    //no op exit node to be used
    InstructionNode* exit_switch = new InstructionNode();
    exit_switch->next = nullptr;
    exit_switch->type = NOOP;

    //instantiate head and tail for the entire statement
    //head used for chaining entire switch
    InstructionNode* head = nullptr;
    //tail used to attach default case at end of other cases
    InstructionNode* tail = nullptr;

    //go through case list
    while (lexer.peek(1).token_type == CASE) {
        expect(CASE);
        Token case_num = expect(NUM);
        int num = stoi(case_num.lexeme);
        int num_loc = int_location(num);
        expect(COLON);

        //generate comparison between switch and current case (switch != current case)
        InstructionNode* case_cond = new InstructionNode();
        case_cond->type = CJMP;
        case_cond->next = nullptr;
        case_cond->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
        case_cond->cjmp_inst.op1_loc = s_loc;
        case_cond->cjmp_inst.op2_loc = num_loc;
        case_cond->cjmp_inst.target = nullptr;

        InstructionNode* body = parse_body();

        //traverse end of body to attach exit jump
        InstructionNode* temp = body;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        //jump to exit
        InstructionNode* jump_node = new InstructionNode();
        jump_node->type = JMP;
        jump_node->jmp_inst.target = exit_switch;
        jump_node->next = nullptr;

        //link the list of nodes (cond, body, jump exit)
        temp->next = jump_node;
        case_cond->cjmp_inst.target = body;
        
        //traverse switch cases
        if (tail != nullptr) {
            tail->next = case_cond;
            tail = case_cond;
        } else {
            head = case_cond;
            tail = case_cond;
        }
    }

    //check for default case
    InstructionNode* default_body = nullptr;
    if (lexer.peek(1).token_type == DEFAULT) {
        expect(DEFAULT);
        expect(COLON);
        default_body = parse_body();

        //append exit to end of default case
        InstructionNode* temp = default_body;
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = exit_switch;
    }
    expect(RBRACE);

    //if cases other than default exist
    if (tail != nullptr) {
        if (default_body != nullptr) {
            tail->next = default_body;
        } else {
            tail->next = exit_switch;
        }
    }
    //if just default
    else {
        if (default_body != nullptr) {
            head = default_body;
        } else {
            head = exit_switch;
        }
    }

    return head;
}

void parse_inputs() {
    //insert integers into inputs vector
    while (lexer.peek(1).token_type == NUM) {
        int curr_input = stoi(lexer.GetToken().lexeme);
        inputs.push_back(curr_input);
    }
}

struct InstructionNode *parse_Generate_Intermediate_Representation()
{
    
    //reset data structures
    next_available = 0;
    location_table.clear();
    int_location_table.clear();
    inputs.clear();

    //parse three main sections
    parse_var_section();
    InstructionNode* body = parse_body();
    parse_inputs();

    return body;
}