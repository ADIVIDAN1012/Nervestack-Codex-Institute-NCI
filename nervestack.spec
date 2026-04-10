# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['src\\frontend\\main.py'],
    pathex=['src\\frontend'],
    binaries=[('src/runtime/main.exe', '.')],
    datas=[],
    hiddenimports=['interpreter', 'frontend', 'lexer', 'parser', 'nervestack_ast', 'symbol_table'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='NSPL',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=True,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon='assets\\nervestack_icon.ico',
)
