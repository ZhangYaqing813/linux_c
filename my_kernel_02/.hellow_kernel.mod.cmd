savedcmd_hellow_kernel.mod := printf '%s\n'   hellow_kernel.o | awk '!x[$$0]++ { print("./"$$0) }' > hellow_kernel.mod
