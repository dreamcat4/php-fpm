dnl
dnl $Id$
dnl

AC_DEFUN([AC_FPM_CHECKS],
[
	AC_FPM_STDLIBS
	AC_FPM_PRCTL
	AC_FPM_CLOCK
	AC_FPM_TRACE
])

AC_DEFUN([AC_FPM_STDLIBS],
[
	AC_CHECK_FUNCS(setenv clearenv)

	AC_SEARCH_LIBS(socket, socket)
	AC_SEARCH_LIBS(inet_addr, nsl)

	AC_CHECK_HEADERS([errno.h fcntl.h stdio.h stdlib.h unistd.h sys/uio.h])
	AC_CHECK_HEADERS([sys/select.h sys/socket.h sys/time.h])
	AC_CHECK_HEADERS([arpa/inet.h netinet/in.h])
])

AC_DEFUN([AC_FPM_PRCTL],
[
	AC_MSG_CHECKING([for prctl])

	AC_TRY_COMPILE([ #include <sys/prctl.h> ], [prctl(0, 0, 0, 0, 0);], [
		AC_DEFINE([HAVE_PRCTL], 1, [do we have prctl?])
		AC_MSG_RESULT([yes])
	], [
		AC_MSG_RESULT([no])
	])
])

AC_DEFUN([AC_FPM_CLOCK],
[
	have_clock_gettime=no

	AC_MSG_CHECKING([for clock_gettime])

	AC_TRY_LINK([ #include <time.h> ], [struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);], [
		have_clock_gettime=yes
		AC_MSG_RESULT([yes])
	], [
		AC_MSG_RESULT([no])
	])

	if test "$have_clock_gettime" = "no"; then
		AC_MSG_CHECKING([for clock_gettime in -lrt])

		SAVED_LIBS="$LIBS"
		LIBS="$LIBS -lrt"

		AC_TRY_LINK([ #include <time.h> ], [struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);], [
			have_clock_gettime=yes
			AC_MSG_RESULT([yes])
		], [
			LIBS="$SAVED_LIBS"
			AC_MSG_RESULT([no])
		])
	fi

	if test "$have_clock_gettime" = "yes"; then
		AC_DEFINE([HAVE_CLOCK_GETTIME], 1, [do we have clock_gettime?])
	fi

	have_clock_get_time=no

	if test "$have_clock_gettime" = "no"; then
		AC_MSG_CHECKING([for clock_get_time])

		AC_TRY_RUN([ #include <mach/mach.h>
			#include <mach/clock.h>
			#include <mach/mach_error.h>

			int main()
			{
				kern_return_t ret; clock_serv_t aClock; mach_timespec_t aTime;
				ret = host_get_clock_service(mach_host_self(), REALTIME_CLOCK, &aClock);

				if (ret != KERN_SUCCESS) {
					return 1;
				}

				ret = clock_get_time(aClock, &aTime);
				if (ret != KERN_SUCCESS) {
					return 2;
				}

				return 0;
			}
		], [
			have_clock_get_time=yes
			AC_MSG_RESULT([yes])
		], [
			AC_MSG_RESULT([no])
		])
	fi

	if test "$have_clock_get_time" = "yes"; then
		AC_DEFINE([HAVE_CLOCK_GET_TIME], 1, [do we have clock_get_time?])
	fi
])

