// UCLA CS 111 Lab 1 command reading

#include "command.h"
#include "command-internals.h"
#include "alloc.h"

#include <error.h>
#include <stdio.h>
#include <string.h>
/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

/* FIXME: Define the type 'struct command_stream' here.  This should
   complete the incomplete type declaration in command.h.  */

#define QUEUESIZE       1000

struct command_stream{
          command_t q[QUEUESIZE+1];// body of queue 
          int first ;                       //position of first element
          int last ;                       // position of last element 
          int count ;                      // number of queue elements 
};
void init_queue(command_stream_t q)
{
          q->first = 0;
          q->last = QUEUESIZE-1;
          q->count = 0;
}
void enqueue(command_stream_t q, command_t x)
{
          if (q->count >= QUEUESIZE)
            printf("Warning: queue overflow enqueue \n");
          else {
            q->last = (q->last+1) % QUEUESIZE;
            q->q[ q->last ] = x;    
            q->count = q->count + 1;
          }
}

command_t dequeue(command_stream_t q)
{
          command_t x;
        if (q->count <= 0) printf("Warning: empty queue dequeue.\n");
        else {
          x = q->q[ q->first ];
          q->first = (q->first+1) % QUEUESIZE;
          q->count = q->count - 1;
        }
        return(x);
}

int empty(command_stream_t q)
{
          if (q->count <= 0) return (1);
          else return (0);
}

/*void print_queue(command_t *q)
{
          int i,j;
          i=q->first; 
          while (i != q->last) {                                     
            printf("%c ",q->q[i]);
            i = (i+1) % QUEUESIZE;                             
          }           
          printf("%2d ",q->q[i]);          
          printf("\n");
}
*/
command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  //error (1, 0, "command reading not yet implemented");
  command_stream_t q = checked_malloc(sizeof(struct command_stream));//must allocate memory
  init_queue(q);

  command_t simpleCommand = checked_malloc(sizeof(struct command));
  simpleCommand->type=SIMPLE_COMMAND;
  char** word = checked_malloc(sizeof(char*)*100);
  word[0] = checked_malloc((strlen("test")+1)); //use index instead of 0
  strcpy(word[0],"test");
  simpleCommand->u.word=word;
 
 enqueue(q,simpleCommand);

   return q; 
  //return 0;
}

command_t
read_command_stream (command_stream_t s)
{
  /* FIXME: Replace this with your implementation too.  */
 if(!empty(s))  {
   return dequeue(s);
 }
  else
  {
    return NULL;
  }
    //return temp->c;
  //error (1, 0, "command reading not yet implemented");
  //return 0;
}
