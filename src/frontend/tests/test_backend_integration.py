import os
import subprocess
from frontend import generate_ast_json

def test_backend_executes_funcall_program(tmp_path):
    src = """
spec check_greater(val) {
    check (val > 5) {
        show("val is greater than 5")
    } altern {
        show("val is not greater than 5")
    }
}

spec main() {
    firm a = 10
    funcall check_greater(a)
    funcall check_greater(3)
}

funcall main
"""
    p = tmp_path / "prog.nspl"
    p.write_text(src)
    generate_ast_json(p.read_text())
    exe = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "compiler_backend_c", "main.exe"))
    if not os.path.exists(exe):
        build = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "compiler_backend_c", "build.bat"))
        subprocess.run([build], check=True)
    ast_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "ast.json"))
    out = subprocess.run([exe, ast_path], capture_output=True, text=True, check=True)
    s = out.stdout.strip()
    assert "val is greater than 5" in s
    assert "val is not greater than 5" in s

def test_signal_listen_event(tmp_path):
    src = """
listen "ev" { show("E!") }
signal "ev"
"""
    p = tmp_path / "prog2.nspl"
    p.write_text(src)
    generate_ast_json(p.read_text())
    exe = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "compiler_backend_c", "main.exe"))
    if not os.path.exists(exe):
        build = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "compiler_backend_c", "build.bat"))
        subprocess.run([build], check=True)
    ast_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "ast.json"))
    out = subprocess.run([exe, ast_path], capture_output=True, text=True, check=True)
    s = out.stdout.strip()
    assert "E!" in s

def test_paral_hold_order(tmp_path):
    src = """
spec main() {
    paral { show("A") }
    show("X")
    hold { }
}
funcall main
"""
    p = tmp_path / "prog3.nspl"
    p.write_text(src)
    generate_ast_json(p.read_text())
    exe = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "compiler_backend_c", "main.exe"))
    if not os.path.exists(exe):
        build = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "compiler_backend_c", "build.bat"))
        subprocess.run([build], check=True)
    ast_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "ast.json"))
    out = subprocess.run([exe, ast_path], capture_output=True, text=True, check=True)
    s = out.stdout
    assert s.find("X") != -1 and s.find("A") != -1 and s.find("X") < s.find("A")

def test_attempt_trap_conclude_runtime(tmp_path):
    src = """
spec main() {
    attempt {
        trigger FailGeneral("boom")
    } trap FailGeneral peek {
        show("Caught: |peek|")
    } conclude {
        show("Done")
    }
}
funcall main
"""
    p = tmp_path / "prog4.nspl"
    p.write_text(src)
    generate_ast_json(p.read_text())
    exe = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "compiler_backend_c", "main.exe"))
    if not os.path.exists(exe):
        build = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "compiler_backend_c", "build.bat"))
        subprocess.run([build], check=True)
    ast_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "ast.json"))
    out = subprocess.run([exe, ast_path], capture_output=True, text=True, check=True)
    s = out.stdout
    assert "Caught: boom" in s and "Done" in s

def test_skip_cease_in_traverse(tmp_path):
    src = """
spec main() {
    traverse i from 1 to 4 {
        check i == 2 { skip }
        check i == 3 { cease }
        show(i)
    }
}
funcall main
"""
    p = tmp_path / "prog5.nspl"
    p.write_text(src)
    generate_ast_json(p.read_text())
    exe = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "compiler_backend_c", "main.exe"))
    if not os.path.exists(exe):
        build = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "compiler_backend_c", "build.bat"))
        subprocess.run([build], check=True)
    ast_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "ast.json"))
    out = subprocess.run([exe, ast_path], capture_output=True, text=True, check=True)
    s = out.stdout
    # traverse prints 1, skips 2, ceases on 3 (no 3 or 4)
    assert "1" in s and "2" not in s and "3" not in s and "4" not in s
