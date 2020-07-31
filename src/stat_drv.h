/********************************************************************/
/*                                                                  */
/*  stat_drv.h    Define functions used by os_stat macros.          */
/*  Copyright (C) 2016  Thomas Mertes                               */
/*                                                                  */
/*  This file is part of the Seed7 Runtime Library.                 */
/*                                                                  */
/*  The Seed7 Runtime Library is free software; you can             */
/*  redistribute it and/or modify it under the terms of the GNU     */
/*  Lesser General Public License as published by the Free Software */
/*  Foundation; either version 2.1 of the License, or (at your      */
/*  option) any later version.                                      */
/*                                                                  */
/*  The Seed7 Runtime Library is distributed in the hope that it    */
/*  will be useful, but WITHOUT ANY WARRANTY; without even the      */
/*  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR */
/*  PURPOSE.  See the GNU Lesser General Public License for more    */
/*  details.                                                        */
/*                                                                  */
/*  You should have received a copy of the GNU Lesser General       */
/*  Public License along with this program; if not, write to the    */
/*  Free Software Foundation, Inc., 51 Franklin Street,             */
/*  Fifth Floor, Boston, MA  02110-1301, USA.                       */
/*                                                                  */
/*  Module: Seed7 Runtime Library                                   */
/*  File: seed7/src/stat_drv.h                                      */
/*  Changes: 2016  Thomas Mertes                                    */
/*  Content: Define functions used by os_stat macros.               */
/*                                                                  */
/********************************************************************/

/* The posix macros S_ISLNK, S_ISSOCK, S_ISCHR, S_ISBLK, S_ISFIFO,  */
/* S_ISDIR, S_ISREG, S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,   */
/* S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH, S_IRWXU, S_IRWXG and S_IRWXO */
/* are used to check the 'st_mode' field of 'struct stat'. The      */
/* 'struct stat' and its variants is used by the stat() functions   */
/* (e.g. stat(), fstat(), _stat(), _stat64(), _stati64(), _wstat(), */
/* _wstat64(), _wstati64() ). The macros are defined in the case    */
/* they are missing in "sys/stat.h".                                */


#ifdef DEFINE_ST_MODE_MACROS

#undef S_ISDIR
#undef S_ISCHR
#undef S_ISBLK
#undef S_ISREG
#undef S_ISFIFO
#undef S_ISLNK
#undef S_ISSOCK

#undef S_IFDIR
#undef S_IFCHR
#undef S_IFBLK
#undef S_IFREG
#undef S_IFIFO
#undef S_IFLNK
#undef S_IFSOCK
#undef S_IFMT

#define S_IFDIR  0040000  /* Directory */
#define S_IFCHR  0020000  /* Character device */
#define S_IFBLK  0060000  /* Block device */
#define S_IFREG  0100000  /* Regular file */
#define S_IFIFO  0010000  /* FIFO */
#define S_IFLNK  0120000  /* Symbolic link */
#define S_IFSOCK 0140000  /* Socket */
#define S_IFMT   (S_IFDIR | S_IFCHR | S_IFBLK | S_IFREG | S_IFIFO | S_IFLNK | S_IFSOCK)

#undef S_IREAD
#undef S_IWRITE
#undef S_IEXEC

#undef S_IRUSR
#undef S_IWUSR
#undef S_IXUSR
#undef S_IRGRP
#undef S_IWGRP
#undef S_IXGRP
#undef S_IROTH
#undef S_IWOTH
#undef S_IXOTH

#endif


#ifndef S_ISLNK
#ifdef S_IFLNK
#define S_ISLNK(mode) (((mode) & S_IFMT) == S_IFLNK)
#else
#define S_ISLNK(mode) FALSE
#endif
#endif

#ifndef S_ISSOCK
#ifdef S_IFSOCK
#define S_ISSOCK(mode) (((mode) & S_IFMT) == S_IFSOCK)
#else
#define S_ISSOCK(mode) FALSE
#endif
#endif

#ifndef S_ISCHR
#ifdef S_IFCHR
#define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)
#else
#define S_ISCHR(mode) FALSE
#endif
#endif

#ifndef S_ISBLK
#ifdef S_IFBLK
#define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK)
#else
#define S_ISBLK(mode) FALSE
#endif
#endif

#ifndef S_ISFIFO
#ifdef S_IFIFO
#define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
#else
#define S_ISFIFO(mode) FALSE
#endif
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#endif

#ifndef S_IRUSR
#ifdef S_IREAD
#define S_IRUSR S_IREAD
#else
#define S_IRUSR 0000400
#endif
#endif

#ifndef S_IWUSR
#ifdef S_IWRITE
#define S_IWUSR S_IWRITE
#else
#define S_IWUSR 0000200
#endif
#endif

#ifndef S_IXUSR
#ifdef S_IEXEC
#define S_IXUSR S_IEXEC
#else
#define S_IXUSR 0000100
#endif
#endif

#ifndef S_IRGRP
#define S_IRGRP 0000040
#endif

#ifndef S_IWGRP
#define S_IWGRP 0000020
#endif

#ifndef S_IXGRP
#define S_IXGRP 0000010
#endif

#ifndef S_IROTH
#define S_IROTH 0000004
#endif

#ifndef S_IWOTH
#define S_IWOTH 0000002
#endif

#ifndef S_IXOTH
#define S_IXOTH 0000001
#endif

#ifndef S_IRWXU
#define S_IRWXU (S_IRUSR | S_IWUSR | S_IXUSR)
#endif

#ifndef S_IRWXG
#define S_IRWXG (S_IRGRP | S_IWGRP | S_IXGRP)
#endif

#ifndef S_IRWXO
#define S_IRWXO (S_IROTH | S_IWOTH | S_IXOTH)
#endif


#ifdef DEFINE_STRUCT_STATI64_EXT
struct stati64Ext {
    dev_t          st_dev;
    ino_t          st_ino;
    unsigned short st_mode;
    short          st_nlink;
    short          st_uid;
    short          st_gid;
    dev_t          st_rdev;
    os_off_t       st_size;
    time_t         st_atime;
    time_t         st_mtime;
    time_t         st_ctime;
  };
#endif


#ifdef DEFINE_WSTATI64_EXT
int wstati64Ext (const wchar_t *path, os_stat_struct *statBuf);
#endif
#ifdef DEFINE_FSTATI64_EXT
int fstati64Ext (int fd, os_fstat_struct *statBuf);
#endif
