#include "code_gen.hpp"

extern FILE *code;
extern int goffset;

extern SymbolTable gcST;
extern int toffset;
extern int address_of_main;
extern Node *curr_decl;

extern bool in_loop;
extern Node *loop;
extern Node *embedded_loop;

Node *current_function = NULL;
int string_offset = -1;

/* ==================================================
   GENERATE FUNCTION
   ================================================== */
void generate_function(Node *func)
{
   gcST.insert(func->literal, func);
   gcST.enter(func->literal);
   current_function = func;
   int toffset_temp = toffset;
   toffset = func->size;
   func->address = emitWhereAmI();
   if (func->isMain)
   {
      address_of_main = func->address;
   }
   emitComment((char *)"** ** ** ** ** ** ** ** ** ** ** **");
   emitComment((char *)"FUNCTION", func->literal);
   emitComment((char *)"TOFF set:", toffset);
   emitRM((char *)"ST", 3, -1, 1, (char *)"Store return address");
   gc_traverse_children(func);
   emitComment((char *)"Add standard closing in case there is no return statement");
   emitRM((char *)"LDC", 2, 0, 6, (char *)"Set return value to 0");
   emitRM((char *)"LD", 3, -1, 1, (char *)"Load return address");
   emitRM((char *)"LD", 1, 0, 1, (char *)"Adjust fp");
   emitRM((char *)"JMP", 7, 0, 3, (char *)"Return");
   toffset = toffset_temp;
   emitComment((char *)"END FUNCTION", func->literal);
   if (!func->isMain)
   {
      emitComment((char *)"");
   }
   gcST.leave();
}

/* ==================================================
   GENERATE FUNCTION COMPOUND
   ================================================== */
void generate_function_compound(Node *node)
{
   int toffset_temp = toffset;
   toffset = node->size;
   emitComment((char *)"COMPOUND");
   emitComment((char *)"TOFF set:", toffset);
   gc_traverse_sibs(node->child[0]); // Declarations
   emitComment((char *)"Compound Body");
   gc_traverse_children(node);
   toffset = toffset_temp;
   emitComment((char *)"TOFF set:", toffset);
   emitComment((char *)"END COMPOUND");
}

/* ==================================================
   GENERATE FUNCTION COMPOUND
   ================================================== */
void generate_compound(Node *node)
{
   gcST.enter("Compound");
   int toffset_temp = toffset;
   toffset = node->size;
   emitComment((char *)"COMPOUND");
   emitComment((char *)"TOFF set:", toffset);
   gc_traverse_sibs(node->child[0]); // Declarations
   emitComment((char *)"Compound Body");
   gc_traverse_children(node);
   toffset = toffset_temp;
   emitComment((char *)"TOFF set:", toffset);
   emitComment((char *)"END COMPOUND");
   gcST.leave();
}

/* ==================================================
   GENERATE OPERATORS
   ================================================== */
