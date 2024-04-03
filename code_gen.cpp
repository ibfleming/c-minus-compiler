#include "code_gen.hpp"
#include <string.h>

#define DEBUG false

extern FILE *code;
extern int goffset;
extern map<int, Node *> g_decl;
int i = 100;
Node *curr_decl = NULL;

bool in_loop = false;
Node *loop = NULL;
Node *embedded_loop = NULL;

SymbolTable gcST;
int toffset;
int address_of_main;

/* ==================================================
   GENERATE CODE
   ================================================== */
void generate_code(Node *AST, Node *rAST)
{
   toffset = 0;
   generate_IO(rAST);
   gc_traverse_sibs(AST);
   generate_init_section();
   fflush(code);
}

/* ==================================================
   TRAVERSE SIBLINGS
   ================================================== */
void gc_traverse_sibs(Node *node)
{
   if (node == NULL)
      return;
   if (node->isLib)
   {
      gc_traverse_sibs(node->sibling);
      return;
   }
   else
   {
      // Declarations
      if (node->is_decl && !node->is_loaded)
      {
         node->is_loaded = true;
         if (node->nodeType == StaticNT)
            node->refType = GlobalRT;
         if (node->nodeType == ParmArrNT || node->nodeType == ParmNT)
            node->refType = LocalRT;

         if (node->refType == GlobalRT)
         {
            if (!gcST.insertGlobal(node->literal, node))
            {
               if (node->isStatic)
               {
                  string str = "-ST";
                  string sym_name = (string)node->literal + str;
                  gcST.insert(sym_name, node);
                  g_decl[i] = node;
                  i++;
               }
            }
         }
         else
         {
            gcST.insert(node->literal, node);
         }
         if (node->isArray && node->refType != GlobalRT)
         {
            if (node->nodeType != ParmArrNT)
            {
               emitRM((char *)"LDC", 3, node->size - 1, 6, (char *)"load size of array", node->literal);
               emitRM((char *)"ST", 3, node->location + 1, 1, (char *)"save size of array", node->literal);
            }
         }
         if (node->child[0] != NULL && node->refType == LocalRT) // HAS AN INITIALIZER
         {
            curr_decl = node;
            if (load_in(node->child[0]) == false)
            {
               switch (node->child[0]->nodeType)
               {
               case OpNT:
                  generate_op(node->child[0]);
                  break;
               case AndNT:
                  generate_and_or(node->child[0]);
                  break;
               case OrNT:
                  generate_and_or(node->child[0]);
                  break;
               case SignNT:
                  generate_ChSign(node->child[0]);
                  break;
               case NotNT:
                  generate_not(node->child[0]);
                  break;
               default:
                  break;
               }
            }
            if (node->child[0]->nodeType != StringConstNT)
            {
               store_var(node);
            }
         }
      }
      generate_code_by_type(node);
      gc_traverse_sibs(node->sibling);
   }
}

/* ==================================================
   TRAVERSE CHILDREN
   ================================================== */
void gc_traverse_children(Node *node)
{
   for (int idx = 0; idx < MAXCHILDREN; idx++)
   {
      gc_traverse_sibs(node->child[idx]);
   }
}

/* ==================================================
   FIND EMBEDDED STATEMENTS
   ================================================== */
void find_embedded_stmts(Node *node, Node *parent)
{
   if (node == NULL)
      return;
   else
   {
      if (node->nodeType == ToNT)
      {
         node->is_embedded = true;
         return;
      }
      if (node->nodeType == IterNT)
      {
         node->is_embedded = true;
         return;
      }
      if (node->nodeType == VarNT)
      {
         node->location -= 1;
      }
      find_embedded_stmts(node->sibling, parent);
   }
}

/* ==================================================
   FIND EMBEDDED STATEMENTS WRAPPER
   ================================================== */
void find_embedded_stmts_wrapper(Node *node, Node *parent)
{
   for (int i = 0; i < MAXCHILDREN; i++)
   {
      if (node->child[i] != NULL)
      {
         find_embedded_stmts(node->child[i], parent);
      }
   }
}

/* ==================================================
   FENERATE CODE BY NODE TYPE
   ================================================== */
