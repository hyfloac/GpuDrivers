@echo off
conan install . -s build_type=Debug
conan install . -s build_type=Release
