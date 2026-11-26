"""Package current Linux and Windows release, preserving historical downloads."""
from pathlib import Path
from runpy import run_path
run_path(str(Path(__file__).with_name("package-crossroads.py")), run_name="__main__")