void generate_code_by_type(Node *node)
{
   // gcST.print(printData);
   switch (node->nodeType)
   {
   case FuncNT:
      generate_function(node);
      break;
   case CompoundNT:
      if (node->is_function_compound)
      {
         generate_function_compound(node);
         if (DEBUG)
            gcST.print(printData);
      }
      else
      {
         // Embedded compounds here!
         generate_compound(node);
      }
      break;
   case OpNT:
      generate_op(node);
      break;
   case AssignNT:
      emitComment((char *)"EXPRESSION");
      generate_assignment(node);
      break;
   case AndNT:
      generate_and_or(node);
      break;
   case OrNT:
      generate_and_or(node);
      break;
   case NotNT:
      generate_not(node);
      // Made the function.
      break;
   case IfNT:
      generate_if(node);
      break;
   case IterNT:
      if (node->is_embedded)
      {
         generate_embedded_while(node);
      }
      else
      {
         generate_while(node);
      }
      break;
   case ToNT:
      if (node->is_embedded)
      {
         generate_embedded_for(node);
      }
      else
      {
         gcST.enter("For");
         generate_for(node);
         gcST.leave();
      }
      break;
   case IdNT:
      load_in(node);
      break;
   case CallNT:
      emitComment((char *)"EXPRESSION");
      generate_call(node);
      break;
   case ReturnNT:
      generate_return(node);
      break;
   case QuesNT:
      // Made the function.
      break;
   case SignNT:
      // Made the function.
      break;
   case BreakNT:
      if (embedded_loop != NULL)
      {
         generate_break(node, embedded_loop);
      }
      else if (loop != NULL)
      {
         generate_break(node, loop);
      }

      break;
   case SizeOfNT:
      emitComment((char *)"EXPRESSION");
      generate_sizeof(node);
      break;
   case ArrNT:
      emitComment((char *)"EXPRESSION");
      load_arr_op(node);
      break;
   default:
      break;
   }
   if (node->isConst)
   {
      node->is_loaded = true;
      load_in(node);
   }
}

bool load_in(Node *node)
{
   if (node->isConst)
   {
      int emitLoc = emitWhereAmI();
      switch (node->tknClass)
      {
      case NUMCONST:
         emitRM((char *)"LDC", 3, node->data.Int, 6, (char *)"Load integer constant");
         break;
      case CHARCONST:
         emitRM((char *)"LDC", 3, node->data.Char, 6, (char *)"Load char constant");
         break;
      case BOOLCONST:
         emitRM((char *)"LDC", 3, node->data.Int, 6, (char *)"Load Boolean constant");
         break;
      case STRINGCONST:
         generate_string(node);
         break;
      default:
         printf("load_in() :: isConst ERROR!\n");
         break;
      }
      return true;
   }
   else if (node->nodeType == IdNT)
   {
      if (node->isArray)
      {

         Node *sym = fetchSymbol(node, &gcST);
         if (sym->nodeType == ParmArrNT)
         {
            emitRM((char *)"LD", 3, sym->location, sym->refType, (char *)"Load address of base of array", sym->literal);
         }
         else
         {
            emitRM((char *)"LDA", 3, sym->location, sym->refType, (char *)"Load address of base of array", sym->literal);
         }
      }
      else
      {
         if (node->isStatic)
         {
            char *tmp_ext = (char *)"-ST";
            char *tmp_name = strdup(node->literal);
            strcat(tmp_name, tmp_ext);
            Node *test = createNode(NULL, StaticNT);
            test->literal = tmp_name;
            if (fetchSymbol(test, &gcST) == NULL)
            {
               Node *sym = fetchSymbol(node, &gcST);
               emitRM((char *)"LD", 3, sym->location, sym->refType, (char *)"Load variable", sym->literal);
            }
            else
            {
               Node *sym = fetchSymbol(test, &gcST);
               emitRM((char *)"LD", 3, sym->location, sym->refType, (char *)"Load variable", sym->literal);
            }
         }
         else
         {
            Node *sym = fetchSymbol(node, &gcST);
            emitRM((char *)"LD", 3, sym->location, sym->refType, (char *)"Load variable", sym->literal);
         }
      }

      return true;
   }
   return false;
}

/* ==================================================
   STORE VARIABLE
   ================================================== */
bool store_var(Node *node)
{
   if (node->nodeType == IdNT)
   {
      Node *sym = fetchSymbol(node, &gcST);
      if (sym != NULL)
      {
         emitRM((char *)"ST", 3, sym->location, sym->refType, (char *)"Store variable", sym->literal);
      }
      return true;
   }
   else if (node->is_decl)
   {
      emitRM((char *)"ST", 3, node->location, node->refType, (char *)"Store variable", node->literal);
   }
   return false;
}

