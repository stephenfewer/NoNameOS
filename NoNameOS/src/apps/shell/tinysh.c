/*
 * tinysh.c
 *
 * Minimal portable shell
 *
 * Copyright (C) 2001 Michel Gutierrez <mig@nerim.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <apps/shell/tinysh.h>
#include <lib/libc/stdio.h>
#include <lib/libc/string.h>
#include <lib/libc/stdlib.h>

static void help_fnt( int argc, char **argv );

static tinysh_cmd_t help_cmd={ 0, "help", "display help", "<cr>", help_fnt, 0, 0, 0 };

static char input_buffers[HISTORY_DEPTH][BUFFER_SIZE+1]={0};
static char trash_buffer[BUFFER_SIZE+1]={0};
static int cur_buf_index=0;
static char context_buffer[BUFFER_SIZE+1]={0};
static int cur_context=0;
static int cur_index=0;
static int echo=1;
static char prompt[PROMPT_SIZE+1]="> ";
static tinysh_cmd_t * root_cmd=&help_cmd;
static tinysh_cmd_t * cur_cmd_ctx=0;
static void * tinysh_arg=0;

// callback for help function
static void help_fnt( int argc, char **argv )
{
  puts("?            display help on given or available commands\n");
  puts("<TAB>        auto-completion\n");
  puts("<RETURN>     execute command line\n");
  puts("<UP>         recall previous input line\n");
  puts("<DOWN>       recall next input line\n");
  puts("<any>        treat as input character\n");
}

enum { NULLMATCH,FULLMATCH,PARTMATCH,UNMATCH,MATCH,AMBIG };

/* verify if the non-spaced part of s2 is included at the begining
 * of s1.
 * return FULLMATCH if s2 equal to s1, PARTMATCH if s1 starts with s2
 * but there are remaining chars in s1, UNMATCH if s1 does not start with
 * s2
 */
int strstart(char *s1, char *s2)
{
  while(*s1 && *s1==*s2) { s1++; s2++; }

  if(*s2==' ' || *s2==0)
    {
      if(*s1==0)
        return FULLMATCH; /* full match */
      else
        return PARTMATCH; /* partial match */ 
    }
  else
    return UNMATCH;     /* no match */
}

/*
 * check commands at given level with input string.
 * _cmd: point to first command at this level, return matched cmd
 * _str: point to current unprocessed input, return next unprocessed
 */
static int parse_command(tinysh_cmd_t **_cmd, char **_str)
{
  char *str=*_str;
  tinysh_cmd_t *cmd;
  //int matched_len=0;
  tinysh_cmd_t *matched_cmd=0;

  /* first eliminate first blanks */
  while(*str==' ') str++;
  if(!*str)
    {
      *_str=str;
      return NULLMATCH; /* end of input */
    }
  
  /* first pass: count matches */
  for(cmd=*_cmd;cmd;cmd=cmd->next)
    {
      int ret=strstart((char *)cmd->name,str);

      if(ret==FULLMATCH)
        {
          /* found full match */
          while(*str && *str!=' ') str++; 
          while(*str==' ') str++;
          *_str=str;
          *_cmd=cmd;
          return MATCH;
        }
      else if (ret==PARTMATCH)
        {
          if(matched_cmd)
            {
              *_cmd=matched_cmd;
              return AMBIG;
            }
          else
            {
              matched_cmd=cmd;
            }
        }
      else /* UNMATCH */
        {
        }
    }
  if(matched_cmd)
    {
      while(*str && *str!=' ') str++; 
      while(*str==' ') str++;
      *_cmd=matched_cmd;
      *_str=str;
      return MATCH;
    }
  else
    return UNMATCH;
}

// create a context from current input line
static void do_context(tinysh_cmd_t *cmd, char *str)
{
  while(*str) 
    context_buffer[cur_context++]=*str++;
  context_buffer[cur_context]=0;
  cur_cmd_ctx=cmd;
}

// execute the given command by calling callback with appropriate arguments
static void exec_command(tinysh_cmd_t *cmd, char *str)
{
  char *argv[MAX_ARGS];
  int argc=0;
  int i;

/* copy command line to preserve it for history */
  for(i=0;i<BUFFER_SIZE;i++)
    trash_buffer[i]=str[i];
  str=trash_buffer;

/* cut into arguments */
  argv[argc++]=cmd->name;
  while(*str && argc<MAX_ARGS)
    {
      while(*str==' ') str++;
      if(*str==0)
        break;
      argv[argc++]=str;
      while(*str!=' ' && *str) str++;
      if(!*str) break;
      *str++=0;
    }
/* call command function if present */
  if(cmd->function)
    {
      tinysh_arg=cmd->arg;
      cmd->function(argc,&argv[0]);
    }
}

/* try to execute the current command line
 */
