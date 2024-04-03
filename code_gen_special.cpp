#include "code_gen.hpp"
extern SymbolTable gcST;
extern int toffset;
extern Node *loop;
extern Node *embedded_loop;

void fix_memory_loops(Node *AST)
{
   traverse_fix(AST);
}

void traverse_fix(Node *node)
{
   if (node == NULL)
   {
      return;
   }

   if (node->nodeType == ToNT && node->is_embedded == false)
   {
      node->size = -2;
      Node *decl = node->child[0];
      while (decl != NULL)
      {
         if (decl->nodeType == VarNT)
         {
            node->size += decl->location;
            node->size -= decl->size;
         }
         decl = decl->sibling;
      }
      Node *stmt = node->child[2];
      if (stmt->nodeType == CompoundNT)
      {
         for (int i = 0; i < MAXCHILDREN; i++)
         {
            if (stmt->child[i] != NULL)
            {
               if (stmt->child[i]->nodeType == ToNT)
               {
                  stmt->child[i]->address = node->size;
                  stmt->child[i]->is_embedded = true;
               }
               if (stmt->child[i]->nodeType == VarNT)
               {
                  Node *decl = stmt->child[i];
                  while (decl != NULL)
                  {
                     if (decl->nodeType == VarNT)
                     {
                        stmt->size -= decl->size;
                     }
                     decl = decl->sibling;
                  }
               }
            }
         }
         if (stmt->size != node->size)
         {
            if (stmt->child[0] != NULL)
            {
               if (stmt->child[0]->nodeType != VarNT)
               {
                  stmt->size = node->size;
               }
            }
            else
            {
               stmt->size = node->size;
            }
         }
      }
   }
   else if (node->nodeType == ToNT && node->is_embedded == true)
   {
      node->size = node->address - 2;
      Node *decl = node->child[0];
      while (decl != NULL)
      {
         if (decl->nodeType == VarNT)
         {
            node->size -= decl->size;
         }
         decl = decl->sibling;
      }
      Node *stmt = node->child[2];
      if (stmt->nodeType == CompoundNT)
      {
         stmt->size = node->size;
      }
      node->is_embedded = false;
   }
   else if (node->nodeType == IterNT)
   {
      Node *stmt = node->child[1];
      if (stmt != NULL)
      {
         if (stmt->nodeType == CompoundNT)
         {
            Node *decl = stmt->child[0];
            while (decl != NULL)
            {
               if (decl->nodeType == VarNT)
               {
                  decl->location += 1;
               }
               decl = decl->sibling;
            }
         }
      }
   }

   for (int i = 0; i < MAXCHILDREN; i++)
   {
      traverse_fix(node->child[i]);
   }
   traverse_fix(node->sibling);
}

void generate_for_compound(Node *node, Node *parent)
{
   if (node->nodeType == CompoundNT)
   {
      toffset = node->size;
      emitComment((char *)"COMPOUND");
      gc_traverse_sibs(node->child[0]);
      find_embedded_stmts(node->child[0], parent);
      emitComment((char *)"TOFF set:", toffset);
      emitComment((char *)"Compound Body");
      find_embedded_stmts_wrapper(node, parent);
      gc_traverse_children(node);
      emitComment((char *)"TOFF set:", toffset);
      emitComment((char *)"END COMPOUND");
   }
   else
   {
      emitComment((char *)"EXPRESSION");
      switch (node->nodeType)
      {
      case CallNT:
         generate_call(node);
         break;
      case OpNT:
         generate_op(node);
         break;
      case AssignNT:
         generate_assignment(node);
         break;
      }
   }
}