AC_DEFUN([AC_FPM_TRACE],
[
	have_ptrace=no
	have_broken_ptrace=no

	AC_MSG_CHECKING([for ptrace])

	AC_TRY_COMPILE([
		#include <sys/types.h>
		#include <sys/ptrace.h> ], [ptrace(0, 0, (void *) 0, 0);], [
		have_ptrace=yes
		AC_MSG_RESULT([yes])
	], [
		AC_MSG_RESULT([no])
	])

	if test "$have_ptrace" = "yes"; then
		AC_MSG_CHECKING([whether ptrace works])

		AC_TRY_RUN([
			#include <unistd.h>
			#include <signal.h>
			#include <sys/wait.h>
			#include <sys/types.h>
			#include <sys/ptrace.h>
			#include <errno.h>

			#if !defined(PTRACE_ATTACH) && defined(PT_ATTACH)
			#define PTRACE_ATTACH PT_ATTACH
			#endif

			#if !defined(PTRACE_DETACH) && defined(PT_DETACH)
			#define PTRACE_DETACH PT_DETACH
			#endif

			#if !defined(PTRACE_PEEKDATA) && defined(PT_READ_D)
			#define PTRACE_PEEKDATA PT_READ_D
			#endif

			int main()
			{
				long v1 = (unsigned int) -1; /* copy will fail if sizeof(long) == 8 and we've got "int ptrace()" */
				long v2;
				pid_t child;
				int status;

				if ( (child = fork()) ) { /* parent */
					int ret = 0;

					if (0 > ptrace(PTRACE_ATTACH, child, 0, 0)) {
						return 1;
					}

					waitpid(child, &status, 0);

			#ifdef PT_IO
					struct ptrace_io_desc ptio = {
						.piod_op = PIOD_READ_D,
						.piod_offs = &v1,
						.piod_addr = &v2,
						.piod_len = sizeof(v1)
					};

					if (0 > ptrace(PT_IO, child, (void *) &ptio, 0)) {
						ret = 1;
					}
			#else
					errno = 0;

					v2 = ptrace(PTRACE_PEEKDATA, child, (void *) &v1, 0);

					if (errno) {
						ret = 1;
					}
			#endif
					ptrace(PTRACE_DETACH, child, (void *) 1, 0);

					kill(child, SIGKILL);

					return ret ? ret : (v1 != v2);
				}
				else { /* child */
					sleep(10);
					return 0;
				}
			}
		], [
			AC_MSG_RESULT([yes])
		], [
			have_ptrace=no
			have_broken_ptrace=yes
			AC_MSG_RESULT([no])
		])
	fi

	if test "$have_ptrace" = "yes"; then
		AC_DEFINE([HAVE_PTRACE], 1, [do we have ptrace?])
	fi

	have_mach_vm_read=no

	if test "$have_broken_ptrace" = "yes"; then
		AC_MSG_CHECKING([for mach_vm_read])

		AC_TRY_COMPILE([ #include <mach/mach.h>
			#include <mach/mach_vm.h>
		], [
			mach_vm_read((vm_map_t)0, (mach_vm_address_t)0, (mach_vm_size_t)0, (vm_offset_t *)0, (mach_msg_type_number_t*)0);
		], [
			have_mach_vm_read=yes
			AC_MSG_RESULT([yes])
		], [
			AC_MSG_RESULT([no])
		])
	fi

	if test "$have_mach_vm_read" = "yes"; then
		AC_DEFINE([HAVE_MACH_VM_READ], 1, [do we have mach_vm_read?])
	fi

	proc_mem_file=""

	if test -r /proc/$$/mem ; then
		proc_mem_file="mem"
	else
		if test -r /proc/$$/as ; then
			proc_mem_file="as"
		fi
	fi

	if test -n "$proc_mem_file" ; then
		AC_MSG_CHECKING([for proc mem file])
	
		AC_TRY_RUN([
			#define _GNU_SOURCE
			#define _FILE_OFFSET_BITS 64
			#if HAVE_INTTYPES_H
			#include <inttypes.h>
			#else
			#include <stdint.h>
			#endif

			#include <unistd.h>
			#include <sys/types.h>
			#include <sys/stat.h>
			#include <fcntl.h>
			#include <stdio.h>
			int main()
			{
				long v1 = (unsigned int) -1, v2 = 0;
				char buf[128];
				int fd;
				sprintf(buf, "/proc/%d/$proc_mem_file", getpid());
				fd = open(buf, O_RDONLY);
				if (0 > fd) {
					return 1;
				}
				if (sizeof(long) != pread(fd, &v2, sizeof(long), (uintptr_t) &v1)) {
					close(fd);
					return 1;
				}
				close(fd);
				return v1 != v2;
			}
		], [
			AC_MSG_RESULT([$proc_mem_file])
		], [
			proc_mem_file=""
			AC_MSG_RESULT([no])
		])
	fi
	
	if test -n "$proc_mem_file"; then
		AC_DEFINE_UNQUOTED([PROC_MEM_FILE], "$proc_mem_file", [/proc/pid/mem interface])
	fi
	
	fpm_trace_type=""

	if test "$have_ptrace" = "yes"; then
		fpm_trace_type=ptrace
		
	elif test -n "$proc_mem_file"; then
		fpm_trace_type=pread
		
	elif test "$have_mach_vm_read" = "yes" ; then
		fpm_trace_type=mach
		
	else
		AC_MSG_ERROR([FPM Trace - ptrace, pread, or mach: could not be found])		
	fi
	
])