void generate_op(Node *node)
{
   Node *lhs = node->child[0];
   Node *rhs = node->child[1];

   // 1. Load LHS
   if (load_in(lhs) == false)
   {
      // If LHS is an array...
      if (load_arr_op(lhs) == false)
      {
         // For any other nodetypes...
         switch (lhs->nodeType)
         {
         case OpNT:
            generate_op(lhs);
            break;
         case QuesNT:
            generate_Ques(lhs);
            break;
         case SignNT:
            generate_ChSign(lhs);
            break;
         case CallNT:
            generate_call(lhs);
            break;
         case AssignNT:
            generate_assignment(lhs);
            break;
         case SizeOfNT:
            generate_sizeof(lhs);
            break;
         }
         emitRM((char *)"ST", 3, toffset, 1, (char *)"Push left side");
      }
      else
      {
         emitRM((char *)"ST", 3, toffset, 1, (char *)"Push left side");
      }
   }
   else
   {
      emitRM((char *)"ST", 3, toffset, 1, (char *)"Push left side");
   }
   toffset -= 1;
   emitComment((char *)"TOFF dec:", toffset);
   // 2. Load RHS
   if (load_in(rhs) == false)
   {
      // If RHS is an array...
      if (load_arr_op(rhs) == false)
      {
         // For any other nodetypes...
         switch (rhs->nodeType)
         {
         case OpNT:
            generate_op(rhs);
            break;
         case QuesNT:
            generate_Ques(rhs);
            break;
         case SignNT:
            generate_ChSign(rhs);
            break;
         case CallNT:
            generate_call(rhs);
            break;
         case SizeOfNT:
            generate_sizeof(rhs);
            break;
         }
      }
   }
   toffset += 1;
   emitComment((char *)"TOFF inc:", toffset);
   emitRM((char *)"LD", 4, toffset, 1, (char *)"Pop left into ac1");
   if (rhs->isArray && rhs->dataType == CharDT)
   {
      emitRM((char *)"LD", AC2, 1, 3, (char *)"AC2 <- |RHS|");
      emitRM((char *)"LD", AC3, 1, 4, (char *)"AC3 <- |LHS|");
      emitRM((char *)"LDA", 2, 0, 5, (char *)"R2 <- |RHS|");
      emitRO((char *)"SWP", 5, 6, 6, (char *)"pick smallest size");
      emitRM((char *)"LD", AC3, 1, 4, (char *)"AC3 <- |LHS|");
      emitRO((char *)"CO", 4, 3, 5, (char *)"setup array compare  LHS vs RHS");
      emitRO((char *)"TNE", 5, AC1, AC, (char *)"if not equal then test (AC1, AC)");
      emitRO((char *)"JNZ", 5, 2, 7, (char *)"jump not equal");
      emitRM((char *)"LDA", AC, 0, 2, (char *)"AC1 <- |RHS|");
      emitRM((char *)"LDA", AC1, 0, 6, (char *)"AC <- |LHS|");
   }
   switch (node->tknClass)
   {
   case MUL:
      emitRO((char *)"MUL", 3, 4, 3, (char *)"Op *");
      break;
   case ADD:
      emitRO((char *)"ADD", 3, 4, 3, (char *)"Op +");
      break;
   case SUB:
      emitRO((char *)"SUB", 3, 4, 3, (char *)"Op -");
      break;
   case DIV:
      emitRO((char *)"DIV", 3, 4, 3, (char *)"Op /");
      break;
   case MOD:
      emitRO((char *)"MOD", 3, 4, 3, (char *)"Op %");
      break;
   case EQL:
      emitRO((char *)"TEQ", 3, 4, 3, (char *)"Op =");
      break;
   case GREAT:
      emitRO((char *)"TGT", 3, 4, 3, (char *)"Op >");
      break;
   case LESS:
      emitRO((char *)"TLT", 3, 4, 3, (char *)"Op <");
      break;
   case LEQ:
      emitRO((char *)"TLE", 3, 4, 3, (char *)"Op <=");
      break;
   case GEQ:
      emitRO((char *)"TGE", 3, 4, 3, (char *)"Op >=");
      break;
   case NEQ:
      emitRO((char *)"TNE", 3, 4, 3, (char *)"Op ><");
      break;
   }
}

/* ==================================================
   GENERATE ASSIGNMENTS
   ================================================== */
