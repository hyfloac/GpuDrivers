import subprocess

def read_version(filename):
    try:
        with open(filename, 'r') as file:
            version = int(file.read())
            return version
    except FileNotFoundError:
        print("File not found.")
        return None
    except ValueError:
        print("Invalid content in file. Please make sure it contains a valid number.")
        return None

def write_version(filename, macroFilename, patchVersion, buildVersion):
    try:
        with open(filename, 'w') as file:
            file.write(str(buildVersion))
    except IOError:
        print("Error writing to file.")

    try:
        with open(macroFilename, 'w') as file:
            file.write(f"#define GS_PATCH_VERSION {patchVersion}\n#define GS_BUILD_VERSION {buildVersion}\n")
    except IOError:
        print("Error writing to file.")
        
    try:
        with open(f"../../UserModeDrivers/D3DUserModeDriver/{macroFilename}", 'w') as file:
            file.write(f"#define GS_PATCH_VERSION {patchVersion}\n#define GS_BUILD_VERSION {buildVersion}\n")
    except IOError:
        print("Error writing to file.")

def increment_build_version(patchFilename, buildFilename, macroFilename):
    patch_version = read_version(patchFilename)
    build_version = read_version(buildFilename)
    if patch_version is None:
        patch_version = 1

    if build_version is not None:
        build_version = build_version + 1
    else:
        build_version = 1

    write_version(buildFilename, macroFilename, patch_version, build_version)
    print(f"Version incremented. New version: {patch_version}.{build_version}")

# File name
major_file_name = "version/majorVersion.txt"
minor_file_name = "version/minorVersion.txt"
patch_file_name = "version/patchVersion.txt"
build_file_name = "version/buildVersion.txt"
macro_file_name = "DriverVersionNumbers.h"

# Increment version
increment_build_version(patch_file_name, build_file_name, macro_file_name)
