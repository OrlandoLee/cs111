// UCLA CS 111 Lab 1 command reading


#include "alloc.h"
#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
////////////////////////////
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
////////////////////////////
command_t make_command(char* buffer, enum command_type type);
command_t make_subshell_command(char* buffer);
int line_count;
int (*get_byte) (void *);
void *get_byte_argument;

void
syntax_error()
{
  error(1, 0, "Syntax Error: the Line is %d", line_count);
}

void 
eleminateEmptySpace()
{
  if(!feof(get_byte_argument))
  {
    char c = get_byte(get_byte_argument);
    while(strchr("\t\n ", c))
    {
      if(c == '\n')
        line_count++;
      c = get_byte(get_byte_argument);
    }
    ungetc(c, get_byte_argument);
  }
}

enum command_type
scan(char *buffer)
{
  while(!feof(get_byte_argument))
  { 
    char c = get_byte(get_byte_argument);
    char next;
    switch(c)
    {
      case '#':
        next = get_byte(get_byte_argument);
      	while(next != '\n')
      	{
            if(next == EOF)
              return SIMPLE_COMMAND;
        	  next = get_byte(get_byte_argument);
      	}
        return scan(buffer);
      case '&':
       next = get_byte(get_byte_argument);
        if(next == '&')
        {
          eleminateEmptySpace();
          return AND_COMMAND;
        }
        else if(next == EOF)
          syntax_error();
        else 
          ungetc(next, get_byte_argument);
        break;
      case '|':
       next = get_byte(get_byte_argument);
        if(next == '|')
        {
          eleminateEmptySpace();
          return OR_COMMAND;
        }
        else if( isalnum(next) || strchr("!%+,-./:@^_\n\t ", next))
        {
          ungetc(next, get_byte_argument);
          eleminateEmptySpace();
          return PIPE_COMMAND;
        }
        else if(next == EOF)
          syntax_error();
       case '(':
      {
        eleminateEmptySpace();
        return SUBSHELL_COMMAND;
      }
      case ')':
      {
       ungetc(c, get_byte_argument);
        return SIMPLE_COMMAND;
      }
      case '\n': line_count++;
      case ';':
      case EOF:
        return SIMPLE_COMMAND;
    }
    buffer[strlen(buffer)] = c;
  }
  return SEQUENCE_COMMAND;
}

command_t
make_simple_command(char *buffer)
{
  if(!strlen(buffer))
    syntax_error();
  
  command_t command = checked_malloc(sizeof(struct command));
  command->type = SIMPLE_COMMAND; command->status = -1;
  command->input = NULL; command->output = NULL;
  command->u.word = checked_malloc(8*sizeof(char*)); size_t word_size = 8;
  size_t input_size = 8;
  size_t output_size = 8;      
  size_t cur_word_size;
  size_t index = 0; 
  bool word_flag = false;
  bool input_flag = false; 
  bool output_flag = false;
  bool input = false; 
  bool output = false; 
  int i;
  for(i = 0; buffer[i]; i++)
  {
    if(buffer[i] == '<')
    {
      if(i == 0 || input || output 
        || input_flag || output_flag)
        syntax_error();
      command->input = checked_malloc(8*sizeof(char));
      input_flag = true;
    }
    else if(buffer[i] == '>')
    {
      if(i == 0 || output || output_flag)
        syntax_error();
      command->output = checked_malloc(8*sizeof(char)); 
      input_flag = false;
      output_flag = true;
    }
    else if(isalnum(buffer[i]) || strchr("!%+,-./:@^_", buffer[i]))
    {
      if(input_flag)
      {
        input = true;
        char* string = command->input;
        if(strlen(string) >= input_size)
          checked_grow_alloc(string, &input_size);
        string[strlen(string)] = buffer[i];
      }
      else if(output_flag)
      {
        output = true;
        char* string = command->output;
        if(strlen(string) >= output_size)
          checked_grow_alloc(string, &output_size);
        string[strlen(string)] = buffer[i];
      }
      else if(!word_flag)
      {
        if((input || output) && !input_flag && !output_flag)
          syntax_error();
        if(index >= word_size)
          checked_grow_alloc(command->u.word, &word_size);
        command->u.word[index] = checked_malloc(8*sizeof(char));
        cur_word_size = 8;
        command->u.word[index][0] = buffer[i];
        word_flag = true;
      }
      else if(word_flag)
      {
        char *string = command->u.word[index];
        if(strlen(string) >= cur_word_size)
          checked_grow_alloc(string, &cur_word_size);
        string[strlen(string)] = buffer[i];
      }
    }
    else if(strchr(" \t", buffer[i]))
    {
      if(word_flag)
      {
        word_flag = false;
        index++;
      }
      else if(input && input_flag)
        input_flag = false;
      else if(output && output_flag)
        output_flag = false; 
    }
      else if(buffer[i] == EOF)
    {
      if(index >= word_size)
        checked_grow_alloc(command->u.word, &word_size);
      return command;
    }
      else
      syntax_error();
  }
  memset((void *) buffer, '\0', 1024);
  if(index >= word_size)
        checked_grow_alloc(command->u.word, &word_size);
  return command;
}