void generate_assignment(Node *node)
{
   Node *lhs = node->child[0];
   Node *rhs = node->child[1];

   if (node->tknClass == ASGN)
   {
      // Check if LHS is an Array
      if (load_arr_assign(lhs, rhs) == false)
      {
         // 1. Check RHS
         if (load_in(rhs) == false)
         {
            switch (rhs->nodeType)
            {
            case AssignNT:
               generate_assignment(rhs);
               break;
            case OpNT:
               generate_op(rhs);
               break;
            case CallNT:
               generate_call(rhs);
               break;
            case AndNT:
               generate_and_or(rhs);
               break;
            case OrNT:
               generate_and_or(rhs);
               break;
            case NotNT:
               generate_not(rhs);
               break;
            case SignNT:
               generate_ChSign(rhs);
               break;
            case ArrNT:
               load_arr_op(rhs);
               break;
            }
         }
         // 2. Store LHS
         if (rhs->isArray && rhs->nodeType == IdNT)
         {
            emitRM((char *)"LDA", 4, lhs->location, 1, (char *)"address of lhs");
            emitRM((char *)"LD", 5, 1, 3, (char *)"size of rhs");
            emitRM((char *)"LD", 6, 1, 4, (char *)"size of lhs");
            emitRO((char *)"SWP", 5, 6, 6, (char *)"pick smallest size");
            emitRO((char *)"MOV", 4, 3, 5, (char *)"array op =");
         }
         else
         {
            store_var(lhs);
         }
      }
      else
      {
         emitRM((char *)"ST", 3, 0, 5, (char *)"Store variable", lhs->child[0]->literal);
      }
   }
   else if (node->tknClass == ADDASS || node->tknClass == SUBASS ||
            node->tknClass == MULASS || node->tknClass == DIVASS)
   {
      char *cmd = NULL;
      char *out_msg = NULL;

      switch (node->tknClass)
      {
      case ADDASS:
         cmd = (char *)"ADD\0";
         out_msg = (char *)"op +=\0";
         break;
      case SUBASS:
         cmd = (char *)"SUB\0";
         out_msg = (char *)"op -=\0";
         break;
      case MULASS:
         cmd = (char *)"MUL\0";
         out_msg = (char *)"op *=\0";
         break;
      case DIVASS:
         cmd = (char *)"DIV\0";
         out_msg = (char *)"op /=\0";
         break;
      }

      // Check if LHS is an Array
      if (load_arr_assign(lhs, rhs) == false)
      {
         // 1. Check RHS
         if (load_in(rhs) == false)
         {
            switch (rhs->nodeType)
            {
            case AssignNT:
               generate_assignment(rhs);
               break;
            case ArrNT:
               load_arr_op(rhs);
               break;
            case OpNT:
               generate_op(rhs);
               break;
            }
         }
         Node *sym = fetchSymbol(lhs, &gcST);
         emitRM((char *)"LD", 4, sym->location, sym->refType, (char *)"load lhs variable", sym->literal);
         emitRO(cmd, 3, 4, 3, out_msg);
         if (lhs->nodeType == ArrNT)
            emitRM((char *)"ST", 3, sym->location, sym->refType, (char *)"Store variable", sym->literal);
         //  2. Store LHS
         store_var(lhs);
      }
      else
      {
         Node *sym = fetchSymbol(lhs->child[0], &gcST);
         emitRM((char *)"LD", 4, 0, 5, (char *)"load lhs variable", sym->literal);
         emitRO(cmd, 3, 4, 3, out_msg);
         if (lhs->nodeType == ArrNT)
            emitRM((char *)"ST", 3, 0, 5, (char *)"Store variable", sym->literal);
      }
   }
   else if (node->tknClass == INC || node->tknClass == DEC)
   {
      Node *val = node->child[0];
      if (val->nodeType == IdNT)
      {
         Node *id = fetchSymbol(val, &gcST);
         emitRM((char *)"LD", 3, id->location, id->refType, (char *)"load lhs variable", id->literal);
         if (node->tknClass == INC)
         {
            emitRM((char *)"LDA", 3, 1, 3, (char *)"increment value of", val->literal);
         }
         else
         {
            emitRM((char *)"LDA", 3, -1, 3, (char *)"decrement value of", val->literal);
         }
         store_var(val);
      }
      else if (val->nodeType == ArrNT)
      {
         Node *id = fetchSymbol(val->child[0], &gcST);
         Node *idx = val->child[1];
         load_in(idx);
         if (id->nodeType == ParmArrNT)
         {
            emitRM((char *)"LD", AC2, id->location, id->refType, (char *)"Load address of base of array", id->literal);
         }
         else
         {
            emitRM((char *)"LDA", AC2, id->location, id->refType, (char *)"Load address of base of array", id->literal);
         }
         emitRO((char *)"SUB", 5, 5, 3, (char *)"Compute offset of value");
         emitRM((char *)"LD", 3, 0, 5, (char *)"load lhs variable", id->literal);
         if (node->tknClass == INC)
         {
            emitRM((char *)"LDA", 3, 1, 3, (char *)"increment value of", id->literal);
         }
         else
         {
            emitRM((char *)"LDA", 3, -1, 3, (char *)"decrement value of", id->literal);
         }
         emitRM((char *)"ST", 3, 0, 5, (char *)"Store variable", id->literal);
      }
   }
}

/* ==================================================
   GENERATE 'AND' & 'OR
   ================================================== */
