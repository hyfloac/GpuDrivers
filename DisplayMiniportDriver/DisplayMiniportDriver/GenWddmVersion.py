import subprocess

def read_version(filename):
    try:
        with open(filename, 'r') as file:
            version = int(file.read().strip())
            return version
    except FileNotFoundError:
        print("File not found.")
        return None
    except ValueError:
        print("Invalid content in file. Please make sure it contains a valid number.")
        return None

def write_version(filename, version):
    try:
        with open(filename, 'w') as file:
            file.write(str(version))
    except IOError:
        print("Error writing to file.")

def generate_wddm_version(majorFilename, minorFilename, macroFilename):
    subprocess.run(["cl", "/EP", "/P", "/I", "include", "WddmMajorVersion.h"])
    subprocess.run(["cl", "/EP", "/P", "/I", "include", "WddmMinorVersion.h"])
    majorVersion = read_version("WddmMajorVersion.i")
    minorVersion = read_version("WddmMinorVersion.i")
    print(f"Preprocessed major and minor version. {majorVersion}.{minorVersion}")
    write_version(majorFilename, majorVersion)
    write_version(minorFilename, minorVersion)
        
    try:
        with open(macroFilename, 'w') as file:
            file.write(f"#define WDDM_MAJOR_VERSION {majorVersion}\n#define WDDM_MINOR_VERSION {minorVersion}\n")
    except IOError:
        print("Error writing to file.")



# File name
major_file_name = "version/majorVersion.txt"
minor_file_name = "version/minorVersion.txt"
patch_file_name = "version/patchVersion.txt"
build_file_name = "version/buildVersion.txt"
macro_file_name = "../../UserModeDrivers/D3DUserModeDriver/WddmVersionNumbers.h"

generate_wddm_version(major_file_name, minor_file_name, macro_file_name)
