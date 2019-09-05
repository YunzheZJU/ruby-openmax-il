require 'mkmf'

# have_func returns false if a C function cannot be found.  Sometimes this
# will be OK (the function changed names between versions) and sometimes it is
# not so you need to exit without creating the Makefile.

# abort 'missing malloc()' unless have_func 'malloc'
# abort 'missing free()'   unless have_func 'free'

HEADER_DIRS = ['/opt/vc/include/IL', '/opt/vc/include', '/opt/vc/src/hello_pi/libs/ilclient']

LIB_DIRS = ['/opt/vc/lib', '/opt/vc/src/hello_pi/libs/ilclient']

libs = ['-lopenmaxil', '-lbcm_host', '-lilclient']

CONFIG['warnflags'].gsub!('-Wpointer-arith', '')
CONFIG['warnflags'].gsub!('-Wno-self-assign', '')
CONFIG['warnflags'].gsub!('-Wno-parentheses-equality', '')
CONFIG['warnflags'].gsub!('-Wno-constant-logical-operand', '')

dir_config 'ruby_openmax_il', HEADER_DIRS, LIB_DIRS

libs.each do |lib|
  $LOCAL_LIBS << " #{lib}"
end

# $CFLAGS << " -g -DRASPBERRY_PI  -DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -g -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM"
$CFLAGS << " -DOMX_SKIP64BIT"

# Now we create the Makefile that will install the extension as
# lib/my_malloc/my_malloc.so.

create_header

create_makefile 'ruby_openmax_il/ruby_openmax_il'