void generate_and_or(Node *node)
{
   Node *lhs = node->child[0];
   Node *rhs = node->child[1];

   // 1. Load LHS
   if (load_in(lhs) == false)
   {
      if (load_arr_op(lhs) == false)
      {
         switch (lhs->nodeType)
         {
         case OpNT:
            generate_op(lhs);
            break;
         case AndNT:
            generate_and_or(lhs);
            break;
         case OrNT:
            generate_and_or(lhs);
            break;
         case NotNT:
            generate_not(lhs);
            break;
         case CallNT:
            generate_call(lhs);
            break;
         }
         emitRM((char *)"ST", 3, toffset, 1, (char *)"Push left side");
      }
      else
      {
         emitRM((char *)"ST", 3, toffset, 1, (char *)"Push left side");
      }
   }
   else
   {
      emitRM((char *)"ST", 3, toffset, 1, (char *)"Push left side");
   }
   toffset -= 1;
   emitComment((char *)"TOFF dec:", toffset);
   // 2. Load RHS
   if (load_in(rhs) == false)
   {
      if (load_arr_op(rhs) == false)
      {
         switch (rhs->nodeType)
         {
         case OpNT:
            generate_op(rhs);
            break;
         case OrNT:
            generate_and_or(rhs);
            break;
         case AndNT:
            generate_and_or(rhs);
            break;
         case NotNT:
            generate_not(rhs);
            break;
         case QuesNT:
            generate_Ques(rhs);
            break;
         case CallNT:
            generate_call(rhs);
            break;
         }
      }
   }
   toffset += 1;
   emitComment((char *)"TOFF inc:", toffset);
   emitRM((char *)"LD", 4, toffset, 1, (char *)"Pop left into ac1");
   switch (node->nodeType)
   {
   case OrNT:
      emitRO((char *)"OR", 3, 4, 3, (char *)"Op OR");
      break;
   case AndNT:
      emitRO((char *)"AND", 3, 4, 3, (char *)"Op AND");
      break;
   }
}

/* ==================================================
   GENERATE NOT
   ================================================== */
void generate_not(Node *node)
{
   if (load_in(node->child[0]) == false)
   {
      switch (node->child[0]->nodeType)
      {
      case OpNT:
         generate_op(node->child[0]);
         break;
      case CallNT:
         generate_call(node->child[0]);
         break;
      case NotNT:
         generate_not(node->child[0]);
         break;
      case ArrNT:
         load_arr_op(node->child[0]);
         break;
      }
   }
   emitRM((char *)"LDC", 4, 1, 6, (char *)"Load 1");
   emitRO((char *)"XOR", 3, 3, 4, (char *)"Op XOR to get logical not");
}

/* ==================================================
   GENERATE IF
   ================================================== */
void generate_if(Node *node)
{

   Node *A = node->child[0];
   Node *B = node->child[1];
   Node *C = node->child[2];

   if (C == NULL) // IF (A) THEN (B)
   {
      int rememberIf;
      emitComment((char *)"IF");

      switch (A->nodeType)
      {
      case ArrNT:
         load_arr_op(A);
         break;
      case OpNT:
         generate_op(A);
         break;
      default:
         gc_traverse_sibs(A);
         break;
      }
      rememberIf = emitSkip(1);
      emitComment((char *)"THEN");
      gc_traverse_sibs(B);
      if (B->nodeType == BreakNT)
      {
         if (embedded_loop != NULL)
         {
            find_embedded_stmts(B, embedded_loop);
         }
         else if (loop != NULL)
         {
            find_embedded_stmts(B, loop);
         }
      }
      int temp_emitLoc = emitWhereAmI();
      emitNewLoc(rememberIf);
      emitRM((char *)"JZR", 3, temp_emitLoc - rememberIf - 1, 7, (char *)"Jump around the THEN if false [backpatch]");
      emitNewLoc(temp_emitLoc);
      emitComment((char *)"END IF");
   }
   else // IF (A) THEN (B) ELSE (C)
   {
      int L1patch, L2patch;
      emitComment((char *)"IF");
      gc_traverse_sibs(A);
      L1patch = emitSkip(1);
      emitComment((char *)"THEN");
      gc_traverse_sibs(B);
      int temp_emitLoc = emitWhereAmI();
      emitNewLoc(L1patch);
      emitRM((char *)"JZR", 3, temp_emitLoc - L1patch, 7, (char *)"Jump around the THEN if false [backpatch]");
      emitNewLoc(temp_emitLoc);
      L2patch = emitSkip(1);
      emitComment((char *)"ELSE");
      // TEMP?
      if (C->nodeType == OpNT)
         emitComment((char *)"EXPRESSION");
      gc_traverse_sibs(C);
      temp_emitLoc = emitWhereAmI();
      emitNewLoc(L2patch);
      emitRM((char *)"JMP", 7, temp_emitLoc - L2patch - 1, 7, (char *)"Jump around the ELSE [backpatch]");
      emitNewLoc(temp_emitLoc);
      emitComment((char *)"END IF");
   }
}