static int exec_command_line(tinysh_cmd_t *cmd, char *_str)
{
  char *str=_str;

  while(TRUE)
    {
      int ret;
      ret=parse_command(&cmd,&str);
      if(ret==MATCH) /* found unique match */
        {
          if(cmd)
            {
              if(!cmd->child) /* no sub-command, execute */
                {
                  exec_command(cmd,str);
                  return 0;
                }
              else
                {
                  if(*str==0) /* no more input, this is a context */
                    {
                      do_context(cmd,_str);
                      return 0;
                    }
                  else /* process next command word */
                    {
                      cmd=cmd->child;
                    }
                }
            } 
          else /* cmd == 0 */
            {
              return 0;
            }
        }
      else if(ret==AMBIG)
        {
          puts("ambiguity: ");
          puts(str);
          putchar('\n');
          return 0;
        }
      else if(ret==UNMATCH) /* UNMATCH */
        {
          puts("no match: ");
          puts(str);
          putchar('\n');
          return 0;
        }
      else /* NULLMATCH */
        return 0;
    }
}

/* display help for list of commands 
*/
static void display_child_help(tinysh_cmd_t *cmd)
{
  tinysh_cmd_t *cm;
  int len=0;

  putchar('\n');
  for(cm=cmd;cm;cm=cm->next)
    if(len<strlen(cm->name))
      len=strlen(cm->name);
  for(cm=cmd;cm;cm=cm->next)
    if(cm->help)
      {
        int i;
        puts(cm->name);
        for(i=strlen(cm->name);i<len+2;i++)
          putchar(' ');
        puts(cm->help);
        putchar('\n');
      }
}

/* try to display help for current comand line
 */
static int help_command_line(tinysh_cmd_t *cmd, char *_str)
{
  char *str=_str;

  while(TRUE)
    {
      int ret;
      ret=parse_command(&cmd,&str);
      if(ret==MATCH && *str==0) /* found unique match or empty line */
        {
          //tinysh_cmd_t *cm;
         // int len=0;
              
          if(cmd->child) /* display sub-commands help */
            {
              display_child_help(cmd->child);
              return 0;
            }
          else  /* no sub-command, show single help */
            {
              if(*(str-1)!=' ')
                putchar(' ');
              if(cmd->usage)
                puts(cmd->usage);
              puts(": ");
              if(cmd->help)
                puts(cmd->help);
              else
                puts("no help available");
              putchar('\n');
            }
          return 0;
        }
      else if(ret==MATCH && *str)
        { /* continue processing the line */
          cmd=cmd->child;
        }
      else if(ret==AMBIG)
        {
          puts("\nambiguity: ");
          puts(str);
          putchar('\n');
          return 0;
        }
      else if(ret==UNMATCH)
        {
          puts("\nno match: ");
          puts(str);
          putchar('\n');
          return 0;
        }
      else /* NULLMATCH */
        {
          if(cur_cmd_ctx)
            display_child_help(cur_cmd_ctx->child);
          else
            display_child_help(root_cmd);
          return 0;
        }
    }
}

// try to complete current command line
static int complete_command_line(tinysh_cmd_t *cmd, char *_str)
{
  char *str=_str;

  while( TRUE )
    {
      int ret;
      int common_len=BUFFER_SIZE;
      int _str_len;
      int i;
      char *__str=str;

     // tinysh_cmd_t *_cmd=cmd;
      ret=parse_command(&cmd,&str);
      for(_str_len=0;__str[_str_len]&&__str[_str_len]!=' ';_str_len++);
      if(ret==MATCH && *str)
        {
          cmd=cmd->child;
        }
      else if(ret==AMBIG || ret==MATCH || ret==NULLMATCH)
        {
          tinysh_cmd_t *cm;
          tinysh_cmd_t *matched_cmd=0;
          int nb_match=0;
              
          for(cm=cmd;cm;cm=cm->next)
            {
              int r=strstart(cm->name,__str);
              if(r==FULLMATCH)
                {
                  for(i=_str_len;cmd->name[i];i++)
                    tinysh_char_in(cmd->name[i]);
                  if(*(str-1)!=' ')
                    tinysh_char_in(' ');
                  if(!cmd->child)
                    {
                      if(cmd->usage)
                        {
                          puts(cmd->usage);
                          putchar('\n');
                          return 1;
                        }
                      else
                        return 0;
                    }
                  else
                    {
                      cmd=cmd->child;
                      break;
                    }
                }
              else if(r==PARTMATCH)
                {
                  nb_match++;
                  if(!matched_cmd)
                    {
                      matched_cmd=cm;
                      common_len=strlen(cm->name);
                    }
                  else
                    {
                      for(i=_str_len;cm->name[i] && i<common_len &&
                            cm->name[i]==matched_cmd->name[i];i++);
                      if(i<common_len)
                        common_len=i;
                    }
                }
            }
          if(cm)
            continue;
          if(matched_cmd)
            {
              if(_str_len==common_len)
                {
                  putchar('\n');
                  for(cm=cmd;cm;cm=cm->next)
                    {
                      int r=strstart(cm->name,__str);
                      if(r==FULLMATCH || r==PARTMATCH)
                        {
                          puts(cm->name);
                          putchar('\n');
                        }
                    }
                  return 1;
                }
              else
                {
                  for(i=_str_len;i<common_len;i++)
                    tinysh_char_in(matched_cmd->name[i]);
                  if(nb_match==1)
                    tinysh_char_in(' ');
                }
            }
          return 0;
        }
      else /* UNMATCH */
        { 
          return 0;
        }
    }
}