/* ==================================================
   LOAD ARRAY IN ASSIGMENT
   ================================================== */
bool load_arr_assign(Node *array, Node *rhs)
{
   if (array->nodeType == ArrNT)
   {
      // 1. Push the index of the array
      Node *idx = array->child[1];
      if (load_in(idx) == false)
      {
         switch (idx->nodeType)
         {
         case OpNT:
            generate_op(idx);
            break;
         case AssignNT:
            generate_assignment(idx);
            break;
            ;
         default:
            break;
         }
         emitRM((char *)"ST", 3, toffset, 1, (char *)"Push index");
      }
      else
      {
         emitRM((char *)"ST", 3, toffset, 1, (char *)"Push index");
      }
      toffset -= 1;
      emitComment((char *)"TOFF dec:", toffset);
      // 2. Check RHS
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
         case ArrNT:
            load_arr_op(rhs);
            break;
         case CallNT:
            generate_call(rhs);
            break;
         case SignNT:
            generate_ChSign(rhs);
            break;
         }
      }
      toffset += 1;
      emitComment((char *)"TOFF inc:", toffset);
      // 3. Pop the array
      Node *sym = fetchSymbol(array->child[0], &gcST);
      emitRM((char *)"LD", 4, toffset, 1, (char *)"Pop index");
      if (sym->nodeType == ParmArrNT)
      {
         emitRM((char *)"LD", AC2, sym->location, sym->refType, (char *)"Load address of base of array", sym->literal);
      }
      else
      {
         emitRM((char *)"LDA", AC2, sym->location, sym->refType, (char *)"Load address of base of array", sym->literal);
      }
      emitRO((char *)"SUB", 5, 5, 4, (char *)"Compute offset of value");
      if (array->nodeType != ArrNT)
         emitRM((char *)"ST", 3, 0, 5, (char *)"Store variable", sym->literal);
      return true;
   }
   return false;
}

/* ==================================================
   LOAD ARRAY IN OPERATOR
   ================================================== */
bool load_arr_op(Node *array)
{
   if (array->nodeType == ArrNT)
   {
      Node *sym = fetchSymbol(array->child[0], &gcST);
      Node *idx = array->child[1];
      if (sym->nodeType == ParmArrNT)
      {
         emitRM((char *)"LD", AC, sym->location, sym->refType, (char *)"Load address of base of array", sym->literal);
      }
      else
      {
         emitRM((char *)"LDA", AC, sym->location, sym->refType, (char *)"Load address of base of array", sym->literal);
      }
      emitRM((char *)"ST", 3, toffset, 1, (char *)"Push left side");
      toffset -= 1;
      emitComment((char *)"TOFF dec:", toffset);
      if (load_in(idx) == false)
      {
         if (load_arr_op(idx) == false)
         {
            switch (idx->nodeType)
            {
            case OpNT:
               generate_op(idx);
               break;
            case CallNT:
               generate_call(idx);
               break;
            case SignNT:
               generate_ChSign(idx);
               break;
            }
         }
      }
      toffset += 1;
      emitComment((char *)"TOFF inc:", toffset);
      emitRM((char *)"LD", 4, toffset, 1, (char *)"Pop left into ac1");
      emitRO((char *)"SUB", 3, 4, 3, (char *)"compute location from index");
      emitRM((char *)"LD", 3, 0, 3, (char *)"Load array element");
      return true;
   }
   return false;
}

/* ==================================================
   GENERATE INIT AND GLOBALS
   ================================================== */
