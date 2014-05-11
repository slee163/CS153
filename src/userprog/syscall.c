#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"

typedef int pid_t;

static void syscall_handler (struct intr_frame *);

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

pid_t
exec (const char* cmd_line)
{
}

int
wait (pid_t pid)
{
}

bool
create (const char* file, unsigned initial_size)
{
}

bool
remove (const char* file)
{
}

int
open (const char* file)
{
}

int
filesize (int fd)
{
}

int
read (int fd, void* buffer, unsigned size)
{
  int bytes_read = 0;
  if(fd ==0)
  {
  }
}

int
write (int fd, const void* buffer, unsigned size)
{
  int bytes_written = 0;
  unsigned bytes_remain = size;
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
  return bytes_written;
}

void
seek (int fd, unsigned position)
{
}

unsigned
tell (int fd)
{
}

void
close (int fd)
{
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
  uint32_t result;

  switch(*esp)
  {
    case SYS_HALT:
      halt();
      break;
    case SYS_EXIT:
      fd = *(esp+1);
      exit(fd);
      //result = fd;
      break;
    case SYS_EXEC:
      buffer = *(esp+1);
      result = exec(buffer);
      break;
    case SYS_WAIT:
      pid = *(esp+1);
      result = wait(pid);
      break;
    case SYS_CREATE:
      buffer = *(esp+1);
      size  = *(esp+2);
      result = create(buffer,size);
      break;
    case SYS_REMOVE:
      buffer = *(esp+1);
      result = remove(buffer);
      break;
    case SYS_OPEN:
      buffer = *(esp+1);
      result = open(buffer);
      break;
    case SYS_FILESIZE:
      fd = *(esp+1);
      result = filesize(fd);
      break;
    case SYS_READ:
      fd = *(esp+1);
      buffer = *(esp+2);
      size = *(esp+3);
      result = read(fd,buffer,size);
      break;
    case SYS_WRITE:
      fd = *(esp+1);
      buffer = *(esp+2);
      size = *(esp+3);
      result = write(fd,buffer,size);
      break;
    case SYS_SEEK:
      fd = *(esp+1);
      size = *(esp+2);
      seek(fd, size);
      break;
    case SYS_TELL:
      fd = *(esp+1);
      result = tell(fd);
      break;
    case SYS_CLOSE:
      fd = *(esp+1);
      close(fd);
      break;
    default:
      break;
      f->eax = result;
  }
}
