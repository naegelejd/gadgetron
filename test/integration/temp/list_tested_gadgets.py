import os
import re
import glob
import subprocess

grep = subprocess.check_output(["grep", "-r", "configuration=", "./cases"])
grep = str(grep, "utf-8")
tested_chain_names = []
for line in grep.splitlines():
    cfg, xml = line.split(":configuration=")
    tested_chain_names.append(xml)


tested_chain_paths = []
for chain in tested_chain_names:
    path = subprocess.check_output(["find", "../..", "-name", chain])
    path = str(path, "utf-8").splitlines()
    assert(len(path) == 1)
    tested_chain_paths.append(path[0])

classnames = []
for path in tested_chain_paths:
    with open(path) as f:
        for line in f:
            match = re.search(r'<classname>(.+)</classname>', line)
            if match:
                classnames.append(match.group(1))

unique_chains = sorted(set(chain[6:] for chain in tested_chain_paths))
unique_classnames = sorted(set(classnames))

chains_file = "tested_chains.txt"
print(f"Writing tested chains to {chains_file}")
with open("tested_chains.txt", "w") as f:
    for chain in unique_chains:
        f.write(chain + "\n")

gadgets_file = "tested_gadgets.txt"
print(f"Writing tested Gadgets to {gadgets_file}")
with open("tested_gadgets.txt", "w") as f:
    for classname in unique_classnames:
        f.write(classname + "\n")