/* ==================================================
   GENERATE WHILE
   ================================================== */
void generate_while(Node *node)
{
   loop = node;
   Node *A = node->child[0];
   Node *B = node->child[1];

   /*
      while (E) A

      L1:                                 rememberL1 = emit(0)
      t1 = E                              traverse(E)
      if_false t1 goto L2    <- backpatch rememberbp = emit(1)
      A                                   traverse(A)
      goto L1                             jumpbackto(rememberL1)
      L2:
   */
   int rememberL1 = emitSkip(0);
   emitComment((char *)"WHILE");
   gc_traverse_sibs(A);
   emitRM((char *)"JNZ", 3, 1, 7, (char *)"Jump to while part");
   int rememberbp = node->break_address = emitSkip(1);
   emitComment((char *)"DO");
   generate_while_compound(B, node);
   emitRM((char *)"JMP", 7, rememberL1 - emitWhereAmI() - 1, 7, (char *)"go to beginning of loop");
   int emitLoc = emitWhereAmI();
   emitNewLoc(rememberbp);
   emitRM((char *)"JMP", 7, emitLoc - rememberbp - 1, 7, (char *)"Jump past loop [backpatch]");
   emitNewLoc(emitLoc);
   emitComment((char *)"END WHILE");
   loop = NULL;
}

/* ==================================================
   GENERATE FOR
   ================================================== */
void generate_for(Node *node)
{
   loop = node;
   Node *cdn = node->child[0];
   Node *range = node->child[1];
   Node *stmt = node->child[2];

   int toffset_temp = toffset;
   toffset = node->size;
   emitComment((char *)"TOFF set:", toffset);
   emitComment((char *)"FOR");
   // 0. Insert cdm
   gcST.insert(cdn->literal, cdn);
   // 1. Load in Range
   if (load_in(range->child[0]) == false)
   {
      switch (range->child[0]->nodeType)
      {
      case OpNT:
         generate_op(range->child[0]);
         break;
      }
   }
   emitRM((char *)"ST", 3, toffset_temp, 1, (char *)"save starting value in index variable");
   if (load_in(range->child[1]) == false)
   {
      switch (range->child[1]->nodeType)
      {
      case OpNT:
         generate_op(range->child[1]);
         break;
      case SizeOfNT:
         generate_sizeof(range->child[1]);
         break;
      }
   }
   emitRM((char *)"ST", 3, toffset_temp - 1, 1, (char *)"save stop value");
   if (range->child[2] == NULL)
   {
      emitRM((char *)"LDC", 3, 1, 6, (char *)"default increment by 1");
   }
   else
   {
      if (load_in(range->child[2]) == false)
      {
         switch (range->child[2]->nodeType)
         {
         case SignNT:
            generate_ChSign(range->child[2]);
            break;
         case OpNT:
            generate_op(range->child[2]);
            break;
         }
      }
   }
   // toffset = toffset_temp;
   int L1 = emitWhereAmI();
   toffset = toffset_temp;
   emitRM((char *)"ST", 3, toffset - 2, 1, (char *)"save step value");
   emitRM((char *)"LD", 4, toffset, 1, (char *)"loop index");
   emitRM((char *)"LD", 5, toffset - 1, 1, (char *)"stop value");
   emitRM((char *)"LD", 3, toffset - 2, 1, (char *)"step value");
   emitRO((char *)"SLT", 3, 4, 5, (char *)"Op <");
   emitRM((char *)"JNZ", 3, 1, 7, (char *)"Jump to loop body");
   toffset = node->size;
   int backpatch = emitSkip(1);
   generate_for_compound(stmt, node);
   emitComment((char *)"Bottom of loop increment and jump");
   emitRM((char *)"LD", AC, toffset_temp, 1, (char *)"Load index");
   emitRM((char *)"LD", AC2, toffset_temp - 2, 1, (char *)"Load step");
   emitRO((char *)"ADD", 3, 3, 5, (char *)"increment");
   emitRM((char *)"ST", 3, toffset_temp, 1, (char *)"store back to index");
   emitRM((char *)"JMP", 7, L1 - emitWhereAmI(), 7, (char *)"go to beginning of loop");
   // printf("l1 %d\n", L1);
   // printf("toff %d\n", toffset);
   // printf("temp toff %d\n", toffset_temp);
   // printf("bp %d\n", backpatch);
   // printf("RN %d\n", emitWhereAmI());
   //  emitRM((char *)"JMP", 7, backpatch - emitWhereAmI() + toffset_temp - 1, 7, (char *)"go to beginning of loop");
   int emitLoc = emitWhereAmI();
   emitNewLoc(backpatch);
   emitRM((char *)"JMP", 7, emitLoc - backpatch - 1, 7, (char *)"Jump past loop [backpatch]");
   emitNewLoc(emitLoc);
   emitComment((char *)"END LOOP");
   toffset = toffset_temp;
   if (node->sibling != NULL)
   {
      if (node->sibling->nodeType != ToNT)
      {
         toffset -= 3;
      }
   }
   loop = NULL;
}

