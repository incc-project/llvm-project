import lit.formats

config.name = "IClangTests"
config.test_format = lit.formats.ShTest(True)
config.suffixes = ['.iclangtest']

config.substitutions.append(('%illvm-test', os.path.join(config.llvm_obj_root, 'bin', 'illvm-test')))

# config.substitutions.append((
#     "%illvm-funcv-reuse-test",
#     "rm -f %S/*.o && rm -f %S/*.log && rm -f %S/a.out && "
#     "%clang++ -c -target x86_64-pc-linux-gnu -ffunction-sections -fdata-sections -o %S/old.o %S/old.cpp && "
#     "%clang++ -c -target x86_64-pc-linux-gnu -ffunction-sections -fdata-sections -o %S/old.o %S/old.cpp && "
#     "%illvm-funcv %S/old.o %S/new.o %S/merge.o %S/funcx.txt && "
#     "%clang++ -fuse-ld=lld -o %S/a.out %S/merge.o && "
#     "%S/a.out"
# ))