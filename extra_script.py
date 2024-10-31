Import("env")
import os

def before_build_spiffs(source, target, env):
    print("Building React App...")
    env.Execute("cd fissioninterface && npm run build")
    print("React App built!")

    print("Removing old SPIFFS image...")
    env.Execute("rm -rf data")

    print("Copying React App to SPIFFS...")
    env.Execute("cp -r fissioninterface/build data")    

env.AddPreAction("$BUILD_DIR/spiffs.bin", before_build_spiffs)