/* ==================================================
   GENERATE EMBEDDED FOR LOOP
   ================================================== */

/* ==================================================
   GENERATE RANGE
   ================================================== */
void generate_range(Node *node)
{
}

/* ==================================================
   GLOBAL CONSTANTS
   ================================================== */
void generate_constant(Node *node)
{
}

/* ==================================================
   GLOBAL CONSTANTS
   ================================================== */
void generate_id(Node *node)
{
}

/* ==================================================
   GENERATE CALL
   ================================================== */
void generate_call(Node *node)
{
   char msg[32];
   int toffset_temp = toffset;
   emitComment((char *)"CALL", node->literal);
   emitRM((char *)"ST", 1, toffset, 1, (char *)"Store fp in ghost frame for", node->literal);
   toffset -= 1;
   emitComment((char *)"TOFF dec:", toffset);
   toffset -= 1;
   emitComment((char *)"TOFF dec:", toffset);

   Node *parm = node->child[0];
   if (parm != NULL)
   {
      int n = 1;
      while (parm != NULL)
      {
         generate_passed_parms(parm, n);
         emitRM((char *)"ST", 3, toffset, 1, (char *)"Push parameter");
         toffset -= 1;
         emitComment((char *)"TOFF dec:", toffset);
         n++;
         parm = parm->sibling;
         if (parm == NULL)
         {
            emitComment((char *)"Param end", node->literal);
         }
      }
   }
   else
   {
      emitComment((char *)"Param end", node->literal);
   }
   toffset = toffset_temp;
   emitRM((char *)"LDA", 1, toffset, 1, (char *)"Ghost frame becomes new active frame");
   emitRM((char *)"LDA", 3, 1, 7, (char *)"Return address in ac");

   Node *call_sym = fetchSymbol(node, &gcST);
   emitRM((char *)"JMP", 7, call_sym->address - emitWhereAmI() - 1, 7, (char *)"CALL", node->literal);
   emitRM((char *)"LDA", 3, 0, 2, (char *)"Save the result in ac");
   emitComment((char *)"Call end", node->literal);
   emitComment((char *)"TOFF set:", toffset);
}

/* ==================================================
   GENERATE PARAMETERS
   ================================================== */
void generate_passed_parms(Node *node, int c)
{
   emitComment((char *)"Param", c);
   if (load_in(node) == false)
   {
      // Handle other nodetypes...
      switch (node->nodeType)
      {
      case AssignNT:
         generate_assignment(node);
         break;
      case OpNT:
         generate_op(node);
         break;
      case AndNT:
         generate_and_or(node);
         break;
      case OrNT:
         generate_and_or(node);
         break;
      case SignNT:
         generate_ChSign(node);
         break;
      case NotNT:
         generate_not(node);
         break;
      case QuesNT:
         generate_Ques(node);
         break;
      case ArrNT:
         load_arr_op(node);
         break;
      case CallNT:
         generate_call(node);
         break;
      case SizeOfNT:
         generate_sizeof(node);
         break;
      }
   }
   if (node->nodeType == StringConstNT)
   {
      emitStrLit(string_offset, node->data.String);
      string_offset -= node->size;
      emitRM((char *)"LDA", 3, node->location, node->refType, (char *)"Load address of char array");
   }
}

