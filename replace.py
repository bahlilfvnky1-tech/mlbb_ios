import glob

files = glob.glob('c:/App2/shh/mlbb_ios/ios_src/esp/*.h')
for f in files:
    with open(f, 'r') as file:
        content = file.read()
    content = content.replace('Memory::', 'InternalMemory::')
    content = content.replace('"memory.h"', '"../memory_internal.h"')
    with open(f, 'w') as file:
        file.write(content)
print("Done")