void generate_init_section()
{
   int emitLoc = emitWhereAmI();
   emitNewLoc(0);
   emitRM((char *)"JMP", 7, emitLoc - 1, 7, (char *)"Jump to init [backpatch]");
   emitNewLoc(emitLoc);
   emitComment((char *)"INIT");
   emitRM((char *)"LDA", 1, goffset, 0, (char *)"set first frame at end of globals");
   emitRM((char *)"ST", 1, 0, 1, (char *)"store old fp (point to self)");
   emitComment((char *)"INIT GLOBALS AND STATICS");
   map<int, Node *> globals = gcST.returnGlobalDecls();
   for (auto const &pair : globals)
   {
      char msg[32];
      if (pair.second->nodeType == VarArrNT)
      {
         emitRM((char *)"LDC", 3, pair.second->size - 1, 6, (char *)"load size of array", pair.second->literal);
         emitRM((char *)"ST", 3, pair.second->location + 1, 0, (char *)"save size of array", pair.second->literal);
      }
      else if (pair.second->isArray && pair.second->nodeType == StaticNT)
      {
         emitRM((char *)"LDC", 3, pair.second->size - 1, 6, (char *)"load size of array", pair.second->literal);
         emitRM((char *)"ST", 3, pair.second->location + 1, 0, (char *)"save size of array", pair.second->literal);
      }
      else if (pair.second->nodeType == VarNT || (!pair.second->isArray && pair.second->nodeType == StaticNT))
      {
         if (pair.second->child[0] != NULL) // Has an INITIALIZER!
         {
            if (load_in(pair.second->child[0]) == false)
            {
               switch (pair.second->child[0]->nodeType)
               {
               case OpNT:
                  toffset = pair.second->location - 2;
                  generate_op(pair.second->child[0]);
                  break;
               case AndNT:
                  generate_and_or(pair.second->child[0]);
                  break;
               case OrNT:
                  generate_and_or(pair.second->child[0]);
                  break;
               case SignNT:
                  generate_ChSign(pair.second->child[0]);
                  break;
               case NotNT:
                  generate_not(pair.second->child[0]);
                  break;
               default:
                  break;
               }
            }
            store_var(pair.second);
         }
      }
   }
   // function todo this init globals and statics stuff
   emitComment((char *)"END INIT GLOBALS AND STATICS");
   emitRM((char *)"LDA", 3, 1, 7, (char *)"Return address in ac");
   emitRM((char *)"JMP", 7, address_of_main - (emitWhereAmI() + 1), 7, (char *)"Jump to main");
   emitRO((char *)"HALT", 0, 0, 0, (char *)"DONE!");
   emitComment((char *)"END INIT");
}

/* ==================================================
   GENERATE IO LIBRARIES/ROUTINES
   ================================================== */
// REPLACE FPRINTFs WITH EMITCOMMENTS
void generate_IO(Node *rAST)
{
   emitSkip(1);
   char msg[32];
   for (Node *itr = rAST; itr != NULL; itr = itr->sibling)
   {
      if (itr->nodeType == FuncNT)
      {
         gcST.insert(itr->literal, itr);
         itr->address = emitWhereAmI();
         fprintf(code, "* ** ** ** ** ** ** ** ** ** ** ** **\n");
         fprintf(code, "* FUNCTION %s\n", itr->literal);
         emitRM((char *)"ST", 3, -1, 1, (char *)"Store return address");
         if (itr->dataType != VoidDT)
         {
            sprintf(msg, "Grab %s input", convertDTStr(itr));
            if ((string)itr->literal == (string) "input")
            {
               emitRO((char *)"IN", 2, 2, 2, msg);
            }
            else if ((string)itr->literal == (string) "inputb")
            {
               emitRO((char *)"INB", 2, 2, 2, msg);
            }
            else if ((string)itr->literal == (string) "inputc")
            {
               emitRO((char *)"INC", 2, 2, 2, msg);
            }
         }
         for (int idx = 0; idx < MAXCHILDREN; idx++)
         {
            if (itr->child[idx] != NULL)
            {
               if (itr->child[idx]->nodeType == ParmNT)
               {
                  emitRM((char *)"LD", 3, -2, 1, (char *)"Load parameter");
                  if (itr->child[idx]->dataType == IntDT)
                  {
                     sprintf(msg, "Output integer");
                  }
                  else
                  {
                     sprintf(msg, "Output %s", convertDTStr(itr->child[idx]));
                  }
               }
            }
         }
         if ((string)itr->literal == (string) "output")
         {
            emitRO((char *)"OUT", 3, 3, 3, msg);
         }
         else if ((string)itr->literal == (string) "outputb")
         {
            emitRO((char *)"OUTB", 3, 3, 3, msg);
         }
         else if ((string)itr->literal == (string) "outputc")
         {
            emitRO((char *)"OUTC", 3, 3, 3, msg);
         }
         else if ((string)itr->literal == (string) "outnl")
         {
            emitRO((char *)"OUTNL", 3, 3, 3, (char *)"Output a newline");
         }
         emitRM((char *)"LD", 3, -1, 1, (char *)"Load return address");
         emitRM((char *)"LD", 1, 0, 1, (char *)"Adjust fp");
         emitRM((char *)"JMP", 7, 0, 3, (char *)"Return");
         fprintf(code, "* END FUNCTION %s\n", itr->literal);
         fprintf(code, "* \n");
      }
   }
}
