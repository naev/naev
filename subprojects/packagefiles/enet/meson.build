project('enet', 'c', version: '1.3.17')

cc = meson.get_compiler('c')
c_args = cc.get_supported_arguments(['-Wno-maybe-uninitialized', '-Wno-pedantic', '-Wno-unused-parameter'])
add_project_arguments(c_args, language: 'c')

if cc.has_function('fcntl')
   add_project_arguments('-DHAS_FCNTL=1', language: 'c')
endif
if cc.has_function('poll')
   add_project_arguments('-DHAS_POLL=1', language: 'c')
endif
if cc.has_function('getnameinfo')
   add_project_arguments('-DHAS_GETNAMEINFO=1', language: 'c')
endif
if cc.has_function('getaddrinfo')
   add_project_arguments('-DHAS_GETADDRINFO=1', language: 'c')
endif
if cc.has_function('gethostbyname_r')
   add_project_arguments('-DHAS_GETHOSTBYNAME_R=1', language: 'c')
endif
if cc.has_function('gethostbyaddr_r')
   add_project_arguments('-DHAS_GETHOSTBYADDR_R=1', language: 'c')
endif
if cc.has_function('inet_pton')
   add_project_arguments('-DHAS_INET_PTON=1', language: 'c')
endif
if cc.has_function('inet_ntop')
   add_project_arguments('-DHAS_INET_NTOP=1', language: 'c')
endif
if cc.has_member('struct msghdr', 'msg_flags',  prefix: '#include <sys/types.h>\n#include<sys/socket.h>')
   add_project_arguments('-DHAS_MSGHDR_FLAGS=1', language: 'c')
endif
if cc.sizeof('socklen_t',  prefix: '#include <sys/types.h>\n#include<sys/socket.h>') > 0
   add_project_arguments('-DHAS_SOCKLEN_T=1', language: 'c')
endif

libenet = static_library(
   'enet',
   ['callbacks.c', 'compress.c', 'host.c', 'list.c', 'packet.c', 'peer.c', 'protocol.c', 'unix.c', 'win32.c'],
   include_directories: 'include',
   dependencies:  host_machine.system() == 'windows' ? [cc.find_library('winmm'), cc.find_library('ws2_32')] : [],
 )

enet_dep = declare_dependency(link_with: libenet, include_directories: 'include', version: meson.project_version())
