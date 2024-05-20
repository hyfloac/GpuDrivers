@echo off
conan install . -s build_type=Debug -o tauutils/*:shared=False
conan install . -s build_type=Release -o tauutils/*:shared=False
