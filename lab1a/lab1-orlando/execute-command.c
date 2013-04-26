// UCLA CS 111 Lab 1 command execution

#include "command.h"
#include "command-internals.h"

#include <error.h>
#include <sys/types.h>//pid_t
#include <unistd.h>//fork,_exit
#include <sys/wait.h>//waitpid
#include <stdio.h>//printf
#include <sys/stat.h>
#include <fcntl.h>//O_RDONLY
#include <stdlib.h>//exit()
/* FIXME: You may need to add #include directives, macro definitions,
   static function definitions, etc.  */

void execute_generic(command_t c);

int
command_status (command_t c)
{
  return c->status;
}

void execute_simple(command_t c)
{
  pid_t pid = fork();

  if(pid > 0)
  {
    int exitStatus;
    if( waitpid(pid,&exitStatus,0)==-1)
     error(1,0,"Child process exit improperly");
    c->status = WEXITSTATUS(exitStatus);
  }
  else if(pid == 0)
  {
    //in child process
   if(c->input != NULL)
   {
     int input_fd = open(c->input,O_RDONLY);
     if(input_fd == -1)
       error(1,0,"Can not open file");
     dup2(input_fd,0);//input redirect input_fd as stdin
    if( close(input_fd)<0)
      error(1,0,"Can not close input file");
   }
   if(c->output != NULL)
   {
     int output_fd = open(c->output,O_WRONLY | O_CREAT,0600);
     if(output_fd == -1)
     error(1,0,"open output file error");
     dup2(output_fd,1);// redirect output_fd as output
    if( close(output_fd)<0)
      error(1,0,"can not close outpout file");
   }
  
    //execute the simple command
    execvp(c->u.word[0],c->u.word);
  //  error(1,0,"Invalid simple command");
  }
  else
    error(1,0,"Could not fork");
}

void
execute_and(command_t c)
{
  //try simple command then change to generic command
  execute_generic(c->u.command[0]);
  c->status = c->u.command[0]->status;

  if(c->u.command[0]->status == 0)
  {
    //if the first command succeed execute the second command
    execute_generic(c->u.command[1]);
    c->status = c->u.command[1]->status;
  }
}

void execute_or(command_t c)
{
  execute_generic(c->u.command[0]);
  c->status = c->u.command[0]->status;
  if(c->u.command[0]->status!=0)
  {
    execute_generic(c->u.command[1]);
    c->status = c->u.command[1]->status;
  }
}
void execute_sequence(command_t c)
{
  execute_generic(c->u.command[0]);
  execute_generic(c->u.command[1]);
  c->status = c->u.command[1]->status;
}
void execute_subshell(command_t c)
{
  execute_generic(c->u.subshell_command);
  c->status = c->u.subshell_command->status;
}

/*
 *fd[1]1 stdout[                           ]fd[0] 0 stdinput
 *
 *
 */
void execute_pipe(command_t c)
{
int fd[2];
if(pipe(fd)== -1 )
  error(1,0,"pipe fail");

pid_t bp = fork();
if(bp>0)
{//in sh
pid_t ap=fork();
if(ap >0 )
  {
    close(fd[0]);
    close(fd[1]);
    int status;
    pid_t wait_pid = waitpid(-1,&status,0);
    if(wait_pid == bp)
    {
      c->status = WEXITSTATUS(status);
      waitpid(ap,&status,0);
      return;
    }
    else if(wait_pid == ap)
    {
      waitpid(bp,&status,0);
      c->status = WEXITSTATUS(status);
      return;
    }
  }
  else if(ap == 0)
  {
    dup2(fd[1],1);
    close(fd[0]);
    execute_generic(c->u.command[0]);
    exit(c->u.command[0]->status);
  }
  close(fd[0]);
  close(fd[1]);

  int status;
  waitpid(bp,&status,0);
  c->status = WEXITSTATUS(status);
}
else if(bp == 0)
{
  dup2(fd[0],0);//the output of pipe as the std input
  close(fd[1]);
  execute_generic(c->u.command[1]);
  exit(c->u.command[1]->status);

}
}
void execute_generic(command_t c)
{
  switch(c->type)
  {
    case AND_COMMAND:
      execute_and(c);
      break;
    case OR_COMMAND:
      execute_or(c);
      break;
    case SIMPLE_COMMAND:
      execute_simple(c);
      break;
    case SEQUENCE_COMMAND:
      execute_sequence(c);
      break;
    case SUBSHELL_COMMAND:
      execute_subshell(c);
        break;
    case PIPE_COMMAND:
        execute_pipe(c);
        break;
    default:
      error(1,0,"Invalid Command Type");
  }

}
void
execute_command (command_t c, bool time_travel)
{
  /* FIXME: Replace this with your implementation.  You may need to
     add auxiliary functions and otherwise modify the source code.
     You can also use external functions defined in the GNU C Library.  */
  execute_generic(c);
  //error (1, 0, "command execution not yet implemented");
}