void generate_embedded_for(Node *node)
{
   embedded_loop = node;
   Node *cdn = node->child[0];
   Node *range = node->child[1];
   Node *stmt = node->child[2];

   int toffset_temp = toffset;

   emitComment((char *)"TOFF set:", toffset - 3);
   // EMBEDDED FOR!
   emitComment((char *)"FOR");
   // 0. Insert cdm
   cdn->location -= 2;
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
   emitRM((char *)"ST", 3, toffset, 1, (char *)"save starting value in index variable");
   if (load_in(range->child[1]) == false)
   {
      switch (range->child[1]->nodeType)
      {
      case OpNT:
         generate_op(range->child[1]);
         break;
      }
   }
   emitRM((char *)"ST", 3, toffset - 1, 1, (char *)"save stop value");
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
   emitRM((char *)"ST", 3, toffset - 2, 1, (char *)"save step value");
   emitRM((char *)"LD", 4, toffset, 1, (char *)"loop index");
   emitRM((char *)"LD", 5, toffset - 1, 1, (char *)"stop value");
   emitRM((char *)"LD", 3, toffset - 2, 1, (char *)"step value");
   emitRO((char *)"SLT", 3, 4, 5, (char *)"Op <");
   emitRM((char *)"JNZ", 3, 1, 7, (char *)"Jump to loop body");
   int backpatch = emitSkip(1);
   generate_for_compound(stmt, node);
   emitComment((char *)"Bottom of loop increment and jump");
   emitRM((char *)"LD", AC, toffset_temp, 1, (char *)"Load index");
   emitRM((char *)"LD", AC2, toffset_temp - 2, 1, (char *)"Load step");
   emitRO((char *)"ADD", 3, 3, 5, (char *)"increment");
   emitRM((char *)"ST", 3, toffset_temp, 1, (char *)"store back to index");
   emitRM((char *)"JMP", 7, backpatch - emitWhereAmI() + toffset_temp - 1, 7, (char *)"go to beginning of loop");
   int emitLoc = emitWhereAmI();
   emitNewLoc(backpatch);
   emitRM((char *)"JMP", 7, emitLoc - backpatch - 1, 7, (char *)"Jump past loop [backpatch]");
   emitNewLoc(emitLoc);
   emitComment((char *)"END LOOP");
   toffset = toffset_temp;
   embedded_loop = NULL;
}

void generate_while_compound(Node *node, Node *parent)
{
   if (node != NULL)
   {
      if (node->nodeType == CompoundNT)
      {
         gcST.enter("Compound");
         int toffset_temp = toffset;
         toffset = node->size;
         emitComment((char *)"COMPOUND");
         emitComment((char *)"TOFF set:", toffset);
         gc_traverse_sibs(node->child[0]); // Declarations
         emitComment((char *)"Compound Body");
         find_embedded_stmts_wrapper(node, parent);
         gc_traverse_children(node);
         toffset = toffset_temp;
         emitComment((char *)"TOFF set:", toffset);
         emitComment((char *)"END COMPOUND");
         gcST.leave();
      }
      else
      {
         gc_traverse_sibs(node);
      }
   }
   else
   {
      gc_traverse_sibs(node);
   }
}

void generate_embedded_while(Node *node)
{
   embedded_loop = node;
   Node *A = node->child[0];
   Node *B = node->child[1];

   int temp_offset = toffset;
   int rememberL1 = emitSkip(0);
   emitComment((char *)"WHILE");
   gc_traverse_sibs(A);
   emitRM((char *)"JNZ", 3, 1, 7, (char *)"Jump to while part");
   int rememberbp = emitSkip(1);
   node->break_address = rememberbp;
   emitComment((char *)"DO");
   generate_while_compound(B, node);
   emitRM((char *)"JMP", 7, rememberL1 - emitWhereAmI() - 1, 7, (char *)"go to beginning of loop");
   int emitLoc = emitWhereAmI();
   emitNewLoc(rememberbp);
   emitRM((char *)"JMP", 7, emitLoc - rememberbp - 1, 7, (char *)"Jump past loop [backpatch]");
   emitNewLoc(emitLoc);
   emitComment((char *)"END WHILE");
   toffset = temp_offset;
   embedded_loop = NULL;
}

void generate_break(Node *node, Node *loop)
{
   emitComment((char *)"BREAK");
   if (loop != NULL)
   {
      emitRM((char *)"JMP", 7, loop->break_address - emitWhereAmI() - 1, 7, (char *)"break");
   }
   else
   {
      printf("no break address for the loop\n");
   }
}