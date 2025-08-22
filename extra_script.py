import os
import shutil
import hashlib
from platformio import fs

def build_spiffs(source, target, env):
    print("Building React App for SPIFFS...")
    react_dir = "fissioninterface"
    react_build_dir = os.path.join(react_dir, "build")
    data_dir = "data"

    if os.path.exists(react_dir):
        print("Running npm run build in fissioninterface...")
        result = env.Execute("cd fissioninterface && npm run build")
        if result != 0:
            print("Error: Failed to build React app!")
            return 1
        print("React App built!")
    else:
        print(f"Error: {react_dir} directory not found!")
        return 1

    # Recreate the data directory
    if os.path.exists(data_dir):
        print("Removing old SPIFFS data directory...")
        shutil.rmtree(data_dir, ignore_errors=True)
    os.makedirs(data_dir)

    if os.path.exists(react_build_dir):
        print("Copying React App to SPIFFS data directory...")
        for root, _, files in os.walk(react_build_dir):
            for file in files:
                if file.endswith(('.map', '.LICENSE.txt', '.svg')):
                    continue  # Skip large/unnecessary files

                src = os.path.join(root, file)
                # Shorten long filenames
                ext = os.path.splitext(file)[1]
                short_name = (
                    hashlib.md5(file.encode()).hexdigest()[:8] + ext
                    if len(file) > 32 else file
                )
                dst = os.path.join(data_dir, short_name)
                shutil.copy2(src, dst)

                if file == "index.html":
                    with open(dst, 'r') as f:
                        content = f.read()
                    content = content.replace('/js/', '/').replace('/css/', '/')
                    with open(dst, 'w') as f:
                        f.write(content)
    else:
        print(f"Error: React build directory {react_build_dir} not found!")
        return 1

    print("SPIFFS data directory prepared!")
    return 0

Import("env")
env.AddPreAction("buildfs", build_spiffs)
