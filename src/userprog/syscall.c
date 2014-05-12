#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"

#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/input.h"
#include <string.h>

typedef tid_t pid_t;

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void
halt (void)
{
  shutdown_power_off();
}

void
exit (int status)
{
  char* proc_name = thread_current()->name;
  printf("%s: exit(%d)\n",proc_name, status);
  //thread_current()->status = status;
  thread_exit();
}

static void syscall_verify(int* esp)
{
  if(esp == NULL)
  {exit(-1);}
  if(is_user_vaddr(esp) == false)
  {exit(-1);}
  if(pagedir_get_page(thread_current()->pagedir, esp) == NULL)
  {exit(-1);}
}

pid_t
exec (char* cmd_line)
{
  tid_t tid = process_execute(cmd_line);
  if(tid == TID_ERROR)
  {return -1;}
  return tid;
}
int
wait (pid_t pid)
{
  int status = process_wait(pid);
  return status;
}

bool
create (const char* file, unsigned initial_size)
{
/*
  if(file == NULL || *file == '\0')
  {exit(-1);}
  if(strlen(file) <= 0 || (strlen(file) +1) > 16)
  {exit(-1);}
  bool result = filesys_create(file, initial_size);
  return result;
  */
return false;
}

bool
remove (const char* file)
{
  return filesys_remove(file);
}

int
open (const char* file)
{
  int fd = -1;

  struct file* f = filesys_open(file);
  struct thread* cur = thread_current();

  if(f == NULL)
  {return fd;}

  fd = cur->fd_count + FD_MIN;
  cur->fd_table[cur->fd_count].fd = fd;
  cur->fd_table[cur->fd_count].the_file = f;
  return fd;
}

int
filesize (int fd)
{
  struct fd_file* f = get_file_from_fd(fd);
  if(f == NULL || f->the_file)
  {return -1;}
  else
  {return file_length(f->the_file);}
}

int
read (int fd, void* buffer, unsigned size)
{
  if(size == 0)
  {return 0;}
  if(fd == 1)
  {return -1;}

  int bytes_read = 0;
  struct fd_file* f = NULL;
  
  if(fd == 0)
  {
    while(size > 0)
    {
      uint8_t inpt = input_getc();
      ((char*)buffer)[bytes_read] = inpt;
      bytes_read++;
      size--;
    }
  }
  else
  {
    f = get_file_from_fd(fd);
    if(f == NULL || f->the_file == NULL)
    {return -1;}
    bytes_read = file_read(f->the_file,buffer,size);
  }

  return bytes_read;
}

int
write (int fd, const void* buffer, unsigned size)
{
  if(size == 0)
  {return 0;}
  if(fd == 0)
  {return -1;}

  int bytes_written = 0;
  unsigned bytes_remain = size;
  struct fd_file* f = NULL;
  if(fd == 1)
  {
    while(bytes_remain >= 128)
    {
      putbuf(buffer,128);
      buffer = buffer+128;
      bytes_written += 128;
      bytes_remain -= 128;
    }
    putbuf(buffer, bytes_remain);
    bytes_written += bytes_remain;
  }
  else
  {
    f = get_file_from_fd(fd);
    if(f == NULL||f->the_file == NULL)
    {return -1;}
    bytes_written = file_write(f->the_file, buffer, size);
  }
  return bytes_written;
}

void
seek (int fd, unsigned position)
{
  if(fd == 1 || fd == 0)
  {exit(-1);}
  struct fd_file* f = get_file_from_fd(fd);
  if(f == NULL|| f->the_file == NULL)
  {exit(-1);}
  file_seek(f->the_file, position);
}

unsigned
tell (int fd)
{
  if(fd == 1 || fd == 0)
  {return -1;}
  struct fd_file* f = get_file_from_fd(fd);
  if(f == NULL|| f->the_file == NULL)
  {return -1;}
  return file_tell(f->the_file);
}

void
close (int fd)
{
   if(fd == 1 || fd == 0)
  {exit(-1);}
  struct fd_file* f = get_file_from_fd(fd);
  if(f == NULL || f->the_file == NULL)
  {exit(-1);}
  f->fd = -1;
  file_close(f->the_file);
  f->the_file = NULL;
}

static void
syscall_handler (struct intr_frame *f)// UNUSED) 
{
  //printf ("system call!\n");
  //thread_exit ();
  int *esp = f->esp;
  int fd = -1;
  pid_t pid;
  char* buffer = NULL;
  unsigned size = 0;
  uint32_t result = 0;

  syscall_verify(esp);
  switch(*esp)
  {
    case SYS_HALT:
      halt();
      break;
    case SYS_EXIT:
      syscall_verify(esp+1);

      fd = *(esp+1);
      exit(fd);
      break;
    case SYS_EXEC:
      syscall_verify(esp+1);
      syscall_verify(esp+2);

      buffer = (char*) *(esp+1);
      result = exec(buffer);
      break;
    case SYS_WAIT:
      syscall_verify(esp+1);
      
      pid = *(esp+1);
      result = wait(pid);
      break;
    case SYS_CREATE:
      syscall_verify(esp+1);
      syscall_verify(esp+2);

      buffer = (char*) *(esp+1);
      size  = *(esp+2);
      result = (uint32_t) create(buffer,size);
      break;
    case SYS_REMOVE:
      syscall_verify(esp+1);

      buffer = (char*) *(esp+1);
      result = remove(buffer);
      break;
    case SYS_OPEN:
      syscall_verify(esp+1);

      buffer = (char*) *(esp+1);
      result = open(buffer);
      break;
    case SYS_FILESIZE:
      syscall_verify(esp+1);

      fd = *(esp+1);
      result = filesize(fd);
      break;
    case SYS_READ:
      syscall_verify(esp+1);
      syscall_verify(esp+2);
      syscall_verify(esp+3);

      fd = *(esp+1);
      buffer = (char*) *(esp+2);
      size = *(esp+3);
      result = read(fd,buffer,size);
      break;
    case SYS_WRITE:
      syscall_verify(esp+1);
      syscall_verify(esp+2);
      syscall_verify(esp+3);

      fd = *(esp+1);
      buffer = (char*) *(esp+2);
      size = *(esp+3);
      result = write(fd,buffer,size);
      break;
    case SYS_SEEK:
      syscall_verify(esp+1);
      syscall_verify(esp+2);

      fd = *(esp+1);
      size = *(esp+2);
      seek(fd, size);
      break;
    case SYS_TELL:
      syscall_verify(esp+1);

      fd = *(esp+1);
      result = tell(fd);
      break;
    case SYS_CLOSE:
      syscall_verify(esp+1);

      fd = *(esp+1);
      close(fd);
      break;
    default:
      break;
  }
  f->eax = result;
}
