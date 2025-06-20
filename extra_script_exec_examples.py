import os
script_path = os.environ.get("PIO_SCRIPTS_PROJECT_PATH")

# hack for IDE
if script_path is None:
    script_path = "/Users/fmeng/Documents/wokwi/pio-extra-scripts"

sc = f"{script_path}/extra_script_exec_examples.py"
with open(sc, "r") as file:
    exec(file.read())