// start a new line 
static void start_of_line()
{
	// display start of new line
	puts( prompt );
	if( cur_context )
    {
    	puts( context_buffer );
    	puts( "> " );
    }
    cur_index = 0;
}

// character input 
void tinysh_char_in( char c )
{
  char * line=input_buffers[cur_buf_index];

  if(c=='\n' || c=='\r') // validate command
    {
      tinysh_cmd_t *cmd;
      
// first, echo the newline
      if(echo)
        putchar(c);

      while(*line && *line==' ') line++;
      if(*line) // not empty line
        {
          cmd=cur_cmd_ctx?cur_cmd_ctx->child:root_cmd;
          exec_command_line(cmd,line);
          cur_buf_index=(cur_buf_index+1)%HISTORY_DEPTH;
          cur_index=0;
          input_buffers[cur_buf_index][0]=0;
        }
      start_of_line();
    }
  else if(c==TOPCHAR) // return to top level
    {
      if(echo)
        putchar(c);

      cur_context=0;
      cur_cmd_ctx=0;
    }
  else if(c==8 || c==127) // backspace
    {
      if(cur_index>0)
        {
          puts("\b \b");
          cur_index--;
          line[cur_index]=0;
        }
    }
  else if(c==16) // CTRL-P: back in history
    {
      int prevline=(cur_buf_index+HISTORY_DEPTH-1)%HISTORY_DEPTH;

      if(input_buffers[prevline][0])
        {
          line=input_buffers[prevline];
          // fill the rest of the line with spaces
          while(cur_index-->strlen(line))
            puts("\b \b");
          putchar('\r');
          start_of_line();
          puts(line);
          cur_index=strlen(line);
          cur_buf_index=prevline;
        }
    }
  else if(c==14) // CTRL-N: next in history
    {
      int nextline=(cur_buf_index+1)%HISTORY_DEPTH;

      if(input_buffers[nextline][0])
        {
          line=input_buffers[nextline];
          // fill the rest of the line with spaces
          while(cur_index-->strlen(line))
            puts("\b \b");
          putchar('\r');
          start_of_line();
          puts(line);
          cur_index=strlen(line);
          cur_buf_index=nextline;
        }
    }
  else if(c=='?') // display help
    {
      tinysh_cmd_t *cmd;
      cmd=cur_cmd_ctx?cur_cmd_ctx->child:root_cmd;
      help_command_line(cmd,line);
      start_of_line();
      puts(line);
      cur_index=strlen(line);
    }
  else if(c==9 || c=='!') // TAB: autocompletion
    {
      tinysh_cmd_t *cmd;
      cmd=cur_cmd_ctx?cur_cmd_ctx->child:root_cmd;
      if(complete_command_line(cmd,line))
        {
          start_of_line();
          puts(line);
        }
      cur_index=strlen(line);
    }      
  else // any input character
    {
      if(cur_index<BUFFER_SIZE)
        {
          if(echo)
            putchar(c);
          line[cur_index++]=c;
          line[cur_index]=0;
        }
    }
}

// add a new command
void tinysh_add_command(tinysh_cmd_t *cmd)
{
  tinysh_cmd_t *cm;

  if(cmd->parent)
    {
      cm=cmd->parent->child;
      if(!cm)
        {
          cmd->parent->child=cmd;
        }
      else
        {
          while(cm->next)
          	cm=cm->next;
          cm->next=cmd;
        }
    }
  else if(!root_cmd)
    {
      root_cmd=cmd;
    }
  else
    {
      cm=root_cmd;
      while(cm->next) cm=cm->next;
      cm->next=cmd;      
    }
}

// modify shell prompt
void tinysh_set_prompt( char * str )
{
  int i;
  for(i=0;str[i] && i<PROMPT_SIZE;i++)
    prompt[i]=str[i];
  prompt[i]=0;
  // force prompt display by generating empty command
  tinysh_char_in( '\r' );
}

// return current command argument
void * tinysh_get_arg()
{
  return tinysh_arg;
}

// string to decimal/hexadecimal conversion

unsigned long tinysh_atoxi( char * s )
{
  int ishex=0;
  unsigned long res=0;

  if(*s==0) return 0;

  if(*s=='0' && *(s+1)=='x')
    {
      ishex=1;
      s+=2;
    }

  while(*s)
    {
      if(ishex)
	res*=16;
      else
	res*=10;

      if(*s>='0' && *s<='9')
	res+=*s-'0';
      else if(ishex && *s>='a' && *s<='f')
	res+=*s+10-'a';
      else if(ishex && *s>='A' && *s<='F')
	res+=*s+10-'A';
      else
	break;
      
      s++;
    }

  return res;
}