command_t
make_complete_command(char *buffer, enum command_type type, command_t parent_Command)
{
  command_t complete_command = checked_malloc(sizeof(struct command));
  complete_command->type = type; 
  complete_command->status = -1;
  if(parent_Command == NULL)
    complete_command->u.command[0] = make_simple_command(buffer);
  else if(type == PIPE_COMMAND && parent_Command->type != PIPE_COMMAND)
     complete_command->u.command[0] = parent_Command->u.command[1];
  else if(parent_Command->type == SUBSHELL_COMMAND || (type != PIPE_COMMAND && parent_Command->type == PIPE_COMMAND) || (type == PIPE_COMMAND) == (parent_Command->type == PIPE_COMMAND))
    complete_command->u.command[0] = parent_Command;
  
  enum command_type adjacent_type = scan(buffer);
  if(adjacent_type == SIMPLE_COMMAND || adjacent_type == SEQUENCE_COMMAND)
  {
    complete_command->u.command[1] = make_simple_command(buffer);
    return complete_command;
  }
  else if(adjacent_type == SUBSHELL_COMMAND)
  {
    command_t subshell = make_subshell_command(buffer);
    adjacent_type = scan(buffer);
    if(adjacent_type == SIMPLE_COMMAND)
    {
      complete_command->u.command[1] = subshell;
      return complete_command;
    }
    else if(type != PIPE_COMMAND && adjacent_type == PIPE_COMMAND)
    {
      complete_command->u.command[1] = subshell;
      complete_command->u.command[1] = make_complete_command(buffer, adjacent_type, complete_command);
      return complete_command;
    }
    else
    {
      complete_command->u.command[1] = subshell;
      command_t adjacent_command = make_complete_command(buffer, adjacent_type, complete_command);
      return adjacent_command;
    }
  }
  else if(type != PIPE_COMMAND && adjacent_type == PIPE_COMMAND)
  {
    complete_command->u.command[1] = make_simple_command(buffer);
    complete_command->u.command[1] = make_complete_command(buffer, adjacent_type, complete_command);
    return complete_command;
  }
  else
  {
    complete_command->u.command[1] = make_simple_command(buffer);
    command_t adjacent_command = make_complete_command(buffer, adjacent_type, complete_command);
    return adjacent_command;
  }
}

command_t
make_subshell_command(char *buffer)
{
  command_t subshell = checked_malloc(sizeof(struct command));
  subshell->type = SUBSHELL_COMMAND; subshell->status = -1;
  enum command_type type = scan(buffer);
  command_t command = make_command(buffer, type);
  eleminateEmptySpace();
  char c;
  if((c = get_byte(get_byte_argument)) == ')')
  {
    subshell->u.subshell_command = command;
    return subshell;
  }
  else
  {
    ungetc(c, get_byte_argument);
    command_t top = checked_malloc(sizeof(struct command)); 
    top->type = SEQUENCE_COMMAND; top->status = -1;
    top->u.command[0] = command; top->u.command[1] = NULL;
    while((c = get_byte(get_byte_argument)) != ')')
    {
      ungetc(c,get_byte_argument);
      enum command_type type = scan(buffer);

      command_t new_sequence = checked_malloc(sizeof(struct command));
      new_sequence->type = SEQUENCE_COMMAND; new_sequence->status = -1;
      new_sequence->u.command[0] = make_command(buffer, type);
      new_sequence->u.command[1] = NULL;
      command_t bottom = top;
      while(bottom->u.command[1] != NULL)
        bottom = bottom->u.command[1];
      bottom->u.command[1] = new_sequence;
    }
    command_t bottom = top;
    while(bottom->u.command[1]->u.command[1] != NULL)
      bottom = bottom->u.command[1];
    bottom->u.command[1] = bottom->u.command[1]->u.command[0];
    subshell->u.subshell_command = top;
    return subshell;
  }
}

command_t
make_command(char* buffer, enum command_type type)
{
  if(type == SIMPLE_COMMAND)
    return make_simple_command(buffer);
  else if(type == SUBSHELL_COMMAND)
  {
    command_t subshell = make_subshell_command(buffer);
    type = scan(buffer);
    if(type == SIMPLE_COMMAND)
      return subshell;
    else
      return make_complete_command(buffer, type, subshell);
  }
  else
    return make_complete_command(buffer, type, NULL);
}

command_t
make_Queuenode(char* buffer, enum command_type type)
{
 return make_command(buffer,type);
}

command_stream_t
make_command_stream (int (*get_next_byte) (void *),
		     void *get_next_byte_argument)
{
  line_count = 1;
  char buffer[1024] = "";
  get_byte = get_next_byte;
  get_byte_argument = get_next_byte_argument;
  command_stream_t stream = checked_malloc(sizeof(struct command_stream));
command_t temp_node = checked_malloc(sizeof(struct command)); 

init_queue(stream);


  if(!feof(get_byte_argument))
  {
    eleminateEmptySpace();
    if((buffer[0] = get_byte(get_byte_argument)) == EOF)
    {
      free(stream);
      free(temp_node);
      return NULL;
    }
    ungetc(buffer[0], get_byte_argument);
    buffer[0] = '\0';
    enum command_type type = scan(buffer);  

    while(1)  
    {
      temp_node = make_Queuenode(buffer, type);	
	    enqueue(stream,temp_node);
      if(type == SEQUENCE_COMMAND)
        break;
      eleminateEmptySpace();
      if((buffer[0] = get_byte(get_byte_argument)) == EOF)
      {
        return stream;
      }
      ungetc(buffer[0], get_byte_argument);
      buffer[0] = '\0';
      type = scan(buffer);
    }
  }
  return stream;
}

command_t
read_command_stream (command_stream_t s)
{
   if(!empty(s))  {
   return dequeue(s);
 }
  else
  {
    return NULL;
  }
}
