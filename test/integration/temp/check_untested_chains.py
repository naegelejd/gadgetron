import os
import re
import subprocess


classnames_in_untested_chains = []
with open("untested_chains.txt", "r") as f:
    for line in f:
        path = line.strip()
        chain_name = os.path.basename(path)

        if subprocess.call(["grep", "-r", chain_name, "./cases/"]) == 0:
            print(f"Found chain {chain_name} in test cases")

        path = os.path.join("..", "..", path)
        with open(path, "r") as ch:
            for line in ch:
                if "<classname>" in line:
                    classname = line.strip()
                    match = re.search(r'<classname>(.+)</classname>', line)
                    if match:
                        classname = match.group(1)
                        classnames_in_untested_chains.append(classname)

tested_classnames = []
with open("tested_gadgets.txt", "r") as f:
    for line in f:
        tested_classnames.append(line.strip())


definitely_untested_classnames = set(classnames_in_untested_chains) - set(tested_classnames)
definitely_untested_classnames = sorted(definitely_untested_classnames)

print("Writing definitely untested gadgets to definitely_untested_gadgets.txt")
with open("definitely_untested_gadgets.txt", "w") as f:
    for classname in definitely_untested_classnames:
        f.write(classname + "\n")