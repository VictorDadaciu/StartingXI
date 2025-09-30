import os
import subprocess
from enum import Enum

COMPILER_EXE  = r"C:\VulkanSDK\1.4.321.1\Bin\glslc.exe"
GLSL_DIR      = r"C:\code\StartingXI\MysteriousGame\shaders"
SPIRV_DIR     = os.path.join(GLSL_DIR, "generated")

def to_glsl_file(filename: str) -> str:
    return os.path.join(GLSL_DIR, filename)

def to_spirv_file(filename: str) -> str:
    return os.path.join(SPIRV_DIR, filename + ".spv")

def is_accepted_file(dir: str, item: str, accepted_extensions: set[str]) -> bool:
    full_path: str = os.path.join(dir, item)
    if not os.path.isfile(full_path):
        return False
    _, extension = os.path.splitext(item)
    if extension not in accepted_extensions:
        return False
    return True

def get_files_with_extensions_from_dir(dir: str, accepted_extensions: set[str]) -> set[str]:
    return {item for item in os.listdir(dir) if is_accepted_file(dir, item, accepted_extensions)}

def clean_from_spirv_dir(to_delete: set[str]) -> None:
    print(f"Cleaning {len(to_delete)} SPIRV file(s)")
    for file in {to_spirv_file(filename) for filename in to_delete}:
        print(f"    Deleting {file}...")
        os.remove(file)

def needs_recompilation(filename: str) -> bool:
    return os.path.getmtime(to_glsl_file(filename)) > os.path.getmtime(to_spirv_file(filename))

def compile_files(to_compile: set[str]) -> int:
    errors = 0
    for filename in to_compile:
        print(f"    Compiling {filename}...", end=" ")
        ret_val = subprocess.run([COMPILER_EXE, to_glsl_file(filename), "-o", to_spirv_file(filename)], capture_output=True)
        if ret_val.returncode != 0:
            print("FAILED")
            error_text = ret_val.stderr.decode()
            error_text = error_text[error_text.find(filename) + len(filename) + 2:]
            print(error_text)
            errors += 1
        else:
            print("SUCCESS")
    return errors

def compile_new_files(to_compile: set[str]) -> int:
    print(f"Compiling {len(to_compile)} new file(s):")
    return compile_files(to_compile)

def recompile_files(to_compile: set[str]) -> int:
    print(f"Recompiling {len(to_compile)} file(s):")
    return compile_files(to_compile)

if __name__ == "__main__":
    os.makedirs(SPIRV_DIR, exist_ok=True)
    glsl_filenames  = get_files_with_extensions_from_dir(GLSL_DIR,  {".vert", ".frag"})
    spirv_filenames = {item[:-4] for item in get_files_with_extensions_from_dir(SPIRV_DIR, {".spv"})}
    to_delete = spirv_filenames - glsl_filenames
    if len(to_delete) > 0:
        clean_from_spirv_dir(to_delete)
    new_filenames = glsl_filenames - spirv_filenames
    new_errors = 0
    if len(new_filenames) > 0:
        new_errors = compile_new_files(new_filenames)
    to_recompile = set(filter(needs_recompilation, glsl_filenames - new_filenames))
    old_errors = 0
    if len(to_recompile) > 0:
        old_errors = recompile_files(to_recompile)
    all_files_num = len(new_filenames) + len(to_recompile)
    all_errors_num = new_errors + old_errors
    if all_files_num > 0 or all_errors_num > 0:
        print(f"{all_files_num - all_errors_num} file(s) compiled successfully, {all_errors_num} file(s) failed")
    else:
        print("All shader modules up-to-date")
