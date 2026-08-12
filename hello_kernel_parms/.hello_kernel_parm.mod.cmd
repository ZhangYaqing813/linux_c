savedcmd_hello_kernel_parm.mod := printf '%s\n'   hello_kernel_parm.o | awk '!x[$$0]++ { print("./"$$0) }' > hello_kernel_parm.mod
