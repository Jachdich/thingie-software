import sys, os
for i in sys.argv[1:]:
    names = i.split("/")
    fname = names[-1]
    libname = names[-2]
    print(os.path.join("lib_obj", libname, fname.replace(".c", ".o")), end=" ")