/* ==================================================
   GENERATE RETURN
   ================================================== */
void generate_return(Node *node)
{
   emitComment((char *)"RETURN");
   if (current_function->hasReturn)
   {
      if (load_in(node->child[0]) == false)
      {
         if (load_arr_op(node->child[0]) == false)
         {
            switch (node->child[0]->nodeType)
            {
            case OpNT:
               generate_op(node->child[0]);
               break;
            case ArrNT:
               load_arr_op(node->child[0]);
               break;
            case CallNT:
               generate_call(node->child[0]);
               break;
            case SizeOfNT:
               generate_sizeof(node->child[0]);
               break;
            case AndNT:
               generate_and_or(node->child[0]);
               break;
            }
            emitRM((char *)"LDA", 2, 0, 3, (char *)"Copy result to return register");
         }
         else
         {
            emitRM((char *)"LDA", 2, 0, 3, (char *)"Copy result to return register");
         }
      }
      else
      {
         emitRM((char *)"LDA", 2, 0, 3, (char *)"Copy result to return register");
      }
   }
   emitRM((char *)"LD", 3, -1, 1, (char *)"Load return address");
   emitRM((char *)"LD", 1, 0, 1, (char *)"Adjust fp");
   emitRM((char *)"JMP", 7, 0, 3, (char *)"Return");
}

/* ==================================================
   GENERATE QUESNT
   ================================================== */
void generate_Ques(Node *node)
{
   load_in(node->child[0]);
   emitRO((char *)"RND", 3, 3, 6, (char *)"Op ?");
}

/* ==================================================
   GENERATE CHSIGNNT
   ================================================== */
void generate_ChSign(Node *node)
{
   if (load_in(node->child[0]) == false)
   {
      switch (node->child[0]->nodeType)
      {
      case OpNT:
         generate_op(node->child[0]);
         break;
      case SignNT:
         generate_ChSign(node->child[0]);
         break;
      }
   }
   emitRO((char *)"NEG", 3, 3, 3, (char *)"Op unary -");
}
/* ==================================================
   GENERATE CHSIGNNT
   ================================================== */
void generate_sizeof(Node *node)
{
   Node *arr = fetchSymbol(node->child[0], &gcST);

   if (arr->nodeType == ParmArrNT)
   {
      emitRM((char *)"LD", AC, arr->location, arr->refType, (char *)"Load address of base of array", arr->literal);
   }
   else if (arr->nodeType == VarArrNT)
   {
      if (arr->dataType == CharDT)
      {
         if (arr->refType == GlobalRT && arr->nodeType == ParmArrNT)
         {
            emitRM((char *)"LDA", AC, arr->location, arr->refType, (char *)"Load address of base of array", arr->literal);
         }
         else
         {
            emitRM((char *)"LD", AC, arr->location, arr->refType, (char *)"Load address of base of array", arr->literal);
         }
      }
      else
      {
         emitRM((char *)"LDA", AC, arr->location, arr->refType, (char *)"Load address of base of array", arr->literal);
      }
   }
   else
   {
      emitRM((char *)"LDA", AC, arr->location, arr->refType, (char *)"Load address of base of array", arr->literal);
   }
   emitRM((char *)"LD", 3, 1, 3, (char *)"Load array size");
}

void generate_string(Node *node)
{
   if (node->nodeType == StringConstNT && curr_decl != NULL)
   {
      emitStrLit(string_offset, node->data.String);
      string_offset -= node->size;
      emitRM((char *)"LDA", 3, node->location, node->refType, (char *)"Load address of char array");
      emitRM((char *)"LDA", 4, curr_decl->location, curr_decl->refType, (char *)"address of lhs");
      emitRM((char *)"LD", 5, 1, 3, (char *)"size of rhs");
      emitRM((char *)"LD", 6, 1, 4, (char *)"size of lhs");
      emitRO((char *)"SWP", 5, 6, 6, (char *)"pick smallest size");
      emitRO((char *)"MOV", 4, 3, 5, (char *)"array op =");
   }
}